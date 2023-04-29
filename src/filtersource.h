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
#ifndef FILTERSOURCE_H
#define FILTERSOURCE_H

#include <QtQml>
#include "meta/metafilter.h"
#include "chart/source.h"
#include "math/fouriertransform.h"

class FilterSource : public chart::Source, public meta::Filter
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QVariant modes READ getAvailableModes CONSTANT)
    Q_PROPERTY(QVariant types READ getAvailableTypes CONSTANT)

    Q_PROPERTY(meta::Measurement::Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(unsigned int sampleRate READ sampleRate WRITE setSampleRate NOTIFY sampleRateChanged)

    Q_PROPERTY(QVariant orders READ getAvailableOrders NOTIFY typeChanged)
    Q_PROPERTY(meta::Filter::Type type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(unsigned int order READ order WRITE setOrder NOTIFY orderChanged)
    Q_PROPERTY(float cornerFrequency READ cornerFrequency WRITE setCornerFrequency NOTIFY cornerFrequencyChanged)

public:
    FilterSource(QObject *parent = nullptr);

    Source *clone() const override;
    Q_INVOKABLE QJsonObject toJSON(const SourceList * = nullptr) const noexcept override;
    void fromJSON(QJsonObject data, const SourceList * list = nullptr) noexcept override;

    Q_INVOKABLE chart::Source *store() noexcept override;

    bool autoName() const;
    void setAutoName(bool newAutoName);

signals:
    void sampleRateChanged(unsigned int) override;
    void typeChanged(meta::Filter::Type) override;
    void modeChanged(meta::Measurement::Mode) override;
    void cornerFrequencyChanged(float) override;
    void orderChanged(unsigned int) override;
    void autoNameChanged();

private slots:
    void update();
    void applyAutoName();

private:
    complex calculate(float frequency) const;
    complex Bessel(bool hpf, complex s) const;

    complex Butterworth(bool hpf, unsigned int order, const complex &s) const;
    complex ButterworthPolinom(unsigned int k, unsigned int order, const complex &s) const;

    complex LinkwitzRiley(bool hpf, const complex &s) const;

    bool m_autoName;
    FourierTransform m_dataFT, m_inverse;

    Q_PROPERTY(bool autoName READ autoName WRITE setAutoName NOTIFY autoNameChanged)
};

#endif // FILTERSOURCE_H
