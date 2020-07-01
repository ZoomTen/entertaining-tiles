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

    enum FlashAnim{
        Fading,
        FlashOut
    };

    explicit ReversiTile(QWidget *parent = nullptr, int id = 0, int type = Empty);
    ~ReversiTile();

    void flip();

    void setType(int type);
    int getType();

    void setHighlightable(bool highlightable);
    bool getHighlightable();

    int getID();

signals:
    void clickedTileID(int id);

public slots:
    void setAppropriateSize(QSize tileSize, QSize boardSize);

private:
    ReversiTilePrivate* d;
    void flashTile();
    void setFlashingAnimation(int flashingAnim);

protected:
    void mousePressEvent(QMouseEvent *e);
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
    void paintEvent(QPaintEvent* event);

};

#endif // REVERSITILE_H
