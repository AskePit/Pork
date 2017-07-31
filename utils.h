#ifndef UTILS_H
#define UTILS_H

#include <QWidget>
#include <QProxyStyle>

class QAbstractScrollArea;

//! This makes QSliders set their position strictly to the pointed position instead of stepping
class QSliderStyle : public QProxyStyle
{
public:
    using QProxyStyle::QProxyStyle;
    virtual int styleHint(QStyle::StyleHint hint, const QStyleOption *option = 0, const QWidget *widget = 0, QStyleHintReturn *returnData = 0) const override;
};

//! Blocks undesired events for a widget
class Blocker : public QWidget
{
public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
};

void block(QAbstractScrollArea *w);

bool fileBelongsTo(const QString &file, const QStringList &list);

#endif // UTILS_H
