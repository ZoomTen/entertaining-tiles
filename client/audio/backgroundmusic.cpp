#include "backgroundmusic.h"

#include <libmodplug/modplug.h>
#include <QAudioOutput>
#include <QBuffer>
#include <QFile>

#include <QDebug>

struct BackgroundMusicPrivate{
    QString filename;

    QAudioOutput* audio;
    QBuffer*      audio_buffer; // used by QAudioOutput
    char*         sndBuffer;    // used by ModPlug, copied to QBuffer*

    bool audioPaused;
};

BackgroundMusic::BackgroundMusic(QObject *parent, QString filename)
    : QObject(parent)
{
    d = new BackgroundMusicPrivate();

    d->filename = filename;

    // load module
    QFile file(d->filename);

    file.open(QIODevice::ReadOnly);
    QByteArray module_data = file.readAll();
    file.close();

    // use default speaker settings
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    QAudioFormat  speakerSettings = info.preferredFormat();

    d->audio_buffer = new QBuffer();
    d->sndBuffer    = new char[MUSIC_BUFFER_SIZE];

    ModPlug_Settings settings;
    settings.mResamplingMode = MODPLUG_RESAMPLE_LINEAR;
    settings.mLoopCount = -1;   // Loop forever

    settings.mStereoSeparation = 128;

    /* use default output format settings
     * unless of course things go janky...
     */
    // channels
    settings.mChannels = speakerSettings.channelCount();

    // bit depth
    settings.mBits = speakerSettings.sampleSize();

    // sample rate
    settings.mFrequency = speakerSettings.sampleRate();

    // initialize ModPlug player
    ModPlug_SetSettings(&settings);
    ModPlugFile* modfile = ModPlug_Load(module_data.data(), module_data.size());
    ModPlug_SetMasterVolume(modfile, 128);

    if (!modfile){
        qDebug() << tr("Entertaining Reversi: Can't read music file");
    } else {
        // actually stream the audio
        d->audio = new QAudioOutput(speakerSettings, this);

        // Handle audio events
        connect(d->audio, &QAudioOutput::stateChanged,
                this,  [=](QAudio::State state){
            if (state == QAudio::IdleState) {
                // try streaming more samples out of ModPlug player
                if (ModPlug_Read(modfile, d->sndBuffer, MUSIC_BUFFER_SIZE) == 0)
                {
                    // we run out of samples, so shut it down
                    d->audio->stop();
                    delete d->audio;
                    delete d->audio_buffer;
                    delete[] d->sndBuffer;
                }
                else
                {
                    // copy the stream to the Qt audio buffer
                    d->audio_buffer->close();
                    d->audio_buffer->setData(d->sndBuffer, MUSIC_BUFFER_SIZE);
                    d->audio_buffer->open(QIODevice::ReadOnly);
                }
            }
        });

        // Handle audio start and restart
        connect(this, &BackgroundMusic::musicStarted,
                this, [=]{
            // Modplug streams directly to buffer
            ModPlug_Seek(modfile, 0);   // at beginning
            ModPlug_Read(modfile, d->sndBuffer, MUSIC_BUFFER_SIZE);

            // Update the corresponding QBuffer...
            d->audio_buffer->setData(d->sndBuffer, MUSIC_BUFFER_SIZE);
            d->audio_buffer->open(QIODevice::ReadOnly);

            d->audio->start(d->audio_buffer);
        });

        // Handle audio stopping
        connect(this, &BackgroundMusic::musicStopped,
                this, [=]{
            d->audio->stop();
            delete d->audio;
            delete d->audio_buffer;
            delete[] d->sndBuffer;
        });

        // Handle audio pause
        connect(this, &BackgroundMusic::musicPaused,
                this, [=]{
            if (!d->audioPaused){
                d->audio->stop();
                d->audioPaused = true;
            }
        });

        // Handle audio continue
        connect(this, &BackgroundMusic::musicUnpaused,
                this, [=]{
            if (d->audioPaused){
                d->audio->start(d->audio_buffer);
                d->audioPaused = false;
            }
        });
    }
}

BackgroundMusic::~BackgroundMusic()
{
    delete[] d->sndBuffer;
    delete d->audio_buffer;
    delete d;
}

void BackgroundMusic::startMusic()
{
    emit musicStarted();
}

void BackgroundMusic::stopMusic()
{
    emit musicStopped();
}

void BackgroundMusic::pauseMusic()
{
    emit musicPaused();
}

void BackgroundMusic::unpauseMusic()
{
    emit musicUnpaused();
}
