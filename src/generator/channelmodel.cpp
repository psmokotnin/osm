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
#include "channelmodel.h"
#include <algorithm>

GeneratorChannelModel::GeneratorChannelModel(QObject *parent) : QAbstractListModel(parent),
    m_data(), m_noneTitle("None"), m_allTitle("All"), m_generator(nullptr)
{

}

int GeneratorChannelModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_data.size();
}

QVariant GeneratorChannelModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || m_data.empty()) {
        return QVariant();
    }

    unsigned int i = index.row();
    if (i >= m_data.size()) {
        return QVariant();
    }

    auto &row = m_data[i];
    QVariant r = {};

    switch (role) {
    case NameRole:
        r.setValue(std::get<0>(row));
        break;

    case TitleRole:
        if (i == m_noneIndex) {
            r.setValue(m_noneTitle);
        } else if (i == m_allIndex) {
            r.setValue(m_allTitle);
        } else {
            r.setValue(std::get<0>(row));
        }
        break;

    case CheckedRole:
        r.setValue(std::get<1>(row));
        break;

    case CheckedAbleRole:
        if (i == m_noneIndex || i == m_allIndex) {
            r.setValue(false);
        } else {
            r.setValue(true);
        }
        break;

//    case ColorRole:
//        r.setValue(std::get<2>(row));
//        break;
    }
    return r;
}

Qt::ItemFlags GeneratorChannelModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsSelectable;
}

QHash<int, QByteArray> GeneratorChannelModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[NameRole]     = "name";
    names[TitleRole]    = "title";
    names[CheckedRole]  = "checked";
    names[CheckedAbleRole] = "checkable";
    //names[ColorRole]    = "color";
    return names;
}

QStringList GeneratorChannelModel::list() const
{
    return m_list;
}

void GeneratorChannelModel::setList(const QStringList &data)
{
    beginResetModel();

    m_list = data;
    m_data.clear();
    m_data.reserve(data.size() + 2);
    m_data.push_back({m_allTitle, 0, -1});
    m_data.push_back({m_noneTitle, 0, -1});

    QSet<int> checked = (m_generator ? m_generator->channels() : QSet<int>());

    int index = 0;
    for (auto &name : data) {
        m_data.push_back({name, checked.contains(index), index});
        ++index;
    }

    endResetModel();
    emit listChanged();
    emit checkedChanged();
}

void GeneratorChannelModel::check(const int &index, const bool &checked) noexcept
{
    if (index == m_allIndex) {
        checkAll(true);
    } else if (index == m_noneIndex) {
        checkAll(false);
    } else {
        std::get<1>(m_data[index]) = checked;
    }
    emit checkedChanged();
}

int GeneratorChannelModel::checkedCount() const
{
    if (m_data.size() < 2) {
        return 0;
    }
    return std::accumulate(m_data.cbegin() + 2, m_data.cend(), 0, [](auto a, auto b) -> int {
        return a + std::get<1>(b);
    });
}

QStringList GeneratorChannelModel::checked() const
{
    QStringList list{};
    if (m_data.size() < 2) {
        return list;
    }
    return std::accumulate(m_data.cbegin() + 2, m_data.cend(), list, [](auto a, auto b) {
        if (std::get<1>(b)) {
            return a << std::get<0>(b);
        }
        return a;
    });
}

QSet<int> GeneratorChannelModel::checkedIndexes() const
{
    QSet<int> list{};
    if (m_data.size() < 2) {
        return list;
    }
    return std::accumulate(m_data.cbegin() + 2, m_data.cend(), list, [ = ](auto a, auto b) {
        if (std::get<1>(b)) {
            return a << std::get<2>(b);
        }
        return a;
    });
}

void GeneratorChannelModel::checkAll(const bool &checked)
{
    if (m_data.size() < 2) {
        return ;
    }
    std::for_each(m_data.begin() + 2, m_data.end(), [checked](auto & element) {
        std::get<1>(element) = checked;
    });
}
