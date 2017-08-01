#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QElapsedTimer>
#include <QMovie>
#include <QFileInfo>

#include <VLCQtCore/Instance.h>
#include <VLCQtCore/MediaPlayer.h>

namespace Ui {
class MainWindow;
}

class VlcMedia;

namespace pork {

enum AppMode
{
    DragDialog = 0,
    Fullscreen,
};

enum InputType
{
    Button = 0,
    Wheel
};

enum Direction
{
    Backward = 0,
    Forward,
};

enum MediaMode
{
    Image = 0,
    Gif,
    Video
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:
    void setupVideoPlayer();

    void setAppMode(AppMode type);
    void setMediaMode(MediaMode type);

    bool openFile(const QString &fileName);
    bool loadFile();
    bool loadImage();
    bool loadGif();
    bool loadVideo();
    void calcImageFactor();
    void calcVideoFactor(const QSizeF &nativeSize);
    void resetScale();
    void applyImage();
    void applyGif();
    void gotoNextFile(Direction dir);
    void videoRewind(Direction dir);
    bool dragImage(QPoint p);
    void showSliders();

    bool zoom(Direction dir, InputType type);
    bool volumeStep(Direction dir, InputType type);


    void onClick();

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual bool event(QEvent *event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;

private:
    Ui::MainWindow *ui;

    MediaMode m_mediaMode { MediaMode::Image };
    AppMode m_appMode { AppMode::DragDialog };

    QFileInfo m_currentFile;

    QImage m_image;
    QMovie m_gifPlayer;
    VlcInstance m_vlcInstance;
    VlcMediaPlayer m_videoPlayer;
    VlcMedia *m_vlcMedia {nullptr};
    VlcAudio *m_vlcAudio {nullptr};

    QSize m_gifOriginalSize;
    qreal m_scaleFactor { 1.0 };
    QElapsedTimer m_zoomTimer;
    QTimer m_slidersTimer;
    QPoint m_clickPoint;
    bool m_mouseDraging { false };
    bool m_userChangedVideoPos { false };
};

} // namespace pork

#endif // MAINWINDOW_H
