#ifndef MAINSCREEN_H
#define MAINSCREEN_H

#include <QWidget>

namespace Ui {
class MainScreen;
}

class MainScreen : public QWidget
{
    Q_OBJECT

public:
    explicit MainScreen(QWidget *parent = nullptr);
    ~MainScreen();

signals:
    void beginOnline();
    void beginPlay(int gameType, QList<QString> names);

private:
    Ui::MainScreen *ui;

    void setGamepadActions();
    void setButtonActions();
};

#endif // MAINSCREEN_H
