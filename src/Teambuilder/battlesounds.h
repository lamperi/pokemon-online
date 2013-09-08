#ifndef BATTLESOUNDS_H
#define BATTLESOUNDS_H

#include <QObject>
#include "../Utilities/functions.h"

class BattleSounds : public QObject {
    Q_OBJECT

    PROPERTY(bool, playBattleCries)
    PROPERTY(bool, playBattleMusic)

public:
    BattleSounds(QObject*);

	virtual int cryVolume() = 0;

	virtual int musicVolume() = 0;

signals:
    void soundsPlaying();
    void soundsDone();

public slots:
    virtual void toggleSounds(bool);

    virtual void playCry(int);

    virtual void setCryVolume(int) = 0;

    virtual void setMusicVolume(int) = 0;

protected slots:
    void enqueueMusic();

protected:
    virtual void playMusicFrom(const QString &) = 0;
    virtual void enqueueFrom(const QString &) = 0;
    virtual void playMusic() = 0;
    virtual void pauseMusic() = 0;
    virtual void playCry(QBuffer&) = 0;
    virtual void stopCry() = 0;

    bool isUndelayOnSounds() { return undelayOnSounds; }
    bool isSoundsOn() { return soundsOn; }
private:
    /* The media sources for the music */
    QList<QString> sources;

    /* The pokemon cries stored in memory */
    QHash<int, QByteArray> cries;
    QBuffer cryBuffer;
    bool undelayOnSounds;
    bool soundsOn;
};

#endif
