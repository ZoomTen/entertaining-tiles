#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QWidget>

namespace Ui {
class GameWindow;
}

struct GameWindowPrivate;
class GameWindow : public QWidget
{
    Q_OBJECT

public:
    explicit GameWindow(QWidget *parent = nullptr);
    ~GameWindow();

private slots:
    void on_actionExit_triggered();
    void on_actionSources_triggered();
    void on_actionAbout_triggered();

private:
    void init();
    void setupFunctions();

    Ui::GameWindow* ui;
    GameWindowPrivate* d;
};

#endif // GAMEWINDOW_H
