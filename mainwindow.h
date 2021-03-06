#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "videoplayer.h"

#include <QMainWindow>
#include <QElapsedTimer>
#include <QMovie>
#include <QFileInfo>
#include <QSettings>

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
    bool dragImage(QPoint p);

    void videoRewind(Direction dir);
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

    QSettings m_settings;

    MediaMode m_mediaMode { MediaMode::Image };
    AppMode m_appMode { AppMode::DragDialog };

    QFileInfo m_currentFile;

    QImage m_image;
    QMovie m_gifPlayer;
    VideoPlayer m_videoPlayer;

    QSize m_gifOriginalSize;
    qreal m_scaleFactor { 1.0 };
    QElapsedTimer m_zoomTimer;
    QTimer m_fileNameTimer;
    QPoint m_clickPoint;
    bool m_mouseDraging { false };
};

} // namespace pork

#endif // MAINWINDOW_H
