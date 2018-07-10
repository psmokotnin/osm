#ifndef SOURCE_H
#define SOURCE_H
#include <QObject>
#include <QColor>
#include "../complex.h"

namespace Fftchart {
class Source : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

    //TODO: remove
    Q_PROPERTY(unsigned int pointsPerOctave READ pointsPerOctave WRITE setPointsPerOctave NOTIFY pointsPerOctaveChanged)

protected:
    bool _active         = true;
    QString _name        = "Measurement";
    QColor _color        = QColor("#209fdf");

    struct FTData {
long fftPoint;
        complex data;
        complex reference;
        float frequency;
float module    = -INFINITY;
float magnitude = 0.0;
float phase     = 0.0;
bool  correct    = false;
    } *_ftdata;

    struct TimeData {
        float time; //ms
        complex value;
    } *_impulseData, *_scopeData;

    int _dataLength, _deconvolutionSize;
    int _fftSize;


    //TODO: remove
    int _pointsPerOctave = 1;//12;

public:
    explicit Source(QObject *parent = nullptr);


    bool active() const noexcept {return _active;}
    virtual void setActive(bool active) {_active = active; emit activeChanged();}

    QString name() const noexcept {return _name;}
    void setName(QString name) {_name = name; emit nameChanged();}

    QColor color() const noexcept {return _color;}
    void setColor(QColor color) {_color = color; emit colorChanged();}


    float size() const  noexcept {return _dataLength;}
    float fftSize() const  noexcept {return _fftSize;}
    float frequency(int i) const noexcept;
    float module(int i) const noexcept;
    float dataAbs(int i) const noexcept;
    float magnitude(int i) const noexcept;
    float phase(int i) const noexcept;

    float impulseSize() const noexcept {return _deconvolutionSize;}
    float impulseTime(int i) const noexcept;
    float impulseValue(int i) const noexcept;

    //TODO: remove
    unsigned int pointsPerOctave() {return _pointsPerOctave;}
    void setPointsPerOctave(unsigned int p) {_pointsPerOctave = p;}

signals:
    void activeChanged();
    void nameChanged();
    void colorChanged();

    //TODO: remove
    void pointsPerOctaveChanged();

public slots:
};
}
#endif // SOURCE_H
