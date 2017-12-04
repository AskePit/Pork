#include "mediawidget.h"
#include "config.h"

#include <QDebug>
#include <QDir>
#include <QResizeEvent>
#include <QMessageBox>
#include <QScrollBar>
#include <QDesktopWidget>
#include <QDirIterator>
#include <functional>

namespace aske
{

bool fileBelongsTo(const QString &file, const QStringList &list)
{
    for(const auto &ext : list) {
        QRegExp reg {ext, Qt::CaseInsensitive, QRegExp::Wildcard};
        if(reg.exactMatch(file)) {
            return true;
        }
    }
    return false;
}

QRect screen()
{
    return QApplication::desktop()->screenGeometry();
}

void centerScrollArea(QScrollArea *area, QLabel* label)
{
    auto screen { QApplication::desktop()->screenGeometry() };
    auto pixmap { label->pixmap() };
    int w { (pixmap->width() - screen.width() + MediaWidgetTune::screen::reserve)/2 };
    int h { (pixmap->height() - screen.height() + MediaWidgetTune::screen::reserve)/2 };

    area->horizontalScrollBar()->setValue(w);
    area->verticalScrollBar()->setValue(h);
}

QFileInfoList getDirFiles(const QString &path)
{
    QFileInfoList res;

    QDirIterator it(path, MediaWidgetCapabilities::supportedFormats(), QDir::Files);
    while(it.hasNext()) {
        it.next();
        res << it.fileInfo();
    }
    return res;
}

MediaWidget::MediaWidget(QWidget *parent)
    : QScrollArea(parent)
    , m_mediaWidget(this)
    , m_mediaWidgetLayout(&m_mediaWidget)
    , m_imageView(&m_mediaWidget)
#ifdef VIDEO_SUPPORT
    , m_videoWidget(&m_mediaWidget)
#endif
{
    setWidgetResizable(true);
    setWidget(&m_mediaWidget);

    m_mediaWidget.setMouseTracking(true);
    m_mediaWidgetLayout.setSpacing(0);
    m_mediaWidgetLayout.setContentsMargins(0, 0, 0, 0);
    m_mediaWidget.setStyleSheet("QWidget{ background-color: white; }");

    m_imageView.setAlignment(Qt::AlignHCenter | Qt::AlignCenter);
    m_imageView.setContentsMargins(0, 0, 0, 0);
    m_imageView.setStyleSheet("QLabel{ background-color: white; }");
    m_imageView.setSizePolicy(QSizePolicy::Expanding,
                              QSizePolicy::Expanding);
    m_imageView.setLineWidth(0);

    m_mediaWidgetLayout.addWidget(&m_imageView);

#ifdef VIDEO_SUPPORT
    m_videoUi.setupUi(&m_videoWidget);
    m_mediaWidgetLayout.addWidget(&m_videoWidget);
#endif

    setMouseTracking(true);

#ifdef VIDEO_SUPPORT
    m_videoPlayer.setWidgets(m_videoUi.videoView, m_videoUi.progressSlider, m_videoUi.volumeSlider, m_videoUi.codecErrorLabel);
    connect(&m_videoPlayer, &aske::VideoPlayer::loaded, this, [this](){calcVideoFactor(m_videoPlayer.size());}, Qt::QueuedConnection);
#endif

    setMediaMode(MediaMode::No);
}

MediaWidget::~MediaWidget()
{

}

#ifdef VIDEO_SUPPORT
void MediaWidget::resizeEvent(QResizeEvent *event)
{
    using namespace MediaWidgetTune;

    QRect window { QPoint{}, event->size()};
    QRect label { m_videoUi.codecErrorLabel->rect() };
    QRect volume { m_videoUi.volumeSlider->rect() };
    QRect progress { m_videoUi.progressSlider->rect() };

    constexpr int pad { slider::pad };

    label.moveCenter(window.center());
    volume.moveRight(window.right()-pad/2);
    volume.moveTop(pad);
    progress.moveLeft(pad);
    progress.moveBottom(window.bottom()-pad/2);
    progress.setWidth(window.width()-2*pad);


    m_videoUi.codecErrorLabel->setGeometry(label);
    m_videoUi.volumeSlider->setGeometry(volume);
    m_videoUi.progressSlider->setGeometry(progress);
}
#endif

bool MediaWidget::loadFile(const QString &fileName)
{
    using namespace MediaWidgetCapabilities;

    m_currentFile = QFileInfo(fileName);

    QString filePath { m_currentFile.absoluteFilePath() };

    if(fileBelongsTo(filePath, supportedImages)) {
        return loadImage();
    }

    if(fileBelongsTo(filePath, supportedGif)) {
        return loadGif();
    }

#ifdef VIDEO_SUPPORT
    if(fileBelongsTo(filePath, supportedVideo)) {
        return loadVideo();
    }
#endif

    return false;
}

bool MediaWidget::loadImage()
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

