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
#include "stored.h"
#include <QUrl>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QtMath>
#include <QtEndian>
#include "common/wavfile.h"

Stored::Stored(QObject *parent) : Abstract::Source(parent), Meta::Stored()
{
    setObjectName("Stored");
    connect(this, &Stored::polarityChanged, this, &Abstract::Source::readyRead);
    connect(this, &Stored::inverseChanged, this, &Abstract::Source::readyRead);
    connect(this, &Stored::ignoreCoherenceChanged, this, &Abstract::Source::readyRead);
    connect(this, &Stored::gainChanged, this, &Abstract::Source::readyRead);
    connect(this, &Stored::delayChanged, this, &Abstract::Source::readyRead);
}

Shared::Source Stored::clone() const
{
    auto cloned = std::make_shared<Stored>(parent());
    cloned->build(*this);
    cloned->setActive(active());
    cloned->setName(name());
    cloned->setInverse(inverse());
    cloned->setIgnoreCoherence(ignoreCoherence());
    cloned->setPolarity(polarity());
    cloned->setDelay(delay());
    cloned->setGain(gain());
    cloned->setNotes(notes());

    return std::static_pointer_cast<Abstract::Source>(cloned);
}

void Stored::build (const Abstract::Source &source)
{
    source.copyTo(*this);
    emit readyRead();
}

void Stored::autoName(const QString &prefix) noexcept
{
    auto time = QTime::currentTime();
    auto name = prefix.mid(0, 7) + time.toString(" @ HH:mm");

    setName(name);
}

QJsonObject Stored::toJSON() const noexcept
{
    auto object = Abstract::Source::toJSON();
    object["notes"]     = notes();

    object["polarity"]  = polarity();
    object["inverse"]   = inverse();
    object["icoherence"] = ignoreCoherence();
    object["delay"]     = delay();
    object["gain"]      = gain();

    QJsonArray ftdata;
    for (unsigned int i = 0; i < frequencyDomainSize(); ++i) {

        //frequecy, module, magnitude, phase, coherence
        QJsonArray ftcell;
        ftcell.append(static_cast<double>(m_ftdata[i].frequency  ));
        ftcell.append(static_cast<double>(m_ftdata[i].module     ));
        ftcell.append(static_cast<double>(m_ftdata[i].magnitude  ));
        ftcell.append(static_cast<double>(m_ftdata[i].phase.arg()));
        ftcell.append(static_cast<double>(m_ftdata[i].coherence  ));
        ftcell.append(static_cast<double>(m_ftdata[i].peakSquared));
        ftcell.append(static_cast<double>(m_ftdata[i].meanSquared));

        ftdata.append(ftcell);
    }
    object["ftdata"] = ftdata;

    QJsonArray impulse;
    for (unsigned int i = 0; i < timeDomainSize(); ++i) {

        //time, value
        QJsonArray impulsecell;
        impulsecell.append(static_cast<double>(m_impulseData[i].time));
        impulsecell.append(static_cast<double>(m_impulseData[i].value.real));
        impulse.append(impulsecell);
    }
    object["impulse"] = impulse;

    return object;
}
void Stored::fromJSON(QJsonObject data, const SourceList *) noexcept
{
    Abstract::Source::fromJSON(data);

    auto ftdata         = data["ftdata"].toArray();
    auto impulse        = data["impulse"].toArray();

    setFrequencyDomainSize(static_cast<unsigned int>(ftdata.count()));
    setTimeDomainSize(     static_cast<unsigned int>(impulse.count()));

    for (int i = 0; i < ftdata.count(); i++) {
        auto row = ftdata[i].toArray();
        if (row.count() > 0) m_ftdata[i].frequency    = static_cast<float>(row[0].toDouble());
        if (row.count() > 1) m_ftdata[i].module       = static_cast<float>(row[1].toDouble());
        if (row.count() > 2) m_ftdata[i].magnitude    = static_cast<float>(row[2].toDouble());
        if (row.count() > 3) m_ftdata[i].phase.polar(   static_cast<float>(row[3].toDouble()));
        if (row.count() > 4) m_ftdata[i].coherence    = static_cast<float>(row[4].toDouble());
        if (row.count() > 5) m_ftdata[i].peakSquared  = static_cast<float>(row[5].toDouble());
        if (row.count() > 6) m_ftdata[i].meanSquared  = static_cast<float>(row[6].toDouble());
    }

    for (int i = 0; i < impulse.count(); i++) {
        auto row = impulse[i].toArray();
        m_impulseData[i].time    = static_cast<float>(row[0].toDouble());
        m_impulseData[i].value   = static_cast<float>(row[1].toDouble());
    }

    setPolarity(data["polarity"].toBool(false));
    setInverse( data["inverse" ].toBool(false));
    setIgnoreCoherence(data["icoherence"].toBool(false));
    setDelay(data["delay"].toDouble(0));
    setGain( data["gain" ].toDouble(0));
    setNotes(data["notes"].toString());
}

