#ifndef ONLINECONTROLLER_H
#define ONLINECONTROLLER_H

#include <QObject>
#include <QJsonDocument>

// copied from Entertaining Mines
// I want this in the library because I'm a pleb lol

struct OnlineControllerPrivate;
class OnlineWebSocket;
class OnlineController : public QObject
{
    Q_OBJECT
public:
    OnlineController(QObject* parent = nullptr);
    static OnlineController* instance();
    bool isConnected();
    void attachToWebSocket(OnlineWebSocket* ws);
    void sendJson(QJsonDocument doc);
    void sendJsonO(QJsonObject object);

    void setDiscordJoinSecret(QString joinSecret);
    QString discordJoinSecret();

    int ping();

public slots:
    void close();

signals:
    void disconnected(int closeCode);
    void jsonMessage(QJsonDocument doc);
    void pingChanged();

private:
    static OnlineControllerPrivate* d;
};

#endif // ONLINECONTROLLER_H
