#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QElapsedTimer>
#include <QMediaPlayer>
#include <QGraphicsVideoItem>
#include <QSlider>
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

enum_class(ZoomDir) {
    In = 0,
    Out
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

    bool zoom(ZoomDir::type dir, ZoomType::type type);
    void setMode(MediaMode::type type);

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
    virtual void dragEnterEvent(QDragEnterEvent *) override;
    virtual void dropEvent(QDropEvent *) override;

private:
    Ui::MainWindow *ui;

    MediaMode::type m_mode { MediaMode::Image };
    QFileInfo m_currentFile;
    QImage m_image;
    QMovie m_gifPlayer;
    QSize m_gifOriginalSize;
    qreal m_scaleFactor { 1.0 };

    QMediaPlayer m_player;
    QGraphicsVideoItem m_graphicsItem;
    QGraphicsTextItem m_codecErrorLabel;
    QSlider m_volumeSlider;
    QSlider m_progressSlider;
    int m_volume {0};
    QElapsedTimer m_zoomTimer;
};

#endif // MAINWINDOW_H
