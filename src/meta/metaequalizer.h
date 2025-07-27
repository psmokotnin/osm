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
#ifndef META_EQUALIZER_H
#define META_EQUALIZER_H

#include <QObject>
#include "metabase.h"
#include "metameasurement.h"

namespace Meta {

class Equalizer : public Base
{
    Q_GADGET

public:
    Equalizer();

    Measurement::Mode mode() const;
    void setMode(const Measurement::Mode &newMode);
    static QVariant getAvailableModes();

    virtual unsigned size() const  = 0;
    virtual void setSize(unsigned) = 0;
    virtual SourceList     *sourceList() = 0;

//virtual signals:
    virtual void modeChanged(Meta::Measurement::Mode) = 0;
    virtual void sizeChanged() = 0;

private:
    Measurement::Mode m_mode;
};

} // namespace Meta

#endif // META_EQUALIZER_H
