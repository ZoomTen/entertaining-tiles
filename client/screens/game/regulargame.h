#ifndef SINGLEPLAYERGAME_H
#define SINGLEPLAYERGAME_H

#include <QWidget>

namespace Ui {
class RegularGame;
}

struct RegularGamePrivate;
class RegularGame : public QWidget
{
    Q_OBJECT

public:
    explicit RegularGame(QWidget *parent = nullptr);
    ~RegularGame();

    enum GameType{
        VsComputer,
        TakeTurns
    };

    QSize tileSize();

signals:
    void windowResized(QSize tileSize, QSize boardSize);

public slots:
    void startGame(int width, int height, int gameType);

private:
    Ui::RegularGame *ui;
    RegularGamePrivate* d;

    void clearTiles();

protected:
    void resizeEvent(QResizeEvent *resize);
};

#endif // SINGLEPLAYERGAME_H
