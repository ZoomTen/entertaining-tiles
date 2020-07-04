#include "turnsscreen.h"
#include "ui_turnsscreen.h"

#include <pauseoverlay.h>
#include <QTimer>

TurnsScreen::TurnsScreen(QWidget *parent, QString playerName) :
    QWidget(parent),
    ui(new Ui::TurnsScreen)
{
    ui->setupUi(this);

    ui->playerCap->setText(tr("%1's").arg(playerName));

    PauseOverlay::overlayForWindow(this->parentWidget())->pushOverlayWidget(this);

    //this->setFocus();

    QTimer::singleShot(1000, this, [=]{
        PauseOverlay::overlayForWindow(this->parentWidget())->popOverlayWidget([=]{
             emit doneShown();
        });
    });
}

TurnsScreen::~TurnsScreen()
{
    delete ui;
}
