#ifndef CONFIG_H
#define CONFIG_H

namespace pork {

//! Capabilities
namespace cap
{
    const QStringList supportedImages { "*.jpg", "*.jpeg", "*.png", "*.bmp" };
    const QStringList supportedGif { "*.gif" };
    const QStringList supportedVideo { "*.webm", "*.wmv", "*.mp4", "*.mpg" };
    inline const QStringList supportedFormats() {
        return QStringList() << supportedImages << supportedGif << supportedVideo;
    }
}

//! Tuning
namespace tune
{
    namespace screen
    {
        constexpr int reserve {2};              //! image padding from screen border to prevent scroll bars appearance
        constexpr qreal backwardSection {1/5.}; //! x of screen area where mouse click is treated as a previous file request
        constexpr qreal forwardSection {4/5.};  //! x of screen area where mouse click is treated as a next file request
    }

    namespace slider
    {
        constexpr int pad {20};        //! padding from screen border
        constexpr int showTime {2000}; //! time when sliders are shown on a screen after user's last ineraction. in ms
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
}

} // namespace pork

#endif // CONFIG_H