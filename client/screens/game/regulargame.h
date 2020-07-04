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

    virtual void setActivePlayer(int player);
    virtual void switchPlayers();

    ReversiTile* getTileAt(int row, int col);
    ReversiTile* getTileAtIndex(int index);
    int getIndex(int row, int col);
    bool isInBoard(int row, int col);
    QPair<int, int> rowAndColFromID(int tileId);

    bool playerIsComputer();

signals:
    void windowResized(QSize tileSize, QSize boardSize);
    void playerChanged(int player);
    void numMovesChanged(int numMoves);

    //QList<int> tileCounts = {empty, dark, light}
    void tileCountChanged(QList<int> tileCounts);

    void setHighlightVisibility(bool);

public slots:
    virtual void startGame(int size, int gameType, QList<QString> names);

private:
    Ui::RegularGame *ui;
    RegularGamePrivate* d;

    virtual void clearTiles();
    int calculateLegalMoves(int size);
    void refreshTileCount(int size);

    QMap<QString, QVariant> getFlippable(int from,
                                         int row_offset, int col_offset,
                                         int row, int col);

    virtual void pauseSession();
    virtual void performComputer();
    virtual void flipRelevantTiles(int row, int col);
    virtual void playersTurnScreen();

private slots:
    virtual void processMove(int tileId);

    virtual void on_pauseButton_clicked();

protected:
    void resizeEvent(QResizeEvent *resize);
    void keyPressEvent(QKeyEvent *e);
    bool focusNextPrevChild(bool next);
};

#endif // SINGLEPLAYERGAME_H
