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
#ifndef STORED_H
#define STORED_H

#include <QJsonObject>
#include "chart/source.h"
#include "meta/metastored.h"

class Stored: public chart::Source, public meta::Stored
{
    Q_OBJECT
    Q_PROPERTY(QString notes READ notes WRITE setNotes NOTIFY notesChanged)
    Q_PROPERTY(bool polarity READ polarity WRITE setPolarity NOTIFY polarityChanged)
    Q_PROPERTY(bool inverse READ inverse WRITE setInverse NOTIFY inverseChanged)
    Q_PROPERTY(bool ignoreCoherence READ ignoreCoherence WRITE setIgnoreCoherence NOTIFY ignoreCoherenceChanged)
    Q_PROPERTY(float gain READ gain WRITE setGain NOTIFY gainChanged)
    Q_PROPERTY(float delay READ delay WRITE setDelay NOTIFY delayChanged)

public:
    explicit Stored(QObject *parent = nullptr);
    Source *clone() const override;
    void build (chart::Source *source);

    Q_INVOKABLE void autoName(const QString &prefix) noexcept;

    Q_INVOKABLE bool save(const QUrl &fileName) const noexcept;
    Q_INVOKABLE bool saveCal(const QUrl &fileName) const noexcept;
    Q_INVOKABLE bool saveFRD(const QUrl &fileName) const noexcept;
    Q_INVOKABLE bool saveTXT(const QUrl &fileName) const noexcept;
    Q_INVOKABLE bool saveCSV(const QUrl &fileName) const noexcept;
    Q_INVOKABLE bool saveWAV(const QUrl &fileName) const noexcept;
    Q_INVOKABLE QJsonObject toJSON(const SourceList * = nullptr) const noexcept override;
    void fromJSON(QJsonObject data, const SourceList * = nullptr) noexcept override;

    float module(const unsigned int &i) const noexcept override;
    float magnitudeRaw(const unsigned int &i) const noexcept override;
    float magnitude(const unsigned int &i) const noexcept override;
    complex phase(const unsigned int &i) const noexcept override;
    const float &coherence(const unsigned int &i) const noexcept override;

    float impulseTime(const unsigned int &i) const noexcept override;
    float impulseValue(const unsigned int &i) const noexcept override;

signals:
    void notesChanged() override;
    void polarityChanged() override;
    void inverseChanged() override;
    void ignoreCoherenceChanged() override;
    void gainChanged() override;
    void delayChanged() override;
};

#endif // STORED_H
