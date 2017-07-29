#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QElapsedTimer>
#include <QTimer>
#include <QMediaPlayer>
#include <QGraphicsVideoItem>
#include <QMovie>
#include <QDir>

namespace Ui {
class MainWindow;
}

#define enum_class(x) class x { public: enum type
#define enum_interface };
#define enum_end ;}

enum_class(ZoomType) {
    Button = 0,
    Wheel
} enum_end;

enum_class(Direction) {
    Forward = 0,
    Backward
} enum_end;

enum_class(MediaMode) {
    Image = 0,
    Gif,
    Video
} enum_end;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    bool openFile(const QString &fileName);
    bool loadFile();
    bool loadImage();
    bool loadGif();
    bool loadVideo();
    void calcImageFactor();
    void calcVideoFactor(const QSizeF &nativeSize);
    void applyImage();
    void applyGif();
    void gotoNextFile(Direction::type dir);
    void videoRewind(Direction::type dir);
    bool dragImage(QPoint p);
    void showSliders();

    bool zoom(Direction::type dir, ZoomType::type type);
    void setMode(MediaMode::type type);

    void onClick();

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual bool event(QEvent *event) override;
    virtual void dragEnterEvent(QDragEnterEvent *) override;
    virtual void dropEvent(QDropEvent *) override;

private:
    Ui::MainWindow *ui;

    MediaMode::type m_mode { MediaMode::Image };
    QFileInfo m_currentFile;

    QImage m_image;
    QMovie m_gifPlayer;
    QMediaPlayer m_videoPlayer;
    QGraphicsVideoItem m_graphicsItem;

    QSize m_gifOriginalSize;
    qreal m_scaleFactor { 1.0 };
    QElapsedTimer m_zoomTimer;
    QTimer m_slidersTimer;
    QPoint m_clickPoint;
    bool m_mouseDraging { false };
    bool m_userChangedVideoPos { false };
};

#endif // MAINWINDOW_H
