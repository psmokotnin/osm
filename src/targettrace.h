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

#include <QObject>
#include <QColor>
#include <array>
#include "settings.h"

class TargetTrace : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(bool show READ show WRITE setShow NOTIFY showChanged)
    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

    struct Segment {
        qreal start = 0;
        qreal end = 0;
    };

public:
    explicit TargetTrace(Settings *settings = nullptr, QObject *parent = nullptr);
    static TargetTrace *getInstance();
    static const size_t SEGMENT_COUNT = 3;

    bool active() const;
    void setActive(bool active);

    Q_INVOKABLE qreal start(unsigned int i) const noexcept;
    Q_INVOKABLE qreal end(unsigned int i) const noexcept;
    Q_INVOKABLE qreal point(unsigned int i) const noexcept;

    Q_INVOKABLE void setStart(unsigned int i, qreal value) noexcept;
    Q_INVOKABLE void setEnd(unsigned int i, qreal value) noexcept;
    Q_INVOKABLE void setPoint(unsigned int i, qreal value) noexcept;

    qreal width() const;
    void setWidth(const qreal &width);

    QColor color() const;
    void setColor(const QColor &color);

    bool show() const;
    void setShow(bool show);

signals:
    void activeChanged(bool);
    void showChanged(bool);
    void widthChanged(qreal);
    void colorChanged(QColor);
    void changed();

private:
    void loadSettings();

    bool m_active = true, m_show = false;
    qreal m_width = 3.0;
    QColor m_color = "#8BC34A";
    std::array<Segment, SEGMENT_COUNT> m_segments;
    std::array < qreal, SEGMENT_COUNT - 1 > m_points;

    Settings *m_settings = nullptr;
    static TargetTrace *m_instance;
};

#endif // TARGETTRACE_H
