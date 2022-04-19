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
#ifndef PROFILER_H
#define PROFILER_H

#include <QString>
#include <QMap>
#include <chrono>

class Profiler
{
    using time_type = std::chrono::time_point<std::chrono::system_clock>;
    using time_interval = std::chrono::microseconds;
public:
    explicit Profiler(QString name, unsigned int divider = 60);
    ~Profiler();

private:
    time_type m_begin;
    unsigned int m_divider;
    QString m_name;
    static QMap<QString, std::pair<int, time_interval>> m_calls;
};

#endif // PROFILER_H
