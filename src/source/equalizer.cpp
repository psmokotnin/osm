/**
 *  OSM
 *  Copyright (C) 2025  Pavel Smokotnin

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
#include <set>

#include "equalizer.h"
#include "source/filtersource.h"

namespace Source {

Equalizer::Equalizer(QObject *parent)
    : Abstract::Source{parent}, Meta::Equalizer(),
      m_filterList(std::make_shared<SourceList>(this, false))
{
    setObjectName("Equalizer");
    setName("Equalizer");

    m_timer.setInterval(80);//12.5 per sec
    m_timer.setSingleShot(true);
    m_timer.moveToThread(&m_timerThread);


    connect(this, &Equalizer::sizeChanged, this, &Equalizer::update);
    connect(m_filterList.get(), &SourceList::countChanged, this, &Equalizer::sizeChanged);
    connect(m_filterList.get(), &SourceList::postItemAppended, this, &Equalizer::postFilterAppended);

    connect(&m_timer, SIGNAL(timeout()), SLOT(doUpdate()), Qt::DirectConnection);
    connect(this, SIGNAL(needUpdate()), &m_timer, SLOT(start()));
    connect(&m_timerThread, SIGNAL(finished()), &m_timer, SLOT(stop()), Qt::DirectConnection);
    m_timerThread.start();

    setActive(true);
}

Equalizer::~Equalizer()
{
    m_timerThread.quit();
    m_timerThread.wait();
}

Shared::Source Equalizer::clone() const
{
    auto cloned = std::make_shared<Equalizer>();
    cloned->setActive(active());
    cloned->setName(name());
    cloned->setMode(mode());
    cloned->setSampleRate(sampleRate());

    for (unsigned i = 0; i < m_filterList->size(); ++i) {
        auto clonedFilter = m_filterList->get_ref(i)->clone();
        cloned->m_filterList->takeItem(clonedFilter);
    }

    return std::static_pointer_cast<Abstract::Source>(cloned);
}

QJsonObject Equalizer::toJSON() const noexcept
{
    auto object = Abstract::Source::toJSON();

    object["mode"] = mode();
    object["list"] = m_filterList->toJSON();

    return object;
}

void Equalizer::fromJSON(QJsonObject data, const SourceList *) noexcept
{
    Abstract::Source::fromJSON(data);

    m_filterList->fromJSON(data["list"].toArray(), nullptr);
    auto variantMode = data["mode"].toVariant();
    if (variantMode.isValid()) {
        setMode(variantMode.value<Meta::Measurement::Mode>());
    }
}

unsigned int Equalizer::size() const
{
    return m_filterList->size();
}

void Equalizer::setSize(unsigned int newSize)
{
    auto currentSize = m_filterList->size();
    while (m_filterList->size() > newSize) {
        auto to_delete = m_filterList->get( m_filterList->size() - 1);
        m_filterList->removeItem(to_delete);
        update();

        if (currentSize == m_filterList->size()) {
            break;
        }
    }

    while (m_filterList->size() < newSize) {
        auto filter = std::make_shared<FilterSource>(this);
        filter->setSampleRate(sampleRate());
        filter->setType(Meta::Filter::Type::Peak);
        filter->setOrder(1);
        m_filterList->appendItem(Shared::Source(filter), true);
        update();

        if (currentSize == m_filterList->size()) {
            break;
        }
    }
}

SourceList *Equalizer::sourceList()
{
    return m_filterList.get();
}

bool Equalizer::save(const QUrl &fileName) const noexcept
{
    QFile saveFile(fileName.toLocalFile());
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }
    QJsonObject object;
    object["type"] = "Equalizer";
    object["data"] = toJSON();

    QJsonDocument document(object);
    if (saveFile.write(document.toJson(QJsonDocument::JsonFormat::Compact)) != -1) {
        return true;
    }

    return false;
}

void Equalizer::update()
{
    if (!m_timer.isActive()) {
        emit needUpdate();
    }

}
void Equalizer::doUpdate()
{
    if (!active())
        return;

    {
        std::lock_guard guard{m_dataMutex};

        std::set<Shared::Source> sources;
        auto primary = m_filterList->get_ref(0);
        setFrequencyDomainSize(static_cast<unsigned int>(primary ? primary->frequencyDomainSize() : 1));
        setTimeDomainSize(     static_cast<unsigned int>(primary ? primary->timeDomainSize()      : 1));

        for (auto &s : *m_filterList) {
            if (s) {
                sources.insert(s);
                if (s->frequencyDomainSize() != primary->frequencyDomainSize()) {
                    return;
                }
            }
        }

        //lock each unique source, prevent deadlock
        for (auto &s : sources) {
            if (s && s.get() != this) s->lock();
        }

        calc(primary);

        for (auto &s : sources) {
            if (s) s->unlock();
        }

    }
    emit readyRead();
}

void Equalizer::calc(const Shared::Source &primary) noexcept
{

    for (unsigned int i = 0; i < primary->frequencyDomainSize(); i++) {
        float magnitude = 1, module = 1, coherence = 1 ;
        Complex phase { 1, 0 };

        for (auto it = m_filterList->begin(); it != m_filterList->end(); ++it) {

            if ( (*it)->active()) {
                magnitude *= (*it)->magnitudeRaw(i);
                module    *= (*it)->magnitudeRaw(i);
                coherence  = std::min(coherence, (*it)->coherence(i));
                phase.polar(phase.arg() + (*it)->phase(i).arg());
            }
        }
        if (std::isnan(phase.real) || std::isnan(phase.imag)) {
            phase = {1, 0};
        }

        m_ftdata[i].frequency  = primary->frequency(i);
        m_ftdata[i].module     = module;
        m_ftdata[i].phase      = phase.normalize();
        m_ftdata[i].magnitude  = magnitude;
        m_ftdata[i].coherence  = coherence;
    }

    for (unsigned int i = 0; i < primary->timeDomainSize(); i++) {
        m_impulseData[i].time = primary->impulseTime(i);
        m_impulseData[i].value = NAN;
    }
}

void Equalizer::postFilterAppended(const Shared::Source &source)
{
    if (auto filter = std::dynamic_pointer_cast<FilterSource>(source)) {
        filter->setLimited(true);
        connect(filter.get(), &Source::readyRead,     this, &Equalizer::update     );
        connect(filter.get(), &Source::activeChanged, this, &Equalizer::update     );

        connect(this, &Equalizer::modeChanged, filter.get(), [ filter ](auto mode) {
            filter->setMode(mode);
        });
        connect(this, &Equalizer::sampleRateChanged, filter.get(), &Source::setSampleRate );
    }
}

} // namespace Source
