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
#ifndef SOURCE_EQUALIZER_H
#define SOURCE_EQUALIZER_H

#include <QObject>
#include "abstract/source.h"
#include "sourcelist.h"
#include "meta/metaequalizer.h"

namespace Source {

class Equalizer : public Abstract::Source, public Meta::Equalizer
{
    Q_OBJECT
    Q_PROPERTY(QVariant modes READ getAvailableModes CONSTANT)
    Q_PROPERTY(unsigned size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(Meta::Measurement::Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(SourceList *sourceList READ sourceList CONSTANT)

public:
    explicit Equalizer(QObject *parent = nullptr);
    ~Equalizer();

    Shared::Source clone() const override;
    Q_INVOKABLE QJsonObject toJSON() const noexcept override;
    void fromJSON(QJsonObject data, const SourceList * = nullptr) noexcept override;

    unsigned        size() const override;
    void            setSize(unsigned newSize) override;

    SourceList     *sourceList() override;

    Q_INVOKABLE bool save(const QUrl &fileName) const noexcept;

signals:
    void modeChanged(Meta::Measurement::Mode) override;
    void sizeChanged() override;
    void needUpdate();

private slots:
    void update();
    void doUpdate();

private:
    void calc(const Shared::Source &primary) noexcept;
    void postFilterAppended(const Shared::Source &source);

    std::shared_ptr<SourceList> m_filterList;
    QTimer m_timer;
    QThread m_timerThread;
};

} // namespace Source

#endif // SOURCE_EQUALIZER_H
