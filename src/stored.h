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

#include "chart/source.h"
#include <QJsonObject>

class Stored: public chart::Source
{
    Q_OBJECT
    Q_PROPERTY(QString notes READ notes WRITE setNotes NOTIFY notesChanged)
    Q_PROPERTY(bool polarity READ polarity WRITE setPolarity NOTIFY polarityChanged)
    Q_PROPERTY(bool inverse READ inverse WRITE setInverse NOTIFY inverseChanged)
    Q_PROPERTY(float gain READ gain WRITE setGain NOTIFY gainChanged)
    Q_PROPERTY(float delay READ delay WRITE setDelay NOTIFY delayChanged)

public:
    explicit Stored(QObject *parent = nullptr);
    Source *clone() const override;
    void build (chart::Source *source);

    Q_INVOKABLE bool save(const QUrl &fileName) const noexcept;
    Q_INVOKABLE bool saveCal(const QUrl &fileName) const noexcept;
    Q_INVOKABLE bool saveFRD(const QUrl &fileName) const noexcept;
    Q_INVOKABLE bool saveTXT(const QUrl &fileName) const noexcept;
    Q_INVOKABLE bool saveCSV(const QUrl &fileName) const noexcept;
    Q_INVOKABLE QJsonObject toJSON(const SourceList * = nullptr) const noexcept override;
    void fromJSON(QJsonObject data, const SourceList * = nullptr) noexcept override;

    QString notes() const noexcept
    {
        return m_notes;
    }
    void setNotes(const QString &notes) noexcept;

    bool polarity() const;
    void setPolarity(bool polarity);

    bool inverse() const;
    void setInverse(bool inverse);

    float gain() const;
    void setGain(float gain);

    float delay() const;
    void setDelay(float delay);

    float module(const unsigned int &i) const noexcept override;
    float magnitudeRaw(const unsigned int &i) const noexcept override;
    float magnitude(const unsigned int &i) const noexcept override;
    complex phase(const unsigned int &i) const noexcept override;

    float impulseTime(const unsigned int &i) const noexcept override;
    float impulseValue(const unsigned int &i) const noexcept override;

signals:
    void notesChanged();
    void polarityChanged();
    void inverseChanged();
    void gainChanged();
    void delayChanged();

private:
    QString m_notes;
    bool m_polarity;
    bool m_inverse;
    float m_gain;
    float m_delay;
};

#endif // STORED_H
