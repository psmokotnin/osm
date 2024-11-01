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
#include "metastored.h"

namespace Meta {

Stored::Stored() : Base(),
    m_notes(), m_polarity(false), m_inverse(false),
    m_ignoreCoherence(false), m_gain(0), m_delay(0)
{

}

QString Stored::notes() const
{
    return m_notes;
}

void Stored::setNotes(const QString &notes)
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

bool Stored::ignoreCoherence() const
{
    return m_ignoreCoherence;
}

void Stored::setIgnoreCoherence(bool ignoreCoherence)
{
    if (m_ignoreCoherence != ignoreCoherence) {
        m_ignoreCoherence = ignoreCoherence;
        emit ignoreCoherenceChanged();
    }
}

float Stored::gain() const
{
    return m_gain;
}

void Stored::setGain(float gain)
{
    if (!qFuzzyCompare(m_gain, gain)) {
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
    if (!qFuzzyCompare(m_delay, delay)) {
        m_delay = delay;
        emit delayChanged();
    }
}

} // namespace meta
