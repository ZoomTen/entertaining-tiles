#include "onlinescreen.h"
#include "ui_onlinescreen.h"

// entertaining
#include <online/onlineapi.h>
#include <online/onlinewebsocket.h>
#include <questionoverlay.h>

// local controller
#include "onlinecontroller/onlinecontroller.h"

// included screens
#include "screens/online/menu/onlinemenuscreen.h"

OnlineScreen::OnlineScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OnlineScreen)
{
    ui->setupUi(this);

    setupMenuHooks();
    setupGenericHooks();
    setupJsonHooks();

    ui->onlinePageSwitcher->setCurrentAnimation(tStackedWidget::SlideHorizontal);
}

OnlineScreen::~OnlineScreen()
{
    //delete bgm;
    delete ui;
}

void OnlineScreen::doConnect()
{
    // show the waiting screen
    ui->onlinePageSwitcher->setCurrentWidget(ui->pendingScreen);

    // connect to our game
    /* this also requires a server-side application to be created
     * as well, under "applications/EntertainingTiles/0.5
     */
    OnlineApi::instance()->play("EntertainingTiles","0.5",this)
            ->then([=](OnlineWebSocket* socket){
        // on successful connect, show the menu page
        ui->onlinePageSwitcher->setCurrentWidget(ui->menuScreen);
        ui->menuScreen->setFocus();

        OnlineController::instance()->attachToWebSocket(socket);

        // TODO: join room
    })->error([=](QString error){
        // Clear join secret
        OnlineController::instance()->discordJoinSecret();

        if (error == "disconnect"){ emit backToMain(); return; }

        QuestionOverlay* question = new QuestionOverlay(this);
                question->setIcon(QMessageBox::Critical);
                question->setTitle(tr("Error"));
                question->setText(OnlineApi::errorFromPromiseRejection(error));
                question->setButtons(QMessageBox::Ok);

                // this only functions as an infobox, so quit either way
                auto handler = [=] {
                    question->deleteLater();
                    emit backToMain();
                };
                connect(question, &QuestionOverlay::accepted, this, handler);
                connect(question, &QuestionOverlay::rejected, this, handler);
    });
}

void OnlineScreen::setupGenericHooks()
{
    // on disconnect
    connect(OnlineController::instance(), &OnlineController::disconnected, this, [=](int closeCode) {
            QString error;
            switch (closeCode) {
                case QWebSocketProtocol::CloseCodeNormal:
                    break;
                case QWebSocketProtocol::CloseCodeGoingAway:
                    error = tr("You've been disconnected because the server is about to undergo maintenance.");
                    break;
                case QWebSocketProtocol::CloseCodeProtocolError:
                    error = tr("You've been disconnected from the server because there was a communication error.");
                    break;
                case QWebSocketProtocol::CloseCodeDatatypeNotSupported:
                case QWebSocketProtocol::CloseCodeReserved1004:
                case QWebSocketProtocol::CloseCodeMissingStatusCode:
                case QWebSocketProtocol::CloseCodeWrongDatatype:
                case QWebSocketProtocol::CloseCodePolicyViolated:
                case QWebSocketProtocol::CloseCodeTooMuchData:
                case QWebSocketProtocol::CloseCodeAbnormalDisconnection:
                default:
                    error = tr("You've been disconnected from the server.");
            }

            if (error.isEmpty()) {
                emit backToMain();
            } else {
                QuestionOverlay* question = new QuestionOverlay(this);
                question->setIcon(QMessageBox::Critical);
                question->setTitle(tr("Error"));
                question->setText(error);
                question->setButtons(QMessageBox::Ok);

                // this only functions as an infobox, so quit either way
                auto handler = [=] {
                    question->deleteLater();
                    emit backToMain();
                };
                connect(question, &QuestionOverlay::accepted, this, handler);
                connect(question, &QuestionOverlay::rejected, this, handler);
            }
    });
}

void OnlineScreen::setupMenuHooks()
{
    connect(ui->menuScreen, &OnlineMenuScreen::quitOnline, this, [=]{
        OnlineController::instance()->close();
    });
}

void OnlineScreen::setupJsonHooks()
{
    connect(OnlineController::instance(), &OnlineController::jsonMessage, this, [=](QJsonDocument doc) {
        QJsonObject object = doc.object();
        QString type = object.value("type").toString();

        if (type == "stateChange") {
            QString state = object.value("newState").toString();
            qDebug() << state;

            if (state == "idle") {
                ui->onlinePageSwitcher->setCurrentWidget(ui->menuScreen);

//                DiscordIntegration::instance()->setPresence({
//                    {"state", tr("Idle")},
//                    {"details", tr("Main Menu")}
//                });
            }
            else if (state == "lobby") {
                BackgroundMusic* bgm = new BackgroundMusic(this, ":/audio/bgm/game-lobby.mod");
                bgm->startMusic();
//                MusicEngine::setBackgroundMusic("airlounge");
//                MusicEngine::playBackgroundMusic();

                ui->onlinePageSwitcher->setCurrentWidget(ui->lobbyScreen);
            }
            else if (state == "game") {
//                MusicEngine::setBackgroundMusic("crypto");
//                MusicEngine::playBackgroundMusic();

                ui->onlinePageSwitcher->setCurrentWidget(ui->onlineGameScreen);
            }
        }

//        else if (type == "availableRoomsReply") {
//            OnlineJoinScreen* joinScreen = new OnlineJoinScreen(object, this);
//        }

        else if (type == "joinRoomFailed") {
            QString error = tr("Give it another go.");
            QString serverMessage = object.value("reason").toString();
            if (serverMessage == "room.invalid") {
                error = tr("That room doesn't exist.");
            } else if (serverMessage == "room.full") {
                error = tr("That room is full.");
            } else if (serverMessage == "room.closed") {
                error = tr("That room is closed. Wait for the current game to end and then you'll be able to join.");
            }

            QuestionOverlay* question = new QuestionOverlay(this);
            question->setIcon(QMessageBox::Critical);
            question->setTitle(tr("Can't join that room"));
            question->setText(error);
            question->setButtons(QMessageBox::Ok);
            connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
            connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);
        }
    });
}
