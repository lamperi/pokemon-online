#include "battlesounds.h"

#include "../PokemonInfo/pokemoninfo.h"

BattleSounds::BattleSounds(QObject *parent) : QObject(parent){
    undelayOnSounds = true;
}

void BattleSounds::toggleSounds(bool soundsOn) {
    this->soundsOn = soundsOn;
    if (!soundsOn) {
        playBattleCries() = false;
        playBattleMusic() = false;
        pauseMusic();
        return;
    }

    QSettings s;
    setMusicVolume(s.value("BattleAudio/MusicVolume").toInt());
    setCryVolume(s.value("BattleAudio/MusicVolume").toInt());

    if (soundsOn) {
        playBattleCries() = s.value("BattleAudio/PlaySounds").toBool();
        playBattleMusic() = s.value("BattleAudio/PlayMusic").toBool() || !s.value("play_battle_cries").toBool();
    }

    if (!playBattleMusic()) {
        return;
    }

    /* If more than 5 songs, start with a new music, otherwise carry on where it left. */
    QDir directory = QDir(s.value("BattleAudio/MusicDirectory").toString());
    QStringList files = directory.entryList(QStringList() << "*.mp3" << "*.ogg" << "*.wav" << "*.it" << "*.mid" << "*.m4a" << "*.mp4",
                                            QDir::Files | QDir::NoSymLinks | QDir::Readable, QDir::Name);

    QStringList tmpSources;

    foreach(QString file, files) {
        tmpSources.push_back(directory.absoluteFilePath(file));
    }

    /* If it's the same musics as before with only 1 file, we start playing again the paused file (would not be nice to restart from the
        start). Otherwise, a random file will be played from the start */
    if (tmpSources == sources && sources.size() == 1) {
        playMusic();
        return;
    }

    sources = tmpSources;

    if (sources.size() == 0)
        return;

    playMusicFrom(sources[true_rand()%sources.size()]);
}

void BattleSounds::playCry(int pokemon) {

    if (!playBattleCries())
        return;

    emit soundsPlaying();

    pokemon = Pokemon::uniqueId(pokemon).pokenum;

    if (!cries.contains(pokemon)) {
        cries.insert(pokemon, PokemonInfo::Cry(pokemon));
    }

    undelayOnSounds = false;

    stopCry();
    undelayOnSounds = true;

    cryBuffer.close();
    cryBuffer.setBuffer(&cries[pokemon]);
    cryBuffer.open(QIODevice::ReadOnly);
    playCry(cryBuffer);
}

void BattleSounds::enqueueMusic() {
    if (sources.size() == 0)
        return;
    QString url = sources[true_rand()%sources.size()];
    enqueueFrom(url);
}
