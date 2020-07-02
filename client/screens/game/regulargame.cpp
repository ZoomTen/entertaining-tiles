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

    int firstPlayerRunOut; // first player to run out of

    QList<QString> playerNames;

    QVector<bool> calculatedMoves;
};

RegularGame::RegularGame(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RegularGame)
{
    ui->setupUi(this);
    d = new RegularGamePrivate();

    d->playerNames = {tr("Empty"), tr("Dark"), tr("Light")};
}

RegularGame::~RegularGame()
{
    clearTiles();
    delete d;
    delete ui;
}

// player control

void RegularGame::setActivePlayer(int player)
{
    d->currentPlayer = player;
    emit playerChanged(player);
}

void RegularGame::switchPlayers()
{
    switch (d->currentPlayer) {
    case ReversiTile::Light:
        setActivePlayer(ReversiTile::Dark);
        break;
    case ReversiTile::Dark:
        setActivePlayer(ReversiTile::Light);
        break;
    default:
        break;
    }
}

//  getting and checking tiles

ReversiTile* RegularGame::getTileAt(int row, int col)
{
    if (isInBoard(row, col)){
        return d->tiles[getIndex(row,col)];
    } else {
        return nullptr;  // invalid tile
    }
}

int RegularGame::getIndex(int row, int col)
{
    return (row * d->squareBoardSize) + col;
}

bool RegularGame::isInBoard(int row, int col)
{
    // greater than /or equal/ because zero indexed stuff
    if ((row < 0) || (row >= d->squareBoardSize)) return false;
    if ((col < 0) || (col >= d->squareBoardSize)) return false;
    return true;
}

QPair<int, int> RegularGame::rowAndColFromID(int tileId)
{
    if ((0 <= tileId) &&
             (tileId < (d->squareBoardSize * d->squareBoardSize))
    ){
        return QPair<int,int>(tileId / d->squareBoardSize,
                              tileId % d->squareBoardSize);
    } else {
        // invalid tile ID
        return QPair<int,int>(-1,-1);
    }
}

void RegularGame::refreshTileCount(int size)
{
    int darkCount = 0;
    int lightCount = 0;
    int emptyCount = 0;
    for (int row = 0; row < size; ++row){
        for (int col = 0; col < size; ++col){
            switch (getTileAt(row, col)->getType()){
            case ReversiTile::Dark:
                darkCount++;
                break;
            case ReversiTile::Light:
                lightCount++;
                break;
            case ReversiTile::Empty:
                emptyCount++;
                break;
            default:
                break;
            }
        }
    }

    QList<int> tileCounts;
    tileCounts.append(emptyCount);
    tileCounts.append(darkCount);
    tileCounts.append(lightCount);

    emit tileCountChanged(tileCounts);
}

// game control

QMap<QString, QVariant> RegularGame::getFlippable(int from, int row_offset, int col_offset, int row, int col)
{
    /* checks the tile at row, col and traverses in the direction of
     * row_offset and col_offset (call that a "vector") to see if there
     * is a flippable tile 1 step in that direction
     *
     * that tile is only flippable if it is the other players', and the
     * tile 1 step next to it is ours
     *
     * if so, we return a QMap specifying if we can flip a tile <bool>,
     * and if so, which ones <ReversiTile*>
     */
    QMap<QString, QVariant> map;
    int opposingTile;

    bool          canFlip = false;
    ReversiTile*  tileToFlip = nullptr;

    // determine opposing tile from the tile which this was called from
    switch(from){
    case ReversiTile::Dark:
        opposingTile = ReversiTile::Light;
        break;
    case ReversiTile::Light:
        opposingTile = ReversiTile::Dark;
        break;
    default:
        opposingTile = -1;  // no opposing tile
        break;
    }

    // check the middle tile
    ReversiTile* middleTileType = getTileAt(row+row_offset, col+col_offset);
    ReversiTile* lastTileType = getTileAt(row+(row_offset*2), col+(col_offset*2));

    if (lastTileType != nullptr){
        // if last tile exists
        // surely the middle one must exist as well
        if(middleTileType->getType() == opposingTile){
            if(lastTileType->getType() == from){
                // if last tile is the same as player tile
                canFlip = true;
                tileToFlip = getTileAt(row+row_offset, col+col_offset);
            }
        }
    }

    map["canFlip"]    = canFlip;
    map["tileToFlip"] = QVariant::fromValue(tileToFlip);

    return map;
}

