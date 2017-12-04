#ifndef ASKE_MEDIAWIDGET_H
#define ASKE_MEDIAWIDGET_H

#include "videoplayer.h"
#include "ui_videoplayerwidget.h"

#include <QScrollArea>
#include <QLabel>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QMovie>
#include <QElapsedTimer>

#define ASKELIB_USE_VLC

#ifdef ASKELIB_USE_VLC
#define VIDEO_SUPPORT
#endif

namespace aske
{

enum InputType
{
    Button = 0,
    Wheel
};

enum MediaMode
{
    No = 0,
    Image,
    Gif,
#ifdef VIDEO_SUPPORT
    Video
#endif
};

typedef aske::VideoPlayer::Direction Direction;

class MediaWidget : public QScrollArea
{
    Q_OBJECT
public:
    explicit MediaWidget(QWidget *parent = 0);
    ~MediaWidget();

    void setMediaMode(MediaMode type);

    bool loadFile(const QString &fileName);
    bool loadImage();
    bool loadGif();
    void calcImageFactor();
    void resetScale();
    void applyImage();
    void applyGif();
    void gotoNextFile(Direction dir);
    bool dragImage(QPoint p);
    bool zoom(Direction dir, InputType type);

#ifdef VIDEO_SUPPORT
    bool loadVideo();
    void calcVideoFactor(const QSizeF &nativeSize);
    void videoRewind(Direction dir);
    bool volumeStep(Direction dir, InputType type);
#endif

    void onClick();

protected:
#ifdef VIDEO_SUPPORT
    virtual void resizeEvent(QResizeEvent *event) override;
#endif
    virtual bool event(QEvent *event) override;
	
private:
    QWidget m_mediaWidget;
    QVBoxLayout m_mediaWidgetLayout;

	QLabel m_imageView;

#ifdef VIDEO_SUPPORT
    QWidget m_videoWidget;
    Ui::VideoPlayerWidget m_videoUi;
#endif

    MediaMode m_mediaMode { MediaMode::Image };

    QFileInfo m_currentFile;

    QImage m_image;
    QMovie m_gifPlayer;
#ifdef VIDEO_SUPPORT
    VideoPlayer m_videoPlayer;
#endif

    QSize m_gifOriginalSize;
    qreal m_scaleFactor { 1.0 };
    QElapsedTimer m_zoomTimer;
    QPoint m_clickPoint;
    bool m_mouseDraging { false };
};

} // namespace aske

#endif // ASKE_MEDIAWIDGET_H
