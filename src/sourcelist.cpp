/**
 *  OSM
 *  Copyright (C) 2019  Pavel Smokotnin

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
#include "sourcelist.h"
#include "measurement.h"
#include "union.h"
#include "elc.h"
#include "common/wavfile.h"
#include <qmath.h>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>

SourceList::SourceList(QObject *parent, bool appendMeasurement) :
    QObject(parent),
    m_items(0),
    m_currentFile(),
    m_colorIndex(3),
    m_selected(-1)
{
    if (appendMeasurement) {
        add<Measurement>();
    }
}
SourceList *SourceList::clone(QObject *parent, bool filtered) const noexcept
{
    SourceList *list = new SourceList(parent, false);
    for (auto item : items()) {
        if (!filtered || item->objectName() == "Measurement" || item->objectName() == "Stored") {
            list->appendItem(item);
        }
    }

    connect(this, &SourceList::preItemRemoved, list, [ = ](int index) {
        if (!list) return;
        auto item = get(index);
        list->removeItem(item, false);
    });
    connect(this, &SourceList::postItemAppended, list, [ = ](auto item) {
        list->appendItem(item, false);
    });
    connect(this, &SourceList::preItemMoved, list, [ = ](int from, int to) {
        list->move(from, to);
    });

    return list;
}

chart::Source *SourceList::firstSource() const noexcept
{
    return m_items.at(0);
}

const QVector<chart::Source *> &SourceList::items() const
{
    return m_items;
}

SourceList::iterator SourceList::begin() noexcept
{
    return m_items.begin();
}
SourceList::iterator SourceList::end() noexcept
{
    return m_items.end();
}
int SourceList::count() const noexcept
{
    return m_items.size();
}
QUrl SourceList::currentFile() const noexcept
{
    return m_currentFile;
}
chart::Source *SourceList::get(int i) const noexcept
{
    if (i < 0 || i >= m_items.size())
        return nullptr;

    return m_items.at(i);
}
void SourceList::clean() noexcept
{
    m_selected = -1;
    emit selectedChanged();
    while (m_items.size() > 0) {
        emit preItemRemoved(0);
        auto item = get(0);
        m_items.removeAt(0);
        emit postItemRemoved();
        item->deleteLater();
    }
    m_colorIndex = 3;
}
void SourceList::reset() noexcept
{
    clean();
    add<Measurement>();
}
bool SourceList::move(int from, int to) noexcept
{
    if (from == to)
        return false;

    if (from < to) {
        return move(to, from);
    }

    emit preItemMoved(from, to);
    if (m_items.size() > from) {
        chart::Source *item = m_items.takeAt(from);
        m_items.insert((to > from ? to - 1 : to), item);
    } else {
        qWarning() << "move element from out the bounds";
    }
    emit postItemMoved();

    return true;
}

int SourceList::indexOf(chart::Source *item) const noexcept
{
    return m_items.indexOf(item);
}
bool SourceList::save(const QUrl &fileName) const noexcept
{
    QFile saveFile(fileName.toLocalFile());
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open file");
        return false;
    }

    QJsonObject object;
    object["type"] = "sourcelsist";

    QJsonArray data;
    for (int i = 0; i < m_items.size(); ++i) {
        auto item = m_items.at(i);
        QJsonObject itemJson;
        itemJson["type"] = item->objectName();
        itemJson["data"] = item->toJSON(this);
        data.append(itemJson);
    }
    object["list"] = data;
    object["selected"] = m_selected;

    QJsonDocument document(object);
    if (saveFile.write(document.toJson(QJsonDocument::JsonFormat::Compact)) != -1) {
        return true;
    }

    return false;
}
bool SourceList::load(const QUrl &fileName) noexcept
{
    QFile loadFile(fileName.toLocalFile());
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open file");
        return false;
    }
    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadedDocument(QJsonDocument::fromJson(saveData));
    if (loadedDocument.isNull() || loadedDocument.isEmpty())
        return false;

    enum LoadType {ListType, StoredType};
    static std::map<QString, LoadType> typeMap = {
        {"sourcelsist", ListType},
        {"stored",      StoredType},
    };

    if (typeMap.find(loadedDocument["type"].toString()) != typeMap.end()) {
        switch (typeMap.at(loadedDocument["type"].toString())) {
        case ListType:
            m_currentFile = fileName;
            return loadList(loadedDocument, fileName);

        case StoredType:
            return loadObject<Stored>(loadedDocument["data"].toObject());
        }
    }

    return false;
}

bool SourceList::import(const QUrl &fileName, const int &type) noexcept
{
    switch (type) {
    case TRANSFER_TXT:
        return importFile(fileName, "\t");

    case TRANSFER_CSV:
        return importFile(fileName, ",");

    case IMPULSE_TXT:
        return importImpulse(fileName, "\t");

    case IMPULSE_CSV:
        return importImpulse(fileName, ",");

    case IMPULSE_WAV:
        return importWav(fileName);
    }
    return false;
}

bool SourceList::importFile(const QUrl &fileName, QString separator) noexcept
{
    QFile file(fileName.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open file");
        return false;
    }

    QString notes = "Imported from " + fileName.toDisplayString(QUrl::PreferLocalFile);
    char line[1024];
    bool fOk, mOk;
    float frequency, magnitude = 0.f, coherence = 1.f, maxMagnitude = -100;
    complex phase;

    std::vector<chart::Source::FTData> d;
    d.reserve(480); //48 ppo
    auto s = new Stored();

    while (file.readLine(line, 1024) > 0) {
        QString qLine(line);
        auto list = qLine.split(separator);
        fOk = mOk = false;

        if (list.size() > 1) {
            frequency = list[0].replace(",", ".").toFloat(&fOk);
            magnitude = list[1].replace("*", "0").replace(",", ".").toFloat(&mOk);
            phase.polar(M_PI * (list.size() > 2 ? list[2].replace("*", "0").replace(",", ".").toFloat() : 0) / 180.f);
            coherence = (list.size() > 3 ? list[3].replace("*", "0").replace(",", ".").toFloat() : 1);

            if (fOk && mOk && list.size() > 1) {
                if (magnitude > maxMagnitude) {
                    maxMagnitude = magnitude;
                }

                d.push_back({
                    frequency,
                    magnitude,
                    magnitude,
                    phase,
                    coherence,
                    magnitude,
                    NAN
                });
            }
        }
    }

    for (auto &row : d) {
        row.module = std::pow(10.f, (row.module - maxMagnitude) / 20.f);
        row.magnitude = std::pow(10.f, (row.magnitude) / 20.f);
    }
    s->copyFrom(d.size(), 0, d.data(), nullptr);
    s->setName(fileName.fileName());
    s->setNotes(notes);
    s->setActive(true);
    appendItem(s, true);
    return true;
}
bool SourceList::importImpulse(const QUrl &fileName, QString separator) noexcept
{
    QFile file(fileName.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open file");
        return false;
    }

    QString notes = "Imported from " + fileName.toDisplayString(QUrl::PreferLocalFile);
    char line[1024];
    float time, value;

    std::vector<chart::Source::TimeData> d;
    d.reserve(6720); //140ms @ 48kHz
    auto s = new Stored();

    while (file.readLine(line, 1024) > 0) {
        QString qLine(line);
        auto list = qLine.split(separator);

        if (list.size() > 1) {
            time = list[0].replace(",", ".").toFloat();
            value = list[1].replace("*", "0").replace(",", ".").toFloat();

            d.push_back({
                time,
                value
            });

        }
    }

    s->copyFrom(0, d.size(), nullptr, d.data());
    s->setName(fileName.fileName());
    s->setNotes(notes);
    s->setActive(true);
    appendItem(s, true);
    return true;
}
bool SourceList::importWav(const QUrl &fileName) noexcept
{
    WavFile wav;
    if (!wav.load(fileName.toLocalFile())) {
        qWarning("Couldn't open file");
        return false;
    }

    //TODO: load meta from file
    QString notes = "Imported from " + fileName.toDisplayString(QUrl::PreferLocalFile);
    float time = 0, value = 0, dt = 1.f / wav.sampleRate(), maxValue = 0, offset = 0;
    bool finished = false;

    std::vector<chart::Source::TimeData> d;
    d.reserve(6720); //140ms @ 48kHz
    auto s = new Stored();

    value = wav.nextSample(false, &finished);
    while (!finished) {
        d.push_back({
            time,
            value
        });

        if (maxValue < std::abs(value)) {
            maxValue = std::abs(value);
            offset = time;
        }
        value = wav.nextSample(false, &finished);
        time += dt;
    }

    for (auto &row : d) {
        row.time -= offset;
    }

    s->copyFrom(0, d.size(), nullptr, d.data());
    //s->restoreFromImpulse();
    s->setName(fileName.fileName());
    s->setNotes(notes);
    s->setActive(true);
    appendItem(s, true);
    return true;
}
int SourceList::selectedIndex() const
{
    return m_selected;
}

chart::Source *SourceList::selected() const noexcept
{
    return m_items.value(m_selected);
}

void SourceList::setSelected(int selected)
{
    if (m_selected != selected) {
        m_selected = selected;
        emit selectedChanged();
    }
}

bool SourceList::loadList(const QJsonDocument &document, const QUrl &fileName) noexcept
{
    enum LoadType {MeasurementType, StoredType, UnionType, ElcType};
    static std::map<QString, LoadType> typeMap = {
        {"Measurement", MeasurementType},
        {"Stored",      StoredType},
        {"Union",       UnionType},
        {"ELC",         ElcType},
    };

    clean();
    QJsonArray list = document["list"].toArray();

    for (const auto item : list) {
        auto object = item.toObject();

        if (typeMap.find(object["type"].toString()) == typeMap.end())
            continue;

        switch (typeMap.at(object["type"].toString())) {
        case MeasurementType:
            loadObject<Measurement>(object["data"].toObject());
            break;

        case StoredType:
            loadObject<Stored>(object["data"].toObject());
            break;

        case UnionType:
            loadObject<Union>(object["data"].toObject());
            break;

        case ElcType:
            loadObject<ELC>(object["data"].toObject());
            break;
        }
    }
    setSelected(document["selected"].toInt(-1));

    emit loaded(fileName);
    return true;
}
template<typename T> bool SourceList::loadObject(const QJsonObject &data) noexcept
{
    if (data.isEmpty())
        return false;

    auto s = new T();
    s->fromJSON(data, this);
    appendItem(s, false);
    nextColor();
    return true;
}
template<typename T> T *SourceList::add() noexcept
{
    auto *t = new T();
    appendItem(t, true);
    return t;
}
Union *SourceList::addUnion()
{
    return add<Union>();
}
ELC *SourceList::addElc()
{
    return add<ELC>();
}
Measurement *SourceList::addMeasurement()
{
    return add<Measurement>();
}
void SourceList::appendNone()
{
    m_items.prepend(nullptr);
}
void SourceList::appendItem(chart::Source *item, bool autocolor)
{
    emit preItemAppended();

    if (autocolor) {
        item->setColor(nextColor());
    }
    m_items.append(item);
    emit postItemAppended(item);
}
void SourceList::removeItem(chart::Source *item, bool deleteItem)
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i) == item) {
            auto item = get(i);
            emit preItemRemoved(i);
            m_items.removeAt(i);
            emit postItemRemoved();
            if (deleteItem)
                item->deleteLater();
            break;
        }
    }
}
void SourceList::cloneItem(chart::Source *item)
{
    auto newItem = item->clone();
    if (newItem) {
        appendItem(newItem, true);
    }
}
QColor SourceList::nextColor()
{
    m_colorIndex += 3;
    if (m_colorIndex >= m_colors.length()) {
        m_colorIndex -= m_colors.length();
    }

    return m_colors.at(m_colorIndex);
}
