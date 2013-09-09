#ifndef MACBATTLESOUNDS_H
#define MACBATTLESOUNDS_H

#include "MacCrySupport.h"
#include "../battlesounds.h"

class MacBattleSoundsPrivate;

class MacBattleSounds : public BattleSounds, public MacCrySupport {
public:
    MacBattleSounds(QObject*);
    ~MacBattleSounds();

    int cryVolume();
    int musicVolume();

    void playingFinished(int);

public slots:
    void setCryVolume(int);
    void setMusicVolume(int);

protected:
    void playMusicFrom(const QString &);
    void enqueueFrom(const QString &);
    void playMusic();
    void pauseMusic();
    void playCry(QBuffer &);
    void stopCry();

private slots:
    void cryDone();

private:
    MacBattleSoundsPrivate *d;
};

#endif
