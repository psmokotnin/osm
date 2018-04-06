#ifndef PAINTEDITEM_H
#define PAINTEDITEM_H

#include <QtQuick/QQuickPaintedItem>

namespace Fftchart {
class PaintedItem : public QQuickPaintedItem
{
    Q_OBJECT

protected:
    const struct Padding {
        float   left    = 50,
                right   = 10,
                top     = 10,
                bottom  = 20;
    } padding;

    QString format(float v);
    float pwidth()  {return width()  - (padding.left + padding.right); }
    float pheight() {return height() - (padding.top  + padding.bottom);}

public:
    PaintedItem (QQuickItem *parent = Q_NULLPTR);

};
}
#endif // PAINTEDITEM_H
