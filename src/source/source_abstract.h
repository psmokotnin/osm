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

#include <mutex>
#include <unordered_map>

#include <QObject>
#include <QColor>
#include <QJsonObject>

#include "math/complex.h"
#include "math/meter.h"
#include "source/source_shared.h"
#include "abstract/source.h"

class SourceList;

namespace Source {
class Abstract : public ::Abstract::Source
{

    Q_OBJECT

public:
    struct FTData {
        float frequency = 0;
        float module = 0;
        float magnitude = 0;
        complex phase = 0;
        float coherence = 0;
        float peakSquared = 0;
        float meanSquared = NAN;
    };

    struct TimeData {
        float time; //ms
        complex value;
    };

    explicit Abstract(QObject *parent = nullptr);
    virtual ~Abstract();
    virtual ::Source::Shared clone() const = 0;

    virtual Q_INVOKABLE void destroy();

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

    virtual Q_INVOKABLE QJsonObject toJSON(const SourceList * = nullptr) const noexcept;
    virtual void fromJSON(QJsonObject data, const SourceList * = nullptr) noexcept;

    virtual float level(const Weighting::Curve curve = Weighting::Z, const Meter::Time time = Meter::Fast) const;
    virtual float peak(const Weighting::Curve curve = Weighting::Z, const Meter::Time time = Meter::Fast) const;
    virtual float referenceLevel() const;
    virtual QJsonObject levels();
    virtual void setLevels(const QJsonObject &data);

    Q_INVOKABLE virtual ::Source::Shared store();

signals:
    void readyRead();
    void beforeDestroy(::Source::Abstract *);//TODO: check if still needed

protected:

    std::mutex m_dataMutex;   //NOTE: shared_mutex (C++17)
    std::atomic<bool>       m_onReset; //move to measurement
    std::vector<FTData>     m_ftdata;
    std::vector<TimeData>   m_impulseData;

    unsigned int m_dataLength;
    unsigned int m_deconvolutionSize;
    const float m_zero{0.f};

    struct Levels {
        Levels();

        auto begin();
        auto end();

        struct Key {
            Weighting::Curve curve;
            Meter::Time time;

            bool operator==(const Key &other) const
            {
                return curve == other.curve && time == other.time;
            }
            struct Hash {
                std::size_t operator()(const Key &k) const
                {
                    return std::hash<size_t>()(k.curve * 10 + k.time);
                }
            };
        };
        std::unordered_map<Key, float, Key::Hash> m_data;
        float m_referenceLevel;
    } m_levelsData;
};
}
#endif // SOURCE_H
