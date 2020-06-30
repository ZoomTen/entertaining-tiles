#include "onlinependingscreen.h"
#include "ui_onlinependingscreen.h"

OnlinePendingScreen::OnlinePendingScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OnlinePendingScreen)
{
    ui->setupUi(this);
}

OnlinePendingScreen::~OnlinePendingScreen()
{
    delete ui;
}
