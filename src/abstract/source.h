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

#ifndef ABSTRACT_SOURCE_H
#define ABSTRACT_SOURCE_H

#include <atomic>

#include <QObject>
#include <QColor>
#include <QJsonObject>
#include <QUuid>

#include "abstract/data.h"
#include "shared/source_shared.h"

class SourceList;

namespace Abstract {

class Source : public QObject, public Data
{
    Q_OBJECT
    Q_PROPERTY(QString  name       READ name       WRITE setName       NOTIFY nameChanged)
    Q_PROPERTY(QColor   color      READ color      WRITE setColor      NOTIFY colorChanged)
    Q_PROPERTY(bool     active     READ active     WRITE setActive     NOTIFY activeChanged)
    Q_PROPERTY(unsigned sampleRate READ sampleRate WRITE setSampleRate NOTIFY sampleRateChanged)

    Q_PROPERTY(QUuid    uuid       READ uuid       CONSTANT)
    Q_PROPERTY(bool     cloneable  READ cloneable  CONSTANT)

public:
    explicit         Source(QObject *parent = nullptr);
    virtual         ~Source();

    virtual             Shared::Source      clone() const = 0;
    virtual Q_INVOKABLE Shared::Source      store();
    virtual             void                destroy(); //TODO: delete
    virtual             QJsonObject         toJSON() const noexcept;
    virtual             void                fromJSON(QJsonObject data, const SourceList * = nullptr) noexcept;
    virtual             QJsonObject         levels();
    virtual             void                setLevels(const QJsonObject &data);


    QString          name() const;
    void             setName(const QString &newName);

    QColor           color() const;
    void             setColor(const QColor &newColor);
    Q_INVOKABLE bool isColorValid();

    QUuid            uuid() const;
    void             setUuid(const QUuid &newUuid);

    unsigned int     sampleRate() const;
    void             setSampleRate(unsigned int newSampleRate);

    bool             active() const;
    virtual void     setActive(bool newActive);

    virtual bool     cloneable() const;

public slots:
    void    setGlobalColor(int globalValue);

signals:
    void    readyRead();
    void    activeChanged();
    void    nameChanged(QString);
    void    colorChanged(QColor);
    void    sampleRateChanged(unsigned int);
    void    beforeDestroy(Source *);   //TODO: delete

private:
    QString                 m_name;
    QColor                  m_color;
    QUuid                   m_uuid;
    std::atomic<unsigned>   m_sampleRate;
    std::atomic<bool>       m_active;
    bool                    m_cloneable;
};

} // namespace Abstract

#endif // ABSTRACT_SOURCE_H
