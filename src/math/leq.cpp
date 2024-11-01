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
#include "leq.h"

namespace math {

Leq::Leq() : m_integration((*m_timeMap.begin()).first, 12)
{

}

void Leq::addOneSecondValue(const float value)
{
    m_integration.add_value(std::pow(10.f, value / 10));
}

QVariant Leq::availableTimes()
{
    QStringList typeList;
    for (const auto &type : m_timeMap) {
        typeList << type.second;
    }
    typeList.sort();
    return typeList;
}

QString Leq::timeName() const
{
    return m_timeMap.at(m_integration.size());
}

void Leq::setTime(const QString &time)
{
    auto it = std::find_if(m_timeMap.begin(), m_timeMap.end(), [time](const auto & el) {
        return el.second == time;
    });

    if (it != m_timeMap.end()) {
        m_integration.setSize(it->first);
    }
}

float Leq::value() const
{
    return 10.f * std::log10(m_integration.value() / m_integration.size());
}

const std::unordered_map<std::size_t, QString> Leq::m_timeMap = {
    {   1 * 60, "  1 min" },
    {   5 * 60, "  5 min" },
    {  10 * 60, " 10 min" },
    {  15 * 60, " 15 min" },
    {  30 * 60, " 30 min" },
    {  60 * 60, " 60 min" },
    { 120 * 60, "120 min" },
};

} // namespace math
