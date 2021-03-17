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
#include "appearance.h"
#include <QGuiApplication>
#include <QPalette>

Appearance::Appearance(Settings *settings) : QObject(settings)
{
    auto app = qobject_cast<QGuiApplication *>(QGuiApplication::instance());
    connect(app, &QGuiApplication::paletteChanged, this, &Appearance::setDarkModeFromSystem);

    if (
        (settings->value("darkMode").type() != QVariant::Bool) ||
        (darkModeFromSystem() && !darkMode())
    ) {
        setDarkModeFromSystem();
    }
}

bool Appearance::darkMode() const
{
    auto store = settings();
    Q_ASSERT(store);

    auto value = store->value("darkMode");
    return value.toBool();
}

void Appearance::setDarkMode(const bool &setDark)
{
    if (setDark == darkMode()) {
        return;
    }
    auto store = settings();
    Q_ASSERT(store);

    store->setValue("darkMode", setDark);
    emit darkModeChanged(setDark);
}

bool Appearance::setDarkModeFromSystem()
{
    bool darkMode = darkModeFromSystem();

    setDarkMode(darkMode);
    return darkMode;
}

Settings *Appearance::settings() const
{
    return qobject_cast<Settings *>(parent());
}

bool Appearance::darkModeFromSystem() const
{
    QColor middleGrey(127, 127, 127);
    auto app = qobject_cast<QGuiApplication *>(QGuiApplication::instance());
    return middleGrey.lightnessF() > app->palette().color(QPalette::Window).lightnessF();
}
