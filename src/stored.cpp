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

Stored::Stored(QObject *parent) : Fftchart::Source(parent), m_notes()
{
    setObjectName("Stored");
}
void Stored::build (Fftchart::Source *source)
{
    source->lock();
    _dataLength = source->size();
    m_deconvolutionSize = source->impulseSize();
    setFftSize(source->fftSize());
    _ftdata = new FTData[_dataLength];
    _impulseData = new TimeData[m_deconvolutionSize];
    source->copy(_ftdata, _impulseData);
    source->unlock();
    emit readyRead();
}
QJsonObject Stored::toJSON() const noexcept
{
    QJsonObject object;
    object["active"]    = active();
    object["name"]      = name();
    object["notes"]     = notes();
    object["fftSize"]   = static_cast<int>(fftSize());

    QJsonObject color;
    color["red"]    = _color.red();
    color["green"]  = _color.green();
    color["blue"]   = _color.blue();
    color["alpha"]  = _color.alpha();
    object["color"] = color;

    QJsonArray ftdata;
    for (unsigned int i = 0; i < _dataLength; ++i) {

        //frequecy, module, magnitude, phase, coherence
        QJsonArray ftcell;
        ftcell.append(static_cast<double>(_ftdata[i].frequency  ));
        ftcell.append(static_cast<double>(_ftdata[i].module     ));
        ftcell.append(static_cast<double>(_ftdata[i].magnitude  ));
        ftcell.append(static_cast<double>(_ftdata[i].phase.arg()));
        ftcell.append(static_cast<double>(_ftdata[i].coherence  ));

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

    _dataLength         = static_cast<unsigned int>(ftdata.count());
    m_deconvolutionSize = static_cast<unsigned int>(impulse.count());
    _ftdata             = new FTData[_dataLength];
    _impulseData        = new TimeData[m_deconvolutionSize];

    setFftSize(static_cast<unsigned int>(data["fftSize"].toInt()));

    for (int i = 0; i < ftdata.count(); i++) {
        auto row = ftdata[i].toArray();
        _ftdata[i].frequency    = static_cast<float>(row[0].toDouble());
        _ftdata[i].module       = static_cast<float>(row[1].toDouble());
        _ftdata[i].magnitude    = static_cast<float>(row[2].toDouble());
        _ftdata[i].phase.polar(   static_cast<float>(row[3].toDouble()));
        _ftdata[i].coherence    = static_cast<float>(row[4].toDouble());
    }

    for (int i = 0; i < impulse.count(); i++) {
        auto row = impulse[i].toArray();
        _impulseData[i].time    = static_cast<float>(row[0].toDouble());
        _impulseData[i].value   = static_cast<float>(row[1].toDouble());
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
    for (unsigned int i = 0; i < _dataLength; ++i) {
        out << _ftdata[i].frequency << "\t"
            << -1 * magnitude(i) << "\t"
            << _ftdata[i].phase.arg() * -180.f / static_cast<float>(M_PI) << "\n";


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
    for (unsigned int i = 1; i < _dataLength; ++i) {
        auto m = magnitude(i);
        auto p = _ftdata[i].phase.arg() * -180.f / static_cast<float>(M_PI);
        if (std::isnormal(m) && std::isnormal(p)) {
            out << _ftdata[i].frequency << " " << m << " " << p << " " << coherence(i) << "\n";
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
