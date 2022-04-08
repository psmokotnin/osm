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
#include "server.h"
#include "../sourcelist.h"
#include "item.h"
#include "apikey.h"

namespace remote {

Server::Server(QObject *parent) : QObject(parent),
    m_uuid(QUuid::createUuid()), m_networkThread(), m_network(), m_sourceList(nullptr), m_sourceJsons()
{
    m_network.moveToThread(&m_networkThread);
    m_networkThread.setObjectName("NetworkServer");

    connect(&m_networkThread, &QThread::started,  &m_network, &Network::startTCPServer);
    connect(&m_networkThread, &QThread::finished, &m_network, &Network::stopTCPServer);

    m_timer.setInterval(TIMER_INTERVAL);
    connect(&m_timer, &QTimer::timeout, this, &Server::sendHello);
    connect(&m_networkThread, &QThread::finished, &m_timer, &QTimer::stop);

    m_network.setTcpCallback([this] (const QHostAddress & address, const QByteArray & data) -> QByteArray {
        return tcpCallback(address, data);
    });
}

Server::~Server()
{
    stop();
}

void Server::setSourceList(SourceList *list)
{
    m_sourceList = list;
    auto onAdded = [this](auto * source) {

        if (!source || dynamic_cast<remote::Item *>(source)) {
            return ;
        }

        sourceNotify(source->uuid(), "added");

        connect(source, &chart::Source::readyRead, this, [this, source]() {
            sourceNotify(source->uuid(), "readyRead");
        });

        connect(source, &chart::Source::activeChanged, this, [this, source]() {
            sourceNotify(source->uuid(), "changed");
        });

        connect(source, &chart::Source::colorChanged, this, [this, source]() {
            sourceNotify(source->uuid(), "changed");
        });

        connect(source, &chart::Source::nameChanged, this, [this, source]() {
            sourceNotify(source->uuid(), "changed");
        });
    };

    for (auto *source : *list) {
        onAdded(source);
    }

    connect(list, &SourceList::postItemAppended, this, onAdded);
    connect(list, &SourceList::preItemRemoved, this, [this, list](auto index) {
        auto *source = list->get(index);
        if (source) {
            sourceNotify(source->uuid(), "removed");
        }
    });
}

bool Server::start()
{
    m_networkThread.start();
    sendHello();
    m_timer.start();
    emit activeChanged();
    return m_networkThread.isRunning();
}

void Server::stop()
{
    m_network.disconnect();
    m_networkThread.quit();
    m_networkThread.wait();
    emit activeChanged();
}

bool Server::active() const
{
    return m_networkThread.isRunning();
}

void Server::setActive(bool state)
{
    if (active() && !state) {
        stop();
    }
    if (!active() && state) {
        start();
    }
}

QString Server::lastConnected() const
{
    return m_lastConnected;
}

void Server::setLastConnected(const QString &lastConnected)
{
    if (m_lastConnected != lastConnected) {
        m_lastConnected = lastConnected;
        emit lastConnectedChanged();
    }
}

QByteArray Server::tcpCallback([[maybe_unused]] const QHostAddress &address, const QByteArray &data)
{
    auto document = QJsonDocument::fromJson(data);
    if (document.isNull()) {
        return {};
    }

    if (!m_sourceList) {
        return {};
    }
    auto guard = m_sourceList->lock();
    auto message = document["message"].toString();
    auto sourceId = QUuid::fromString(document["uuid"].toString());
    auto source = m_sourceList->getByUUid(sourceId);

    auto license = document["license"];
    auto owner = license["owner"].toString();
    auto sign = license["sign"].toString();

    if (m_validKeys[owner].isEmpty()) {
        m_validKeys[owner] = ApiKey{owner, sign};
        setLastConnected(owner);
    }

    if (!m_validKeys[owner].valid()) {
        QJsonObject object;
        object["api"]     = "Open Sound Meter";
        object["version"] = APP_GIT_VERSION;
        object["message"] = "error";
        object["string"]  = "wrong API key";

        QJsonDocument document(object);
        return document.toJson(QJsonDocument::JsonFormat::Compact);
    }


    if (message == "requestChanged") {
        QJsonObject object;
        object["api"]     = "Open Sound Meter";
        object["version"] = APP_GIT_VERSION;
        object["message"] = "sourceSettings";
        object["uuid"]    = source->uuid().toString();

        QJsonObject color;
        color["red"]     = source->color().red();
        color["green"]   = source->color().green();
        color["blue"]    = source->color().blue();
        color["alpha"]   = source->color().alpha();
        object["color"]  = color;
        object["active"] = source->active();
        object["name"]   = source->name();


        QJsonDocument document(object);
        return document.toJson(QJsonDocument::JsonFormat::Compact);
    }

    if (message == "requestData") {
        QJsonObject object;
        object["api"]     = "Open Sound Meter";
        object["version"] = APP_GIT_VERSION;
        object["message"] = "sourceData";
        object["uuid"]    = source->uuid().toString();

        QJsonArray &ftdata = m_sourceJsons[source->uuid()];
        if (static_cast<unsigned int>(ftdata.size()) != source->size()) {
            ftdata = QJsonArray();
        }
        QJsonArray ftcell = {0, 0, 0, 0, 0, 0};
        for (unsigned int i = 0; i < source->size(); ++i) {
            ftcell[0] = static_cast<double>(source->frequency(i)  );
            ftcell[1] = static_cast<double>(source->module(i)     );
            ftcell[2] = static_cast<double>(source->magnitudeRaw(i)  );
            ftcell[3] = static_cast<double>(source->phase(i).arg());
            ftcell[4] = static_cast<double>(source->coherence(i)  );
            ftcell[5] = static_cast<double>(source->peakSquared(i));
            //ftcell[6] = static_cast<double>(source->meanSquared(i));

            if ( i >= static_cast<unsigned int>(ftdata.size())) {
                ftdata << ftcell;
            } else {
                ftdata[i] = ftcell;
            }
        }
        object["ftdata"] = ftdata;

        QJsonDocument document(object);
        return document.toJson(QJsonDocument::JsonFormat::Compact);
    }

    return {};
}

QJsonObject Server::prepareMessage(const QString &message) const
{
    QJsonObject object;
    object["api"] = "Open Sound Meter";
    object["version"] = APP_GIT_VERSION;
    object["host"] = QHostInfo::localHostName();
    object["message"] = message;
    object["uuid"] = m_uuid.toString();
    object["time"] = QTime::currentTime().toString();
    return object;
}

void Server::sourceNotify(const QUuid &id, const QString &message)
{
    if (active()) {
        auto object = prepareMessage(message);
        object["source"] = id.toString();
        QJsonDocument document(object);
        sendMulticast(document.toJson(QJsonDocument::JsonFormat::Compact));
    }
}

void Server::sendMulticast(const QByteArray &data)
{
    QMetaObject::invokeMethod(
        &m_network,
        "sendUDP",
        Qt::QueuedConnection,
        Q_ARG(QByteArray, std::move(data)),
        Q_ARG(QString, m_network.MULTICAST_IP));
}

void Server::sendHello()
{
    auto object = prepareMessage("hello");
    object["port"] = m_network.port();
    object["multicast"] = m_network.MULTICAST_IP;
    QJsonArray sources {};
    for (auto *source : *m_sourceList) {
        if (source && !dynamic_cast<remote::Item *>(source)) {
            sources.push_back(source->uuid().toString());
        }
    }

    object["sources"] = sources;
    QJsonDocument document(object);
    QByteArray data = document.toJson(QJsonDocument::JsonFormat::Compact);
    QMetaObject::invokeMethod(
        &m_network,
        "sendUDP",
        Qt::QueuedConnection,
        Q_ARG(QByteArray, std::move(data))
    );
}

} // namespace remote
