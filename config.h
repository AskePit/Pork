#ifndef CONFIG_H
#define CONFIG_H

#include "utils.h"
#include <QRgb>
#include <QImageReader>
#include <QMovie>

namespace pork {

//! Capabilities
namespace cap
{
    inline const QStringList& supportedImages() {
        static const QStringList images { toStringList(QImageReader::supportedImageFormats()) };
        return images;
    };

    inline const QStringList& supportedGif() {
        static const QStringList gifs { toStringList(QMovie::supportedFormats()) };
        return gifs;
    };

    inline const QStringList& supportedVideo() {
        static const QStringList videos {
            "3g2",
            "3gp",
            "a52",
            "aac",
            "asf",
            "amv",
            "au",
            "avi",
            "drc",
            "dts",
            "dv"
            "dvr-ms",
            "f4a",
            "f4b",
            "f4p",
            "f4v",
            "flv",
            "gifv",
            "m2ts",
            "m2v",
            "m4p",
            "m4v",
            "mka",
            "mkv",
            "mng",
            "mov",
            "mp4",
            "mpe",
            "mpeg",
            "mpg",
            "mpv",
            "mts",
            "mxf",
            "nsc",
            "nsv",
            "nut",
            "ogg",
            "ogm",
            "ogv",
            "qt",
            "ra",
            "ram",
            "rm",
            "rmbv",
            "roq",
            "rv",
            "svi",
            "tac",
            "ts",
            "tta",
            "tsv",
            "ty",
            "vid",
            "viv",
            "vob",
            "webm",
            "wmv",
            "xa",
            "yuv",
        };

        return videos;
    };

    inline const QStringList& supportedFormats() {
        static const QStringList formats = QStringList() << supportedImages() << supportedGif() << supportedVideo();
        return formats;
    }
}

//! Tuning
namespace tune
{
    namespace reg
    {
        static const QString dragWindowGeometry {"dragWindowGeometry"};
    }

    namespace screen
    {
        constexpr int reserve {2};              //! image padding from screen border to prevent scroll bars appearance
        constexpr qreal backwardSection {1/6.}; //! x of screen area where mouse click is treated as a previous file request
        constexpr qreal forwardSection {5/6.};  //! x of screen area where mouse click is treated as a next file request
    }

    namespace slider
    {
        constexpr int pad {20};        //! padding from screen border
        constexpr int showTime {2000}; //! time when sliders are shown on a screen after user's last ineraction. in ms
        constexpr int range {1000};    //! slider `maxValue - minValue` range
    }

    namespace zoom
    {
        constexpr qreal origin {1.0};   //! original zoom (zero zoom)

        constexpr qreal factors[2][2] { //! zoom factors [Direction::type][ZoomType::type]
            //Button, Wheel
            {-0.07,  -0.15}, // Backward
            { 0.07,   0.15}, // Forward
        };

        constexpr qreal min {0.0};
        constexpr qreal max {4.0};

        constexpr int delay {5};        //! time between two actual zoom performances. in ms
    }

    namespace video
    {
        constexpr int bufferingTime {400}; //! aproximate time to buffer video
        constexpr qreal rewind {0.05};  //! rewind speed
    }

    namespace volume
    {
        constexpr int min {0};
        constexpr int max {100};

        constexpr qreal factors[2][2] { //! zoom factors [Direction::type][ZoomType::type]
            //Button, Wheel
            {-4,  -4}, // Backward
            { 4,   4}, // Forward
        };
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
