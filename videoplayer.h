#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include "utils.h"

#include <VLCQtCore/Instance.h>
#include <VLCQtCore/MediaPlayer.h>

class VlcWidgetVideo;
class QSlider;
class QLabel;

namespace pork
{

class VideoPlayer : public QObject
{
    Q_OBJECT

public:
    VideoPlayer(QWidget *parent = 0);
    void setWidgets(VlcWidgetVideo *view, QSlider *progress, QSlider *volume, QLabel *codecErrorLabel);

    bool load(const QString &file);
    bool reload();

    void rewind(Direction dir);
    void resume();
    void toggle();
    void showSliders();

    void stop() { m_player.stop(); }

    const QSizeF videoSize();

signals:
    void loaded();

private:
    VlcInstance m_vlc;
    VlcMediaPlayer m_player;
    VlcWidgetVideo *m_view {nullptr};
    QSlider *m_progressSlider {nullptr};
    QSlider *m_volumeSlider {nullptr};
    QLabel *m_codecErrorLabel {nullptr};

    VlcMedia *m_media {nullptr};
    VlcAudio *m_audio {nullptr};

    QString m_currentFile;
    QTimer m_slidersTimer;
    bool m_userChangedVideoPos {false};
    bool m_firstLoad {true};
};

} // namespace pork

#endif // VIDEOPLAYER_H
