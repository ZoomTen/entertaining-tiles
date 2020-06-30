#ifndef ONLINEMENUSCREEN_H
#define ONLINEMENUSCREEN_H

#include <QWidget>

namespace Ui {
class OnlineMenuScreen;
}

class OnlineMenuScreen : public QWidget
{
    Q_OBJECT

public:
    explicit OnlineMenuScreen(QWidget *parent = nullptr);
    ~OnlineMenuScreen();

signals:
    void quitOnline();

private:
    Ui::OnlineMenuScreen *ui;
    void setupButtonEvents();
    void setupUI();
};

#endif // ONLINEMENUSCREEN_H
