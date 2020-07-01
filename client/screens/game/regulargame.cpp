#include "regulargame.h"
#include "ui_regulargame.h"

#include "game/reversitile.h"

struct RegularGamePrivate{
    QVector<ReversiTile*> tiles;
    int gameType;

    QSize tileSize;
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

QSize RegularGame::tileSize()
{
    return d->tileSize;
}

void RegularGame::startGame(int width, int height, int gameType)
{
    for (int row = 0; row < height; ++row){
        for (int col = 0; col < width; ++col){
            // make tiles
            ReversiTile* tile = new ReversiTile(this,
                                                8*row + col);

            // link the window's resize event to set the proper tile width
            connect(this, &RegularGame::windowResized,
                    tile, &ReversiTile::setAppropriateSize);

            // add tile to game board
            d->tiles.append(tile);
            ui->gameGrid->addWidget(tile, row, col);
        }
    }

    d->tiles[27]->setType(ReversiTile::Light);
    d->tiles[28]->setType(ReversiTile::Dark);
    d->tiles[35]->setType(ReversiTile::Dark);
    d->tiles[36]->setType(ReversiTile::Light);

    d->gameType = gameType;

    d->tileSize.setWidth(width);
    d->tileSize.setHeight(height);
}

void RegularGame::clearTiles()
{
    for(ReversiTile* tile : d->tiles){
        ui->gameGrid->removeWidget(tile);
        tile->deleteLater();
    }
    d->tiles.clear();
}

void RegularGame::resizeEvent(QResizeEvent*)
{
    // resize the tiles sensibly
    emit windowResized(d->tileSize, ui->gameWidget->size());
}
