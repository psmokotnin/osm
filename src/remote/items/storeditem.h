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
#ifndef REMOTE_STOREDITEM_H
#define REMOTE_STOREDITEM_H

#include "item.h"
#include "meta/metastored.h"

namespace remote {

class StoredItem : public remote::Item, public Meta::Stored
{
    Q_OBJECT
    Q_PROPERTY(QString notes READ notes WRITE setNotes NOTIFY notesChanged)
    Q_PROPERTY(bool polarity READ polarity WRITE setPolarity NOTIFY polarityChanged)
    Q_PROPERTY(bool inverse READ inverse WRITE setInverse NOTIFY inverseChanged)
    Q_PROPERTY(bool ignoreCoherence READ ignoreCoherence WRITE setIgnoreCoherence NOTIFY ignoreCoherenceChanged)
    Q_PROPERTY(float gain READ gain WRITE setGain NOTIFY gainChanged)
    Q_PROPERTY(float delay READ delay WRITE setDelay NOTIFY delayChanged)

public:
    StoredItem(QObject *parent = nullptr);

signals:
    void notesChanged() override;
    void polarityChanged() override;
    void inverseChanged() override;
    void gainChanged() override;
    void delayChanged() override;
    void ignoreCoherenceChanged() override;
};

} // namespace remote

#endif // REMOTE_STOREDITEM_H
