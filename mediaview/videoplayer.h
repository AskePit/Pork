#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QWidget>
#include <QTimer>
#include <vlc/libvlc.h>
#include <vlc_common.h>
#include <vlc/libvlc_media.h>

struct libvlc_instance_t;
struct libvlc_media_player_t;
struct libvlc_event_manager_t;
struct libvlc_event_t;
struct vlc_object_t;

class QSlider;
class QLabel;

namespace aske
{

class VideoView : public QWidget
{
    Q_OBJECT
public:
    explicit VideoView(QWidget *parent = 0);
    WId window();
};

class VideoPlayer : public QObject
{
    Q_OBJECT

public:
    enum Direction
    {
        Backward = 0,
        Forward,
    };

    VideoPlayer();
    ~VideoPlayer();
    void setWidgets(VideoView *view, QSlider *progress, QSlider *volume, QLabel *codecErrorLabel);

    bool load(const QString &file);
    void play();
    void replay();
    void rewind(Direction dir, qreal step);
    void pause();
    void resume();
    void toggle();
    void stop();

    libvlc_state_t state() const;

    int volume() const;
    void setVolume(int volume);

    qreal position() const;
    void setPosition(qreal pos);

    const QSize size() const;

    void showSliders();

signals:
    void loaded();
    void volumeChanged(qreal);
    void muteChanged(bool);

private:
    libvlc_instance_t *m_vlc {nullptr};
    libvlc_media_player_t *m_vlcMediaPlayer {nullptr};
    libvlc_media_t *m_vlcMedia {nullptr};
    libvlc_event_manager_t *m_vlcEvents {nullptr};

    static void libvlc_callback(const libvlc_event_t *event, void *data);

    VideoView *m_view;

    QSlider *m_progressSlider {nullptr};
    QSlider *m_volumeSlider {nullptr};
    QLabel *m_codecErrorLabel {nullptr};

    QString m_currentFile;
    QTimer m_slidersTimer;
    bool m_userChangedVideoPos {false};
    bool m_firstLoad {true};

    static int volumeCallback(vlc_object_t *, const char *, vlc_value_t, vlc_value_t, void *);
    static int muteCallback(vlc_object_t *, const char *, vlc_value_t, vlc_value_t, void *);
};

} // namespace aske

#endif // VIDEOPLAYER_H
