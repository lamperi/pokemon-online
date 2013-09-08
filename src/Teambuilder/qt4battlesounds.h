#ifndef QT4BATTLESOUNDS_H
#define QT4BATTLESOUNDS_H

#include "battlesounds.h"

#include <phonon/mediaobject.h>
#include <phonon/audiooutput.h>

class Qt4BattleSounds : public BattleSounds {
public:
    Qt4BattleSounds(QObject*);

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
    void cryStateChanged(Phonon::State);

private:
    /* The device which outputs the sound */
    Phonon::AudioOutput *audioOutput;
    /* The media the device listens from */
    Phonon::MediaObject *mediaObject;
    /* The device for cries */
    Phonon::AudioOutput *cryOutput;
    /* The media the device listens from for pokemon cries */
    Phonon::MediaObject *cryObject;
};

#endif 
