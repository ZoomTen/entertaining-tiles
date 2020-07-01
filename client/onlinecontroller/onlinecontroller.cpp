#include "onlinecontroller.h"

#include <QJsonObject>
#include <online/onlinewebsocket.h>
#include <QDebug>

struct OnlineControllerPrivate {
    OnlineController* instance = nullptr;
    OnlineWebSocket* ws = nullptr;

    QString discordJoinSecret;
};

OnlineControllerPrivate* OnlineController::d = new OnlineControllerPrivate();

OnlineController::OnlineController(QObject *parent) :
    QObject(parent)
{

}

OnlineController*OnlineController::instance()
{
    if (!d->instance) d->instance = new OnlineController();
    return d->instance;
}

bool OnlineController::isConnected()
{
    return d->ws != nullptr;
}

void OnlineController::attachToWebSocket(OnlineWebSocket*ws)
{
    d->ws = ws;

    connect(ws, &OnlineWebSocket::disconnected, this, [=] {
        emit disconnected(ws->closeCode());

        //Clear out the WebSocket
        d->ws->deleteLater();
        d->ws = nullptr;
    });

    connect(ws, &OnlineWebSocket::jsonMessageReceived, this, &OnlineController::jsonMessage);

    connect(ws, &OnlineWebSocket::jsonMessageReceived, this, [=](QJsonDocument doc) {
        qDebug() << doc.toJson();
    });

    connect(ws, &OnlineWebSocket::pingChanged, this, &OnlineController::pingChanged);
}

void OnlineController::sendJson(QJsonDocument doc)
{
    d->ws->sendJson(doc);
}

void OnlineController::sendJsonO(QJsonObject object)
{
    if (d->ws == nullptr) {
        qWarning() << "Tried to send a message to an unopen socket";
        return;
    }
    d->ws->sendJsonO(object);
}

void OnlineController::setDiscordJoinSecret(QString joinSecret)
{
    d->discordJoinSecret = joinSecret;
}

QString OnlineController::discordJoinSecret()
{
    QString discordJoinSecret = d->discordJoinSecret;
    d->discordJoinSecret = "";
    return discordJoinSecret;
}

int OnlineController::ping()
{
    return d->ws->ping();
}

void OnlineController::close()
{
    d->ws->close();
}
