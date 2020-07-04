#include "lobbyscreen.h"
#include "ui_lobbyscreen.h"

#include <online/onlineapi.h>

LobbyScreen::LobbyScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LobbyScreen)
{
    ui->setupUi(this);


}

LobbyScreen::~LobbyScreen()
{
    delete ui;
}
