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

Stored::Stored(QObject *parent) : chart::Source(parent), meta::Stored()
{
    setObjectName("Stored");
    connect(this, &Stored::polarityChanged, this, &Source::readyRead);
    connect(this, &Stored::inverseChanged, this, &Source::readyRead);
    connect(this, &Stored::ignoreCoherenceChanged, this, &Source::readyRead);
    connect(this, &Stored::gainChanged, this, &Source::readyRead);
    connect(this, &Stored::delayChanged, this, &Source::readyRead);
}

chart::Source *Stored::clone() const
{
    auto cloned = new Stored(parent());
    cloned->build(const_cast<Stored *>(this));
    cloned->setActive(active());
    cloned->setName(name());
    cloned->setInverse(inverse());
    cloned->setIgnoreCoherence(ignoreCoherence());
    cloned->setPolarity(polarity());
    cloned->setDelay(delay());
    cloned->setGain(gain());
    cloned->setNotes(notes());
    return cloned;
}

void Stored::build (chart::Source *source)
{
    source->lock();
    m_dataLength = source->size();
    m_deconvolutionSize = source->impulseSize();
    m_ftdata.resize(m_dataLength);
    m_impulseData.resize(m_deconvolutionSize);
    source->copy(m_ftdata.data(), m_impulseData.data());
    source->unlock();
    emit readyRead();
}

void Stored::autoName(const QString &prefix) noexcept
{
    auto time = QTime::currentTime();
    auto name = prefix.mid(0, 7) + time.toString(" @ HH:mm");

    setName(name);
}

QJsonObject Stored::toJSON(const SourceList *list) const noexcept
{
    auto object = Source::toJSON(list);
    object["notes"]     = notes();

    object["polarity"]  = polarity();
    object["inverse"]   = inverse();
    object["icoherence"] = ignoreCoherence();
    object["delay"]     = delay();
    object["gain"]      = gain();

    QJsonArray ftdata;
    for (unsigned int i = 0; i < m_dataLength; ++i) {

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
    for (unsigned int i = 0; i < m_deconvolutionSize; ++i) {

        //time, value
        QJsonArray impulsecell;
        impulsecell.append(static_cast<double>(m_impulseData[i].time));
        impulsecell.append(static_cast<double>(m_impulseData[i].value.real));
        impulse.append(impulsecell);
    }
    object["impulse"] = impulse;

    return object;
}
void Stored::fromJSON(QJsonObject data, const SourceList *list) noexcept
{
    Source::fromJSON(data, list);

    auto ftdata         = data["ftdata"].toArray();
    auto impulse        = data["impulse"].toArray();

    m_dataLength         = static_cast<unsigned int>(ftdata.count());
    m_deconvolutionSize = static_cast<unsigned int>(impulse.count());
    m_ftdata.resize(m_dataLength);
    m_impulseData.resize(m_deconvolutionSize);

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
    QTextStream out(&saveFile);
    for (unsigned int i = 0; i < m_dataLength; ++i) {
        out << m_ftdata[i].frequency << "\t"
            << magnitude(i) << "\t"
            << m_ftdata[i].phase.arg() * 180.f / static_cast<float>(M_PI) << "\n";


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
    for (unsigned int i = 0; i < m_dataLength; ++i) {
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

    for (unsigned int i = 0; i < m_dataLength; ++i) {
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

    for (unsigned int i = 0; i < m_dataLength; ++i) {
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
    data.resize(m_deconvolutionSize * 4);
    auto dst = data.data();
    for (unsigned int i = 0; i < m_deconvolutionSize; ++i, dst += 4) {
        qToLittleEndian(m_impulseData[i].value.real, dst);
    }

    int sampleRate = std::round(10 / std::abs(m_impulseData[1].time - m_impulseData[2].time)) * 100;
    return file.save(fileName.toLocalFile(), sampleRate, data);
}

float Stored::module(const unsigned int &i) const noexcept {
    return Source::module(i) * std::pow(10, gain() / 20.f);
}

float Stored::magnitudeRaw(const unsigned int &i) const noexcept
{
    return std::pow(Source::magnitudeRaw(i), (inverse() ? -1 : 1)) * std::pow(10, gain() / 20.f);
}

float Stored::magnitude(const unsigned int &i) const noexcept
{
    return (inverse() ? -1 : 1) * (Source::magnitude(i) + gain());
}

complex Stored::phase(const unsigned int &i) const noexcept
{
    auto alpha = (polarity() ? M_PI : 0) - 2 * M_PI * delay() * frequency(i) / 1000.f;
    return Source::phase(i).rotate(alpha);
}

const float &Stored::coherence(const unsigned int &i) const noexcept
{
    if (ignoreCoherence()) {
        static const float one = 1.f;
        return one;
    }
    return Source::coherence(i);
}

float Stored::impulseTime(const unsigned int &i) const noexcept
{
    return Source::impulseTime(i) + delay();
}

float Stored::impulseValue(const unsigned int &i) const noexcept
{
    return (polarity() ? -1 : 1) * Source::impulseValue(i) * std::pow(10, gain() / 20.f);;
}
