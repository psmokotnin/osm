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
#include <QDebug>
const QList<std::pair<QString, std::vector<QPointF>>> TargetTrace::m_presets = {
    {
        "Default", {
            QPointF{20, 9},   QPointF{100, 9},  QPointF{250, 0},  QPointF{500, 0},
            QPointF{1000, 0}, QPointF{2000, 0}, QPointF{20000, 0}
        }
    },
    {
        "Immersive studio", {
            QPointF{20, -1},  QPointF{40, 1},        QPointF{125, 1},  QPointF{160, 0},
            QPointF{1600, 0}, QPointF{10000, -3.97}, QPointF{20000, -7.93}
        }
    },
    {
        "Headphones", {
            QPointF{20, 9.2},   QPointF{63, 9.2},  QPointF{200, 0},  QPointF{4000, 0},
            QPointF{10000, -4}, QPointF{16000, -5}, QPointF{20000, -6.2}
        }
    },
    {
        "Car", {
            QPointF{20, 9.0}, QPointF{60, 9.0}, QPointF{160, 0},  QPointF{1000, 0},
            QPointF{1000, 0}, QPointF{1000, 0}, QPointF{20000, -6.0}
        }
    },
};

TargetTrace::TargetTrace(Settings *settings, QObject *parent) : QObject(parent),
    m_points{ m_presets[0].second }, m_settings(settings), m_preset(0)
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

unsigned TargetTrace::size() const
{
    return m_points.size();
}

qreal TargetTrace::frequency(unsigned int i) const
{
    return m_points[i].x();
}

qreal TargetTrace::gain(unsigned int i) const
{
    return m_points[i].y();
}

void TargetTrace::setFrequency(unsigned int i, qreal value)
{
    if (!qFuzzyCompare(m_points[i].x(), value)) {
        m_points[i].setX(value);
        emit changed();
    }
}

void TargetTrace::setGain(unsigned int i, qreal value)
{
    if (!qFuzzyCompare(m_points[i].y(), value)) {
        m_points[i].setY(value);
        emit changed();
    }
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

        setPreset(m_settings->reactValue<TargetTrace, unsigned>(
                      "preset", this, &TargetTrace::presetChanged, preset()
                  ).toUInt());

        for (size_t i = 0; i < m_points.size(); ++i) {
            setFrequency(i, m_settings->value(QString("frequency") + QString::number(i), frequency(i)).toFloat());
            setGain(i, m_settings->value(QString("gain") + QString::number(i), gain(i)).toFloat());
        }

        connect(this, &TargetTrace::changed, this, [this]() {
            for (size_t i = 0; i < m_points.size(); ++i) {
                m_settings->setValue(QString("frequency") + QString::number(i), frequency(i));
                m_settings->setValue(QString("gain") + QString::number(i),   gain(i));
            }
        });

    }
}

unsigned TargetTrace::preset() const
{
    return m_preset;
}

void TargetTrace::setPreset(unsigned newPreset)
{
    if (m_preset == newPreset)
        return;

    std::lock_guard<std::mutex> guard(m_mutex);

    if (m_presets.size() > newPreset) {
        m_preset = newPreset;
        for (auto i = 0; i < m_points.size(); ++i) {
            m_points[i].setX(m_presets[m_preset].second[i].x());
            m_points[i].setY(m_presets[m_preset].second[i].y());
        }
        emit presetChanged(m_preset);
        emit changed();
    }
}

std::mutex &TargetTrace::mutex()
{
    return m_mutex;
}

const std::vector<QPointF> &TargetTrace::points() const
{
    return m_points;
}

qreal TargetTrace::offset() const
{
    return m_offset;
}

void TargetTrace::setOffset(qreal newOffset)
{
    if (qFuzzyCompare(m_offset, newOffset))
        return;
    m_offset = newOffset;
    emit offsetChanged();
}

QVariant TargetTrace::getAvailablePresets() const
{
    QStringList typeList;
    for (const auto &preset : m_presets) {
        typeList << std::get<0>(preset);
    }
    return typeList;
}
