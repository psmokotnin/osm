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

#ifndef ABSTRACT_LEVELSDATA_H
#define ABSTRACT_LEVELSDATA_H

#include <cstdlib>
#include <unordered_map>

#include "math/meter.h"

namespace Abstract {

struct LevelsData {

    struct Key {
        Weighting::Curve curve;
        Meter::Time time;

        bool operator==(const Key &other) const;
        struct Hash {
            std::size_t operator()(const Key &k) const;
        };
    };

    typedef std::unordered_map<Key, float, Key::Hash> key_map;

    LevelsData();

    key_map::iterator begin();
    key_map::iterator end();

    key_map     m_data;
    float       m_referenceLevel;
};

} // namespace Abstract

#endif // ABSTRACT_LEVELSDATA_H
