#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "config.h"
#include "utils.h"
#include "widgets/mediawidget/config.h"

#include <QLabel>
#include <QDropEvent>
#include <QMimeData>
#include <QDir>

namespace pork {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_settings("PitM", "Pork")
{
    ui->setupUi(this);

    setAppMode(AppMode::DragDialog);
    ui->fileNameLabel->setContentsMargins(tune::info::fileName::pad, tune::info::fileName::pad, 0, 0);
    m_fileNameTimer.setSingleShot(true);
    connect(&m_fileNameTimer, &QTimer::timeout, ui->fileNameLabel, &QLabel::hide);

    restoreGeometry(m_settings.value(tune::reg::dragWindowGeometry).toByteArray());
    connect(qApp, &QApplication::aboutToQuit, [this]() {
        if(m_appMode == AppMode::DragDialog) {
            m_settings.setValue(tune::reg::dragWindowGeometry, saveGeometry());
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    QRect window { QPoint{}, event->size()};
    ui->fileNameLabel->resize(window.width(), tune::info::fileName::fontSize*2 + tune::info::fileName::pad);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *data {event->mimeData()};

    if(!data->hasUrls()) {
        event->ignore();
        return;
    }

    QString url { data->urls().first().toString() };

    using namespace aske::MediaWidgetCapabilities;

    bool pic { fileBelongsTo(url, supportedImages) };
    bool gif { fileBelongsTo(url, supportedGif) };
#ifdef ASKELIB_USE_VLC
    bool video { fileBelongsTo(url, supportedVideo) };
#else
    bool video {false};
#endif

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

void MainWindow::setAppMode(AppMode type)
{
    m_appMode = type;
    if(type == AppMode::Fullscreen) {
        showFullScreen();
        ui->fileNameLabel->show();
    } else {
        showNormal();
        //setLabelText(ui->label, tr("Drag image/video here..."), tune::info::dragLabelColor);
        ui->fileNameLabel->hide();
    }
}

bool MainWindow::openFile(const QString &filename)
{
    m_currentFile = filename;
    bool ok { ui->mediaWidget->loadFile(m_currentFile) };
    if(ok) {
        setAppMode(AppMode::Fullscreen);
        setLabelText(ui->fileNameLabel, m_currentFile, tune::info::fileName::darkColor, tune::info::fileName::fontSize);
        ui->fileNameLabel->show();
        m_fileNameTimer.start(tune::info::fileName::showTime);
    }

    return ok;
}

bool MainWindow::event(QEvent *event)
{
    if(m_appMode == AppMode::DragDialog) {
        return QMainWindow::event(event);
    }

    switch(event->type()) {
        case QEvent::KeyPress: {
            QKeyEvent *keyEvent { static_cast<QKeyEvent *>(event) };
            auto key { keyEvent->key() };

            switch(key) {
                case Qt::Key_Escape: ui->mediaWidget->setMediaMode(aske::MediaMode::No); setAppMode(AppMode::DragDialog); return true;
                default: break;
            }
        } break;
        default: break;
    }

    return QMainWindow::event(event);
}

} // namespace pork
