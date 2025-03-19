/**
 *  OSM
 *  Copyright (C) 2024  Pavel Smokotnin

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
#ifndef SOURCE_SHARED_H
#define SOURCE_SHARED_H

#include <memory>

#include <QObject>
#include <QColor>
#include <QUuid>

namespace Source {
class Abstract;
}

Q_DECLARE_OPAQUE_POINTER(Source::Abstract*)

namespace Source {

class Shared : public std::shared_ptr<Source::Abstract>
{
    Q_GADGET
    Q_PROPERTY(Source::Abstract *data READ get CONSTANT)
    Q_PROPERTY(QUuid uuid READ uuid CONSTANT)
    Q_PROPERTY(QString objectName READ objectName CONSTANT)

public:
    Shared(std::shared_ptr<Source::Abstract> ptr = nullptr);
    ~Shared();

    QUuid uuid() const;
    Q_INVOKABLE QColor color() const;
    QString objectName() const;
};

} // namespace Source

Q_DECLARE_METATYPE(Source::Shared)

#endif // SOURCE_SHARED_H
