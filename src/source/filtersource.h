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
#include "abstract/source.h"
#include "math/fouriertransform.h"

class FilterSource : public Abstract::Source, public Meta::Filter
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QVariant modes READ getAvailableModes CONSTANT)
    Q_PROPERTY(QVariant types READ getAvailableTypes CONSTANT)

    Q_PROPERTY(bool autoName READ autoName WRITE setAutoName NOTIFY autoNameChanged)

    Q_PROPERTY(Meta::Measurement::Mode mode READ mode WRITE setMode NOTIFY modeChanged)

    Q_PROPERTY(QVariant orders READ getAvailableOrders NOTIFY typeChanged)
    Q_PROPERTY(Meta::Filter::Type type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(unsigned int order READ order WRITE setOrder NOTIFY orderChanged)

    Q_PROPERTY(float cornerFrequency READ cornerFrequency WRITE setCornerFrequency NOTIFY cornerFrequencyChanged)
    Q_PROPERTY(float gain READ gain WRITE setGain NOTIFY gainChanged)
    Q_PROPERTY(float q READ q WRITE setQ NOTIFY qChanged)
    Q_PROPERTY(bool polarity READ polarity WRITE setPolarity NOTIFY polarityChanged)

    Q_PROPERTY(bool qAvailable READ isQAvailable NOTIFY availableQChanged)
    Q_PROPERTY(bool gainAvailable READ isGainAvailable NOTIFY typeChanged)

public:
    FilterSource(QObject *parent = nullptr);

    Shared::Source clone() const override;
    Q_INVOKABLE QJsonObject toJSON() const noexcept override;
    void fromJSON(QJsonObject data, const SourceList * = nullptr) noexcept override;

    Q_INVOKABLE Shared::Source store()  override;

    bool autoName() const;
    void setAutoName(bool newAutoName);

signals:
    void typeChanged(Meta::Filter::Type) override;
    void modeChanged(Meta::Measurement::Mode) override;
    void cornerFrequencyChanged(float) override;
    void orderChanged(unsigned int) override;
    void gainChanged(float) override;
    void qChanged(float) override;
    void availableQChanged() override;
    void polarityChanged(bool) override;
    void autoNameChanged();

private slots:
    void update();
    void applyAutoName();

private:
    Complex calculate(float frequency) const;
    Complex Bessel(bool hpf, Complex s) const;
    Complex calculateAPF(Complex s) const;
    Complex calculateBPF(Complex s) const;
    Complex calculatePeak(Complex s) const;
    Complex calculateNotch(Complex s) const;

    Complex calculateShelf(bool high, Complex s) const;

    Complex Butterworth(bool hpf, unsigned int order, const Complex &s) const;
    Complex ButterworthPolinom(unsigned int k, unsigned int order, const Complex &s) const;

    Complex LinkwitzRiley(bool hpf, const Complex &s) const;

    bool m_autoName;
    FourierTransform m_dataFT, m_inverse;
};

#endif // FILTERSOURCE_H
