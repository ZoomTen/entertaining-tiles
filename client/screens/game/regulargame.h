#ifndef SINGLEPLAYERGAME_H
#define SINGLEPLAYERGAME_H

#include <QWidget>
#include "game/reversitile.h"

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
    void switchPlayers();

    ReversiTile* getTileAt(int row, int col);
    ReversiTile* getTileAtIndex(int index);
    int getIndex(int row, int col);
    bool isInBoard(int row, int col);
    QPair<int, int> rowAndColFromID(int tileId);

signals:
    void windowResized(QSize tileSize, QSize boardSize);
    void playerChanged(int player);
    void numMovesChanged(int numMoves);

    //QList<int> tileCounts = {empty, dark, light}
    void tileCountChanged(QList<int> tileCounts);

public slots:
    void startGame(int size, int gameType);

private:
    Ui::RegularGame *ui;
    RegularGamePrivate* d;

    void clearTiles();
    void calculateLegalMoves(int size);
    void refreshTileCount(int size);

    QMap<QString, QVariant> getFlippable(int from,
                                         int row_offset, int col_offset,
                                         int row, int col);

    void pauseSession();

private slots:
    void processMove(int tileId);

    void on_pauseButton_clicked();

protected:
    void resizeEvent(QResizeEvent *resize);
    void keyPressEvent(QKeyEvent *e);
    bool focusNextPrevChild(bool next);
};

#endif // SINGLEPLAYERGAME_H
