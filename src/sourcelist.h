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
#ifndef SOURCELIST_H
#define SOURCELIST_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QUrl>
#include "source/source_abstract.h"
#include "source/source_shared.h"


class Measurement;
class Union;
class Stored;
class StandardLine;
class FilterSource;
class Windowing;

class SourceList : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QUrl currentFile READ currentFile)
    Q_PROPERTY(QUuid first READ firstSource)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelected NOTIFY selectedChanged)//TODO: toUUID
    Q_PROPERTY(Source::Shared selected READ selected NOTIFY selectedChanged)
    Q_PROPERTY(QColor highlightColor READ highlightColor NOTIFY selectedChanged)
    Q_PROPERTY(bool isRoot READ isRoot CONSTANT)
    using iterator = QVector<Source::Shared>::iterator;
    using const_iterator = QVector<Source::Shared>::const_iterator;

public:
    explicit SourceList(QObject *parent = nullptr, bool appendMeasurement = true);
    SourceList *clone(QObject *parent, QUuid filter = {}, bool unrollGroups = false) const;

    int count() const noexcept;
    const QVector<Source::Shared> &items() const;
    SourceList::iterator begin() noexcept;
    SourceList::iterator end() noexcept;
    SourceList::const_iterator cbegin() const noexcept;
    SourceList::const_iterator cend() const noexcept;

    QUrl currentFile() const noexcept;

    const Source::Shared &get_ref(int i) const noexcept;
    unsigned size() const;

    Q_INVOKABLE Source::Shared get(int i) const noexcept;
    Q_INVOKABLE Source::Shared getByUUid(QUuid id) const noexcept;
    int getIndexByUUid(QUuid id) const noexcept;
    //Q_INVOKABLE Source::Shared getGroupByUUid(QUuid id) const noexcept;
    Q_INVOKABLE QUuid getUUid(int id) const noexcept;
    Q_INVOKABLE QUuid firstSource() const noexcept;
    Q_INVOKABLE void clean() noexcept;
    Q_INVOKABLE void reset() noexcept;
    Q_INVOKABLE bool save(const QUrl &fileName) const noexcept;
    Q_INVOKABLE bool load(const QUrl &fileName) noexcept;
    Q_INVOKABLE bool import(const QUrl &fileName, int type);
    Q_INVOKABLE bool importImpulse(const QUrl &fileName, QString separator);
    Q_INVOKABLE bool importWav(const QUrl &fileName) ;
    Q_INVOKABLE bool move(int from, int to) noexcept;
    Q_INVOKABLE void moveToGroup(QUuid targetId, QUuid groupId) noexcept;
    Q_INVOKABLE int indexOf(const Source::Shared &item) const noexcept;
    Q_INVOKABLE int indexOf(const QUuid &id) const noexcept;

    int selectedIndex() const;
    Source::Shared selected() const noexcept;
    QUuid selectedUuid() const noexcept;
    void setSelected(int selected);

    QColor highlightColor() const;

    void check(const QUuid item);
    void uncheck(const QUuid item);
    void checkAll();
    void uncheckAll();
    bool isChecked(const QUuid &item) const noexcept;
    int checkedCount() const;
    QUuid firstChecked() const noexcept;

    bool isRoot() const
    {
        return !!parent();
    }

    enum {
        TRANSFER_TXT    = 0,
        TRANSFER_CSV    = 1,
        IMPULSE_TXT     = 2,
        IMPULSE_CSV     = 3,
        IMPULSE_WAV     = 4
    };

    QList<QUuid> checked() const;
    void setChecked(const QList<QUuid> &checked);

    std::lock_guard<std::mutex> lock() const;

    QJsonArray  toJSON(const SourceList *list ) const noexcept;
    void        fromJSON(const QJsonArray &list, const SourceList *topList) noexcept;

public slots:
    Q_INVOKABLE QColor nextColor();

    Q_INVOKABLE Source::Shared  addMeasurement();
    Q_INVOKABLE Source::Shared  addUnion();
    Q_INVOKABLE Source::Shared  addStandardLine();
    Q_INVOKABLE Source::Shared  addFilter();
    Q_INVOKABLE Source::Shared  addWindowing();
    Q_INVOKABLE Source::Shared  addGroup();

    Q_INVOKABLE void appendItem(const Source::Shared &item, bool autocolor = false);
    Q_INVOKABLE void takeItem(Source::Shared item);
    void removeItem(const Source::Shared &item, bool deleteItem = true);
    Q_INVOKABLE void removeItem(const QUuid &uuid, bool deleteItem = true);
    Q_INVOKABLE void cloneItem(const Source::Shared &item);
    Q_INVOKABLE void storeItem(const Source::Shared &item);

    int appendNone();
    int appendAll();

signals:
    void preItemAppended();
    void postItemAppended(const Source::Shared &);

    void preItemRemoved(QUuid index);
    void postItemRemoved();

    void preItemMoved(int from, int to);
    void postItemMoved();

    void postItemChanged(const Source::Shared &, const QVector<int> &roles);

    void selectedChanged();
    void loaded(QUrl fileName);

    void countChanged();

private:
    bool loadList(const QJsonDocument &document, const QUrl &fileName) noexcept;
    template<typename T> bool loadObject(const QJsonObject &data, const SourceList *topList);
    template<typename T, typename... Ts> Source::Shared add(Ts...);
    bool importFile(const QUrl &fileName, QString separator);
    void appendItemsFrom(const SourceList *list, QUuid filter, bool unrollGroups);

    QVector<Source::Shared> m_items; //TODO: unordered_map<uuid, shared_ptr>
    QList<QUuid> m_checked{};
    QUrl m_currentFile;
    const QList<QColor> m_colors {
        "#F44336", "#FFEB3B", "#9C27B0", "#673AB7",
        "#3F51B5", "#2196F3", "#03A9F4", "#00BCD4",
        "#009688", "#4CAF50", "#8BC34A", "#CDDC39",
        "#E91E63", "#FFC107", "#FF9800", "#FF5722",
        "#795548", "#9E9E9E", "#607D8B"
    };
    int m_colorIndex;
    int m_selected;
    mutable std::mutex m_mutex;
};

#endif // SOURCELIST_H
