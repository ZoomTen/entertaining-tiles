#include "regulargame.h"
#include "ui_regulargame.h"

#include "game/reversitile.h"

#include <QVector>

#include <QDebug>

struct RegularGamePrivate{
    QVector<ReversiTile*> tiles;
    int gameType;

    int squareBoardSize;

    int currentPlayer;

    QVector<bool> legalMoves;
    QVector<bool> tilesToFlip;
};

RegularGame::RegularGame(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RegularGame)
{
    ui->setupUi(this);
    d = new RegularGamePrivate();
}

RegularGame::~RegularGame()
{
    clearTiles();
    delete d;
    delete ui;
}

void RegularGame::setActivePlayer(int player)
{
    d->currentPlayer = player;
}

int RegularGame::getIndex(int row, int col)
{
    return (row * d->squareBoardSize) + col;
}

bool RegularGame::isInBoard(int row, int col)
{
    if ((row < 0) || (row > d->squareBoardSize)) return false;
    if ((col < 0) || (col > d->squareBoardSize)) return false;
    return true;
}

void RegularGame::startGame(int size, int gameType)
{
    // TODO: size should be 6 <= x <= 20 and must be even
    // if a board exists, delete all the tiles first
    if (d->tiles.count() != 0){
        clearTiles();
    }

    // create new tiles
    for (int row = 0; row < size; ++row){
        for (int col = 0; col < size; ++col){
            // make tiles
            ReversiTile* tile = new ReversiTile(this,
                                                8*row + col);

            // link the window's resize event to set the proper tile width
            connect(this, &RegularGame::windowResized,
                    tile, &ReversiTile::setAppropriateSize);

            // clicked on a tile, process the move
            connect(tile, &ReversiTile::clickedTileID,
                    this, &RegularGame::processMove);

            // add tile to game board
            d->tiles.append(tile);
            ui->gameGrid->addWidget(tile, row, col);
        }
    }

    // default positions
    // TODO: this assumes a 8x8 board.
    d->tiles[27]->setType(ReversiTile::Light);
    d->tiles[28]->setType(ReversiTile::Dark);
    d->tiles[35]->setType(ReversiTile::Dark);
    d->tiles[36]->setType(ReversiTile::Light);

    d->gameType = gameType;

    d->squareBoardSize = size;

    // initialize lists
    d->legalMoves.resize(size*size);
    d->tilesToFlip.resize(size*size);

    calculateLegalMoves(size);
}

void RegularGame::clearTiles()
{
    for(ReversiTile* tile : d->tiles){
        ui->gameGrid->removeWidget(tile);
        tile->deleteLater();
    }
    d->tiles.clear();
}

void RegularGame::calculateLegalMoves(int size)
{
    int index;
    for (int row = 0; row < size; ++row){
        for (int col = 0; col < size; ++col){
            index = getIndex(row, col);
            //d->legalMoves[index] = true;
        }
    }
}

void RegularGame::processMove(int tileId)
{

}

void RegularGame::resizeEvent(QResizeEvent*)
{
    // resize the tiles sensibly
    QSize boardSize = QSize(d->squareBoardSize,d->squareBoardSize);

    emit windowResized(boardSize, ui->gameWidget->size());
}
