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
#include "chart/source.h"
#include "math/fouriertransform.h"
#include "meta/metawindowing.h"

#ifndef SOURCE_WINDOWING_H
#define SOURCE_WINDOWING_H


class Windowing : public chart::Source, public meta::Windowing
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(float wide READ wide WRITE setWide NOTIFY wideChanged)
    Q_PROPERTY(float offset READ offset WRITE setOffset NOTIFY offsetChanged)
    Q_PROPERTY(QUuid source READ sourceId WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(meta::Windowing::Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(meta::Windowing::SourceDomain domain READ domain WRITE setDomain NOTIFY domainChanged)
    Q_PROPERTY(QString tipName READ tipName WRITE setTipName NOTIFY tipNameChanged)
    Q_PROPERTY(WindowFunction::Type window READ windowFunctionType WRITE setWindowFunctionType NOTIFY
               windowFunctionTypeChanged)

    //constant meta properties
    Q_PROPERTY(QVariant modes READ getAvailableModes CONSTANT)
    Q_PROPERTY(QVariant windows READ getAvailableWindowTypes CONSTANT)

public:
    Windowing(QObject *parent = nullptr);
    ~Windowing();

    Source *clone() const override;
    Q_INVOKABLE QJsonObject toJSON(const SourceList *list) const noexcept override;
    void fromJSON(QJsonObject data, const SourceList *list = nullptr) noexcept override;

    chart::Source *source() const;
    void setSource(chart::Source *newSource);

    QUuid sourceId() const;
    void setSource(QUuid id);

    Q_INVOKABLE chart::Source *store() override;

    unsigned sampleRate() const;

public slots:
    void applyAutoName();
    void update();
    void applyWindowFunctionType();
    void applyAutoWide();

signals:
    void wideChanged(float) override;
    void offsetChanged(float) override;
    void modeChanged(meta::Windowing::Mode) override;
    void domainChanged(meta::Windowing::SourceDomain) override;
    void tipNameChanged(QString) override;
    void windowFunctionTypeChanged(WindowFunction::Type) override;
    void sourceChanged();

private:
    void resizeData();
    void syncData();
    void updateFromDomain();
    void updateFromFrequencyDomain();
    void updateFromTimeDomain(Source *source);
    void transform();

    unsigned m_sampleRate;
    chart::Source *m_source;
    WindowFunction m_window;
    FourierTransform m_dataFT;
};

#endif // SOURCE_WINDOWING_H
