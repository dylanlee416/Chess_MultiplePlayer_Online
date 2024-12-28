#include <QAudioOutput>
#include <QMediaPlayer>

class AudioPlayer : public QObject
{
    Q_OBJECT

public:
    AudioPlayer(QObject *parent = nullptr)
        : QObject(parent)
    {
        player = new QMediaPlayer(this);
        audioOutput = new QAudioOutput(this);
        player->setAudioOutput(audioOutput);
    }

    ~AudioPlayer()
    {
        delete player;
        delete audioOutput;
    }

    void play(const QString &filePath)
    {
        // 设置音频输出设备
        audioOutput->setVolume(50); // 设置音量为50%

        // 设置要播放的音频文件
        player->setSource(QUrl::fromLocalFile(filePath));

        // 开始播放
        player->play();
    }

    void stop() { player->stop(); }

private:
    QMediaPlayer *player;
    QAudioOutput *audioOutput;
};
