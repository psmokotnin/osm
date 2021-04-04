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
#include "dialogPlugin.h"
#include <QStandardPaths>

namespace filesystem {

DialogPlugin::DialogPlugin(QObject *parent) : QObject(parent), m_title(),
    m_startFolder(StandardFolder::Home), m_mode(Dialog::OpenFile),
    m_defaultSuffix(), m_nameFilters()
{
}

DialogPlugin::~DialogPlugin() = default;

QString DialogPlugin::title() const
{
    return m_title;
}

void DialogPlugin::setTitle(const QString &title)
{
    m_title = title;
}

QString DialogPlugin::defaultSuffix() const
{
    return m_defaultSuffix;
}

void DialogPlugin::setDefaultSuffix(const QString &defaultSuffix)
{
    m_defaultSuffix = defaultSuffix;
}

QStringList DialogPlugin::nameFilters() const
{
    return m_nameFilters;
}

void DialogPlugin::setNameFilters(const QStringList &nameFilters)
{
    m_nameFilters = nameFilters;
}

Dialog::Mode DialogPlugin::mode() const
{
    return m_mode;
}

void DialogPlugin::setMode(Dialog::Mode mode)
{
    m_mode = mode;
}

StandardFolder DialogPlugin::folder() const
{
    return m_startFolder;
}

QUrl DialogPlugin::folderURL() const
{
    QString directory;
    switch (folder()) {
    case StandardFolder::Home:
        directory = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        break;
    case StandardFolder::Documents:
        directory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        break;
    case StandardFolder::Images:
        directory = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        break;
    case StandardFolder::Music:
        directory = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
        break;
    case StandardFolder::Movies:
        directory = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
        break;
    case StandardFolder::Desktop:
        directory = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        break;
    }

    return QUrl::fromLocalFile(directory);
}

void DialogPlugin::setFolder(StandardFolder folder)
{
    m_startFolder = folder;
}

} // namespace filesystem