bool Stored::save(const QUrl &fileName) const noexcept
{
    QFile saveFile(fileName.toLocalFile());
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }
    QJsonObject object;
    object["type"] = "stored";
    object["data"] = toJSON();

    QJsonDocument document(object);
    if (saveFile.write(document.toJson(QJsonDocument::JsonFormat::Compact)) != -1) {
        return true;
    }

    return false;
}
bool Stored::saveCal(const QUrl &fileName) const noexcept
{
    QFile saveFile(fileName.toLocalFile());
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    unsigned int count = 0;
    float _frequencyFactor = powf(2.f, 1.f / 12);
    float bandStart = 1.f,
          bandEnd   = bandStart * _frequencyFactor,
          lastBandEnd = bandStart,
          ppo_frequency = 0;

    float avg_magnitude = 0;
    float avg_coherence = 0;
    Complex avg_phase = 0;

    QTextStream out(&saveFile);
    for (unsigned int i = 0; i < frequencyDomainSize(); ++i) {

        ppo_frequency = frequency(i);

        if (ppo_frequency < bandStart) continue;

        while (ppo_frequency > bandEnd) {

            if (count) {

                avg_magnitude /= count;
                avg_coherence /= count;
                avg_phase /= count;

                out << (lastBandEnd + bandEnd) / 2 << "\t"
                    << avg_magnitude << "\t"
                    << avg_phase.arg() * 180.f / static_cast<float>(M_PI) << "\t"
                    //<< avg_coherence
                    << "\n";
                count = 0;
                avg_magnitude = 0;
                avg_coherence = 0;
                avg_phase = 0;

                //extend current band to the end of the pervious collected
                lastBandEnd = bandEnd;
            }

            bandStart = bandEnd;
            bandEnd   *= _frequencyFactor;
        }

        if (coherence(i) > 0.97f) {
            count ++;

            avg_magnitude += magnitude(i);
            avg_coherence += coherence(i);
            avg_phase    += phase(i);
        }

    }
    saveFile.close();
    return true;
}

bool Stored::saveFRD(const QUrl &fileName) const noexcept
{
    QFile saveFile(fileName.toLocalFile());
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }
    QTextStream out(&saveFile);
    for (unsigned int i = 0; i < frequencyDomainSize(); ++i) {
        auto m = magnitude(i);
        auto p = m_ftdata[i].phase.arg() * 180.f / static_cast<float>(M_PI);
        if (std::isnormal(m) && std::isnormal(p)) {
            out << m_ftdata[i].frequency << " " << m << " " << p << " " << coherence(i) << "\n";
        }
    }
    saveFile.close();
    return true;
}
bool Stored::saveTXT(const QUrl &fileName) const noexcept
{
    QFile saveFile(fileName.toLocalFile());
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }
    QTextStream out(&saveFile);
    out << "Created with Open Sound Meter\n\n";

    for (unsigned int i = 0; i < frequencyDomainSize(); ++i) {
        auto m = magnitude(i);
        auto p = m_ftdata[i].phase.arg() * 180.f / static_cast<float>(M_PI);
        if (std::isnormal(m) && std::isnormal(p)) {
            out << m_ftdata[i].frequency << "\t" << m << "\t" << p << "\t" << coherence(i) << "\n";
        } else {
            out << m_ftdata[i].frequency << "\t*\t*\t" << coherence(i) << "\n";
        }
    }
    saveFile.close();
    return true;
}

bool Stored::saveCSV(const QUrl &fileName) const noexcept
{
    QFile saveFile(fileName.toLocalFile());
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }
    QTextStream out(&saveFile);

    for (unsigned int i = 0; i < frequencyDomainSize(); ++i) {
        auto m = magnitude(i);
        auto p = m_ftdata[i].phase.arg() * 180.f / static_cast<float>(M_PI);
        if (std::isnormal(m) && std::isnormal(p)) {
            out << m_ftdata[i].frequency << "," << m << "," << p << "," << coherence(i) << "\n";
        } else {
            out << m_ftdata[i].frequency << ",*,*," << coherence(i) << "\n";
        }
    }
    saveFile.close();
    return true;
}

bool Stored::saveWAV(const QUrl &fileName) const noexcept
{
    WavFile file;
    QByteArray data;
    data.resize(timeDomainSize() * 4);
    auto dst = data.data();
    for (unsigned int i = 0; i < timeDomainSize(); ++i, dst += 4) {
        qToLittleEndian(m_impulseData[i].value.real, dst);
    }

    int sampleRate = std::round(10 / std::abs(m_impulseData[1].time - m_impulseData[2].time)) * 100;
    return file.save(fileName.toLocalFile(), sampleRate, data);
}

float Stored::module(unsigned int i) const noexcept {
    return ::Abstract::Source::module(i) * std::pow(10, gain() / 20.f);
}

float Stored::magnitudeRaw(unsigned int i) const noexcept
{
    return std::pow(::Abstract::Source::magnitudeRaw(i), (inverse() ? -1 : 1)) * std::pow(10, gain() / 20.f);
}

float Stored::magnitude(unsigned int i) const noexcept
{
    return (inverse() ? -1 : 1) * (::Abstract::Source::magnitude(i) + gain());
}

Complex Stored::phase(unsigned int i) const noexcept
{
    auto alpha = (polarity() ? M_PI : 0) - 2 * M_PI * delay() * frequency(i) / 1000.f;
    return ::Abstract::Source::phase(i).rotate(alpha);
}

float Stored::coherence(unsigned int i) const noexcept
{
    if (ignoreCoherence()) {
        return 1.f;
    }
    return ::Abstract::Source::coherence(i);
}

float Stored::impulseTime(unsigned int i) const noexcept
{
    return ::Abstract::Source::impulseTime(i) + delay();
}

float Stored::impulseValue(unsigned int i) const noexcept
{
    return (polarity() ? -1 : 1) * ::Abstract::Source::impulseValue(i) * std::pow(10, gain() / 20.f);
}
