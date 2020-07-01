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

    void setActivePlayer(int player);
    int getIndex(int row, int col);
    bool isInBoard(int row, int col);

signals:
    void windowResized(QSize tileSize, QSize boardSize);

public slots:
    void startGame(int size, int gameType);

private:
    Ui::RegularGame *ui;
    RegularGamePrivate* d;

    void clearTiles();
    void calculateLegalMoves(int size);

private slots:
    void processMove(int tileId);

protected:
    void resizeEvent(QResizeEvent *resize);
};

#endif // SINGLEPLAYERGAME_H
