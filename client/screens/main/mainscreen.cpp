#include "mainscreen.h"
#include "ui_mainscreen.h"

// entertaining
#include <gamepadbuttons.h>
#include <textinputoverlay.h>

// game modes
#include "screens/game/regulargame.h"

#include <QTimer>

MainScreen::MainScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainScreen)
{
    ui->setupUi(this);

    // make page switcher slide left on menu switch
    ui->pageSwitcher->setCurrentAnimation(tStackedWidget::SlideHorizontal);
    // I want to make it the same thing but it messes up the back button stuff
    ui->infoSwitcher->setCurrentAnimation(tStackedWidget::None);

    // set initial info page
    ui->infoSwitcher->setCurrentWidget(ui->infoPageMain);

    // add button to info submenu title
    ui->infoMenuTitle->setBackButtonShown(true);
    // info submenu back button goes back to main menu (if on the main page)
    connect(ui->infoMenuTitle, &tTitleLabel::backButtonClicked, this, [=]{
        if (ui->infoSwitcher->currentWidget() == ui->infoPageMain){
            ui->pageSwitcher->setCurrentWidget(ui->mainMenu);
        } else {
            ui->infoSwitcher->setCurrentWidget(ui->infoPageMain);
        }
    });

    // set menu barriers for keyboard / joypad control
    /* for the top barrier we want it to bounce back to the topmost button
     * (vs Computer) when the focus is on it
     */
    ui->AbarrierA->setBounceWidget(ui->vsComputerButton);
    /* likewise, we set the bottom barrier to bounce back to the exit button
     */
    ui->AbarrierB->setBounceWidget(ui->exitButton);

    // set info submenu barriers for keyboard / joypad control
    ui->BbarrierA->setBounceWidget(ui->rulesButton);
    ui->BbarrierB->setBounceWidget(ui->authorsButton);

    setGamepadActions();
    setButtonActions();

    ui->scrollArea->setFocusPolicy(Qt::NoFocus);
    ui->scrollArea_2->setFocusPolicy(Qt::NoFocus);
    ui->vsComputerButton->setFocus();
}

MainScreen::~MainScreen()
{
    delete ui;
}

void MainScreen::setGamepadActions()
{
    // assign "A" button to select
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA,
                                    GamepadHud::standardAction(
                                        GamepadHud::SelectAction));

    // assign "B" button to exit
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Exit"));
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [=]{
        switch(ui->pageSwitcher->currentIndex()){
        case 0: // main menu
            // simulate clicking the exit button
            ui->exitButton->click();
            break;
        case 1: // info subpage
            // switch back to main menu
            if (ui->infoSwitcher->currentWidget() == ui->infoPageMain){
                ui->pageSwitcher->setCurrentWidget(ui->mainMenu);
            } else {
                ui->infoSwitcher->setCurrentWidget(ui->infoPageMain);
            }
            break;
        default: // undefined
            break;
        }
    });
}

void MainScreen::setButtonActions()
{
    // main buttons
    connect(ui->exitButton, &QCommandLinkButton::clicked, this, [=]{
        QApplication::exit();
    });

    connect(ui->onlineButton, &QCommandLinkButton::clicked, this, [=]{
        emit beginOnline();
    });

    connect(ui->vsComputerButton, &QCommandLinkButton::clicked, this, [=]{
        bool canceled;

        QString playerName;

        playerName = TextInputOverlay::getText(this,
                                                   tr("What is your name?"),
                                                   &canceled,
                                                   "Player");
        if (canceled) return;

        emit beginPlay(RegularGame::VsComputer, {tr("Empty"),
                                                 playerName,
                                                 tr("Computer")});
    });

    connect(ui->localMultiplayerButton, &QCommandLinkButton::clicked, this, [=]{
        bool canceled;

        QString darkPlayerName, lightPlayerName;

        askPlayer1:
            darkPlayerName = TextInputOverlay::getText(this,
                                                       tr("What is your name?"),
                                                       &canceled,
                                                       "Player");
            if (canceled) return;
        //askPlayer2:
            lightPlayerName = TextInputOverlay::getText(this,
                                                       tr("What is your friend's name?"),
                                                       &canceled,
                                                       "Sidekick");
            if (canceled) goto askPlayer1;

            emit beginPlay(RegularGame::TakeTurns, {tr("Empty"),
                                                     darkPlayerName,
                                                     lightPlayerName});
    });

    connect(ui->infoButton, &QCommandLinkButton::clicked, this, [=]{
        // info button goes to info submenu
        ui->pageSwitcher->setCurrentWidget(ui->infoMenu);
    });

    // info subpages
    connect(ui->rulesButton, &QCommandLinkButton::clicked, this, [=]{
        // info button goes to info submenu
        ui->infoSwitcher->setCurrentWidget(ui->infoPageRules);
    });
    connect(ui->authorsButton, &QCommandLinkButton::clicked, this, [=]{
        // info button goes to info submenu
        ui->infoSwitcher->setCurrentWidget(ui->infoPageAuthors);
    });
}
