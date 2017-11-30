#ifndef UTILS_H
#define UTILS_H

#include <QWidget>
#include <QProxyStyle>
#include <QFileInfoList>

class QAbstractScrollArea;
class QScrollArea;
class QLabel;

namespace pork {

bool fileBelongsTo(const QString &file, const QStringList &list);
QString toString(QRgb color);
void setLabelText(QLabel *label, const QString &text, QRgb color, int fontSize = -1, bool bold = false);

} // namespace pork

#endif // UTILS_H
