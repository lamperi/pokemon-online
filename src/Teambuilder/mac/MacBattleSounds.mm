#include "MacBattleSounds.h"
#include "SoundDelegate.h"

#import <AppKit/NSSound.h>
#import <Foundation/NSData.h>
#import <Foundation/NSUrl.h>

#include <QMediaPlayer>
#include <QSettings>

class MacBattleSoundsPrivate {
    friend class MacBattleSounds;
    MacBattleSoundsPrivate() : cryVolume(0.0f),
        sound(0), music(0), currentId(0), receivedId(0),
        useDelegate(true) {}
    float cryVolume;
    NSSound *sound;
    QMediaPlayer *music;
    int currentId;
    int receivedId;
    bool useDelegate;
};

inline void releaseSounds(NSSound *sound) {
    [[sound delegate] removeCallback];
    [sound stop];
    [sound release];
}


enum SoundType{
    cry,
    music
};

MacBattleSounds::MacBattleSounds(QObject *parent) : BattleSounds(parent) {
    qDebug() << "MacBattleSounds constructor";
    d = new MacBattleSoundsPrivate;
    d->sound = [[NSSound alloc] init];
    d->music = new QMediaPlayer();
    connect(d->music, &QMediaPlayer::stateChanged,
            this, &MacBattleSounds::enqueueMusic);

    // Not a perfect solution but ...
    QSettings s;
    d->useDelegate = s.value("Battle/OldWindow", true).toBool();
}

MacBattleSounds::~MacBattleSounds() {
    qDebug() << "destructor";
    releaseSounds(d->sound);
    d->music->blockSignals(true);
    d->music->stop();
    d->music->deleteLater();
    delete d;
}

void MacBattleSounds::playMusic() {
    qDebug() << QDateTime::currentDateTime().toString("yyyy-mm-dd hh:mm:ss") << Q_FUNC_INFO;
    d->music->play();
}

int MacBattleSounds::cryVolume() {
    return d->cryVolume * 100;
}

int MacBattleSounds::musicVolume() {
    return d->music->volume();
}

void MacBattleSounds::setCryVolume(int vol) {
    d->cryVolume = float(vol) / 100;
}

void MacBattleSounds::setMusicVolume(int vol) {
    d->music->setVolume(vol);
}

void MacBattleSounds::playCry(QBuffer &bufferData) {
    qDebug() << QDateTime::currentDateTime().toString("yyyy-mm-dd hh:mm:ss") << Q_FUNC_INFO;
    const QByteArray &data = bufferData.data();
    NSData* soundData = [NSData dataWithBytes:data.constData() length:data.size()];
    releaseSounds(d->sound);

    d->sound = [[NSSound alloc] initWithData: soundData];

    const int currentId =  ++d->currentId;

    [d->sound setVolume: d->cryVolume];
    [d->sound play];

    if (d->useDelegate) {
        SoundDelegate *delegate = [[SoundDelegate alloc]
                    initWithCallback: this
                    WithId: currentId
                    WithType: SoundType::cry];
        [d->sound setDelegate: delegate];
    } else {

        // For some reason delay() on basebattlewindow will make sound never
        // call for delegate's method. That is why we lie a bit here and claim
        // we have finished. Maybe the animation will keep the sounds from playing at the
        // same time...
        emit soundsDone();
    }
}

void MacBattleSounds::pauseMusic() {
    d->music->pause();
}

void MacBattleSounds::stopCry() {
    [d->sound stop];
}

void MacBattleSounds::enqueueFrom(const QString &url) {
    qDebug() << QDateTime::currentDateTime().toString("yyyy-mm-dd hh:mm:ss") << Q_FUNC_INFO;
    if (d->music->state() != QMediaPlayer::PlayingState && isSoundsOn()) {
        d->music->setMedia(QUrl::fromLocalFile(url));
        d->music->play();
    }
}

void MacBattleSounds::playMusicFrom(const QString &url) {
    qDebug() << QDateTime::currentDateTime().toString("yyyy-mm-dd hh:mm:ss") << Q_FUNC_INFO;
    d->music->setMedia(QUrl::fromLocalFile(url));
    d->music->play();
}

void MacBattleSounds::playingFinished(int id, int type) {
    qDebug() << QDateTime::currentDateTime().toString("yyyy-mm-dd hh:mm:ss") << Q_FUNC_INFO;
    if (id > d->receivedId && type == SoundType::cry) {
        d->receivedId = id;
        emit soundsDone();
    }
}


