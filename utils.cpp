#include "utils.h"
#include "config.h"

#include <QEvent>
#include <QLabel>

namespace pork {

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

} // namespace pork
