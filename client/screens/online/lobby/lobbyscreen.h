#ifndef LOBBYSCREEN_H
#define LOBBYSCREEN_H

#include <QWidget>

namespace Ui {
class LobbyScreen;
}

class LobbyScreen : public QWidget
{
    Q_OBJECT

public:
    explicit LobbyScreen(QWidget *parent = nullptr);
    ~LobbyScreen();

private:
    Ui::LobbyScreen *ui;
};

#endif // LOBBYSCREEN_H
