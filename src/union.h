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

    Q_PROPERTY(int count READ count WRITE setCount NOTIFY countChanged)
    Q_PROPERTY(bool autoName READ autoName WRITE setAutoName NOTIFY autoNameChanged)
    Q_PROPERTY(Operation operation READ operation WRITE setOperation NOTIFY operationChanged)
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged)

    using SourceVector = QVector<QPointer<chart::Source>>;

public:
    enum Operation {Summation, Subtract, Avg};
    enum Type {Vector, Polar, dB, Power};
    const std::map<Operation, QString> operationMap = {
        {Summation, "Sum"},
        {Subtract,  "Subtract"},
        {Avg,       "Average"}
    };
    const std::map<Type, QString> typeMap = {
        {Vector,    "Vector"},
        {Polar,     "Polar"},
        {dB,        "dB"},
        {Power,     "Power"}
    };
    Q_ENUMS(Operation)
    Q_ENUMS(Type)

    explicit Union(Settings *settings = nullptr, QObject *parent = nullptr);
    ~Union() override;
    Source *clone() const override;

    int count() const noexcept;
    void setCount(int count) noexcept;

    Q_INVOKABLE chart::Source *getSource(int index) const noexcept;
    Q_INVOKABLE bool setSource(int index, chart::Source *s) noexcept;

    Q_INVOKABLE QJsonObject toJSON(const SourceList *list = nullptr) const noexcept override;
    void fromJSON(QJsonObject data, const SourceList *list = nullptr) noexcept override;

    Operation operation() const noexcept;
    void setOperation(Operation operation) noexcept;

    void setActive(bool active) noexcept override;

    Type type() const;
    void setType(Type type);

    bool autoName() const;
    void setAutoName(bool autoName);

    bool checkLoop(Union *source) const;
    const SourceVector &sources() const;

public slots:
    void update() noexcept;
    void calc() noexcept;
    QObject *store();
    void applyAutoName() noexcept;
    void sourceDestroyed(chart::Source *source);

signals:
    void countChanged(int);
    void operationChanged(Union::Operation);
    void needUpdate();
    void typeChanged();
    void autoNameChanged();
    void modelChanged();

private:
    void init() noexcept;
    void resize();
    void calcPolar(unsigned int count, chart::Source *primary) noexcept;
    void calcVector(unsigned int count, chart::Source *primary) noexcept;
    void calcdB(unsigned int count, chart::Source *primary) noexcept;
    void calcPower(unsigned int count, chart::Source *primary) noexcept;

    Settings *m_settings;
    SourceVector m_sources;
    QTimer m_timer;
    QThread m_timerThread;
    Operation m_operation;
    Type m_type;
    bool m_autoName;

    static std::mutex s_calcmutex;
};
#endif // UNION_H
