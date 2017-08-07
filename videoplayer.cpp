#include "videoplayer.h"
#include "config.h"

#include <QSlider>
#include <QLabel>
#include <QDebug>

#include <VLCQtCore/Common.h>
#include <VLCQtCore/Media.h>
#include <VLCQtCore/Video.h>
#include <VLCQtCore/Audio.h>
#include <VLCQtWidgets/WidgetVideo.h>

namespace pork
{

VideoPlayer::VideoPlayer(QWidget *parent)
    : m_vlc(VlcCommon::args(), parent)
    , m_player(&m_vlc)
{}

void VideoPlayer::setWidgets(VlcWidgetVideo *view, QSlider *progress, QSlider *volume, QLabel *codecErrorLabel)
{
    m_view = view;
    m_progressSlider = progress;
    m_volumeSlider = volume;
    m_codecErrorLabel = codecErrorLabel;

    // basic setup
    m_player.setVideoWidget(view);

    // make sliders well-responsible
    m_volumeSlider->setStyle(new QSliderStyle(m_volumeSlider->style()));
    m_progressSlider->setStyle(new QSliderStyle(m_progressSlider->style()));


    // video volume change
    m_volumeSlider->setRange(tune::volume::min, tune::volume::max);
    QObject::connect(m_volumeSlider, &QSlider::valueChanged, [this](int volume) {
        if(m_audio) {
            m_audio->setVolume(volume);
        }
        showSliders();
    });

    // video position change
    QObject::connect(&m_player, &VlcMediaPlayer::positionChanged, [this](float position) {
        if(m_userChangedVideoPos) {
            showSliders();
            m_userChangedVideoPos = false;
        } else {
            m_progressSlider->setValue(position*100);
        }
    });
    QObject::connect(m_progressSlider, &QSlider::sliderPressed, [this]() {
        m_userChangedVideoPos = true;
        m_player.pause();
    });
    QObject::connect(m_progressSlider, &QSlider::sliderReleased, [this]() {
        m_userChangedVideoPos = true;
        m_player.setPosition(m_progressSlider->value()/100.);
        resume();
    });

    QObject::connect(&m_player, &VlcMediaPlayer::stateChanged, [this]() {
        auto state = m_player.state();
        qDebug() << state;
        // unknown codec case
        if(state == Vlc::Error) {
            m_codecErrorLabel->show();
            m_volumeSlider->hide();
            m_progressSlider->hide();
        } else if (state == Vlc::Playing){
            qDebug() << "PLAYING!!";
        }
    });

    // sliders auto-hide
    m_slidersTimer.setSingleShot(true);
    QObject::connect(&m_slidersTimer, &QTimer::timeout, m_progressSlider, &QSlider::hide);
    QObject::connect(&m_slidersTimer, &QTimer::timeout, m_volumeSlider, &QSlider::hide);
}

bool VideoPlayer::load(const QString &file)
{
    m_currentFile = file;

    reload();
    if(m_audio) {
        qDebug() << "try change to 0";
        m_audio->setVolume(0);
    }

    return true;
}

bool VideoPlayer::reload()
{
    if(m_media) {
        delete m_media;
    }
    m_media = new VlcMedia(m_currentFile, true, &m_vlc);
    m_player.open(m_media);
    m_player.play();
    m_audio = m_player.audio();

    /*QObject::connect(m_audio, &VlcAudio::volumeChanged, [this](int v){
        qDebug() << "VOLUME" << v;
        if(v == 100) {
            m_audio->setVolume(0);
        }
    });*/

    showSliders();

    return true;
}

void VideoPlayer::rewind(Direction dir)
{
    m_userChangedVideoPos = true;

    qreal step { tune::video::rewind };
    if(dir == Direction::Backward) {
        step *= -1;
    }

    m_player.setPosition(m_player.position() + step);
}

void VideoPlayer::showSliders()
{
    m_progressSlider->show();
    m_volumeSlider->show();

    m_slidersTimer.start(tune::slider::showTime);
}

void VideoPlayer::resume()
{
    auto state = m_player.state();
    if(state == Vlc::Paused) {
        m_player.resume();
    } else if(state == Vlc::Ended) {
        m_player.setPosition(0);
        m_player.play();
    }
}

void VideoPlayer::toggle()
{
    auto state = m_player.state();
    if(state == Vlc::Paused || state == Vlc::Playing) {
        m_player.togglePause();
    } else if(state == Vlc::Ended) {
        reload();
    }
}

const QSizeF VideoPlayer::size()
{
    return m_player.video()->size();
}

} // namespace pork
