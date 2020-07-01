#ifndef ONLINESCREEN_H
#define ONLINESCREEN_H

#include <QWidget>

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
    Ui::OnlineScreen *ui;

    void setupGenericHooks();
    void setupMenuHooks();
};

#endif // ONLINESCREEN_H
