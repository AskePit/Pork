#include "utils.h"

#include <QAbstractScrollArea>
#include <QEvent>

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

bool fileBelongsTo(const QString &file, const QStringList &list)
{
    for(const auto &ext : list) {
        QRegExp reg {ext, Qt::CaseInsensitive, QRegExp::Wildcard};
        if(reg.exactMatch(file)) {
            return true;
        }
    }
    return false;
}
