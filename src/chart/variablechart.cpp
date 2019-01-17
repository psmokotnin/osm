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
#include "variablechart.h"
#include "rtaplot.h"
#include "magnitudeplot.h"
#include "phaseplot.h"
#include "impulseplot.h"

using namespace Fftchart;

VariableChart::VariableChart(QQuickItem *parent) :
    QQuickItem(parent),
    s_plot(nullptr),
    m_selected(RTA)
{
    setFlag(QQuickItem::ItemHasContents);
    initType();
}
void VariableChart::initType()
{
    Plot* newPlot = nullptr;
    switch (m_selected) {
    case RTA:
        newPlot = new RTAPlot(this);
        break;

    case Magnitude:
        newPlot = new MagnitudePlot(this);
        break;

    case Phase:
        newPlot = new PhasePlot(this);
        break;

    case Impulse:
        newPlot = new ImpulsePlot(this);
        break;

    default:
        return;
    }
    newPlot->setParentItem(this);

    if (s_plot) {
        std::lock_guard<std::mutex> guard(s_plot->renderMutex);
        s_plot->clear();
        s_plot->disconnectFromParent();
        s_plot->setParent(nullptr);
        s_plot->setParentItem(nullptr);
        s_plot->deleteLater();
    }

    s_plot = newPlot;
}
QString VariableChart::typeString() const
{
    return typeMap.at(m_selected);
}
void VariableChart::setType(const Type type)
{
    if (m_selected != type) {
        m_selected = type;
        initType();
        emit typeChanged();
    }
}
void VariableChart::setTypeByString(const QString &type)
{
    for (auto it = typeMap.begin(); it != typeMap.end(); ++it) {
        if (it->second == type) {
            setType(it->first);
        }
    }
}
void VariableChart::appendDataSource(Source *source)
{
    if (s_plot) {
        s_plot->appendDataSource(source);
    }
}
void VariableChart::removeDataSource(Source *source)
{
    if (s_plot) {
        s_plot->removeDataSource(source);
    }
}
