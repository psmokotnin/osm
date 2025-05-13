/**
 *  OSM
 *  Copyright (C) 2023  Pavel Smokotnin

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

#include <QJsonObject>
#include "abstract/source.h"
#include "math/fouriertransform.h"
#include "meta/metawindowing.h"

#ifndef SOURCE_WINDOWING_H
#define SOURCE_WINDOWING_H


class Windowing : public Abstract::Source, public Meta::Windowing
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(float wide READ wide WRITE setWide NOTIFY wideChanged)
    Q_PROPERTY(float offset READ offset WRITE setOffset NOTIFY offsetChanged)
    Q_PROPERTY(float minFrequency READ minFrequency WRITE setMinFrequency NOTIFY minFrequencyChanged)
    Q_PROPERTY(float maxFrequency READ maxFrequency WRITE setMaxFrequency NOTIFY maxFrequencyChanged)
    Q_PROPERTY(Shared::Source source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QUuid sourceId READ sourceId NOTIFY sourceChanged)
    Q_PROPERTY(Meta::Windowing::Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(Meta::Windowing::SourceDomain domain READ domain WRITE setDomain NOTIFY domainChanged)
    Q_PROPERTY(QString tipName READ tipName WRITE setTipName NOTIFY tipNameChanged)
    Q_PROPERTY(WindowFunction::Type window READ windowFunctionType WRITE setWindowFunctionType NOTIFY
               windowFunctionTypeChanged)

    //constant meta properties
    Q_PROPERTY(QVariant modes READ getAvailableModes CONSTANT)
    Q_PROPERTY(QVariant windows READ getAvailableWindowTypes CONSTANT)

public:
    Windowing(QObject *parent = nullptr);
    ~Windowing();

    Shared::Source clone() const override;
    Q_INVOKABLE QJsonObject toJSON() const noexcept override;
    void fromJSON(QJsonObject data, const SourceList *list = nullptr) noexcept override;

    Shared::Source source() const;
    void setSource(const Shared::Source &newSource);

    QUuid sourceId() const;

    Q_INVOKABLE Shared::Source store() override;

    unsigned sampleRate() const;

public slots:
    void applyAutoName();
    void update();
    void applyWindowFunctionType();
    void applyAutoWide();

signals:
    void wideChanged(float) override;
    void offsetChanged(float) override;
    void minFrequencyChanged() override;
    void maxFrequencyChanged() override;
    void modeChanged(Meta::Windowing::Mode) override;
    void domainChanged(Meta::Windowing::SourceDomain) override;
    void tipNameChanged(QString) override;
    void windowFunctionTypeChanged(WindowFunction::Type) override;
    void sourceChanged();

private:
    void resizeData();
    void syncData();
    void updateFromDomain();
    void updateFromFrequencyDomain();
    void updateFromTimeDomain(const Shared::Source &source);
    void transform();

    unsigned m_sampleRate;
    Shared::Source m_source;
    WindowFunction m_window;
    FourierTransform m_dataFT;
    Mode m_usedMode;
    bool m_resize;
};

#endif // SOURCE_WINDOWING_H
