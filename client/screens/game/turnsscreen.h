#ifndef TURNSSCREEN_H
#define TURNSSCREEN_H

#include <QWidget>

namespace Ui {
class TurnsScreen;
}

class TurnsScreen : public QWidget
{
    Q_OBJECT

public:
    explicit TurnsScreen(QWidget *parent = nullptr, QString playerName = "");
    ~TurnsScreen();

signals:
    void doneShown();

private:
    Ui::TurnsScreen *ui;
};

#endif // TURNSSCREEN_H
