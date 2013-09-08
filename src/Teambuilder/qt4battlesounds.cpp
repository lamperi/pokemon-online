#include "qt4battlesounds.h"

Qt4BattleSounds::Qt4BattleSounds(QObject* parent) : BattleSounds(parent) {
    audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    mediaObject = new Phonon::MediaObject(this);

    cryOutput = new Phonon::AudioOutput(Phonon::GameCategory, this);
    cryObject = new Phonon::MediaObject(this);

    /* To link both */
    Phonon::createPath(mediaObject, audioOutput);
    Phonon::createPath(cryObject, cryOutput);

    connect(mediaObject, SIGNAL(aboutToFinish()), this, SLOT(enqueueMusic()));
    connect(cryObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(criesProblem(Phonon::State)));
}

void Qt4BattleSounds::playMusic() {
    mediaObject->play();
}

int Qt4BattleSounds::cryVolume() {
    return cryOutput->volume()*100;
}

int Qt4BattleSounds::musicVolume() {
    return audioOutput->volume()*100;
}

void Qt4BattleSounds::setCryVolume(int vol) {
    cryOutput->setVolume(float(vol)/100);
}

void Qt4BattleSounds::setMusicVolume(int vol) {
    audioOutput->setVolume(float(vol)/100);
}

void Qt4BattleSounds::playCry(QBuffer &buffer) {
    cryObject->setCurrentSource(&buffer);
    cryObject->play();
}

void Qt4BattleSounds::stopCry() {
    cryObject->stop();
}

void Qt4BattleSounds::cryStateChanged(Phonon::State state) {
    if (state != Phonon::PlayingState && state != Phonon::LoadingState && isUndelayOnSounds()) {
        emit soundsDone();
    }
}

void Qt4BattleSounds::playMusicFrom(const QString &url) {
    mediaObject->setCurrentSource(url);
    mediaObject->play();
}

void Qt4BattleSounds::enqueueFrom(const QString& url) {
    mediaObject->enqueue(url);
}

void Qt4BattleSounds::pauseMusic() {
    mediaObject->pause();
}
