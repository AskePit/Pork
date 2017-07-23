#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool openFile(const QString &fileName);
    bool loadImage();
    void calcRatio();
    void applyImage();

    QFileInfoList getDirFiles();

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::MainWindow *ui;

    QFileInfo m_currentFile;
    QImage m_image;
    qreal m_scaleFactor { 1.0 };
};

#endif // MAINWINDOW_H
