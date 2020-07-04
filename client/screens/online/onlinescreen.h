#ifndef ONLINESCREEN_H
#define ONLINESCREEN_H

#include <QWidget>
#include "audio/backgroundmusic.h"

namespace Ui {
class OnlineScreen;
}

class OnlineScreen : public QWidget
{
    Q_OBJECT

public:
    explicit OnlineScreen(QWidget *parent = nullptr);
    ~OnlineScreen();

    void doConnect();

signals:
    void backToMain();

private:
    BackgroundMusic* bgm;
    Ui::OnlineScreen *ui;

    void setupGenericHooks();
    void setupMenuHooks();
    void setupJsonHooks();
};

#endif // ONLINESCREEN_H
