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
#include "autosaver.h"
#include "sourcelist.h"
#include "settings.h"
#include "workingfolder.h"

AutoSaver::AutoSaver(Settings *settings, SourceList *parent) : QObject(parent),
    m_settings(settings), m_timer()
{
    m_timer.setInterval(30'000); //30 sec
    m_timer.moveToThread(&m_timerThread);

    connect(&m_timer, &QTimer::timeout, this, &AutoSaver::save, Qt::DirectConnection);
    //Qt bug: &QThread::started doesn't work
    connect(&m_timerThread, SIGNAL(started()),  &m_timer, SLOT(start()), Qt::DirectConnection);
    connect(&m_timerThread, &QThread::finished, &m_timer, &QTimer::stop,  Qt::DirectConnection);

    m_timerThread.start();
    load();
}

AutoSaver::~AutoSaver()
{
    if (m_timerThread.isRunning()) {
        m_timerThread.quit();
        m_timerThread.wait();
    }
}

SourceList *AutoSaver::list() const
{
    return static_cast<SourceList *>(parent());
}

QUrl AutoSaver::fileName() const
{
    return "file:/" + workingfolder::autosaveFilePath();
}

void AutoSaver::load()
{
    auto file = m_settings->value(FILE_KEY).toUrl();
    if (!file.isValid()) {
        return;
    }

    //don't reload this file on next open if this failed/crashed
    m_settings->setValue(FILE_KEY, "");
    m_settings->flush();
    QUrl url(file);
    if (list()->load(url)) {
        //restore if we still alive
        m_settings->setValue(FILE_KEY, url);
    }
}

void AutoSaver::save()
{
    auto url = fileName();
    if (list()->save(url)) {
        m_settings->setValue(FILE_KEY, url);
    }
}

void AutoSaver::stop()
{
    m_timerThread.quit();
    m_timerThread.wait();
}
