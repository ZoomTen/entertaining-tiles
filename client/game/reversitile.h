#ifndef REVERSITILE_H
#define REVERSITILE_H

#include <QWidget>

struct ReversiTilePrivate;
class ReversiTile : public QWidget
{
    Q_OBJECT
public:

    enum TileType{
      Empty,
      Dark,
      Light
    };
    explicit ReversiTile(QWidget *parent = nullptr, int id = 0, int type = Empty);
    ~ReversiTile();

    void flip();

    void setType(int type);
    int getType();

    void setHighlightable(bool highlightable);

    int getID();

private:
    ReversiTilePrivate* d;


signals:

};

#endif // REVERSITILE_H
