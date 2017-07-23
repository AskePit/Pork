#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QImageReader>
#include <QKeyEvent>
#include <QMessageBox>
#include <QDirIterator>
#include <QDesktopWidget>

static const QStringList supportedPics { "*.jpg", "*.jpeg", "*.png" };


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    openFile("cube.jpg");
    qApp->installEventFilter(this);
    showFullScreen();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::openFile(const QString &filename)
{
    m_currentFile = QFileInfo {filename};
    return loadImage();
}

bool MainWindow::loadImage()
{
    QString filePath { m_currentFile.absoluteFilePath() };

    QImageReader reader(filePath);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (newImage.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                 .arg(QDir::toNativeSeparators(filePath), reader.errorString()));
        return false;
    }

    m_image = newImage;

    calcRatio();
    applyImage();

    return true;
}

void MainWindow::calcRatio()
{
    int w = m_image.width();
    int h = m_image.height();
    auto screen = QApplication::desktop()->screenGeometry();
    qreal sW = screen.width();
    qreal sH = screen.height();

    qreal wRatio = sW/w;
    qreal hRatio = sH/h;
    if(wRatio < 1.0 || hRatio < 1.0) {
        m_scaleFactor = std::min(wRatio, hRatio);
    }
}

void MainWindow::applyImage()
{
    int w = m_image.width();
    int h = m_image.height();
    ui->label->setPixmap(QPixmap::fromImage(m_image)
                         .scaled(w*m_scaleFactor, h*m_scaleFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

QFileInfoList MainWindow::getDirFiles()
{
    QFileInfoList res;

    QDirIterator it(m_currentFile.dir().path(), supportedPics, QDir::Files);
    while(it.hasNext()) {
        it.next();
        res << it.fileInfo();
    }
    return res;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        auto key = keyEvent->key();
        switch(key) {
            case Qt::Key_Escape: qApp->quit(); return true;
            case Qt::Key_Left:
            case Qt::Key_Right: {
                QFileInfoList files = getDirFiles();

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
                loadImage();
                return true;
            } break;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}
