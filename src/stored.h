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

class Stored: public Fftchart::Source
{
    Q_OBJECT
    Q_PROPERTY(QString notes READ notes WRITE setNotes NOTIFY notesChanged)

    QString m_notes;

public:
    explicit Stored(QObject *parent = nullptr);
    void build (Fftchart::Source *source);

    Q_INVOKABLE bool save(const QUrl &fileName) const noexcept;
    Q_INVOKABLE bool saveCal(const QUrl &fileName) const noexcept;
    Q_INVOKABLE bool saveFRD(const QUrl &fileName) const noexcept;
    Q_INVOKABLE QJsonObject toJSON() const noexcept override;
    void fromJSON(QJsonObject data) noexcept override;

    QString notes() const noexcept {return m_notes;}
    void setNotes(const QString &notes) noexcept;

signals:
    void notesChanged();
};

#endif // STORED_H
