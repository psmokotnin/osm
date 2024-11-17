/**
 *  OSM
 *  Copyright (C) 2021  Pavel Smokotnin

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
#ifndef TARGETTRACE_H
#define TARGETTRACE_H

#include <mutex>
#include <QObject>
#include <QColor>
#include <QPointF>
#include "common/settings.h"

class TargetTrace : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(bool show READ show WRITE setShow NOTIFY showChanged)
    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(qreal offset READ offset WRITE setOffset NOTIFY offsetChanged)

    Q_PROPERTY(QVariant presets READ getAvailablePresets CONSTANT)
    Q_PROPERTY(unsigned preset READ preset WRITE setPreset NOTIFY presetChanged)

public:
    explicit TargetTrace(Settings *settings = nullptr, QObject *parent = nullptr);
    static TargetTrace *getInstance();

    bool active() const;
    void setActive(bool active);

    Q_INVOKABLE unsigned int size() const;
    Q_INVOKABLE qreal frequency(unsigned int i) const;
    Q_INVOKABLE qreal gain(unsigned int i) const;

    Q_INVOKABLE void setFrequency(unsigned int i, qreal value);
    Q_INVOKABLE void setGain(unsigned int i, qreal value);

    qreal width() const;
    void setWidth(const qreal &width);

    QColor color() const;
    void setColor(const QColor &color);

    bool show() const;
    void setShow(bool show);

    qreal offset() const;
    void setOffset(qreal newOffset);

    const std::vector<QPointF> &points() const;

    static const QList<std::pair<QString, std::vector<QPointF>>>    m_presets;
    QVariant getAvailablePresets() const;

    unsigned preset() const;
    void setPreset(unsigned newPreset);

    std::mutex &mutex();

signals:
    void changed();

    void activeChanged(bool);
    void showChanged(bool);
    void widthChanged(qreal);
    void colorChanged(QColor);
    void offsetChanged();
    void presetChanged(unsigned);

private:
    void loadSettings();

    std::mutex m_mutex;

    bool m_active = true, m_show = false;
    qreal m_width = 3.0;
    qreal m_offset = 72.0;
    QColor m_color = "#8BC34A";

    std::vector<QPointF>    m_points;
    unsigned m_preset;

    Settings *m_settings = nullptr;
    static TargetTrace *m_instance;

};

#endif // TARGETTRACE_H
