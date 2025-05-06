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
#ifndef CHART_METERPLOT_H
#define CHART_METERPLOT_H

#include <QTimer>
#include "levelobject.h"
#include "abstract/source.h"
#include "math/leq.h"
#include "common/settings.h"
#include "shared/source_shared.h"

namespace Chart {

class MeterPlot : public QObject, public LevelObject
{
public:
    enum Type {
        RMS     = 0x00,
        Peak    = 0x01,
        Crest   = 0x02,
        THDN    = 0x03,
        Time    = 0x04,
        Leq     = 0x05,
        Gain    = 0x06,
        Delay   = 0x07,
    };
    Q_OBJECT
    Q_ENUM(Type);

    Q_PROPERTY(QVariant availableCurves READ getAvailableCurves CONSTANT)
    Q_PROPERTY(QVariant availableTimes READ getAvailableTimes NOTIFY typeChanged)
    Q_PROPERTY(QVariant availableTypes READ getAvailableTypes CONSTANT)

    Q_PROPERTY(QUuid source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(SourceList *list READ sourceList WRITE setSourceList NOTIFY sourceListChanged)

    Q_PROPERTY(QString curve READ curveName WRITE setCurve NOTIFY curveChanged)
    Q_PROPERTY(QString time READ timeName WRITE setTime NOTIFY timeChanged)
    Q_PROPERTY(QString type READ typeName WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(Chart::LevelObject::Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(QString modeName READ modeName NOTIFY modeChanged)

    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString value READ value NOTIFY valueChanged)
    Q_PROPERTY(QString sourceName READ sourceName NOTIFY sourceNameChanged)
    Q_PROPERTY(float threshold READ threshold WRITE setThreshold NOTIFY thresholdChanged)
    Q_PROPERTY(bool pause READ pause WRITE setPause NOTIFY pauseChanged)
    Q_PROPERTY(bool peakHold READ peakHold WRITE setPeakHold NOTIFY peakHoldChanged)

public:
    MeterPlot(QObject *parent = nullptr);

    QUuid source() const;
    void setSource(QUuid sourceId);

    QString title() const;
    QString value() const;
    QString sourceName() const;

    float threshold() const;
    void setThreshold(float threshold);

    QVariant getAvailableTypes() const;
    QVariant getAvailableTimes() const override;
    QString timeName() const override;
    void setTime(const QString &time) override;

    const Type &type() const;
    QString typeName() const noexcept;
    void setType(const Type &type);
    void setType(const QString &type);

    SourceList *sourceList() const;
    void setSourceList(SourceList *sourceList);

    void setSettings(Settings *newSettings);

    bool peakHold() const;
    void setPeakHold(bool newPeakHold);
    Q_INVOKABLE void reset();

signals:
    void curveChanged(QString) override;
    void timeChanged(QString) override;
    void modeChanged(Chart::LevelObject::Mode) override;
    void pauseChanged(bool) override;

    void sourceChanged(QUuid);
    void valueChanged();
    void thresholdChanged(float);

    void typeChanged(QString);
    void titleChanged();
    void sourceNameChanged();

    void sourceListChanged();

    void peakHoldChanged();

private slots:
    void updateThreshold();
    void resetSource();
    void sourceReadyRead();
    void timeReadyRead();

private:
    QString dBValue() const;
    QString timeValue() const;
    QString thdnValue() const;
    QString delayValue() const;

    Shared::Source m_source;
    SourceList *m_sourceList;
    Settings   *m_settings;
    QTimer m_timer;
    Type m_type;
    math::Leq m_leq;
    QMetaObject::Connection m_sourceConnection;
    float m_threshold;
    bool m_peakHold;
    mutable float m_peakLevel;

    static const std::map<Type, QString> m_typesMap;
};

} // namespace chart

Q_DECLARE_METATYPE(Chart::MeterPlot *)

#endif // CHART_METERPLOT_H
