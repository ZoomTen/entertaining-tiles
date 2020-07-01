#include "reversitile.h"

struct ReversiTilePrivate{
    int id;
    int type;
    bool highlightable;
};

ReversiTile::ReversiTile(QWidget *parent, int id, int type) : QWidget(parent)
{
    d = new ReversiTilePrivate();
    d->id = id;
    d->type = type;
}

ReversiTile::~ReversiTile()
{
    delete d;
}

void ReversiTile::flip()
{
    // if type = Dark then flip to Light
    // undefined if Empty
}

void ReversiTile::setType(int type)
{
    d->type = type;
}

int ReversiTile::getType()
{
    return d->type;
}

void ReversiTile::setHighlightable(bool highlightable)
{
    // highlightable if it is a legal move
    d->highlightable = highlightable;
}

int ReversiTile::getID()
{
   return d->id;
}


