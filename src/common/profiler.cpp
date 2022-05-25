/**
 *  OSM
 *  Copyright (C) 2021  Pavel Smokotnin

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
#include "profiler.h"
#include <QDebug>

QMap<QString, std::pair<int, Profiler::time_interval>> Profiler::m_calls = {};

Profiler::Profiler(QString name, unsigned int divider) : m_divider(divider), m_name(name)
{
    m_begin = std::chrono::system_clock::now();
}

Profiler::~Profiler()
{
    auto &l = m_calls[m_name];
    l.first++;
    auto interval = std::chrono::system_clock::now() - m_begin;
    l.second +=  std::chrono::duration_cast<time_interval>(interval);
    if (m_calls[m_name].first % m_divider == 0) {
        qDebug() << "Profile " << m_name <<
                 " average:" <<  (m_calls[m_name].second.count() / m_calls[m_name].first) << "us";
        m_calls[m_name] = {};
    }
}
