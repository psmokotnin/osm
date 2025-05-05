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
#ifndef SOURCE_H
#define SOURCE_H


#include <QObject>
#include <QColor>
#include <QJsonObject>

#include "math/meter.h"
#include "source/source_shared.h"
#include "abstract/source.h"

class SourceList;

namespace Source {
class Abstract : public ::Abstract::Source
{

    Q_OBJECT

public:
    explicit Abstract(QObject *parent = nullptr);
    virtual ~Abstract();
    virtual ::Source::Shared clone() const = 0;

    virtual Q_INVOKABLE void destroy();

    virtual Q_INVOKABLE QJsonObject toJSON(const SourceList * = nullptr) const noexcept;
    virtual void fromJSON(QJsonObject data, const SourceList * = nullptr) noexcept;

    virtual QJsonObject levels();
    virtual void setLevels(const QJsonObject &data);

    Q_INVOKABLE virtual ::Source::Shared store();

signals:
    void readyRead();
    void beforeDestroy(::Source::Abstract *);//TODO: check if still needed

protected:
    std::atomic<bool>       m_onReset; //move to measurement
};
}
#endif // SOURCE_H
