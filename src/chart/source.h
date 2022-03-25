/**
 *  OSM
 *  Copyright (C) 2018  Pavel Smokotnin

 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SOURCE_H
#define SOURCE_H
#include <QObject>
#include <QColor>
#include <QJsonObject>
#include <mutex>

#include "../math/complex.h"
class SourceList;

namespace chart {
class Source : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

public:
    struct FTData {
        float frequency;
        float module;
        float magnitude;
        complex phase;
        float coherence;
        float peakSquared;
        float meanSquared = NAN;
    };

    struct TimeData {
        float time; //ms
        complex value;
    };

    explicit Source(QObject *parent = nullptr);
    virtual Source *clone() const = 0;

    Q_INVOKABLE void destroy()
    {
        deleteLater();   //Schedules ~Source() from qml
    }

    bool active() const noexcept
    {
        return m_active;
    }
    virtual void setActive(bool active);

    QString name() const noexcept
    {
        return m_name;
    }
    void setName(QString name);

    QColor color() const noexcept
    {
        return m_color;
    }
    void setColor(QColor color);
    Q_INVOKABLE bool isColorValid()
    {
        return m_color.isValid();
    }

    const unsigned int &size() const  noexcept;
    const float &frequency(const unsigned int &i) const noexcept;
    virtual float module(const unsigned int &i) const noexcept;
    virtual float magnitude(const unsigned int &i) const noexcept;
    virtual float magnitudeRaw(const unsigned int &i) const noexcept;
    virtual complex phase(const unsigned int &i) const noexcept;
    virtual const float &coherence(const unsigned int &i) const noexcept;
    const float &peakSquared(const unsigned int &i) const noexcept;
    float crestFactor(const unsigned int &i) const noexcept;

    virtual unsigned int impulseSize() const noexcept;
    virtual float impulseTime(const unsigned int &i) const noexcept;
    virtual float impulseValue(const unsigned int &i) const noexcept;

    void copy(FTData *dataDist, TimeData *timeDist);
    void copyFrom(size_t dataSize, size_t timeSize, FTData *dataSrc, TimeData *timeSrc);

    void lock()
    {
        m_dataMutex.lock();
    }
    void unlock()
    {
        m_dataMutex.unlock();
    }

    virtual Q_INVOKABLE QJsonObject toJSON(const SourceList * = nullptr) const noexcept = 0;
    virtual void fromJSON(QJsonObject data, const SourceList * = nullptr) noexcept = 0;

    QUuid uuid() const;

signals:
    void activeChanged();
    void nameChanged(QString);
    void colorChanged(QColor);
    void readyRead();

public slots:
    void setGlobalColor(int globalValue);

protected:
    QString m_name;
    QColor m_color;

    std::mutex m_dataMutex;   //NOTE: shared_mutex (C++17)
    std::atomic<bool> m_onReset;
    FTData *m_ftdata;
    TimeData *m_impulseData;

    unsigned int m_dataLength;
    unsigned int m_deconvolutionSize;
    bool m_active;
    const float m_zero{0.f};

private:
    QUuid m_uuid;
};
}
#endif // SOURCE_H
