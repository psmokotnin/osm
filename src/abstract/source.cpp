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

#include "source.h"

namespace Abstract {

Source::Source(QObject *parent) : QObject{ parent },
    m_name          {},
    m_color         {},
    m_uuid          { QUuid::createUuid() },
    m_sampleRate    { 48000 },
    m_active        { false },
    m_cloneable     { true  }
{
    qRegisterMetaType<::Abstract::Source *>("AbstractSource*");
}

QString Source::name() const
{
    return m_name;
}

void Source::setName(const QString &newName)
{
    if (m_name == newName)
        return;
    m_name = newName;
    emit nameChanged(m_name);
}

QColor Source::color() const
{
    return m_color;
}

void Source::setColor(const QColor &newColor)
{
    if (m_color == newColor)
        return;
    m_color = newColor;
    emit colorChanged(m_color);
}

bool Source::isColorValid()
{
    return m_color.isValid();
}

QUuid Source::uuid() const
{
    return m_uuid;
}

void Source::setUuid(const QUuid &newUuid)
{
    m_uuid = newUuid;
}

unsigned int Source::sampleRate() const
{
    return m_sampleRate;
}

void Source::setSampleRate(unsigned int newSampleRate)
{
    if (m_sampleRate == newSampleRate) {
        return;
    }

    m_sampleRate = newSampleRate;
    emit sampleRateChanged(m_sampleRate);
}

bool Source::active() const
{
    return m_active;
}

void Source::setActive(bool newActive)
{
    if (m_active == newActive)
        return;
    m_active = newActive;
    emit activeChanged();
}

bool Source::cloneable() const
{
    return m_cloneable;
}

void Source::setGlobalColor(int globalValue)
{
    if (globalValue < 19) {
        setColor(Qt::GlobalColor(globalValue));
    }
}

} // namespace Abstract
