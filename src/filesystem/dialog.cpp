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
#include "dialog.h"
#include "dialogPlugin.h"

#ifdef Q_OS_IOS
#   define FILE_DIALOG_PLUGIN IosDialogPlugin
#   include "plugins/iosdialogplugin.h"
#elif false
//  example plugin based on QWidgiet FileDialog:
#   define FILE_DIALOG_PLUGIN WidgetDialogPlugin
#   include "plugins/widgetdialogplugin.h"
#endif

namespace filesystem {

Dialog::Dialog(QQuickItem *parent) : QQuickItem(parent), m_plugin(nullptr), m_files()
{
#ifdef FILE_DIALOG_PLUGIN
    m_plugin = new FILE_DIALOG_PLUGIN(this);
    connect(m_plugin, &DialogPlugin::rejected, this, &Dialog::rejected);
    connect(m_plugin, &DialogPlugin::accepted, this, [this](auto files, auto selectedFilter) {
        m_selectedFilter = selectedFilter;
        m_files = files;
        emit accepted();
    });
#endif
}

QString Dialog::title() const
{
    Q_ASSERT(m_plugin);
    return m_plugin->title();
}

void Dialog::setTitle(const QString &title)
{
    Q_ASSERT(m_plugin);
    m_plugin->setTitle(title);
}

QString Dialog::defaultSuffix() const
{
    Q_ASSERT(m_plugin);
    return m_plugin->defaultSuffix();
}

void Dialog::setDefaultSuffix(const QString &defaultSuffix)
{
    Q_ASSERT(m_plugin);
    m_plugin->setDefaultSuffix(defaultSuffix);
}

QStringList Dialog::nameFilters() const
{
    Q_ASSERT(m_plugin);
    return m_plugin->nameFilters();
}

void Dialog::setNameFilters(const QStringList &nameFilters)
{
    Q_ASSERT(m_plugin);
    m_plugin->setNameFilters(nameFilters);
}

Dialog::Mode Dialog::mode() const
{
    Q_ASSERT(m_plugin);
    return m_plugin->mode();
}

void Dialog::setMode(Mode mode)
{
    Q_ASSERT(m_plugin);
    m_plugin->setMode(mode);
}

StandardFolder Dialog::folder() const
{
    Q_ASSERT(m_plugin);
    return m_plugin->folder();
}

void Dialog::setFolder(StandardFolder folder)
{
    Q_ASSERT(m_plugin);
    m_plugin->setFolder(folder);
}

void Dialog::open()
{
    Q_ASSERT(m_plugin);
    m_files.clear();
    m_plugin->open(window());
}

QUrl Dialog::fileUrl() const
{
    if (m_files.isEmpty()) {
        return {};
    }
    return m_files.first();
}

bool Dialog::selectExisting() const
{
    return (mode() == OpenFile);
}

void Dialog::setSelectExisting(const bool &existing)
{
    if (existing) {
        setMode(OpenFile);
    } else {
        setMode(SaveFile);
    }
}

QString Dialog::selectedNameFilter() const
{
    return m_selectedFilter;
}

} // namespace filesystem
