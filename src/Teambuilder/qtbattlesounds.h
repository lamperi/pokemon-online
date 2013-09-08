#ifndef QTBATTLESOUNDS_H
#define QTBATTLESOUNDS_H

#include "battlesounds.h"
#include <QAudio>

class QMediaPlayer;
class QAudioOutput;

class QtBattleSounds : public BattleSounds {
public:
    QtBattleSounds(QObject*);

    int cryVolume();
    int musicVolume();

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
    void cryStateChanged(QAudio::State);

private:
    QMediaPlayer *audio;
    QAudioOutput *cry;
};

#endif 
