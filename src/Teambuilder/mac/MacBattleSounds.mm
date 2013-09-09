#include "MacBattleSounds.h"
#include "SoundDelegate.h"

#import <AppKit/NSSound.h>
#import <Foundation/NSData.h>

#include <QtConcurrentRun>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

class MacBattleSoundsPrivate {
    friend class MacBattleSounds;
    float cryVolume;
    float musicVolume;
    NSSound *sound;
    int currentId = 0;
    int receivedId = 0;
};

MacBattleSounds::MacBattleSounds(QObject *parent) : BattleSounds(parent) {
    qDebug() << "MacBattleSounds constructor";
    d = new MacBattleSoundsPrivate;
    d->sound = [[NSSound alloc] init];
}

MacBattleSounds::~MacBattleSounds() {
    [d->sound release];
    delete d;
}

void MacBattleSounds::playMusic() {

}

int MacBattleSounds::cryVolume() {
    return d->cryVolume * 100;
}

int MacBattleSounds::musicVolume() {
    return d->musicVolume * 100;
}

void MacBattleSounds::setCryVolume(int vol) {
    d->cryVolume = float(vol) / 100;
}

void MacBattleSounds::setMusicVolume(int vol) {
    d->musicVolume = float(vol) / 100;
}

void MacBattleSounds::playCry(QBuffer &bufferData) {
    const QByteArray &data = bufferData.data();
    NSData* soundData = [NSData dataWithBytes:data.constData() length:data.size()];
    [d->sound stop];
    [d->sound release];
    d->sound = [[NSSound alloc] initWithData: soundData];

    const int currentId =  ++d->currentId;
    SoundDelegate *delegate = [[SoundDelegate alloc]
                initWithCallback: this
                WithId: currentId];
    [d->sound setDelegate: delegate];
    [d->sound setVolume: d->cryVolume];
    [d->sound play];
    qDebug() << "MacBattleSounds::playCry";

    QtConcurrent::run([=]() {
        QMutex dummy;
        dummy.lock();
        QWaitCondition waitCondition;
        waitCondition.wait(&dummy, 4000);
        playingFinished(currentId);
    });
}

void MacBattleSounds::pauseMusic() {

}

void MacBattleSounds::stopCry() {
    [d->sound stop];
}

void MacBattleSounds::enqueueFrom(const QString &url) {

}

void MacBattleSounds::playMusicFrom(const QString &url) {

}

void MacBattleSounds::playingFinished(int id) {
    qDebug() << "MacBattleSounds::playingFinished";
    if (id > d->receivedId) {
        d->receivedId = id;
        emit soundsDone();
    }
}