void RegularGame::startGame(int size, int gameType)
{
    // TODO: size should be 6 <= x <= 20 and must be even
    // if a board exists, delete all the tiles first
    if (d->tiles.count() != 0){
        clearTiles();
    }

    d->squareBoardSize = size;
    d->gameType = gameType;
    d->firstPlayerRunOut = ReversiTile::Empty;

    // create new tiles
    for (int row = 0; row < size; ++row){
        for (int col = 0; col < size; ++col){
            // make tiles
            ReversiTile* tile = new ReversiTile(this,
                                                8*row + col);

            // link the window's resize event to set the proper tile width
            connect(this, &RegularGame::windowResized,
                    tile, &ReversiTile::setAppropriateSize);

            // if clicked on a tile, process the move
            connect(tile, &ReversiTile::clickedTileID,
                    this, &RegularGame::processMove);

            // add tile to game board
            d->tiles.append(tile);
            ui->gameGrid->addWidget(tile, row, col);
        }
    }

    /* initialize calculated legal moves grid
     * this isn't used for main functions, but rather
     * to be used for hints and computer moves
     */
    d->calculatedMoves.resize(size * size);

    // default positions
    // TODO: this assumes a 8x8 board.
    d->tiles[27]->setType(ReversiTile::Light);
    d->tiles[28]->setType(ReversiTile::Dark);
    d->tiles[35]->setType(ReversiTile::Dark);
    d->tiles[36]->setType(ReversiTile::Light);

    // UI functions
    ui->noMoreMoves->hide();

    connect(this, &RegularGame::numMovesChanged,
            this, [=](int movesLeft){
        qDebug() << "Moves left:" << movesLeft;
        if (movesLeft == 0){
            // pass it back to the other player

            if (d->firstPlayerRunOut == 0){
                d->firstPlayerRunOut = d->currentPlayer;
                switchPlayers();
                calculateLegalMoves(size); // calculate moves for the other player
            } else {
                // the game ends here
                ui->noMoreMoves->show();
            }
        } else {
            /* clear "first player to run out of moves" flag since
             * we can still play
             */
            d->firstPlayerRunOut = 0;
        }
    });

    connect(this, &RegularGame::tileCountChanged,
            this, [=](QList<int> tileCounts){
        ui->darkCount->setText(tr("Dark: %1").arg(tileCounts[1]));
        ui->lightCount->setText(tr("Light: %1").arg(tileCounts[2]));
    });

    connect(this, &RegularGame::playerChanged,
            this, [=](int player){
        ui->playerName->setText(d->playerNames[player]);
    });


    connect(ui->hintButton, &QPushButton::clicked,
            this, [=](bool){
        for (int i = 0; i < (d->squareBoardSize*d->squareBoardSize); ++i) {
            if(d->calculatedMoves[i]){
                d->tiles[i]->flashTile();
                break;
            }
        }
    });

    // TODO: make first player adjustable
    setActivePlayer(ReversiTile::Dark); // dark goes first

    // count all the tiles
    refreshTileCount(d->squareBoardSize);

    calculateLegalMoves(size);
}

void RegularGame::calculateLegalMoves(int size)
{
    /* traverses the entire grid to see if there's any moves that can be
     * made by the player
     *
     * highlightable tile == player can perform move on that tile
     */
    int numLegalMoves = 0;
    for (int row = 0; row < size; ++row){
        for (int col = 0; col < size; ++col){

            // clear out the highlightable for the tile
            getTileAt(row, col)->setHighlightable(false);

            // similarly, clear out our list "cache"
            d->calculatedMoves[getIndex(row,col)] = false;

            // check every direction
            if  (getTileAt(row, col)->getType() == ReversiTile::Empty){
            for (int row_offset = -1; row_offset < 2; ++row_offset){
            for (int col_offset = -1; col_offset < 2; ++col_offset){
                QMap<QString, QVariant> flippable = getFlippable(d->currentPlayer,
                                                                 row_offset,
                                                                 col_offset,
                                                                 row,
                                                                 col);
                if (flippable["canFlip"].toBool()){
                    /*qDebug() << "Legal at" << row  << col <<
                                "Affecting" <<
                                rowAndColFromID(flippable["tileToFlip"]
                                .value<ReversiTile*>()
                                ->getID());*/
                    // set legal move on the origin tile
                    ReversiTile* tile = getTileAt(row, col);
                    tile->setHighlightable(true);

                    // set it on our "cache" too
                    d->calculatedMoves[getIndex(row,col)] = true;

                    // count available moves
                    ++numLegalMoves;
                }
            }
            }
            }
        }
    }
    emit numMovesChanged(numLegalMoves);
}

void RegularGame::processMove(int tileId)
{
    /* called on tile click
     * if tile is highlightable (i.e. legal move), flip every tile that
     * can be flipped
     */
    QPair<int,int> rowCol = rowAndColFromID(tileId);
    int row = rowCol.first;
    int col = rowCol.second;

    if (d->tiles[tileId]->getHighlightable()){
        for (int row_offset = -1; row_offset < 2; ++row_offset){
        for (int col_offset = -1; col_offset < 2; ++col_offset){
            QMap<QString, QVariant> flippable = getFlippable(d->currentPlayer,
                                                             row_offset,
                                                             col_offset,
                                                             row,
                                                             col);
            if (flippable["canFlip"].toBool()){
                qDebug() << "Flipping" << row  << col <<
                            ".." <<
                            rowAndColFromID(flippable["tileToFlip"]
                            .value<ReversiTile*>()
                            ->getID());

                // add our tile to the board
                d->tiles[tileId]->setType(d->currentPlayer);
                flippable["tileToFlip"].value<ReversiTile*>()->flip();
            }
        }
        }
        // take turns
        switchPlayers();
        calculateLegalMoves(d->squareBoardSize);
        refreshTileCount(d->squareBoardSize);
    }
}

void RegularGame::clearTiles()
{
    for(ReversiTile* tile : d->tiles){
        ui->gameGrid->removeWidget(tile);
        tile->deleteLater();
    }
    d->tiles.clear();
}

// events

void RegularGame::resizeEvent(QResizeEvent*)
{
    // resize the tiles sensibly
    QSize boardSize = QSize(d->squareBoardSize,d->squareBoardSize);

    emit windowResized(boardSize, ui->gameWidget->size());
}
