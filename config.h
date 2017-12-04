#ifndef CONFIG_H
#define CONFIG_H

#include <QRgb>

namespace pork {

//! Tuning
namespace tune
{
    namespace reg
    {
        static const QString dragWindowGeometry {"dragWindowGeometry"};
    }

    namespace info
    {
        constexpr QRgb dragLabelColor {0x676767};

        namespace fileName
        {
            constexpr int pad {25};        //! padding from screen border
            constexpr QRgb darkColor  {0x000000};
            constexpr QRgb lightColor {0xDDDDDD};
            constexpr int fontSize {10};
            constexpr int showTime {2000}; //! time when filename label is shown on a screen after file open action. in ms
        }
    }
}

} // namespace pork

#endif // CONFIG_H
