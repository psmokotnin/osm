/**
 *  OSM
 *  Copyright (C) 2020  Pavel Smokotnin

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
#ifndef UNION_H
#define UNION_H

#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QThread>

#include <set>
#include "chart/source.h"
#include "common/settings.h"

class Union : public chart::Source
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(Operation operation READ operation WRITE setOperation NOTIFY operationChanged)
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged)

public:
    enum Operation {Sum, Diff, Avg};
    enum Type {Vector, Polar};
    const std::map<Operation, QString> operationMap = {
        {Sum,       "Summation"},
        {Diff,      "Difference"},
        {Avg,       "Average"}
    };
    Q_ENUMS(Operation)
    Q_ENUMS(Type)

private:
    Settings *m_settings;

    QVector<QPointer<chart::Source>> m_sources;

    QTimer m_timer;
    QThread m_timerThread;
    Operation m_operation;
    Type m_type;

    void init() noexcept;
    void resize();
    void calcPolar(unsigned int count, std::set<chart::Source *> sources,
                   chart::Source *primary) noexcept;
    void calcVector(unsigned int count, std::set<Source *> sources, chart::Source *primary) noexcept;

public:
    explicit Union(Settings *settings = nullptr, QObject *parent = nullptr);
    ~Union() override;
    Source *clone() const override;

    int count() const noexcept
    {
        return m_sources.count();
    }

    Q_INVOKABLE chart::Source *getSource(int index) const noexcept;
    Q_INVOKABLE void setSource(int index, chart::Source *s) noexcept;

    Q_INVOKABLE QJsonObject toJSON() const noexcept override;
    void fromJSON(QJsonObject data) noexcept override;

    Operation operation() const noexcept
    {
        return m_operation;
    }
    void setOperation(const Operation &operation) noexcept;

    void setActive(bool active) noexcept override;

    Type type() const;
    void setType(const Type &type);

public slots:
    void update() noexcept;
    void calc() noexcept;
    QObject *store();

signals:
    void countChanged(int);
    void operationChanged(Operation);
    void needUpdate();
    void typeChanged();
};
#endif // UNION_H