    return true;
}

bool MediaWidget::loadGif()
{
    QString filePath { m_currentFile.absoluteFilePath() };

    setMediaMode(MediaMode::Gif);
    m_gifPlayer.setFileName(filePath);
    m_gifOriginalSize = QImageReader(filePath).size();
    m_gifPlayer.setScaledSize(m_gifOriginalSize);
    m_imageView.setMovie(&m_gifPlayer);
    m_gifPlayer.start();

    return true;
}

#ifdef VIDEO_SUPPORT
bool MediaWidget::loadVideo()
{
    QString filePath { m_currentFile.absoluteFilePath() };
    setMediaMode(MediaMode::Video);
    m_videoPlayer.load(filePath);

    return true;
}
#endif

void MediaWidget::setMediaMode(MediaMode type)
{
    using namespace MediaWidgetTune;

    m_mediaMode = type;
    m_scaleFactor = zoom::origin;
#ifdef VIDEO_SUPPORT
    m_videoUi.progressSlider->setValue(0);
    m_videoUi.volumeSlider->setValue(0);
    m_videoUi.codecErrorLabel->hide();
#endif
    m_imageView.clear();

    if(m_mediaMode == MediaMode::No) {
        m_imageView.hide();
#ifdef VIDEO_SUPPORT
        m_videoWidget.hide();
    } else if(m_mediaMode == MediaMode::Video) {
        m_imageView.hide();
        m_videoWidget.show();
        m_zoomTimer.invalidate();
#endif
    } else {
#ifdef VIDEO_SUPPORT
        m_videoPlayer.stop();
        m_videoWidget.hide();
#endif
        m_gifPlayer.stop();
        m_imageView.show();
        m_zoomTimer.start();
    }
}

void MediaWidget::calcImageFactor()
{
    int w { m_image.width() };
    int h { m_image.height() };

    qreal sW = screen().width() - MediaWidgetTune::screen::reserve;
    qreal sH = screen().height() - MediaWidgetTune::screen::reserve;

    qreal wRatio { sW/w };
    qreal hRatio { sH/h };
    constexpr qreal orig {MediaWidgetTune::zoom::origin};

    if(wRatio < orig || hRatio < orig) {
        m_scaleFactor = qMin(wRatio, hRatio);
    } else {
        m_scaleFactor = orig;
    }
}

#ifdef VIDEO_SUPPORT
void MediaWidget::calcVideoFactor(const QSizeF &nativeSize)
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

    m_videoUi.videoView->setGeometry(geom.toRect());
    m_videoUi.videoView->setMaximumSize(geom.size().toSize());
    m_videoUi.videoView->setMinimumSize(geom.size().toSize());
}
#endif

void MediaWidget::resetScale()
{
    using namespace MediaWidgetTune;

    if(m_mediaMode == MediaMode::Image) {
        calcImageFactor();
        applyImage();
    } else if(m_mediaMode == MediaMode::Gif) {
        m_scaleFactor = zoom::origin;
        applyGif();
    }
}

