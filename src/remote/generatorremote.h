/**
 *  OSM
 *  Copyright (C) 2024  Pavel Smokotnin

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
#ifndef REMOTE_GENERATORREMOTE_H
#define REMOTE_GENERATORREMOTE_H

#include <memory>
#include <QObject>
#include <QUuid>
#include <QTimer>

namespace remote {

class GeneratorRemote : public QObject
{
    Q_OBJECT
    //common:
    Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)
    Q_PROPERTY(remote::GeneratorRemote::State state READ state WRITE setState NOTIFY stateChanged)
    //Generator:
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(int type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(int frequency READ frequency WRITE setFrequency NOTIFY frequencyChanged)
    Q_PROPERTY(int startFrequency READ startFrequency WRITE setStartFrequency NOTIFY startFrequencyChanged)
    Q_PROPERTY(int endFrequency READ endFrequency WRITE setEndFrequency NOTIFY endFrequencyChanged)
    Q_PROPERTY(float gain READ gain WRITE setGain NOTIFY gainChanged)
    Q_PROPERTY(float duration READ duration WRITE setDuration NOTIFY durationChanged)

public:
    GeneratorRemote(QObject *parent = nullptr);
    enum State {
        NOT_AVAILABLE   = 1,
        AVAILABLE       = 2,
        ERROR_STATE     = 3
    };
    Q_ENUM(State);

    bool enabled() const;
    void setEnabled(bool newEnabled);

    int type() const;
    void setType(int newType);

    int frequency() const;
    void setFrequency(int newFrequency);

    int startFrequency() const;
    void setStartFrequency(int newStartFrequency);

    int endFrequency() const;
    void setEndFrequency(int newEndFrequency);

    float gain() const;
    void setGain(float newGain);

    float duration() const;
    void setDuration(float newDuration);

    QUuid serverId() const;
    void setServerId(const QUuid &newServerId);

    QString host() const;
    void setHost(const QString &newHost);

    State state() const;
    void setState(const State &newState);

    // remote:
    QJsonObject metaJsonObject(QString propertyName = {}) const;
    void connectProperties();

    QUuid sourceId() const;
    void setSourceId(const QUuid &newSourceId);
    void setEventSilence(bool eventSilence);

public slots:
    void dataError(const uint hash, const bool deactivate);

signals:
    void enabledChanged();
    void typeChanged();
    void frequencyChanged();
    void startFrequencyChanged();
    void endFrequencyChanged();
    void gainChanged();
    void durationChanged();
    void hostChanged();
    void stateChanged();

    void localChanged(QString);

private slots:
    void startResetTimer();
    void resetState();
    void properiesChanged();

private:
    bool  m_enabled;
    int   m_type;
    int   m_frequency;
    int   m_startFrequency;
    int   m_endFrequency;
    float m_gain;
    float m_duration;

    bool m_eventSilence;
    QUuid m_serverId, m_sourceId;
    QString m_host;
    State m_state;
    QTimer m_stateTimer;
};

} // namespace remote

class SharedGeneratorRemote : public std::shared_ptr<remote::GeneratorRemote>
{
    Q_GADGET
    Q_PROPERTY(remote::GeneratorRemote *data READ get CONSTANT)

public:
    SharedGeneratorRemote(std::shared_ptr<remote::GeneratorRemote> ptr = nullptr);
    ~SharedGeneratorRemote();
};
Q_DECLARE_METATYPE(SharedGeneratorRemote)

#endif // REMOTE_GENERATORREMOTE_H
