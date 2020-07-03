#ifndef BACKGROUNDMUSIC_H
#define BACKGROUNDMUSIC_H

#include <QObject>

#define MUSIC_BUFFER_SIZE 16384 // for ModPlug player

struct BackgroundMusicPrivate;
class BackgroundMusic : public QObject
{
    Q_OBJECT
public:
    explicit BackgroundMusic(QObject *parent = nullptr, QString filename = "");
    ~BackgroundMusic();

    void startMusic();
    void stopMusic();
    void pauseMusic();
    void unpauseMusic();

private:
    BackgroundMusicPrivate* d;

signals:
    void musicStarted();
    void musicStopped();
    void musicPaused();
    void musicUnpaused();

};

#endif // BACKGROUNDMUSIC_H
