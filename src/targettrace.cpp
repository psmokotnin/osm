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
#include "targettrace.h"

TargetTrace::TargetTrace(Settings *settings, QObject *parent) : QObject(parent),
    m_segments({{{9, 9}, {9, 0.0}, {0.0, 0.0}}}), m_points({{100, 1000}}), m_settings(settings)
{
    Q_ASSERT(!m_instance);

    loadSettings();
    m_instance = this;
}

TargetTrace *TargetTrace::m_instance = nullptr;

TargetTrace *TargetTrace::getInstance()
{
    if (!m_instance) {
        m_instance = new TargetTrace();
    }

    return m_instance;
}

bool TargetTrace::active() const
{
    return m_active;
}

void TargetTrace::setActive(bool active)
{
    if (m_active != active) {
        m_active = active;
        emit activeChanged(active);
    }
}

qreal TargetTrace::start(unsigned int i) const noexcept
{
    Q_ASSERT(i < SEGMENT_COUNT);
    return m_segments[i].start;
}

qreal TargetTrace::end(unsigned int i) const noexcept
{
    Q_ASSERT(i < SEGMENT_COUNT);
    return m_segments[i].end;
}

qreal TargetTrace::point(unsigned int i) const noexcept
{
    Q_ASSERT(i < SEGMENT_COUNT - 1);
    return m_points[i];
}

void TargetTrace::setStart(unsigned int i, qreal value) noexcept
{
    Q_ASSERT(i < SEGMENT_COUNT);
    m_segments[i].start = value;
    emit changed();
}

void TargetTrace::setEnd(unsigned int i, qreal value) noexcept
{
    Q_ASSERT(i < SEGMENT_COUNT);
    m_segments[i].end = value;
    emit changed();
}

void TargetTrace::setPoint(unsigned int i, qreal value) noexcept
{
    Q_ASSERT(i < SEGMENT_COUNT);
    m_points[i] = value;
    emit changed();
}

qreal TargetTrace::width() const
{
    return m_width;
}

void TargetTrace::setWidth(const qreal &width)
{
    if (!qFuzzyCompare(m_width, width)) {
        m_width = width;
        emit widthChanged(m_width);
        emit changed();
    }
}

QColor TargetTrace::color() const
{
    return m_color;
}

void TargetTrace::setColor(const QColor &color)
{
    if (m_color != color) {
        m_color = color;
        emit colorChanged(m_color);
        emit changed();
    }
}

bool TargetTrace::show() const
{
    return m_show;
}

void TargetTrace::setShow(bool show)
{
    if (m_show != show) {
        m_show = show;
        emit showChanged(m_show);
        emit changed();
    }
}

void TargetTrace::loadSettings()
{
    if (m_settings) {
        setWidth(m_settings->reactValue<TargetTrace, qreal>(
                     "width", this, &TargetTrace::widthChanged, width()
                 ).toFloat());

        setShow(m_settings->reactValue<TargetTrace, bool>(
                    "show", this, &TargetTrace::showChanged, show()
                ).toBool());

        setActive(m_settings->reactValue<TargetTrace, bool>(
                      "active", this, &TargetTrace::activeChanged, active()
                  ).toBool());

        setColor(m_settings->reactValue<TargetTrace, QColor>(
                     "color", this, &TargetTrace::colorChanged, color()
                 ).value<QColor>());

        setPoint(0, m_settings->value("point0", point(0)).toFloat());
        setPoint(1, m_settings->value("point1", point(1)).toFloat());

        for (size_t i = 0; i < SEGMENT_COUNT; ++i) {
            setStart(i, m_settings->value(QString("start") + QString::number(i), start(i)).toFloat());
            setEnd(i, m_settings->value(QString("end") + QString::number(i), end(i)).toFloat());
        }

        connect(this, &TargetTrace::changed, this, [this]() {
            for (size_t i = 0; i < SEGMENT_COUNT; ++i) {
                m_settings->setValue(QString("start") + QString::number(i), start(i));
                m_settings->setValue(QString("end") + QString::number(i),   end(i));
            }

            m_settings->setValue(QString("point0"), point(0));
            m_settings->setValue(QString("point1"), point(1));
        });
    }
}
