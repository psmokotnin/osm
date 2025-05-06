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
#include "stepplot.h"
#include "coherenceplot.h"
#include "groupdelayplot.h"
#include "phasedelayplot.h"
#include "spectrogramplot.h"
#include "crestfactorplot.h"
#include "nyquistplot.h"
#include "levelplot.h"
#include "src/sourcelist.h"
#include "source/group.h"

using namespace Chart;

VariableChart::VariableChart(QQuickItem *parent) :
    QQuickItem(parent),
    m_plot(nullptr),
    m_sources(nullptr),
    m_settings(nullptr),
    m_selected(RTA)
{
    setFlag(QQuickItem::ItemHasContents);
    initType();
}
void VariableChart::initType()
{
    Plot *newPlot = nullptr;
    switch (m_selected) {
    case RTA:
        newPlot = new RTAPlot(m_settings, this);
        break;

    case Magnitude:
        newPlot = new MagnitudePlot(m_settings, this);
        break;

    case Phase:
        newPlot = new PhasePlot(m_settings, this);
        break;

    case Impulse:
        newPlot = new ImpulsePlot(m_settings, this);
        break;

    case Step:
        newPlot = new StepPlot(m_settings, this);
        break;

    case Coherence:
        newPlot = new CoherencePlot(m_settings, this);
        break;

    case GroupDelay:
        newPlot = new GroupDelayPlot(m_settings, this);
        break;

    case PhaseDelay:
        newPlot = new PhaseDelayPlot(m_settings, this);
        break;

    case Spectrogram:
        newPlot = new SpectrogramPlot(m_settings, this);
        newPlot->setSelectAppended(false);
        if (m_sources) {
            auto first = m_sources->firstSource();
            newPlot->setSelected({first});
        }
        break;

    case CrestFactor:
        newPlot = new CrestFactorPlot(m_settings, this);
        break;

    case Nyquist:
        newPlot = new NyquistPlot(m_settings, this);
        break;

    case Level:
        newPlot = new LevelPlot(m_settings, this);
        break;

    case SPL:
        newPlot = nullptr;
        break;

    default:
        qCritical("unknown plot");
        return;
    }
    if (newPlot) {
        newPlot->setParentItem(this);
        newPlot->setDarkMode(darkMode());
        newPlot->inheritSettings(m_plot);
    }

    if (m_plot) {
        if (newPlot && !qobject_cast<SpectrogramPlot *>(m_plot)) {
            newPlot->setSelected(m_plot->selected());
        }

        m_plot->clear();
        m_plot->disconnectFromParent();
        m_plot->setParent(nullptr);
        m_plot->setParentItem(nullptr);
        m_plot->deleteLater();
        if (newPlot) newPlot->storeSettings();
    }

    m_plot = newPlot;
}
QString VariableChart::typeString() const
{
    try {
        return typeMap.at(m_selected);
    } catch (std::exception &e) {
        qDebug() << __FILE__ << ":" << __LINE__  << e.what();
    }
    Q_ASSERT(false);
    return "";
}
void VariableChart::setType(const Type &type)
{
    if (m_selected != type) {
        m_selected = type;
        initType();
        if (m_plot && (type != Spectrogram)) {
            m_plot->setSelectAppended(true);
        }
        connectSources(m_sources);
        updateZOrders();
        emit typeChanged();
    }
}
void VariableChart::setTypeByString(const QString &type)
{
    for (auto &it : typeMap) {
        if (it.second == type) {
            return setType(it.first);
        }
    }
    qDebug() << "unknown type" << type;
}
Settings *VariableChart::settings() const noexcept
{
    return m_settings;
}
void VariableChart::setSettings(Settings *settings) noexcept
{
    m_settings = settings;
    if (m_plot) {
        m_plot->setSettings(m_settings);
    }
}
void VariableChart::removeDataSource(const Shared::Source &source)
{
    if (m_plot) {
        m_plot->removeDataSource(source);
    }
}

void VariableChart::setSourceZIndex(const QUuid &source, int index)
{
    if (m_plot) {
        m_plot->setSourceZIndex(source, index);
    }
}

Plot *VariableChart::plot() const noexcept
{
    return m_plot;
}

QString VariableChart::urlForGrab(QUrl url) const
{
    return url.toLocalFile();
}

SourceList *VariableChart::sources() const noexcept
{
    return m_sources;
}

void VariableChart::setSources(SourceList *sourceList)
{
    if (m_sources == sourceList)
        return;

    if (m_sources)
        m_sources->disconnect(this);

    m_sources = sourceList;
    connectSources(m_sources);
    updateZOrders();
    emit sourcesChanged();
}

void VariableChart::connectSources(SourceList *sourceList)
{
    if (m_plot) {
        m_plot->connectSources(sourceList);
    }
}

const bool &VariableChart::darkMode() const noexcept
{
    return m_darkMode;
}

void VariableChart::setDarkMode(const bool &darkMode) noexcept
{
    m_darkMode = darkMode;
    if (m_plot) {
        m_plot->setDarkMode(darkMode);
        emit darkModeChanged();
    }
}

void VariableChart::updateZOrders() noexcept
{
    if (!m_plot || !m_sources) {
        return ;
    }
    auto total = m_sources->count();
    for (auto &&source : *m_sources) {
        if (source) {
            auto z = total - m_sources->indexOf(source);
            setSourceZIndex(source->uuid(), z);
        }
    }
    update();
}
