#include "videoplayer.h"
#include "config.h"

#include <QFrame>
#include <QDir>
#include <QHBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QDebug>

#include <vlc/libvlc.h>
#include <vlc/libvlc_events.h>
#include <vlc/libvlc_media_player.h>
#include <vlc_variables.h>

namespace aske
{

VideoView::VideoView(QWidget *parent)
    : QWidget(parent)
{
    QPalette plt = palette();
    plt.setColor(QPalette::Window, Qt::black);
    setPalette(plt);
}

WId VideoView::window()
{
    return winId();
}

int QSliderStyle::styleHint(QStyle::StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    if (hint == QStyle::SH_Slider_AbsoluteSetButtons) {
        return (Qt::LeftButton | Qt::MidButton | Qt::RightButton);
    }
    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

static const QList<libvlc_event_e> playerEvents = {
    libvlc_MediaPlayerNothingSpecial,
    libvlc_MediaPlayerOpening,
    libvlc_MediaPlayerBuffering,
    libvlc_MediaPlayerPlaying,
    libvlc_MediaPlayerPaused,
    libvlc_MediaPlayerStopped,
    libvlc_MediaPlayerForward,
    libvlc_MediaPlayerBackward,
    libvlc_MediaPlayerEndReached,
    libvlc_MediaPlayerEncounteredError,
    libvlc_MediaPlayerPositionChanged,
    libvlc_MediaPlayerVout
};

VideoPlayer::VideoPlayer()
{
    m_vlc = libvlc_new(0, 0);
    m_vlcMediaPlayer = libvlc_media_player_new(m_vlc);
    m_vlcEvents = libvlc_media_player_event_manager(m_vlcMediaPlayer);

    // Disable mouse and keyboard events
    libvlc_video_set_key_input(m_vlcMediaPlayer, false);
    libvlc_video_set_mouse_input(m_vlcMediaPlayer, false);

    var_AddCallback((vlc_object_t *)m_vlcMediaPlayer, "volume", volumeCallback, this);
    var_AddCallback((vlc_object_t *)m_vlcMediaPlayer, "mute", muteCallback, this);

    for(const auto &event : playerEvents) {
        libvlc_event_attach(m_vlcEvents, event, libvlc_callback, this);
    }
}

VideoPlayer::~VideoPlayer()
{
    var_DelCallback((vlc_object_t *)m_vlcMediaPlayer, "volume", volumeCallback, this);
    var_DelCallback((vlc_object_t *)m_vlcMediaPlayer, "mute", muteCallback, this);

    for(const auto &event : playerEvents) {
        libvlc_event_detach(m_vlcEvents, event, libvlc_callback, this);
    }
}

#define UNUSED(...) (void)__VA_ARGS__

int VideoPlayer::volumeCallback(vlc_object_t *obj, const char *name, vlc_value_t oldVal, vlc_value_t val, void *data)
{
    UNUSED(obj, name, oldVal);

    VideoPlayer *core = static_cast<VideoPlayer *>(data);
    emit core->volumeChanged(val.f_float);
    return VLC_SUCCESS;
}

int VideoPlayer::muteCallback(vlc_object_t *obj, const char *name, vlc_value_t oldVal, vlc_value_t val, void *data)
{
    UNUSED(obj, name, oldVal);

    VideoPlayer *core = static_cast<VideoPlayer *>(data);
    emit core->muteChanged(val.b_bool);
    return VLC_SUCCESS;
}

void VideoPlayer::setWidgets(VideoView *view, QSlider *progress, QSlider *volume, QLabel *codecErrorLabel)
{
    m_view = view;
    m_progressSlider = progress;
    m_volumeSlider = volume;
    m_codecErrorLabel = codecErrorLabel;

    // make sliders well-responsible
    m_volumeSlider->setStyle(new QSliderStyle(m_volumeSlider->style()));
    m_progressSlider->setStyle(new QSliderStyle(m_progressSlider->style()));

    // video volume change
    m_volumeSlider->setRange(0, 100);
    connect(m_volumeSlider, &QSlider::valueChanged, [this](int volume) {
        setVolume(volume);
        showSliders();
    });

    connect(m_progressSlider, &QSlider::sliderPressed, [this]() {
        m_userChangedVideoPos = true;
        pause();
    });
    connect(m_progressSlider, &QSlider::sliderReleased, [this]() {
        m_userChangedVideoPos = true;
        setPosition(m_progressSlider->value()/100.);
        resume();
    });

    // sliders auto-hide
    m_slidersTimer.setSingleShot(true);
    connect(&m_slidersTimer, &QTimer::timeout, m_progressSlider, &QSlider::hide);
    connect(&m_slidersTimer, &QTimer::timeout, m_volumeSlider, &QSlider::hide);
}

libvlc_state_t VideoPlayer::state() const
{
    if (!libvlc_media_player_get_media(m_vlcMediaPlayer)) {
        return libvlc_NothingSpecial;
    }

    return libvlc_media_player_get_state(m_vlcMediaPlayer);
}

int VideoPlayer::volume() const
{
    int volume = -1;
    if (m_vlcMediaPlayer) {
        volume = libvlc_audio_get_volume(m_vlcMediaPlayer);
    }

    return volume;
}

void VideoPlayer::setVolume(int volume)
{
    if (m_vlcMediaPlayer) {
        if (volume != VideoPlayer::volume()) {
            libvlc_audio_set_volume(m_vlcMediaPlayer, volume);
        }
    }
}

qreal VideoPlayer::position() const
{
    if (!m_vlcMediaPlayer) {
        return -1.0;
    }

    return libvlc_media_player_get_position(m_vlcMediaPlayer);
}

void VideoPlayer::setPosition(qreal pos)
{
    libvlc_media_player_set_position(m_vlcMediaPlayer, pos);
}

bool VideoPlayer::load(const QString &file)
{
    m_currentFile = file;

    reload();

    return true;
}

bool VideoPlayer::reload()
{
    if(m_vlcMedia) {
        libvlc_media_release(m_vlcMedia);
    }
    QString l = QDir::toNativeSeparators(m_currentFile);
    m_vlcMedia = libvlc_media_new_path(m_vlc, l.toUtf8().data());
    open();
    play();

    showSliders();

    return true;
}

void VideoPlayer::open()
{
    libvlc_media_player_set_media(m_vlcMediaPlayer, m_vlcMedia);
}

void VideoPlayer::play()
{
    if (!m_vlcMediaPlayer) {
        return;
    }

    if (m_view) {
        m_currentWId = m_view->window();
    } else {
        m_currentWId = 0;
    }

    if (m_currentWId) {
        libvlc_media_player_set_hwnd(m_vlcMediaPlayer, (void *)m_currentWId);
    }

    libvlc_media_player_play(m_vlcMediaPlayer);
}

void VideoPlayer::rewind(Direction dir, qreal step)
{
    m_userChangedVideoPos = true;

    if(dir == Direction::Backward) {
        step *= -1;
    }

    setPosition(position() + step);
}

void VideoPlayer::showSliders()
{
    m_progressSlider->show();
    m_volumeSlider->show();

    m_slidersTimer.start(2000);
}

void VideoPlayer::pause()
{
    if (!m_vlcMediaPlayer) {
        return;
    }

    if (libvlc_media_player_can_pause(m_vlcMediaPlayer)) {
        libvlc_media_player_pause(m_vlcMediaPlayer);
    }
}

void VideoPlayer::resume()
{
    auto s = state();
    if(s == libvlc_Paused) {
        if (!m_vlcMediaPlayer) {
            return;
        }

        if (libvlc_media_player_can_pause(m_vlcMediaPlayer)) {
            libvlc_media_player_set_pause(m_vlcMediaPlayer, false);
        }
    } else if(s == libvlc_Ended) {
        setPosition(0.0);
        play();
    }
}

void VideoPlayer::toggle()
{
    auto s = state();
    if(s == libvlc_Paused || s == libvlc_Playing) {
        pause();
    } else if(s == libvlc_Ended) {
        reload();
    }
}

void VideoPlayer::stop()
{
    if (!m_vlcMediaPlayer) {
        return;
    }

    m_currentWId = 0;

    libvlc_media_player_stop(m_vlcMediaPlayer);
}

const QSize VideoPlayer::size() const
{
    unsigned x = 640;
    unsigned y = 480;

    if (m_vlcMediaPlayer && libvlc_media_player_has_vout(m_vlcMediaPlayer)) {
        libvlc_video_get_size(m_vlcMediaPlayer, 0, &x, &y);
    }

    return QSize(x, y);
}

void VideoPlayer::libvlc_callback(const libvlc_event_t *event, void *data)
{
    VideoPlayer *core = static_cast<VideoPlayer *>(data);
    if(!core) {
        return;
    }

    VideoPlayer &c = *core;

    switch (event->type) {
    case libvlc_MediaPlayerPositionChanged:
        // first video load resulted in 100 volume anyway. this is a workaround.
        if(c.m_firstLoad) {
            c.m_firstLoad = false;
            c.setVolume(0);
        }

        if(c.m_userChangedVideoPos) {
            c.showSliders();
            c.m_userChangedVideoPos = false;
        } else {
            float position = event->u.media_player_position_changed.new_position;
            c.m_progressSlider->setValue(position*100);
        }
        break;

    case libvlc_MediaPlayerVout:
        emit c.loaded();
        break;

    default:
        break;
    }

    if (event->type >= libvlc_MediaPlayerNothingSpecial
        && event->type <= libvlc_MediaPlayerEncounteredError) {
        auto s = c.state();
        // unknown codec case
        if(s == libvlc_Error) {
            c.m_codecErrorLabel->show();
            c.m_volumeSlider->hide();
            c.m_progressSlider->hide();
        }
    }
}

} // namespace aske
