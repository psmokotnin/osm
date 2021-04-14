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
#include <qmath.h>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>

SourceList::SourceList(QObject *parent, bool appendMeasurement) :
    QObject(parent),
    m_currentFile(),
    m_colorIndex(3),
    m_selected(-1)
{
    if (appendMeasurement) {
        addMeasurement();
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
const QVector<Fftchart::Source *> &SourceList::items() const
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
Fftchart::Source *SourceList::get(int i) const noexcept
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
    addMeasurement();
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
        Fftchart::Source *item = m_items.takeAt(from);
        m_items.insert((to > from ? to - 1 : to), item);
    } else {
        qWarning() << "move element from out the bounds";
    }
    emit postItemMoved();

    return true;
}

int SourceList::indexOf(Fftchart::Source *item) const noexcept
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
        itemJson["data"] = item->toJSON();
        data.append(itemJson);
    }
    object["list"] = data;

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

    enum LoadType {List, Stored};
    static std::map<QString, LoadType> typeMap = {
        {"sourcelsist", List},
        {"stored",      Stored},
    };

    if (typeMap.find(loadedDocument["type"].toString()) != typeMap.end()) {
        switch (typeMap.at(loadedDocument["type"].toString())) {
        case List:
            m_currentFile = fileName;
            return loadList(loadedDocument);

        case Stored:
            return loadStored(loadedDocument["data"].toObject());
        }
    }

    return false;
}

bool SourceList::importAuto(const QUrl &fileName) noexcept
{
    QFileInfo info(fileName.toLocalFile());
    auto suffix = info.suffix();
    if (suffix == "txt") {
        return importTxt(fileName);
    }
    if (suffix == "csv") {
        return importCsv(fileName);
    }
    return false;
}

bool SourceList::importTxt(const QUrl &fileName) noexcept
{
    return importFile(fileName, "\t");
}
bool SourceList::importCsv(const QUrl &fileName) noexcept
{
    return importFile(fileName, ",");
}
bool SourceList::importFile(const QUrl &fileName, QString separator) noexcept
{
    QFile file(fileName.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open file");
        return false;
    }

    QString notes;
    char line[1024];
    bool fOk, mOk;
    float frequency, magnitude, maxMagnitude = -100, coherence = 1.f;
    complex phase;

    std::vector<Fftchart::Source::FTData> d;
    d.reserve(480); //48 ppo
    auto s = new Stored();

    while (file.readLine(line, 1024) > 0) {
        QString qLine(line);
        notes += line;
        auto list = qLine.split(separator);
        fOk = mOk = false;

        if (list.size() > 1) {
            frequency = list[0].replace(",", ".").toFloat(&fOk);
            magnitude = list[1].replace(",", ".").toFloat(&mOk);
            phase.polar(M_PI * (list.size() > 2 ? list[2].replace(",", ".").toFloat() : 0) / 180.f);
            coherence = (list.size() > 3 ? list[3].replace(",", ".").toFloat() : 1);

            if (fOk && mOk && list.size() > 1) {
                if (magnitude > maxMagnitude) {
                    maxMagnitude = magnitude;
                }

                d.push_back({
                    frequency,
                    magnitude,
                    0,
                    phase,
                    coherence
                });
            }
        }
    }

    for (auto &row : d) {
        auto m = row.module;
        row.module = std::pow(10.f, (m - maxMagnitude) / 20.f);
        row.magnitude = std::pow(10.f, (m - maxMagnitude + 15.f) / 20.f);
    }
    s->copyFrom(d.size(), 0, d.data(), nullptr);
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

Fftchart::Source *SourceList::selected() const noexcept
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

bool SourceList::loadList(const QJsonDocument &document) noexcept
{
    enum LoadType {Measurement, Stored};
    static std::map<QString, LoadType> typeMap = {
        {"Measurement", Measurement},
        {"Stored",      Stored},
    };

    clean();
    QJsonArray list = document["list"].toArray();

    for (const auto item : list) {
        auto object = item.toObject();

        if (typeMap.find(object["type"].toString()) == typeMap.end())
            continue;

        switch (typeMap.at(object["type"].toString())) {
        case Measurement:
            loadMeasurement(object["data"].toObject());
            break;

        case Stored:
            loadStored(object["data"].toObject());
            break;
        }
    }
    return true;
}
bool SourceList::loadMeasurement(const QJsonObject &data) noexcept
{
    if (data.isEmpty())
        return false;

    auto *m = new Measurement();
    m->fromJSON(data);
    appendItem(m, false);
    nextColor();

    return true;
}
bool SourceList::loadStored(const QJsonObject &data) noexcept
{
    if (data.isEmpty())
        return false;

    auto s = new Stored();
    s->fromJSON(data);
    appendItem(s, false);
    nextColor();
    return true;
}
Union *SourceList::addUnion()
{
    auto *s = new Union();
    appendItem(s, true);
    return s;
}

ELC *SourceList::addElc()
{
    auto *s = new ELC();
    appendItem(s, true);
    return s;
}
Measurement *SourceList::addMeasurement()
{
    auto *m = new Measurement();
    appendItem(m, true);
    return m;
}
void SourceList::appendNone()
{
    m_items.prepend(nullptr);
}
void SourceList::appendItem(Fftchart::Source *item, bool autocolor)
{
    emit preItemAppended();

    if (autocolor) {
        item->setColor(nextColor());
    }
    m_items.append(item);

    emit postItemAppended(item);
}
void SourceList::removeItem(Fftchart::Source *item, bool deleteItem)
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

void SourceList::cloneItem(Fftchart::Source *item)
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
