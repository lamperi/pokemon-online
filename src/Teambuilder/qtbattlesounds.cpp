#include "qtbattlesounds.h"

#include <QMediaPlayer>
#include <QAudioOutput>
#include "../Utilities/wavreader.h"

QtBattleSounds::QtBattleSounds(QObject* parent) : BattleSounds(parent) {
    audio = new QMediaPlayer(this);
    cry = new QAudioOutput(QAudioFormat(), this);

    connect(audio, &QMediaPlayer::stateChanged,
            this, &QtBattleSounds::enqueueMusic);
    connect(cry, &QAudioOutput::stateChanged,
            this, &QtBattleSounds::cryStateChanged);

}

void QtBattleSounds::playMusic() {
    audio->play();
}

int QtBattleSounds::cryVolume() {
    return cry->volume()*100;
}

int QtBattleSounds::musicVolume() {
    return audio->volume();
}

void QtBattleSounds::setCryVolume(int vol) {
    cry->setVolume(float(vol)/100);
}

void QtBattleSounds::setMusicVolume(int vol) {
    audio->setVolume(vol);
}

void QtBattleSounds::playCry(QBuffer &buffer) {
    qDebug() << "deleting old";
    const float volume = cry->volume();
    cry->deleteLater();

    qDebug() << "constructing new";
    cry = new QAudioOutput(readWavHeader(&buffer), this);
    cry->setBufferSize(buffer.size());
    cry->setVolume(volume);
    connect(cry, &QAudioOutput::stateChanged,
            this, &QtBattleSounds::cryStateChanged);
    qDebug() << "starting to play";
    cry->start(&buffer);
}

void QtBattleSounds::stopCry() {
    cry->stop();
}

void QtBattleSounds::cryStateChanged(QAudio::State state) {
    qDebug() << "crystatechanged" << state;
    if (state != QAudio::ActiveState && isUndelayOnSounds()) {
        emit soundsDone();
    }
}

void QtBattleSounds::playMusicFrom(const QString &url) {
    audio->setMedia(QUrl::fromLocalFile(url));
    audio->play();

}

void QtBattleSounds::enqueueFrom(const QString &url) {
    if (audio->state() != QMediaPlayer::PlayingState && isSoundsOn()) {
        audio->setMedia(QUrl::fromLocalFile(url));
        audio->play();
    }
}

void QtBattleSounds::pauseMusic() {
    audio->pause();
}
