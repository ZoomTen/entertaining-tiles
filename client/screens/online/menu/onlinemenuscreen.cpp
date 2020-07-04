#include "onlinemenuscreen.h"
#include "ui_onlinemenuscreen.h"

// standard entertaining stuff
#include <online/accountdialog.h>
#include <online/friendsdialog.h>
#include "onlinecontroller/onlinecontroller.h"
#include <QJsonObject>

OnlineMenuScreen::OnlineMenuScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OnlineMenuScreen)
{
    ui->setupUi(this);

    setupUI();
    setupButtonEvents();
}

OnlineMenuScreen::~OnlineMenuScreen()
{
    delete ui;
}

void OnlineMenuScreen::setupButtonEvents()
{
    // Create lobby
    connect(ui->openLobbyButton, &QCommandLinkButton::clicked, this, [=]{
        OnlineController::instance()->sendJsonO({
                {"type", "openRoom"}
            });
    });

    // Join lobby
    connect(ui->joinLobbyButton, &QCommandLinkButton::clicked, this, [=]{
        OnlineController::instance()->sendJsonO({
                {"type", "listRooms"}
            });
    });

    // Account button clicked
    connect(ui->accountButton, &QCommandLinkButton::clicked, this, [=]{
        // pull up stock account dialog
        AccountDialog* a = new AccountDialog(this);
        // free up memory when done
        connect(a, &AccountDialog::done,
                a, &AccountDialog::deleteLater);
    });

    // Friends button clicked
    connect(ui->relationsButton, &QCommandLinkButton::clicked, this, [=]{
        // pull up friend management dialog
        FriendsDialog* a = new FriendsDialog(this);
        connect(a, &FriendsDialog::done,
                a, &FriendsDialog::deleteLater);
    });

    // Disconnect button clicked
    connect(ui->exitButton, &QCommandLinkButton::clicked, this, [=]{
        emit quitOnline();
    });
}

void OnlineMenuScreen::setupUI()
{
    ui->gamePadControls->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamePadControls->setButtonText(QGamepadManager::ButtonB, tr("Main Menu"));

    ui->gamePadControls->setButtonAction(QGamepadManager::ButtonA, GamepadHud::standardAction(GamepadHud::SelectAction));
    ui->gamePadControls->setButtonAction(QGamepadManager::ButtonB, [=] {
        ui->exitButton->click();
    });

    this->setFocusProxy(ui->openLobbyButton);
    ui->barrierTop->setBounceWidget(ui->openLobbyButton);
    ui->barrierBottom->setBounceWidget(ui->exitButton);
}
