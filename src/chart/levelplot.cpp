/**
 *  OSM
 *  Copyright (C) 2022  Pavel Smokotnin

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
#include "levelplot.h"

namespace Chart {

const std::map<LevelPlot::Type, QString> LevelPlot::m_typesMap = {
    { LevelPlot::Type::RMS,   "RMS" },
    { LevelPlot::Type::Leq,   "Leq" },
};

LevelPlot::LevelPlot(Settings *settings, QQuickItem *parent): XYPlot(settings, parent), LevelObject()
{
    setFlag(QQuickItem::ItemHasContents);

    m_x.configure(AxisType::Linear, -60.f,  0.f,  30);
    m_x.setCentralLabel(1);

    m_y.configure(AxisType::Linear, 20.f, 140.f,  20);
    m_y.setCentralLabel(m_y.min() - 1.f);
    m_y.setReset(62.f, 140.f);
    m_y.reset();

    m_x.setUnit("s");
    m_y.setUnit("dB");

    connect(this, &LevelPlot::modeChanged, this, &LevelPlot::updateAxes);

    updateAxes();
}

void LevelPlot::setSettings(Settings *settings) noexcept
{
    if (settings && (settings->value("type") == "Level")) {
        XYPlot::setSettings(settings);

        setMode(
            m_settings->reactValue<LevelPlot, Mode>("mode", this, &LevelPlot::modeChanged, mode()).toInt());
        setCurve(m_settings->reactValue<LevelPlot, QString>("curve", this, &LevelPlot::curveChanged, curveName()).toString());
        setTime(m_settings->reactValue<LevelPlot, QString>("time",   this, &LevelPlot::timeChanged,  timeName() ).toString());

    }
}

void LevelPlot::storeSettings() noexcept
{
    if (!m_settings)
        return;

    XYPlot::storeSettings();
    m_settings->setValue("type", "Level");

    m_settings->setValue("mode", mode());
    m_settings->setValue("curve", curveName());
    m_settings->setValue("time", timeName());
}

QVariant LevelPlot::getAvailableTypes() const
{
    QStringList typeList;
    for (const auto &type : m_typesMap) {
        typeList << type.second;
    }
    return typeList;
}

QVariant LevelPlot::getAvailableTimes() const
{
    switch (m_type) {
    case Type::Leq:
        return math::Leq::availableTimes();
    default:
        break;
    }
    return LevelObject::getAvailableTimes();
}

QString LevelPlot::timeName() const
{
    switch (m_type) {
    case Type::Leq:
        return m_leq.timeName();
    default:
        break;
    }
    return LevelObject::timeName();
}

void LevelPlot::setTime(const QString &time)
{
    switch (m_type) {
    case Leq:
        m_leq.setTime(time);
        emit timeChanged(timeName());
        break;
    default:
        LevelObject::setTime(time);
        break;
    }
}

void LevelPlot::updateAxes()
{
    switch (mode()) {
    case SPL:
        m_y.configure(AxisType::Linear, 20.f, 140.f,  20);
        m_y.setCentralLabel(m_y.min() - 1.f);
        m_y.setReset(62.f, 140.f);
        m_y.reset();
        break;

    case dBfs:
        m_y.configure(AxisType::Linear, -144.f, 0.f,  24);
        m_y.setCentralLabel(m_y.min() - 1.f);
        m_y.setReset(-140.f, 0.f);
        m_y.reset();
        break;
    }
    update();
}

LevelPlot::Type LevelPlot::type() const
{
    return m_type;
}

QString LevelPlot::typeName() const noexcept
{
    try {
        return m_typesMap.at(m_type);
    } catch (std::out_of_range) {}

    return "";
}

void LevelPlot::setType(const Type &newType)
{
    if (m_type == newType)
        return;
    m_type = newType;
    emit typeChanged();
    emit timeChanged(timeName());
    if (m_type == Leq) {
        setMode(SPL);
        m_x.configure(AxisType::Linear, -600.f,  0.f,  30);
    } else {
        m_x.configure(AxisType::Linear, -60.f,  0.f,  30);
    }
}

void LevelPlot::setType(const QString &type)
{
    std::find_if(m_typesMap.cbegin(), m_typesMap.cend(),
    [&type, this](auto & e) {
        if (e.second == type) {
            setType(e.first);
            return true;
        }
        return false;
    });
}

} // namespace chart
