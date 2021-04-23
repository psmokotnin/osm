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

Stored::Stored(QObject *parent) : chart::Source(parent), m_notes(),
    m_polarity(false), m_inverse(false), m_gain(0), m_delay(0)
{
    setObjectName("Stored");
    connect(this, &Stored::polarityChanged, this, &Source::readyRead);
    connect(this, &Stored::inverseChanged, this, &Source::readyRead);
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
    m_ftdata = new FTData[m_dataLength];
    m_impulseData = new TimeData[m_deconvolutionSize];
    source->copy(m_ftdata, m_impulseData);
    source->unlock();
    emit readyRead();
}
QJsonObject Stored::toJSON() const noexcept
{
    QJsonObject object;
    object["active"]    = active();
    object["name"]      = name();
    object["notes"]     = notes();

    QJsonObject color;
    color["red"]    = m_color.red();
    color["green"]  = m_color.green();
    color["blue"]   = m_color.blue();
    color["alpha"]  = m_color.alpha();
    object["color"] = color;

    QJsonArray ftdata;
    for (unsigned int i = 0; i < m_dataLength; ++i) {

        //frequecy, module, magnitude, phase, coherence
        QJsonArray ftcell;
        ftcell.append(static_cast<double>(m_ftdata[i].frequency  ));
        ftcell.append(static_cast<double>(m_ftdata[i].module     ));
        ftcell.append(static_cast<double>(m_ftdata[i].magnitude  ));
        ftcell.append(static_cast<double>(m_ftdata[i].phase.arg()));
        ftcell.append(static_cast<double>(m_ftdata[i].coherence  ));

        ftdata.append(ftcell);
    }
    object["ftdata"] = ftdata;

    QJsonArray impulse;
    for (unsigned int i = 0; i < m_deconvolutionSize; ++i) {

        //time, value
        QJsonArray impulsecell;
        impulsecell.append(static_cast<double>(impulseTime(i)));
        impulsecell.append(static_cast<double>(impulseValue(i)));
        impulse.append(impulsecell);
    }
    object["impulse"] = impulse;

    return object;
}
void Stored::fromJSON(QJsonObject data) noexcept
{
    auto ftdata         = data["ftdata"].toArray();
    auto impulse        = data["impulse"].toArray();

    m_dataLength         = static_cast<unsigned int>(ftdata.count());
    m_deconvolutionSize = static_cast<unsigned int>(impulse.count());
    m_ftdata             = new FTData[m_dataLength];
    m_impulseData        = new TimeData[m_deconvolutionSize];

    for (int i = 0; i < ftdata.count(); i++) {
        auto row = ftdata[i].toArray();
        m_ftdata[i].frequency    = static_cast<float>(row[0].toDouble());
        m_ftdata[i].module       = static_cast<float>(row[1].toDouble());
        m_ftdata[i].magnitude    = static_cast<float>(row[2].toDouble());
        m_ftdata[i].phase.polar(   static_cast<float>(row[3].toDouble()));
        m_ftdata[i].coherence    = static_cast<float>(row[4].toDouble());
    }

    for (int i = 0; i < impulse.count(); i++) {
        auto row = impulse[i].toArray();
        m_impulseData[i].time    = static_cast<float>(row[0].toDouble());
        m_impulseData[i].value   = static_cast<float>(row[1].toDouble());
    }

    auto jsonColor = data["color"].toObject();
    QColor c(
        jsonColor["red"  ].toInt(0),
        jsonColor["green"].toInt(0),
        jsonColor["blue" ].toInt(0),
        jsonColor["alpha"].toInt(1));
    setColor(c);
    setName(data["name"].toString());
    setNotes(data["notes"].toString());
    setActive(data["active"].toBool(active()));
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
        }
    }
    saveFile.close();
    return true;
}
void Stored::setNotes(const QString &notes) noexcept
{
    if (m_notes != notes) {
        m_notes = notes;
        emit notesChanged();
    }
}

bool Stored::polarity() const
{
    return m_polarity;
}

void Stored::setPolarity(bool polarity)
{
    if (m_polarity != polarity) {
        m_polarity = polarity;
        emit polarityChanged();
    }
}

bool Stored::inverse() const
{
    return m_inverse;
}

void Stored::setInverse(bool inverse)
{
    if (m_inverse != inverse) {
        m_inverse = inverse;
        emit inverseChanged();
    }
}

float Stored::gain() const
{
    return m_gain;
}

void Stored::setGain(float gain)
{
    if (m_gain != gain) {
        m_gain = gain;
        emit gainChanged();
    }
}

float Stored::delay() const
{
    return m_delay;
}

void Stored::setDelay(float delay)
{
    if (m_delay != delay) {
        m_delay = delay;
        emit delayChanged();
    }
}

float Stored::module(const unsigned int &i) const noexcept {
    return Source::module(i) * std::pow(10, m_gain / 20.f);
}

float Stored::magnitudeRaw(const unsigned int &i) const noexcept
{
    return Source::magnitudeRaw(i) * std::pow(10, m_gain / 20.f);
}

float Stored::magnitude(const unsigned int &i) const noexcept
{
    return (inverse() ? -1 : 1) * Source::magnitude(i) + m_gain;
}

complex Stored::phase(const unsigned int &i) const noexcept
{
    auto alpha = (m_polarity ? M_PI : 0) - 2 * M_PI * m_delay * frequency(i) / 1000.f;
    return Source::phase(i).rotate(alpha);
}

float Stored::impulseTime(const unsigned int &i) const noexcept
{
    return Source::impulseTime(i) + m_delay;
}

float Stored::impulseValue(const unsigned int &i) const noexcept
{
    return (m_polarity ? -1 : 1) * Source::impulseValue(i) * std::pow(10, m_gain / 20.f);;
}
