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
#ifndef META_STORED_H
#define META_STORED_H

#include <QObject>
#include "metabase.h"

namespace Meta {

class Stored : public Base
{
public:
    Stored();

    QString notes() const;
    void setNotes(const QString &notes);

    bool polarity() const;
    void setPolarity(bool polarity);

    bool inverse() const;
    void setInverse(bool inverse);

    bool ignoreCoherence() const;
    void setIgnoreCoherence(bool ignoreCoherence);

    float gain() const;
    void setGain(float gain);

    float delay() const;
    void setDelay(float delay);

//virtual signals:
    virtual void notesChanged() = 0;
    virtual void polarityChanged() = 0;
    virtual void inverseChanged() = 0;
    virtual void gainChanged() = 0;
    virtual void delayChanged() = 0;
    virtual void ignoreCoherenceChanged() = 0;

private:
    QString m_notes;
    bool m_polarity;
    bool m_inverse;
    bool m_ignoreCoherence;
    float m_gain;
    float m_delay;
};

} // namespace meta

#endif // META_STORED_H
