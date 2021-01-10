#include "utils.h"
#include "config.h"

#include <QScrollArea>
#include <QScrollBar>
#include <QEvent>
#include <QDirIterator>
#include <QApplication>
#include <QDesktopWidget>
#include <QLabel>
#include <QScreen>

namespace pork {

int QSliderStyle::styleHint(QStyle::StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    if (hint == QStyle::SH_Slider_AbsoluteSetButtons) {
        return (Qt::LeftButton | Qt::MidButton | Qt::RightButton);
    }
    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

bool Blocker::eventFilter(QObject *watched, QEvent *event)
{
    switch(event->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseMove:
        case QEvent::KeyPress:
        case QEvent::Wheel:
            event->ignore();
            return true;

        default:
            return QWidget::eventFilter(watched, event);
    }
}

static Blocker *blocker { nullptr };

void block(QAbstractScrollArea *w) {
    if(!blocker) {
        blocker = new Blocker;
    }
    w->installEventFilter(blocker);
    w->viewport()->installEventFilter(blocker);
}

void block(QWidget *w) {
    if(!blocker) {
        blocker = new Blocker;
    }
    w->installEventFilter(blocker);
}

bool fileBelongsTo(const QString &file, const QStringList &list)
{
    int dotIndex = file.lastIndexOf('.');
    if(dotIndex == -1) {
        // no extension
        return false;
    }

    const int fileExtLength = file.length() - dotIndex - 1;
    const auto fileExt = file.right(fileExtLength);

    for(const auto &ext : list) {
        if(fileExt.compare(ext, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}

QStringList getWildcardsFromExtentions(const QStringList& l)
{
    QStringList res;
    for(const auto& ext : l) {
        res << "*." + ext;
    }

    return res;
}

QFileInfoList getDirFiles(const QString &path)
{
    QFileInfoList res;

    static const QStringList filters = getWildcardsFromExtentions(cap::supportedFormats());

    QDirIterator it(path, filters, QDir::Files);
    while(it.hasNext()) {
        it.next();
        res << it.fileInfo();
    }
    return res;
}

QRect screen()
{
    const auto& screens = QGuiApplication::screens();
    if(screens.empty()) {
        return QRect();
    }

    auto& fitstScreen = screens.front();
    if(!fitstScreen) {
        return QRect();
    }

    return fitstScreen->geometry();
}

void centerScrollArea(QScrollArea *area, QLabel* label)
{
    const auto& screens = QGuiApplication::screens();
    if(screens.empty()) {
        return;
    }

    QRect scr { screen() };
    auto pixmap { label->pixmap() };
    int w { (pixmap->width() - scr.width() + tune::screen::reserve)/2 };
    int h { (pixmap->height() - scr.height() + tune::screen::reserve)/2 };

    area->horizontalScrollBar()->setValue(w);
    area->verticalScrollBar()->setValue(h);
}

QString toString(QRgb color)
{
    return QColor{color}.name();
}

void setLabelText(QLabel *label, const QString &text, QRgb color, int fontSize, bool bold)
{
    label->setText(QString("<span style=\" color:%2;\">%1</span>").arg(text, toString(color)));
    if(fontSize == -1 && !bold) {
        return;
    }

    const QFont &font {label->font()};
    if(font.pointSize() != fontSize || font.bold() != bold) {
        QFont newFont {font};
        newFont.setPointSize(fontSize);
        newFont.setBold(bold);
        label->setFont(newFont);
    }
}

QStringList toStringList(const QList<QByteArray> list) {
    QStringList strings;
    foreach (const QByteArray &item, list) {
        strings.append(QString::fromLocal8Bit(item)); // Assuming local 8-bit.
    }
    return strings;
}

} // namespace pork
