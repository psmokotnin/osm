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
    float pwidth() const noexcept  {return static_cast<float>(width())  - (padding.left + padding.right); }
    float pheight() const noexcept {return static_cast<float>(height()) - (padding.top  + padding.bottom);}

public:
    PaintedItem (QQuickItem *parent = Q_NULLPTR);

};
}
#endif // PAINTEDITEM_H
