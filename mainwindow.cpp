#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "config.h"
#include "utils.h"

#include <VLCQtCore/Common.h>
#include <VLCQtCore/Media.h>
#include <VLCQtCore/Video.h>
#include <VLCQtCore/Audio.h>

#include <QMessageBox>
#include <QDropEvent>
#include <QMimeData>
#include <QDir>
#include <QScrollBar>
#include <QDebug>
#include <functional>

namespace pork {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_settings("PitM", "Pork")
    , m_vlcInstance(VlcCommon::args(), this)
    , m_videoPlayer(&m_vlcInstance)
{
    ui->setupUi(this);
    ui->fileNameLabel->setContentsMargins(tune::info::fileName::pad, tune::info::fileName::pad, 0, 0);

    setupVideoPlayer();

    block(ui->scrollArea);
    block(ui->videoPane);

    m_fileNameTimer.setSingleShot(true);
    connect(&m_fileNameTimer, &QTimer::timeout, ui->fileNameLabel, &QLabel::hide);

    setMediaMode(MediaMode::Image);
    setAppMode(AppMode::DragDialog);

    restoreGeometry(m_settings.value(tune::reg::dragWindowGeometry).toByteArray());
    connect(qApp, &QApplication::aboutToQuit, [this]() {
        if(m_appMode == AppMode::DragDialog) {
            m_settings.setValue(tune::reg::dragWindowGeometry, saveGeometry());
        }
    });
}

void MainWindow::setupVideoPlayer()
{
    // basic setup
    m_videoPlayer.setVideoWidget(ui->videoView);

    // make sliders well-responsible
    ui->volumeSlider->setStyle(new QSliderStyle(ui->volumeSlider->style()));
    ui->progressSlider->setStyle(new QSliderStyle(ui->progressSlider->style()));


    // video volume change
    ui->volumeSlider->setRange(tune::volume::min, tune::volume::max);
    connect(ui->volumeSlider, &QSlider::valueChanged, [this](int volume) {
        if(m_vlcAudio) {
            m_vlcAudio->setVolume(volume);
        }
        showSliders();
    });

    // video duration change
    ui->volumeSlider->setRange(0, 100);

    // video position change
    connect(&m_videoPlayer, &VlcMediaPlayer::positionChanged, [this](float position) {
        if(m_userChangedVideoPos) {
            showSliders();
            m_userChangedVideoPos = false;
        } else {
            ui->progressSlider->setValue(position*100);
        }
    });
    connect(ui->progressSlider, &QSlider::sliderPressed, [this]() {
        m_userChangedVideoPos = true;
        m_videoPlayer.pause();
    });
    connect(ui->progressSlider, &QSlider::sliderReleased, [this]() {
        m_userChangedVideoPos = true;
        m_videoPlayer.setPosition(ui->progressSlider->value()/100.);
        resumeVideo();
    });

    connect(&m_videoPlayer, &VlcMediaPlayer::stateChanged, [this]() {
        auto state = m_videoPlayer.state();
        // unknown codec case
        if(state == Vlc::Error) {
            ui->codecErrorLabel->show();
            ui->volumeSlider->hide();
            ui->progressSlider->hide();
        }
    });

    // sliders auto-hide
    m_slidersTimer.setSingleShot(true);
    connect(&m_slidersTimer, &QTimer::timeout, ui->progressSlider, &QSlider::hide);
    connect(&m_slidersTimer, &QTimer::timeout, ui->volumeSlider, &QSlider::hide);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_vlcMedia;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *data {event->mimeData()};

    if(!data->hasUrls()) {
        event->ignore();
        return;
    }

    QString url { data->urls().first().toString() };

    bool pic { fileBelongsTo(url, cap::supportedImages) };
    bool gif { fileBelongsTo(url, cap::supportedGif) };
    bool video { fileBelongsTo(url, cap::supportedVideo) };

    if(!pic && !gif && !video) {
        event->ignore();
        return;
    }

    event->accept();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *data { event->mimeData() };
    if(data->hasUrls()) {
        QUrl url {data->urls().first()};
        openFile(url.toLocalFile());
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QRect window { QPoint{}, event->size()};
    QRect label { ui->codecErrorLabel->rect() };
    QRect volume { ui->volumeSlider->rect() };
    QRect progress { ui->progressSlider->rect() };

    constexpr int pad { tune::slider::pad };

    label.moveCenter(window.center());
    volume.moveRight(window.right()-pad/2);
    volume.moveTop(pad);
    progress.moveLeft(pad);
    progress.moveBottom(window.bottom()-pad/2);
    progress.setWidth(window.width()-2*pad);

    ui->codecErrorLabel->setGeometry(label);
    ui->volumeSlider->setGeometry(volume);
    ui->progressSlider->setGeometry(progress);

    ui->fileNameLabel->resize(window.width(), tune::info::fileName::fontSize*2 + tune::info::fileName::pad);
}

void MainWindow::setAppMode(AppMode type)
{
    m_appMode = type;
    if(type == AppMode::Fullscreen) {
        showFullScreen();
        ui->fileNameLabel->show();
    } else {
        setMediaMode(MediaMode::Image);
        ui->label->clear();
        showNormal();
        setLabelText(ui->label, tr("Drag image/video here..."), tune::info::dragLabelColor);
        ui->fileNameLabel->hide();
    }
}

bool MainWindow::openFile(const QString &filename)
{
    m_currentFile = QFileInfo {filename};
    bool ok { loadFile() };
    if(ok) {
        setAppMode(AppMode::Fullscreen);
    }

    return ok;
}

bool MainWindow::loadFile()
{
    QString filePath { m_currentFile.absoluteFilePath() };

    if(fileBelongsTo(filePath, cap::supportedImages)) {
        return loadImage();
    }

    if(fileBelongsTo(filePath, cap::supportedGif)) {
        return loadGif();
    }

    if(fileBelongsTo(filePath, cap::supportedVideo)) {
        return loadVideo();
    }

    return false;
}

bool MainWindow::loadImage()
{
    QString filePath { m_currentFile.absoluteFilePath() };

    QImageReader reader(filePath);
    reader.setAutoTransform(true);
    m_image = reader.read();
    if (m_image.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                 .arg(QDir::toNativeSeparators(filePath), reader.errorString()));
        return false;
    }

    setMediaMode(MediaMode::Image);

    calcImageFactor();
    applyImage();

    setLabelText(ui->fileNameLabel, m_currentFile.fileName(), tune::info::fileName::darkColor, tune::info::fileName::fontSize);
    ui->fileNameLabel->show();
    m_fileNameTimer.start(tune::info::fileName::showTime);

    return true;
}

bool MainWindow::loadGif()
{
    QString filePath { m_currentFile.absoluteFilePath() };
    setLabelText(ui->fileNameLabel, m_currentFile.fileName(), tune::info::fileName::darkColor, tune::info::fileName::fontSize);
    ui->fileNameLabel->show();
    m_fileNameTimer.start(tune::info::fileName::showTime);

    setMediaMode(MediaMode::Gif);
    m_gifPlayer.setFileName(filePath);
    m_gifOriginalSize = QImageReader(filePath).size();
    m_gifPlayer.setScaledSize(m_gifOriginalSize);
    ui->label->setMovie(&m_gifPlayer);
    m_gifPlayer.start();

    return true;
}

bool MainWindow::loadVideo()
{
    QString filePath { m_currentFile.absoluteFilePath() };
    setLabelText(ui->fileNameLabel, m_currentFile.fileName(), tune::info::fileName::lightColor, tune::info::fileName::fontSize, true);
    ui->fileNameLabel->show();
    m_fileNameTimer.start(tune::info::fileName::showTime);

    setMediaMode(MediaMode::Video);

    if(m_vlcMedia) {
        delete m_vlcMedia;
    }
    m_vlcMedia = new VlcMedia(filePath, true, &m_vlcInstance);
    m_videoPlayer.open(m_vlcMedia);
    m_vlcAudio = m_videoPlayer.audio();
    if(m_vlcAudio) {
        m_vlcAudio->setVolume(0);
    }
    QTimer::singleShot(300, [this](){calcVideoFactor(m_videoPlayer.video()->size());});

    return true;
}

bool MainWindow::reloadVideo()
{
    QString filePath { m_currentFile.absoluteFilePath() };

    if(m_vlcMedia) {
        delete m_vlcMedia;
    }
    m_vlcMedia = new VlcMedia(filePath, true, &m_vlcInstance);
    m_videoPlayer.open(m_vlcMedia);
    m_vlcAudio = m_videoPlayer.audio();

    return true;
}

void MainWindow::setMediaMode(MediaMode type)
{
    m_mediaMode = type;
    m_scaleFactor = tune::zoom::origin;
    ui->progressSlider->setValue(0);
    ui->codecErrorLabel->hide();
    ui->label->clear();

    if(m_mediaMode == MediaMode::Video) {
        ui->label->hide();
        ui->videoPane->show();
        showSliders();
        m_zoomTimer.invalidate();
    } else {
        m_videoPlayer.stop();
        m_gifPlayer.stop();
        ui->videoPane->hide();
        ui->label->show();
        m_zoomTimer.start();
    }
}

void MainWindow::calcImageFactor()
{
    int w { m_image.width() };
    int h { m_image.height() };

    qreal sW = screen().width() - tune::screen::reserve;
    qreal sH = screen().height() - tune::screen::reserve;

    qreal wRatio { sW/w };
    qreal hRatio { sH/h };
    constexpr qreal orig {tune::zoom::origin};

    if(wRatio < orig || hRatio < orig) {
        m_scaleFactor = std::min(wRatio, hRatio);
    } else {
        m_scaleFactor = orig;
    }
}

void MainWindow::calcVideoFactor(const QSizeF &nativeSize)
{   
    QSize screenSize { size() };

    QSizeF s;

    bool nativeFits { nativeSize.boundedTo(screenSize) == nativeSize };
    if(nativeFits) {
        s = nativeSize;
    } else {
        s = nativeSize.scaled(screenSize, Qt::KeepAspectRatio);
    }

    QPointF pos { rect().center() };
    pos.rx() -= s.width()/2.;
    pos.ry() -= s.height()/2.;

    QRectF geom(pos, s);

    ui->videoView->setGeometry(geom.toRect());
    ui->videoView->setMaximumSize(geom.size().toSize());
    ui->videoView->setMinimumSize(geom.size().toSize());
}

void MainWindow::resetScale()
{
    if(m_mediaMode == MediaMode::Image) {
        calcImageFactor();
        applyImage();
    } else if(m_mediaMode == MediaMode::Gif) {
        m_scaleFactor = tune::zoom::origin;
        applyGif();
    }
}

void MainWindow::applyImage()
{
    if(m_scaleFactor == tune::zoom::origin) {
        ui->label->setPixmap(QPixmap::fromImage(m_image));
    } else {
        ui->label->setPixmap(QPixmap::fromImage(m_image)
                             .scaled(m_image.size()*m_scaleFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void MainWindow::applyGif()
{
    m_gifPlayer.setScaledSize(m_gifOriginalSize*m_scaleFactor);
}

bool MainWindow::zoom(Direction dir, InputType type)
{
    if(m_mediaMode == MediaMode::Video) {
        return false;
    }

    qreal factor { tune::zoom::factors[dir][type] };

    qreal result { m_scaleFactor + factor };
    if(result <= tune::zoom::min) {
        return false;
    }

    if(result >= tune::zoom::max) {
        return false;
    }

    if(m_zoomTimer.elapsed() > tune::zoom::delay) {
        m_scaleFactor = result;

        if(m_mediaMode == MediaMode::Image) {
            applyImage();
            centerScrollArea(ui->scrollArea, ui->label);
        } else {
            applyGif();
        }

        m_zoomTimer.restart();
    }

    return true;
}

bool MainWindow::volumeStep(Direction dir, InputType type)
{
    int value {ui->volumeSlider->value()};

    if((value == tune::volume::min && dir == Direction::Backward)
    || (value == tune::volume::max && dir == Direction::Forward)) {
        return false;
    }

    value += tune::volume::factors[dir][type];

    value = qBound(tune::volume::min, value, tune::volume::max);
    ui->volumeSlider->setValue(value);
    return true;
}

void MainWindow::gotoNextFile(Direction dir)
{
    QFileInfoList files { getDirFiles(m_currentFile.dir().path()) };

    int i { files.indexOf(m_currentFile) };
    if(dir == Direction::Backward) {
        i -= 1;
        if(i < 0) {
            i = files.size()-1;
        }
    } else {
        i += 1;
        if(i >= files.size()) {
            i = 0;
        }
    }

    m_currentFile = files[i];
    loadFile();
}

void MainWindow::videoRewind(Direction dir)
{
    m_userChangedVideoPos = true;

    qreal step { tune::video::rewind };
    if(dir == Direction::Backward) {
        step *= -1;
    }

    m_videoPlayer.setPosition(m_videoPlayer.position() + step);
}

bool MainWindow::dragImage(QPoint p)
{
    m_mouseDraging = true;

    auto hBar { ui->scrollArea->horizontalScrollBar() };
    auto vBar { ui->scrollArea->verticalScrollBar() };
    bool barsVisible { hBar->isVisible() || vBar->isVisible() };

    if(!barsVisible) {
        return false;
    }

    QPoint diff { m_clickPoint - p };
    hBar->setValue(hBar->value() + diff.x());
    vBar->setValue(vBar->value() + diff.y());
    m_clickPoint = p;

    return true;
}

void MainWindow::showSliders()
{
    ui->progressSlider->show();
    ui->volumeSlider->show();

    m_slidersTimer.start(tune::slider::showTime);
}

void MainWindow::resumeVideo()
{
    auto state = m_videoPlayer.state();
    if(state == Vlc::Paused) {
        m_videoPlayer.resume();
    } else if(state == Vlc::Ended) {
        m_videoPlayer.setPosition(0);
        m_videoPlayer.play();
    }
}

void MainWindow::toggleVideo()
{
    auto state = m_videoPlayer.state();
    if(state == Vlc::Paused || state == Vlc::Playing) {
        m_videoPlayer.togglePause();
    } else if(state == Vlc::Ended) {
        reloadVideo();
    }
}

void MainWindow::onClick()
{
    QWidget *w { ui->videoView->childAt(m_clickPoint) };
    if(w == ui->volumeSlider || w == ui->progressSlider) {
        return;
    }

    int screenWidth { size().width() };
    int x { m_clickPoint.x() };

    qreal rx { x/static_cast<qreal>(screenWidth) };

    if(rx <= tune::screen::backwardSection) {
        gotoNextFile(Direction::Backward);
    } else if(rx >= tune::screen::forwardSection) {
        gotoNextFile(Direction::Forward);
    }
}


#define imageMode m_mediaMode == MediaMode::Image
#define gifMode   m_mediaMode == MediaMode::Gif
#define videoMode m_mediaMode == MediaMode::Video

bool MainWindow::event(QEvent *event)
{
    if(m_appMode == AppMode::DragDialog) {
        return QMainWindow::event(event);
    }

    using namespace std::placeholders;
    auto zoomOrVolumeStep = std::bind(videoMode ? &MainWindow::volumeStep : &MainWindow::zoom, this, _1, _2);

    switch(event->type()) {
        case QEvent::KeyPress: {
            QKeyEvent *keyEvent { static_cast<QKeyEvent *>(event) };
            auto key { keyEvent->key() };
            bool ctrl { keyEvent->modifiers() & Qt::ControlModifier };

            auto rewindOrGotoNext = std::bind(videoMode && ctrl ? &MainWindow::videoRewind : &MainWindow::gotoNextFile, this, _1);

            switch(key) {
                case Qt::Key_Escape: setAppMode(AppMode::DragDialog); return true;
                case Qt::Key_Left:   rewindOrGotoNext(Direction::Backward); return true;
                case Qt::Key_Right:  rewindOrGotoNext(Direction::Forward); return true;
                case Qt::Key_Plus:
                case Qt::Key_Up:     zoomOrVolumeStep(Direction::Forward, InputType::Button); return true;
                case Qt::Key_Minus:
                case Qt::Key_Down:   zoomOrVolumeStep(Direction::Backward, InputType::Button); return true;
                case Qt::Key_Space:  videoMode ? toggleVideo() : resetScale(); return true;
                case Qt::Key_Return: resetScale(); return true;
                default: break;
            }
        } break;

        case QEvent::Wheel: {
            QWheelEvent *wheelEvent { static_cast<QWheelEvent *>(event) };
            Direction dir { wheelEvent->delta() > 0 ? Direction::Forward : Direction::Backward };
            zoomOrVolumeStep(dir, InputType::Wheel);
            return true;
        } break;

        case QEvent::MouseButtonPress: {
            QMouseEvent *pressEvent { static_cast<QMouseEvent *>(event) };
            m_clickPoint = pressEvent->pos();
        } break;

        case QEvent::MouseButtonRelease: {
            if(!m_mouseDraging) {
                onClick();
            }
            m_mouseDraging = false;
        } break;

        case QEvent::MouseMove: {
            if(videoMode) {
                showSliders();
            }

            QMouseEvent *moveEvent { static_cast<QMouseEvent *>(event) };
            bool isLeftClicked { moveEvent->buttons() & Qt::LeftButton };
            if(!isLeftClicked) {
                return false;
            }

            return dragImage(moveEvent->pos());
        } break;
        default: break;
    }

    return QMainWindow::event(event);
}

} // namespace pork
