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
#include "standardline.h"
#include "filtersource.h"
#include "source/sourcewindowing.h"
#include "common/wavfile.h"
#include <qmath.h>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>

SourceList::SourceList(QObject *parent, bool appendMeasurement) :
    QObject(parent),
    m_items(0), m_checked(),
    m_currentFile(),
    m_colorIndex(3),
    m_selected(-1),
    m_mutex()
{
    if (appendMeasurement) {
        add<Measurement>();
    }
}
SourceList *SourceList::clone(QObject *parent, QUuid filter) const
{
    SourceList *list = new SourceList(parent, false);
    for (auto item : items()) {
        if (filter.isNull() || filter != item->uuid()) {
            list->appendItem(item);
        }
    }

    connect(this, &SourceList::preItemRemoved, list, [ = ](int index) {
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

QUuid SourceList::firstSource() const noexcept
{
    if (m_items.count() == 0) {
        return {};
    }
    auto first = m_items.at(0);
    return first ? first->uuid() : QUuid();
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

std::lock_guard<std::mutex> SourceList::lock() const
{
    return std::lock_guard<std::mutex> {m_mutex};
}
chart::Source *SourceList::getByUUid(QUuid id) const noexcept
{
    auto result = std::find_if(m_items.cbegin(), m_items.cend(), [&id](chart::Source * source) {
        return source && source->uuid() == id;
    });
    if (result != m_items.end()) {
        return *result;
    }
    return nullptr;
}

QUuid SourceList::getUUid(int i) const noexcept
{
    if (i < 0 || i >= m_items.size())
        return QUuid{};

    return m_items.at(i)->uuid();
}

void SourceList::clean() noexcept
{
    auto guard = lock();
    m_selected = -1;
    m_checked.clear();
    emit selectedChanged();
    while (m_items.size() > 0) {
        emit preItemRemoved(0);
        auto item = get(0);
        m_items.removeAt(0);
        emit postItemRemoved();
        item->destroy();
    }
    m_colorIndex = 3;
}
void SourceList::reset() noexcept
{
    clean();
    m_checked.clear();
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

int SourceList::indexOf(const QUuid &id) const noexcept
{
    for (auto &e : m_items) {
        if (e) {
            if (e->uuid() == id) {
                return m_items.indexOf(e);
            }
        }
    }

    return -1;
}

bool SourceList::save(const QUrl &fileName) const noexcept
{
    QFile saveFile(fileName.toLocalFile());
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open file");
        return false;
    }
    auto guard = lock();

    QJsonObject object;
    object["type"] = "sourcelsist";

    QJsonArray data;
    for (int i = 0; i < m_items.size(); ++i) {
        auto item = m_items.at(i);
        if (!item) {
            continue;
        }
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

bool SourceList::import(const QUrl &fileName, const int &type)
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

bool SourceList::importFile(const QUrl &fileName, QString separator)
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
        row.magnitude = std::pow(10.f, (row.magnitude - (maxMagnitude > 30 ? maxMagnitude : 0)) / 20.f);
    }
    s->copyFrom(d.size(), 0, d.data(), nullptr);
    s->setName(fileName.fileName());
    s->setNotes(notes);
    s->setActive(true);
    appendItem(s, true);
    return true;
}

QList<QUuid> SourceList::checked() const
{
    return m_checked;
}

void SourceList::setChecked(const QList<QUuid> &checked)
{
    m_checked = checked;
}
bool SourceList::importImpulse(const QUrl &fileName, QString separator)
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
bool SourceList::importWav(const QUrl &fileName)
{
    WavFile wav;
    if (!wav.load(fileName.toLocalFile())) {
        qWarning("Couldn't open file");
        return false;
    }

    //TODO: load meta from file
    QString notes = "Imported from " + fileName.toDisplayString(QUrl::PreferLocalFile);
    float time = 0, value = 0, dt = 1000.f / wav.sampleRate(), maxValue = 0, offset = 0;
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

void SourceList::check(const QUuid item)
{
    if (!isChecked(item)) {
        m_checked.push_back(item);
    }
}

void SourceList::uncheck(const QUuid item)
{
    m_checked.removeAll(item);
}

void SourceList::checkAll()
{
    for (auto &item : m_items) {
        if (item) {
            check(item->uuid());
        }
    }
}

void SourceList::uncheckAll()
{
    m_checked.clear();
}

bool SourceList::isChecked(const QUuid &item) const noexcept
{
    if (item.isNull()) {
        return false;
    }
    return m_checked.contains(item);
}

int SourceList::checkedCount() const
{
    return m_checked.count();
}

QUuid SourceList::firstChecked() const noexcept
{
    if (m_checked.length() == 0) {
        return {};
    }
    return m_checked.at(0);
}

bool SourceList::loadList(const QJsonDocument &document, const QUrl &fileName) noexcept
{
    enum LoadType {MeasurementType, StoredType, UnionType, StandardLineType, FilterType, WindowingType};
    static std::map<QString, LoadType> typeMap = {
        {"Measurement",  MeasurementType},
        {"Stored",       StoredType},
        {"Union",        UnionType},
        {"StandardLine", StandardLineType},
        {"Filter",       FilterType},
        {"Windowing",    WindowingType}
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

        case StandardLineType:
            loadObject<StandardLine>(object["data"].toObject());
            break;

        case FilterType:
            loadObject<FilterSource>(object["data"].toObject());
            break;

        case WindowingType:
            loadObject<Windowing>(object["data"].toObject());
            break;
        }
    }
    setSelected(document["selected"].toInt(-1));

    emit loaded(fileName);
    return true;
}
template<typename T> bool SourceList::loadObject(const QJsonObject &data)
{
    if (data.isEmpty())
        return false;

    auto s = new T(this);
    s->fromJSON(data, this);
    appendItem(s, false);
    nextColor();
    return true;
}
template<typename T> T *SourceList::add()
{
    auto *t = new T(this);
    appendItem(t, true);
    return t;
}
Union *SourceList::addUnion()
{
    return add<Union>();
}
StandardLine *SourceList::addStandardLine()
{
    return add<StandardLine>();
}

FilterSource *SourceList::addFilter()
{
    return add<FilterSource>();
}

Windowing *SourceList::addWindowing()
{
    return add<Windowing>();
}
Measurement *SourceList::addMeasurement()
{
    return add<Measurement>();
}
int SourceList::appendNone()
{
    m_items.prepend(nullptr);
    return 0;
}
int SourceList::appendAll()
{
    m_items.prepend(nullptr);
    return 0;
}
void SourceList::appendItem(chart::Source *item, bool autocolor)
{
    auto guard = lock();
    emit preItemAppended();

    if (autocolor) {
        item->setColor(nextColor());
    }
    m_items.append(item);
    emit postItemAppended(item);
}
void SourceList::removeItem(chart::Source *item, bool deleteItem)//TODO:uuid
{
    auto guard = lock();
    if (!item) {
        return;
    }
    m_checked.removeAll(item->uuid());
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i) == item) {
            auto item = get(i);
            emit preItemRemoved(i);
            m_items.removeAt(i);
            emit postItemRemoved();
            if (deleteItem)
                item->destroy();
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

void SourceList::storeItem(chart::Source *item)
{
    chart::Source *newItem = nullptr;
    QMetaObject::invokeMethod(
        item,
        "store",
        Qt::DirectConnection,
        Q_RETURN_ARG(chart::Source *, newItem));

    if (newItem) {
        newItem->setActive(true);
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
