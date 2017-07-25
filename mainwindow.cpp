#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QImageReader>
#include <QKeyEvent>
#include <QMessageBox>
#include <QDirIterator>
#include <QDesktopWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QScrollBar>
#include <QDebug>

static const QStringList supportedImages { "*.jpg", "*.jpeg", "*.png", "*.bmp" };
static const QStringList supportedGif { "*.gif" };
static const QStringList supportedVideo { "*.webm", "*.wmv", "*.mp4", "*.mpg" };

static bool fileBelongsTo(const QString &file, const QStringList &list)
{
    for(const auto &ext : list) {
        QRegExp reg {ext, Qt::CaseInsensitive, QRegExp::Wildcard};
        if(reg.exactMatch(file)) {
            return true;
        }
    }
    return false;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_player(this)
    , m_volumeSlider(Qt::Vertical)
    , m_progressSlider(Qt::Horizontal)
{
    ui->setupUi(this);

    m_player.setVideoOutput(&m_graphicsItem);
    ui->videoView->setScene(new QGraphicsScene);
    ui->videoView->scene()->addItem(&m_graphicsItem);

    m_codecErrorLabel.setPos(0, 0);
    m_codecErrorLabel.setScale(5);
    m_codecErrorLabel.setDefaultTextColor(QColor(Qt::white));
    m_codecErrorLabel.setPlainText("Unknown Codec!");
    ui->videoView->scene()->addItem(&m_codecErrorLabel);
    m_codecErrorLabel.hide();

    m_volumeSlider.setParent(ui->videoView);
    m_volumeSlider.setRange(0, 100);
    m_volumeSlider.move(0, 0);
    m_volumeSlider.show();

    m_progressSlider.setParent(ui->videoView);
    m_progressSlider.setRange(0, 100);
    m_progressSlider.move(20, 0);
    m_progressSlider.show();

    connect(&m_player, &QMediaPlayer::volumeChanged, &m_volumeSlider, &QSlider::setValue);
    connect(&m_player, &QMediaPlayer::durationChanged, &m_progressSlider, &QSlider::setMaximum);
    connect(&m_player, &QMediaPlayer::positionChanged, &m_progressSlider, &QSlider::setValue);

    connect(&m_graphicsItem, &QGraphicsVideoItem::nativeSizeChanged, this, &MainWindow::calcVideoFactor);
    connect(&m_player, &QMediaPlayer::mediaStatusChanged, [this](QMediaPlayer::MediaStatus status){
        if(status == QMediaPlayer::InvalidMedia) {
            m_codecErrorLabel.show();
        }
    });

    setMode(MediaMode::Image);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *data {event->mimeData()};

    if(!data->hasUrls()) {
        event->ignore();
        return;
    }

    QString url { data->urls().first().toString() };

    bool pic = fileBelongsTo(url, supportedImages);
    bool gif = fileBelongsTo(url, supportedGif);
    bool video = fileBelongsTo(url, supportedVideo);

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

bool MainWindow::openFile(const QString &filename)
{
    m_currentFile = QFileInfo {filename};
    bool ok { loadFile() };
    if(ok) {
        showFullScreen();
        qApp->installEventFilter(this);
    }

    return ok;
}

bool MainWindow::loadFile()
{
    QString filePath { m_currentFile.absoluteFilePath() };

    if(fileBelongsTo(filePath, supportedImages)) {
        return loadImage();
    }

    if(fileBelongsTo(filePath, supportedGif)) {
        return loadGif();
    }

    if(fileBelongsTo(filePath, supportedVideo)) {
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

    setMode(MediaMode::Image);

    calcImageFactor();
    applyImage();

    m_zoomTimer.start();

    return true;
}

bool MainWindow::loadGif()
{
    QString filePath { m_currentFile.absoluteFilePath() };

    setMode(MediaMode::Gif);
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

    setMode(MediaMode::Video);
    m_player.setMedia(QUrl{filePath});
    m_volume = 0;
    m_player.setVolume(0);

    m_player.play();

    return true;
}

const int screenReserve {2};

void MainWindow::calcImageFactor()
{
    int w { m_image.width() };
    int h { m_image.height() };
    auto screen = QApplication::desktop()->screenGeometry();

    qreal sW = screen.width() - screenReserve;
    qreal sH = screen.height() - screenReserve;

    qreal wRatio { sW/w };
    qreal hRatio { sH/h };
    if(wRatio < 1.0 || hRatio < 1.0) {
        m_scaleFactor = std::min(wRatio, hRatio);
    }
}

void MainWindow::calcVideoFactor(const QSizeF &nativeSize)
{
    QSize screenSize = QApplication::desktop()->screenGeometry().size();

    bool nativeFits = nativeSize.boundedTo(screenSize) == nativeSize;
    if(nativeFits) {
        m_graphicsItem.setSize(nativeSize);
    } else {
        m_graphicsItem.setSize(nativeSize.scaled(screenSize, Qt::KeepAspectRatio));
    }
}

void MainWindow::applyImage()
{
    if(m_scaleFactor == 1.0) {
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

static void centerScrollArea(QScrollArea *area, QLabel* label)
{
    auto screen = QApplication::desktop()->screenGeometry();
    auto pixmap = label->pixmap();
    int w = (pixmap->width() - screen.width() + screenReserve)/2;
    int h = (pixmap->height() - screen.height() + screenReserve)/2;

    area->horizontalScrollBar()->setValue(w);
    area->verticalScrollBar()->setValue(h);
}

bool MainWindow::zoom(ZoomDir::type dir, ZoomType::type type)
{
    if(m_mode == MediaMode::Video) {
        return false;
    }

    static const qreal factors[2][2] {
        { 0.07,  0.15},
        {-0.07, -0.15},
    };

    qreal factor { factors[dir][type] };

    qreal result { m_scaleFactor + factor };
    if(result <= 0.0) {
        return false;
    }

    /*if(m_mode == MediaMode::Gif && result >= 2.0) {
        return false;
    }*/

    if(result >= 4.0) {
        return false;
    }

    const int zoomDelay = 5; //ms
    if(m_zoomTimer.elapsed() > zoomDelay) {
        m_scaleFactor = result;

        if(m_mode == MediaMode::Image) {
            applyImage();
            centerScrollArea(ui->scrollArea, ui->label);
        } else {
            applyGif();
        }

        m_zoomTimer.restart();
    }

    return true;
}

void MainWindow::setMode(MediaMode::type type)
{
    m_mode = type;
    m_scaleFactor = 1.0;
    m_codecErrorLabel.hide();

    if(m_mode == MediaMode::Video) {
        ui->label->hide();
        ui->videoView->show();
        m_zoomTimer.invalidate();
    } else {
        ui->label->show();
        ui->videoView->hide();
        m_player.stop();
        m_gifPlayer.stop();
    }
}

static QFileInfoList getDirFiles(const QString &path)
{
    QFileInfoList res;

    QDirIterator it(path, QStringList() << supportedImages << supportedGif << supportedVideo, QDir::Files);
    while(it.hasNext()) {
        it.next();
        res << it.fileInfo();
    }
    return res;
}

static QPoint clickPoint;

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    switch(event->type()) {
        case QEvent::KeyPress: {
            QKeyEvent *keyEvent { static_cast<QKeyEvent *>(event) };

            auto key { keyEvent->key() };
            switch(key) {
                case Qt::Key_Escape: qApp->quit(); return true;
                case Qt::Key_Left:
                case Qt::Key_Right: {
                    QFileInfoList files = getDirFiles(m_currentFile.dir().path());

                    int i { files.indexOf(m_currentFile) };
                    if(key == Qt::Key_Left) {
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
                    return true;
                } break;

                case Qt::Key_Plus:  zoom(ZoomDir::In, ZoomType::Button); return true;
                case Qt::Key_Minus: zoom(ZoomDir::Out, ZoomType::Button); return true;
            }
        } break;

        case QEvent::Wheel: {
            QWheelEvent *wheelEvent { static_cast<QWheelEvent *>(event) };

            if(m_mode == MediaMode::Video) {
                m_volume += wheelEvent->angleDelta().y()/20.0;
                m_player.setVolume(m_volume);
            } else {
                ZoomDir::type dir { wheelEvent->delta() > 0 ? ZoomDir::In : ZoomDir::Out };
                zoom(dir, ZoomType::Wheel);
            }
            return true;
        } break;

        case QEvent::MouseButtonPress: {
            QMouseEvent *pressEvent { static_cast<QMouseEvent *>(event) };
            clickPoint = pressEvent->pos();
            return true;
        } break;

        case QEvent::MouseButtonRelease: {
            return true;
        } break;

        case QEvent::MouseMove: {
            QMouseEvent *moveEvent { static_cast<QMouseEvent *>(event) };
            bool isLeftClicked { moveEvent->buttons() & Qt::LeftButton };
            if(!isLeftClicked) {
                return false;
            }

            auto hBar { ui->scrollArea->horizontalScrollBar() };
            auto vBar { ui->scrollArea->verticalScrollBar() };
            bool barsVisible { hBar->isVisible() || vBar->isVisible() };

            if(!barsVisible) {
                return false;
            }

            QPoint diff { clickPoint - moveEvent->pos() };
            hBar->setValue(hBar->value() + diff.x());
            vBar->setValue(vBar->value() + diff.y());
            clickPoint = moveEvent->pos();
            return true;

        } break;
    }

    return QMainWindow::eventFilter(watched, event);
}
