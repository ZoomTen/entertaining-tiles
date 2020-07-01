#ifndef ONLINEPENDINGSCREEN_H
#define ONLINEPENDINGSCREEN_H

#include <QWidget>

namespace Ui {
class OnlinePendingScreen;
}

class OnlinePendingScreen : public QWidget
{
    Q_OBJECT

public:
    explicit OnlinePendingScreen(QWidget *parent = nullptr);
    ~OnlinePendingScreen();

private:
    Ui::OnlinePendingScreen *ui;
};

#endif // ONLINEPENDINGSCREEN_H
