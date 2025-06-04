/**
 *  OSM
 *  Copyright (C) 2022  Pavel Smokotnin

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
#include "metafilter.h"

namespace Meta {

const std::map<Filter::Type, QString>Filter::m_typeMap = {
    {Filter::ButterworthLPF,    "Butterworth LPF"    },
    {Filter::ButterworthHPF,    "Butterworth HPF"    },
    {Filter::LinkwitzRileyLPF,  "Linkwitz-Riley LPF" },
    {Filter::LinkwitzRileyHPF,  "Linkwitz-Riley HPF" },
    {Filter::BesselLPF,         "Bessel LPF"         },
    {Filter::BesselHPF,         "Bessel HPF"         },
    {Filter::APF,               "All Pass"           },
    {Filter::Peak,              "Peak"               },
    {Filter::HighShelf,         "High Shelf"         },
    {Filter::LowShelf,          "Low Shelf"          },
    {Filter::Notch,             "Notch"              },
    {Filter::BPF,               "Band Pass"          }
};

const std::map<Filter::Type, QString>Filter::m_typeShortMap = {
    {Filter::ButterworthLPF,   "BW LPF"     },
    {Filter::ButterworthHPF,   "BW HPF"     },
    {Filter::LinkwitzRileyLPF, "LR LPF"     },
    {Filter::LinkwitzRileyHPF, "LR HPF"     },
    {Filter::BesselLPF,        "Bessel LPF" },
    {Filter::BesselHPF,        "Bessel HPF" },
    {Filter::APF,              "APF"        },
    {Filter::Peak,             "Peak"       },
    {Filter::HighShelf,        "HighShelf"  },
    {Filter::LowShelf,         "LowShelf"   },
    {Filter::Notch,            "Notch"      },
    {Filter::BPF,              "BPF"        }
};

Filter::Filter() : Base(), m_type(ButterworthLPF), m_mode(Measurement::FFT14),
    m_order(3), m_cornerFrequency(1000), m_q(1.f / sqrt(2)), m_polarity(false)
{

}

Filter::Type Filter::type() const
{
    return m_type;
}

void Filter::setType(Filter::Type newType)
{
    if (m_type == newType) {
        return;
    }

    m_type = newType;
    emit typeChanged(m_type);
    emit availableQChanged();
}

QVariant Filter::getAvailableTypes() const
{
    QStringList typeList;
    for (const auto &type : m_typeMap) {
        typeList << type.second;
    }
    return typeList;
}

const Measurement::Mode &Filter::mode() const
{
    return m_mode;
}

void Filter::setMode(const Measurement::Mode &newMode)
{
    if (m_mode == newMode) {
        return;
    }

    m_mode = newMode;
    emit modeChanged(m_mode);
}

QVariant Filter::getAvailableModes()
{
    return Measurement::getAvailableModes();
}

float Filter::cornerFrequency() const
{
    return m_cornerFrequency;
}

void Filter::setCornerFrequency(float newFrequency)
{
    if (qFuzzyCompare(m_cornerFrequency, newFrequency))
        return;
    m_cornerFrequency = newFrequency;
    emit cornerFrequencyChanged(m_cornerFrequency);
}

unsigned int Filter::order() const
{
    return m_order;
}

void Filter::setOrder(unsigned int newOrder)
{
    if (m_order == newOrder)
        return;
    m_order = newOrder;
    emit orderChanged(m_order);
    emit availableQChanged();
}

QVariant Filter::getAvailableOrders()
{
    QList<QVariant> orders;
    switch (type()) {
    case ButterworthHPF:
    case ButterworthLPF:
        orders = QList<QVariant>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
        break;
    case LinkwitzRileyHPF:
    case LinkwitzRileyLPF:
        orders = QList<QVariant>({2, 4, 6, 8, 10});
        break;
    case BesselHPF:
    case BesselLPF:
        orders = QList<QVariant>({1, 2, 3, 4, 5, 6});
        break;
    case APF:
        orders = QList<QVariant>({2, 4});
        break;
    case Peak:
    case HighShelf:
    case LowShelf:
    case Notch:
    case BPF:
        orders = QList<QVariant>({1});
        break;
    }
    return orders;
}

bool Filter::isQAvailable() const
{
    bool r;
    switch (type()) {
    case ButterworthHPF:
    case ButterworthLPF:
    case LinkwitzRileyHPF:
    case LinkwitzRileyLPF:
    case BesselHPF:
    case BesselLPF:
        r = false;
        break;
    case APF:
        r = order() == 4 ? true : false;
        break;
    case Peak:
    case HighShelf:
    case LowShelf:
    case Notch:
    case BPF:
        r = true;
        break;
    }
    return r;
}

bool Filter::isGainAvailable() const
{
    bool r;
    switch (type()) {
    case ButterworthHPF:
    case ButterworthLPF:
    case LinkwitzRileyHPF:
    case LinkwitzRileyLPF:
    case BesselHPF:
    case BesselLPF:
    case APF:
        r = false;
        break;
    case Peak:
    case HighShelf:
    case LowShelf:
    case Notch:
    case BPF:
        r = true;
        break;
    }
    return r;
}

bool Filter::polarity() const
{
    return m_polarity;
}

void Filter::setPolarity(bool newPolarity)
{
    if (m_polarity == newPolarity)
        return;
    m_polarity = newPolarity;
    emit polarityChanged(m_polarity);
}

float Filter::q() const
{
    return m_q;
}

void Filter::setQ(float newQ)
{
    if (qFuzzyCompare(m_q, newQ))
        return;
    m_q = newQ;
    emit qChanged(m_q);
}

float Filter::gain() const
{
    return m_gain;
}

void Filter::setGain(float newGain)
{
    if (qFuzzyCompare(m_gain, newGain))
        return;
    m_gain = newGain;
    emit gainChanged(m_gain);
}

} // namespace meta
