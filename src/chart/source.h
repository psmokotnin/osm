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

protected:
    bool _active         = true;
    QString _name        = "Measurement";
    QColor _color;

    struct FTData {
        complex data;
        complex reference;
        float frequency;
    } *_ftdata;

    struct TimeData {
        float time; //ms
        complex value;
    } *_impulseData, *_scopeData;

    unsigned int _dataLength;
    unsigned int _deconvolutionSize;
    unsigned int _fftSize;

public:
    explicit Source(QObject *parent = nullptr);
    Q_INVOKABLE void destroy() {deleteLater();}//Schedules ~Source() from qml

    bool active() const noexcept {return _active;}
    virtual void setActive(bool active) {_active = active; emit activeChanged();}

    QString name() const noexcept {return _name;}
    void setName(QString name) {_name = name; emit nameChanged();}

    QColor color() const noexcept {return _color;}
    void setColor(QColor color) {_color = color; emit colorChanged();}

    unsigned int size() const  noexcept {return _dataLength;}
    unsigned int fftSize() const  noexcept {return _fftSize;}
    void setFftSize(unsigned int size) {_fftSize = size;}
    float frequency(unsigned int i) const noexcept;
    float module(unsigned int i) const noexcept;
    float dataAbs(unsigned int i) const noexcept;
    float magnitude(unsigned int i) const noexcept;
    float phase(unsigned int i) const noexcept;

    unsigned int impulseSize() const noexcept {return _deconvolutionSize;}
    float impulseTime(unsigned int i) const noexcept;
    float impulseValue(unsigned int i) const noexcept;

    void copy(FTData *dataDist, TimeData *timeDist);

signals:
    void activeChanged();
    void nameChanged();
    void colorChanged();
    void readyRead();

public slots:
    void setGlobalColor(int globalValue);
};
}
#endif // SOURCE_H
