#include "reversitile.h"

#include <QPainter>

#include <tvariantanimation.h>

#include <QDebug>  // DEBUG

struct ReversiTilePrivate{
    int id;
    int type;
    bool highlightable;
    bool highlighted;

    int highlightFlashOpacity;
    tVariantAnimation highlightFlashAnimation;
};

ReversiTile::ReversiTile(QWidget *parent, int id, int type) : QWidget(parent)
{
    d = new ReversiTilePrivate();
    d->id = id;
    d->type = type;
    d->highlightable = false;   // by default this is not a legal move tile
    d->highlighted = false; // tile hovered over

    // let QPainter set the background for us
    this->setAttribute(Qt::WA_TranslucentBackground);

    this->setMouseTracking(true);
}

ReversiTile::~ReversiTile()
{
    delete d;
}

void ReversiTile::flip()
{
    // switch the type
    if(d->type == Dark){
        d->type = Light;
    } else {
        d->type = Dark;
    }

    // run the animation
    flashTile();
}

void ReversiTile::setType(int type)
{
    d->type = type;
    this->update();
}

int ReversiTile::getType()
{
    return d->type;
}

void ReversiTile::setHighlightable(bool highlightable)
{
    // highlightable if it is a legal move
    d->highlightable = highlightable;
    this->update();
}

bool ReversiTile::getHighlightable()
{
    return d->highlightable;
}

int ReversiTile::getID()
{
    return d->id;
}

void ReversiTile::flashTile()
{
    /* flashTile shall only be called on changed tiles
     * hopefully this causes no problems...
     */

    // temporarily set highlight
    d->highlighted = true;

    // flash the tile
    setFlashingAnimation(FlashOut);
    d->highlightFlashAnimation.start();
    connect(&d->highlightFlashAnimation, &tVariantAnimation::valueChanged,
            this, [=](QVariant value){
        d->highlightFlashOpacity = value.toInt();
        this->update();
    });
    connect(&d->highlightFlashAnimation, &tVariantAnimation::finished,
            this, [=](){
        d->highlighted = false;
        this->update();
    });
}

void ReversiTile::setFlashingAnimation(int flashingAnim)
{
    switch(flashingAnim){
    case Fading:
        d->highlightFlashAnimation.setStartValue(0); // fade in
        d->highlightFlashAnimation.setEndValue(96);  // highlight opacity
        d->highlightFlashAnimation.setEasingCurve(QEasingCurve::OutBack);
        d->highlightFlashAnimation.setDuration(128);
        break;
    case FlashOut:
        d->highlightFlashAnimation.setStartValue(255); // fade out
        d->highlightFlashAnimation.setEndValue(0);
        d->highlightFlashAnimation.setEasingCurve(QEasingCurve::Linear);
        d->highlightFlashAnimation.setDuration(500);
        break;
    default:
        break;
    }
}

void ReversiTile::setAppropriateSize(QSize tileSize, QSize boardSize)
{
    // divide the widget size by the tiles size and use whichever is smaller
    int width = (boardSize.width() / tileSize.width() / 1.25);
    int height = (boardSize.height() / tileSize.height() / 1.25);
    int squareSize = qMin(width, height);
    //qDebug() << squareSize;
    this->setFixedSize(squareSize, squareSize);
}

void ReversiTile::mousePressEvent(QMouseEvent*)
{
   // qDebug() << "press" << d->id;  // DEBUG
    if (d->highlightable){
        qDebug() << "HIGHLIGHTABLE";
    }
    emit clickedTileID(d->id);
    this->update();
}

void ReversiTile::enterEvent(QEvent*)
{
    if (d->highlightable){
        //qDebug() << "highlighted";  // DEBUG

        // only empty tiles can be highlighted
        if (d->type == Empty){
            d->highlighted = true;

            setFlashingAnimation(Fading);
            d->highlightFlashAnimation.start();
            connect(&d->highlightFlashAnimation, &tVariantAnimation::valueChanged,
                    this, [=](QVariant value){
                d->highlightFlashOpacity = value.toInt();
                this->update();
            });
        }
    }
}

void ReversiTile::leaveEvent(QEvent*)
{
    //qDebug() << "off highlight";  // DEBUG
    d->highlighted = false;
    this->update();
}

void ReversiTile::paintEvent(QPaintEvent* e)
{
    QBrush fill;
    QColor color;

    fill.setStyle(Qt::SolidPattern);

    switch(d->type){
    case Empty:
        color.setRgb(128,128,255,64); // blueish-grey
        fill.setColor(color);
        break;
    case Dark:
        color.setRgb(0,0,0,255); // black
        fill.setColor(color);
        break;
    case Light:
        color.setRgb(255,255,255,255); // white
        fill.setColor(color);
        break;
    default:
        // no color
        fill.setStyle(Qt::NoBrush);
        break;
    }

    // setup drawing
    QPainter p(this);

    // draw background
    p.fillRect(0, 0, this->width(), this->height(), fill);

    // hovered over
    if (d->highlighted){
        if (d->type == Light){
            color.setRgb(55,244,55,d->highlightFlashOpacity);
        } else {
            color.setRgb(244,55,55,d->highlightFlashOpacity);
        }
        fill.setColor(color);
        p.fillRect(0, 0, this->width(), this->height(), fill);
    }

    // fallback here
    QWidget::paintEvent(e);
}


