/**
 *  OSM
 *  Copyright (C) 2025  Pavel Smokotnin

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

#include "abstract/levelsdata.h"

namespace Abstract {

LevelsData::LevelsData()  : m_referenceLevel(0)
{
    for (auto &curve : Weighting::allCurves) {
        for (auto &time : Meter::allTimes) {
            Key   key   {curve, time};
            Meter meter {curve, time};
            m_data[key] = -INFINITY;
        }
    }
}

LevelsData::key_map::iterator LevelsData::begin()
{
    return m_data.begin();
}

LevelsData::key_map::iterator LevelsData::end()
{
    return m_data.end();
}

bool LevelsData::Key::operator==(const Key &other) const
{
    return curve == other.curve && time == other.time;
}

std::size_t LevelsData::Key::Hash::operator()(const Key &k) const
{
    return std::hash<size_t>()(k.curve * 10 + k.time);
}

} // namespace Abstract
