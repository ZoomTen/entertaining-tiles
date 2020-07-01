// window
#include "gamewindow.h"
#include "ui_gamewindow.h"

// the-libs
#include <tapplication.h>
#include <tcsdtools.h>
#include <taboutdialog.h>

// icon menu
#include <QMenu>

// linkage
#include <QDesktopServices>
#include <QUrl>

// entertaining games stuff
#include <online/logindialog.h>
#include <pauseoverlay.h>

// debug
#include <QDebug>

// individual windows
#include "screens/main/mainscreen.h"

// local stuff
#include "onlinecontroller/onlinecontroller.h"

struct GameWindowPrivate{
    tCsdTools csd;
};

GameWindow::GameWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GameWindow)
{
    init();
    setupFunctions();
}

GameWindow::~GameWindow()
{
    delete d;
    delete ui;
}

void GameWindow::init()
{
    // init the entire thing
    ui->setupUi(this);

    d = new GameWindowPrivate();

    // csd actions
    d->csd.installMoveAction(ui->csdArea);
    d->csd.installResizeAction(this);

    // csd layout
    QWidget* csd_widget = d->csd.csdBoxForWidget(this);

    if(tCsdGlobal::windowControlsEdge() == tCsdGlobal::Left){
        ui->leftSideCsd->addWidget(csd_widget);
    } else {
        ui->rightSideCsd->addWidget(csd_widget);
    }

    // fix icon size
    ui->menuButton->setIconSize(SC_DPI_T(QSize(24, 24), QSize));
    ui->menuButton->setFixedSize(SC_DPI(42), QWIDGETSIZE_MAX);

    // create Help submenu
    QMenu* menu = new QMenu(this);
    menu->addAction(ui->actionSources); // Source code link
    menu->addAction(ui->actionAbout);   // call About page
    menu->addAction(ui->actionExit);        // Exit item

    ui->menuButton->setMenu(menu);

    // set animation for the page switcher
    ui->mainSwitcher->setCurrentAnimation(tStackedWidget::Lift);
}

void GameWindow::setupFunctions()
{
    // Register local play
    connect(ui->mainScreen, &MainScreen::beginPlay, this, [=](int gameType){
        // setup game
        ui->gameScreen->startGame(8, gameType); // TODO: make width and height adjustable
        ui->mainSwitcher->setCurrentWidget(ui->gameScreen);
    });

    // Register "back to main menu" event
    connect(ui->onlineScreen, &OnlineScreen::backToMain, this, [=]{
        ui->mainSwitcher->setCurrentWidget(ui->mainScreen);
    });

    // Register overlay for the login screen
    PauseOverlay::registerOverlayForWindow(this, ui->mainSwitcher);

    // Setup online functionality
    connect(ui->mainScreen, &MainScreen::beginOnline, this, [=]{
        LoginDialog* login = new LoginDialog(this);
        connect(login, &LoginDialog::destroyed,
                login, &LoginDialog::deleteLater);
        if(login->exec()){
            // after login or (when logged in) on click
            // switch to online menu and connect to server
            ui->mainSwitcher->setCurrentWidget(ui->onlineScreen);
            ui->onlineScreen->doConnect();
        } else {
            //Clear the join secret if there is one
            OnlineController::instance()->discordJoinSecret();
        }
    });
}


// actions

void GameWindow::on_actionExit_triggered()
{
    tApplication::exit();
}

void GameWindow::on_actionSources_triggered()
{
    QDesktopServices::openUrl(QUrl("http://github.com/ZoomTen/theApp"));
}

void GameWindow::on_actionAbout_triggered()
{
    // open the library-provided about dialog
    tAboutDialog d;
    d.exec();
}

