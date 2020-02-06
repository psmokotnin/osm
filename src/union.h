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

#include "chart/source.h"
#include "settings.h"

class Union : public Fftchart::Source
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(Operation operation READ operation WRITE setOperation NOTIFY operationChanged)

public:
    enum Operation {SUM, DIFF, AVG};
    const std::map<Operation, QString> operationMap = {
        {SUM,       "Summation"},
        {DIFF,      "Difference"},
        {AVG,       "Average"}
    };
    Q_ENUMS(Operation)

private:
    Settings *m_settings;

    QVector<QPointer<Fftchart::Source>> m_sources;

    QTimer m_timer;
    QThread m_timerThread;
    Operation m_operation;

    void init() noexcept;

public:
    explicit Union(Settings *settings = nullptr, QObject *parent = nullptr);
    ~Union() override;

    int count() const noexcept {return m_sources.count();}

    Q_INVOKABLE Fftchart::Source *getSource(int index) const noexcept;
    Q_INVOKABLE void setSource(int index, Fftchart::Source *s) noexcept;

    Q_INVOKABLE QJsonObject toJSON() const noexcept override;
    void fromJSON(QJsonObject data) noexcept override;

    Operation operation() const noexcept {return m_operation;}
    void setOperation(const Operation &operation) noexcept;

    void setActive(bool active) noexcept override;

public slots:
    void update() noexcept;
    void calc() noexcept;
    QObject *store();

signals:
    void countChanged(int);
    void operationChanged(Operation);
    void needUpdate();
};
#endif // UNION_H
