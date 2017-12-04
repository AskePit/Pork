#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileInfo>
#include <QSettings>
#include <QTimer>

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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:
    void setAppMode(AppMode type);


    bool openFile(const QString &fileName);

protected:
    virtual bool event(QEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;

private:
    Ui::MainWindow *ui;

    QSettings m_settings;
    AppMode m_appMode { AppMode::DragDialog };
    QString m_currentFile;
    QTimer m_fileNameTimer;
};

} // namespace pork

#endif // MAINWINDOW_H