void MediaWidget::applyImage()
{
    using namespace MediaWidgetTune;

    if(m_scaleFactor == zoom::origin) {
        m_imageView.setPixmap(QPixmap::fromImage(m_image));
    } else {
        m_imageView.setPixmap(QPixmap::fromImage(m_image)
                             .scaled(m_image.size()*m_scaleFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void MediaWidget::applyGif()
{
    m_gifPlayer.setScaledSize(m_gifOriginalSize*m_scaleFactor);
}

#ifdef VIDEO_SUPPORT
void MediaWidget::videoRewind(Direction dir)
{
    using namespace MediaWidgetTune;

    m_videoPlayer.rewind(dir, video::rewind);
}
#endif

bool MediaWidget::zoom(Direction dir, InputType type)
{
    using namespace MediaWidgetTune;

#ifdef VIDEO_SUPPORT
    if(m_mediaMode == MediaMode::Video) {
        return false;
    }
#endif

    qreal factor { zoom::factors[dir][type] };

    qreal result { m_scaleFactor + factor };
    if(result <= zoom::min) {
        return false;
    }

    if(result >= zoom::max) {
        return false;
    }

    if(m_zoomTimer.elapsed() > zoom::delay) {
        m_scaleFactor = result;

        if(m_mediaMode == MediaMode::Image) {
            applyImage();
            centerScrollArea(this, &m_imageView);
        } else {
            applyGif();
        }

        m_zoomTimer.restart();
    }

    return true;
}

#ifdef VIDEO_SUPPORT
bool MediaWidget::volumeStep(Direction dir, InputType type)
{
    using namespace MediaWidgetTune;

    int value {m_videoUi.volumeSlider->value()};

    if((value == volume::min && dir == Direction::Backward)
    || (value == volume::max && dir == Direction::Forward)) {
        return false;
    }

    value += volume::factors[dir][type];

    value = qBound(volume::min, value, volume::max);
    m_videoUi.volumeSlider->setValue(value);
    return true;
}
#endif

void MediaWidget::gotoNextFile(Direction dir)
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
    loadFile(m_currentFile.absoluteFilePath());
}

bool MediaWidget::dragImage(QPoint p)
{
    m_mouseDraging = true;

    auto hBar { horizontalScrollBar() };
    auto vBar { verticalScrollBar() };
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

void MediaWidget::onClick()
{
    using namespace MediaWidgetTune;

#ifdef VIDEO_SUPPORT
    QWidget *w { m_videoUi.videoView->childAt(m_clickPoint) };
    if(w == m_videoUi.volumeSlider || w == m_videoUi.progressSlider) {
        return;
    }
#endif

    int screenWidth { size().width() };
    int x { m_clickPoint.x() };

    qreal rx { x/static_cast<qreal>(screenWidth) };

    if(rx <= screen::backwardSection) {
        gotoNextFile(Direction::Backward);
    } else if(rx >= screen::forwardSection) {
        gotoNextFile(Direction::Forward);

#ifdef VIDEO_SUPPORT
    } else {
        if(m_mediaMode == MediaMode::Video) {
            m_videoPlayer.toggle();
        }
#endif
    }
}


#define imageMode m_mediaMode == MediaMode::Image
#define gifMode   m_mediaMode == MediaMode::Gif
#ifdef VIDEO_SUPPORT
#define videoMode m_mediaMode == MediaMode::Video
#endif

bool MediaWidget::event(QEvent *event)
{
    using namespace std::placeholders;
    auto zoomOrVolumeStep = std::bind(
#ifdef VIDEO_SUPPORT
                videoMode ? &MediaWidget::volumeStep :
#endif
                            &MediaWidget::zoom, this, _1, _2);

    switch(event->type()) {
        case QEvent::KeyPress: {
            QKeyEvent *keyEvent { static_cast<QKeyEvent *>(event) };
            auto key { keyEvent->key() };

#ifdef VIDEO_SUPPORT
            bool ctrl { keyEvent->modifiers() & Qt::ControlModifier };
#endif

            auto rewindOrGotoNext = std::bind(
#ifdef VIDEO_SUPPORT
                        videoMode && ctrl ? &MediaWidget::videoRewind :
#endif
                                            &MediaWidget::gotoNextFile, this, _1);

            switch(key) {
                case Qt::Key_Left:   rewindOrGotoNext(Direction::Backward); return true;
                case Qt::Key_Right:  rewindOrGotoNext(Direction::Forward); return true;
                case Qt::Key_Plus:
                case Qt::Key_Up:     zoomOrVolumeStep(Direction::Forward, InputType::Button); return true;
                case Qt::Key_Minus:
                case Qt::Key_Down:   zoomOrVolumeStep(Direction::Backward, InputType::Button); return true;
                case Qt::Key_Space:
#ifdef VIDEO_SUPPORT
                                     if(videoMode) m_videoPlayer.toggle(); return true;
#else
                //[[fallthrough]]
#endif

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
#ifdef VIDEO_SUPPORT
            if(videoMode) {
                m_videoPlayer.showSliders();
            }
#endif

            QMouseEvent *moveEvent { static_cast<QMouseEvent *>(event) };
            bool isLeftClicked { moveEvent->buttons() & Qt::LeftButton };
            if(!isLeftClicked) {
                return false;
            }

            return dragImage(moveEvent->pos());
        } break;
        default: break;
    }

    return QScrollArea::event(event);
}

#undef imageMode
#undef gifMode
#ifdef VIDEO_SUPPORT
#undef videoMode
#endif

} // namespace aske
