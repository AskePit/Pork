#ifndef UTILS_H
#define UTILS_H

#include <QWidget>
#include <QProxyStyle>
#include <QFileInfoList>

class QAbstractScrollArea;
class QScrollArea;
class QLabel;

namespace pork {

//! Blocks undesired events for a widget
class Blocker : public QWidget
{
public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
};

void block(QAbstractScrollArea *w);
void block(QWidget *w);

bool fileBelongsTo(const QString &file, const QStringList &list);
QFileInfoList getDirFiles(const QString &path);
QRect screen();
void centerScrollArea(QScrollArea *area, QLabel* label);

inline QString toString(QRgb color);
void setLabelText(QLabel *label, const QString &text, QRgb color, int fontSize = -1, bool bold = false);

} // namespace pork

#endif // UTILS_H
