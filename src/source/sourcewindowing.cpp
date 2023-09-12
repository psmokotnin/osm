/**
 *  OSM
 *  Copyright (C) 2023  Pavel Smokotnin

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

#include "sourcewindowing.h"
#include "sourcelist.h"
#include "stored.h"

Windowing::Windowing(QObject *parent) : chart::Source(parent), meta::Windowing(),
    m_source(nullptr),
    m_window(WindowFunction::Type::Rectangular, this)
{
    m_name = "Windowing";
    setObjectName("Windowing");
    connect(this, &Windowing::windowFunctionTypeChanged, this, &Windowing::applyAutoName);
    connect(this, &Windowing::sourceChanged, this, &Windowing::applyAutoName);
    connect(this, &Windowing::windowFunctionTypeChanged, this, &Windowing::applyWindowFunctionType);

    connect(this, &Windowing::wideChanged,   this, &Windowing::update);
    connect(this, &Windowing::modeChanged,   this, &Windowing::update);
    connect(this, &Windowing::offsetChanged, this, &Windowing::update);
    connect(this, &Windowing::sourceChanged, this, &Windowing::update);
    connect(this, &Windowing::domainChanged, this, &Windowing::update);
    connect(this, &Windowing::modeChanged,   this, &Windowing::applyAutoWide);
    connect(this, &Windowing::windowFunctionTypeChanged, this, &Windowing::update);

    applyAutoName();
    //own Thread
}

Windowing::~Windowing()
{
}

chart::Source *Windowing::clone() const
{
    auto cloned = new Windowing(parent());
    cloned->setMode(mode());
    cloned->setOffset(offset());
    cloned->setWide(wide());
    cloned->setWindowFunctionType(windowFunctionType());
    cloned->setSource(source());
    return cloned;
}

QJsonObject Windowing::toJSON(const SourceList *list) const noexcept
{
    auto object = Source::toJSON(list);

    object["mode"]                  = mode();
    object["domain"]                = domain();
    object["offset"]                = offset();
    object["wide"]                  = wide();
    object["windowFunctionType"]    = windowFunctionType();

    auto dataSource = source();
    object["source"] = (dataSource ? dataSource->uuid().toString() : "");

    return object;
}

void Windowing::fromJSON(QJsonObject data, const SourceList *list) noexcept
{
    Source::fromJSON(data, list);

    setMode(data["mode"].toInt(mode()));
    setDomain(data["domain"].toInt(domain()));
    setOffset(data["offset"].toDouble(offset()));
    setWide(data["wide"].toDouble(wide()));
    setWindowFunctionType(data["windowFunctionType"].toInt(windowFunctionType()));

    auto sourceUuid = QUuid::fromString(data["source"].toString(""));
    auto connection = std::make_shared<QMetaObject::Connection>();
    auto connection_ptr = connection.get();
    auto fillSources = [ = ]() {
        setSource(list->getByUUid(sourceUuid));
        if (connection) {
            disconnect(*connection.get());
        }
    };
    *connection_ptr = connect(list, &SourceList::loaded, fillSources);
}

void Windowing::applyAutoName()
{
    try {
        setTipName(
            (m_source ? m_source->name() : "none") + " " +
            WindowFunction::TypeMap.at(m_windowFunctionType)
        );
    } catch (std::exception &e) {
        qDebug() << "Windowing" << ":" << __LINE__  << e.what();
    }
}

void Windowing::update()
{
    {
        std::lock_guard<std::mutex> guard(m_dataMutex);
        if (!m_source) {
            return;
        }

        // [1] lock source
        m_source->lock();

        // [2] resize
        resizeData();

        // [3] create impulse and apply Window
        updateFromDomain();

        // [4] ifft
        transform();

        // [5] unlock
        m_source->unlock();
    }
    emit readyRead();
}

void Windowing::applyWindowFunctionType()
{
    m_window.setType(windowFunctionType());
}

void Windowing::applyAutoWide()
{
    if (m_source && m_deconvolutionSize > 0) {
        auto windowSizeMs = 1000.f * m_deconvolutionSize / sampleRate();
        setWide(windowSizeMs / 10.f);
    }
}

void Windowing::resizeData()
{
    Q_ASSERT(m_source);

    auto size = pow(2, m_FFTsizes.at(m_mode));
    if (impulseSize() != size) {
        m_sampleRate = std::round(10.0 / (m_source->impulseTime(1) - m_source->impulseTime(0)));
        m_sampleRate *= 100;
        if (!m_sampleRate) {
            m_sampleRate = 48'000; //TODO: get from source, apply default in the source
        }
        m_deconvolutionSize = size;
        m_impulseData.resize(m_deconvolutionSize);
        m_window.setSize(m_deconvolutionSize);

        // FFT:
        m_dataFT.setSize(m_deconvolutionSize);
        m_dataFT.setType(FourierTransform::Fast);
        m_dataFT.setSampleRate(sampleRate());
        m_dataFT.prepare();

        auto frequencyList = m_dataFT.getFrequencies();
        m_dataLength = frequencyList.size();
        m_ftdata.resize(m_dataLength);

        unsigned int i = 0;
        for (auto frequency : frequencyList) {
            m_ftdata[i++].frequency = frequency;
        }
    }

    //fill frequency data
    int t = 0;
    float kt = 1000.f / sampleRate();
    for (unsigned int i = 0, j = m_deconvolutionSize / 2 - 1; i < m_deconvolutionSize; i++, j++, t++) {
        if (t > static_cast<int>(m_deconvolutionSize / 2)) {
            t -= static_cast<int>(m_deconvolutionSize);
            j -= m_deconvolutionSize;
        }

        m_impulseData[j].time = t * kt;//ms
    }
}

void Windowing::updateFromDomain()
{
    switch (domain()) {
    case Windowing::SourceDomain::Frequency:
        updateFromFrequencyDomain();
        break;
    case Windowing::SourceDomain::Time:
        updateFromTimeDomain(m_source);
        break;
    }
}

void Windowing::updateFromFrequencyDomain()
{
    Q_ASSERT(m_source);

    unsigned last = 0;
    int j = 0;
    float kg, bg, g, g1, g2, f1, f2;
    complex p1, p2, kp, bp, p;
    bool inList = false;

    for (unsigned i = 0; i < size(); ++i) {

        while (frequency(i) > m_source->frequency(j)) {
            last = j;
            if (j + 1 < m_source->size()) {
                ++j;
                inList = true;
            } else {
                inList = false;
                break;
            }
        }

        f1 = m_source->frequency(last);
        g1 = m_source->magnitudeRaw(last);
        p1 = m_source->phase(last);
        f2 = m_source->frequency(j);
        g2 = m_source->magnitudeRaw(j);
        p2 = m_source->phase(j);

        if (inList) {
            kg = (g2 - g1) / (f2 - f1);
            bg = g2 - f2 * kg;

            kp = (p2 - p1) / (f2 - f1);
            bp = p2 - kp * f2;

            g = kg * frequency(i) + bg;
            p = kp * frequency(i) + bp;
        } else {
            g = g2;
            p = p2;
        }

        auto complexMagnitude = i == 0 ? 0 : p * g;
        m_ftdata[i].magnitude = g;
        m_ftdata[i].phase = p;
        m_dataFT.set(                i,     complexMagnitude.conjugate(), 0);
        m_dataFT.set(impulseSize() - i - 1, complexMagnitude,             0);
    }

    // apply iFFT
    m_dataFT.transformSingleChannel();

    //fill time data
    int t = 0;
    float kt = 1000.f / sampleRate();
    float norm = 1 / sqrt(m_deconvolutionSize);
    for (unsigned int i = 0, j = m_deconvolutionSize / 2 - 1; i < m_deconvolutionSize; i++, j++, t++) {

        if (t > static_cast<int>(m_deconvolutionSize / 2)) {
            t -= static_cast<int>(m_deconvolutionSize);
            j -= m_deconvolutionSize;
        }

        m_impulseData[j].value.real = norm * m_dataFT.af(i).real;
        m_impulseData[j].time = t * kt;//ms
    }

    updateFromTimeDomain(this);
}

void Windowing::updateFromTimeDomain(chart::Source *source)
{
    Q_ASSERT(source);

    float dt = 1000.f / m_sampleRate, time = -dt * m_deconvolutionSize / 2;

    //=m_source->impulseZeroOffset()
    auto zeroOffset = source->impulseSize() / 2;

    int center  = zeroOffset + m_offset * m_sampleRate / 1000;
    int from    = center - m_deconvolutionSize / 2;
    int end     = center + m_deconvolutionSize / 2;

    //Tukey Window:
    auto alpha = 0.25f;
    auto sampleWide = m_wide * m_sampleRate / 1000; // wide in samples == N of Tukey window
    auto leftBorder = alpha * sampleWide / 2;
    auto rightBorder = sampleWide - alpha * sampleWide / 2;

    auto tukeyFrom = center - sampleWide / 2;
    auto tukeyEnd  = center + sampleWide / 2;

    auto windowKoefficient = m_window.gain() / m_window.norm();

    for (int i = from, j = 0, f = m_deconvolutionSize / 2 + 1; i < end; ++i, ++j, time += dt, ++f) {

        float value = (i >= 0 ? source->impulseValue(i) * m_window.get(j) * windowKoefficient : 0);

        if (i < tukeyFrom || i > tukeyEnd) {
            value = 0;
        } else {
            auto n = i - tukeyFrom;
            if (n < leftBorder || n > rightBorder) {
                value *= (1 - std::cos(M_PI * n / leftBorder)) / 2;
            }
        }

        m_impulseData[j].value = value;
        if (f == m_deconvolutionSize) {
            f = 0;
        }
        m_dataFT.set(f, value, 0.f);
    }
}

void Windowing::transform()
{
    m_dataFT.transformSingleChannel(true);
    auto m_norm = 1.f / sqrtf(m_deconvolutionSize);

    auto criticalFrequency = 1000 / wide();
    for (unsigned i = 0; i < size(); ++i) {
        if (m_ftdata[i].frequency < criticalFrequency) {
            m_ftdata[i].coherence = 0;
        } else if (m_ftdata[i].frequency > criticalFrequency * 2) {
            m_ftdata[i].coherence = 1;
        } else {
            m_ftdata[i].coherence = (m_ftdata[i].frequency - criticalFrequency) / criticalFrequency;
        }

        m_ftdata[i].magnitude   = m_dataFT.af(i).abs() / m_norm;
        m_ftdata[i].module      = m_ftdata[i].magnitude;
        m_ftdata[i].meanSquared = m_ftdata[i].magnitude * m_ftdata[i].magnitude;
        m_ftdata[i].peakSquared = 0;
        m_ftdata[i].phase       = m_dataFT.af(i);
    }
}

chart::Source *Windowing::source() const
{
    return m_source;
}

void Windowing::setSource(chart::Source *newSource)
{
    if (m_source == newSource) {
        return;
    }

    {
        std::lock_guard<std::mutex> guard(m_dataMutex);
        if (m_source) {
            disconnect(this, nullptr, m_source, nullptr);
        }

        m_source = newSource;

        if (m_source) {
            connect(m_source, &chart::Source::beforeDestroy, this, [this]() {
                setSource(nullptr);
            }, Qt::DirectConnection);

            connect(
                m_source, &chart::Source::readyRead,
                this, &Windowing::update
            );
        }
    } //guard
    emit sourceChanged();
}

QUuid Windowing::sourceId() const
{
    if (m_source) {
        return m_source->uuid();
    }

    return QUuid{};
}

void Windowing::setSource(QUuid id)
{
    auto list = qobject_cast<SourceList *>(parent());
    if ( list) {
        auto source = list->getByUUid(id);
        setSource(source);
    }
}

chart::Source *Windowing::store() noexcept
{
    auto *store = new Stored();
    store->build(this);
    store->autoName(name());


    store->setNotes(
        "Windowing on " + (source() ? source()->name() : "")                + "\n" +
        "Wide:"         + QString("%1").arg(double(wide()),   0, 'f', 2)    + "ms\t" +
        "Offset:"       + QString("%1").arg(double(offset()), 0, 'f', 2)    + "ms\n" +
        "Window: "      + WindowFunction::name(m_windowFunctionType) + "\t" +
        "FFT power: "   + QString::number(m_FFTsizes.at(mode()), 10)        + "\n" +
        "Date: "        + QDateTime::currentDateTime().toString()
    );
    return store;
}

unsigned Windowing::sampleRate() const
{
    return m_sampleRate;
}
