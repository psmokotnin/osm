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
#ifndef APPEARANCE_H
#define APPEARANCE_H

#include "settings.h"
#include <QWindow>

class Appearance : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool darkMode READ darkMode WRITE setDarkMode NOTIFY darkModeChanged)
    Q_PROPERTY(bool showAboutOnStartup READ showAboutOnStartup CONSTANT)
    Q_PROPERTY(bool experimentFunctions READ experimentFunctions WRITE setExperimentFunctions NOTIFY
               experimentFunctionsChanged)
    Q_PROPERTY(bool showMenuBar READ showMenuBar CONSTANT)
    Q_PROPERTY(QWindow::Visibility visibility READ visibility NOTIFY visibilityChanged)
    Q_PROPERTY(int cursorOffset READ cursorOffset CONSTANT)

public:
    explicit Appearance(Settings *settings = nullptr);

    enum Visibility {
        Automatic = QWindow::Visibility::AutomaticVisibility,
        FullScreen = QWindow::Visibility::FullScreen
    };
    Q_ENUM(Visibility)

    bool darkMode() const;
    void setDarkMode(const bool &setDark);

    bool experimentFunctions() const;
    void setExperimentFunctions(bool value);

    bool showMenuBar() const;
    bool showAboutOnStartup() const;

    QWindow::Visibility visibility() const;
    int cursorOffset() const;

public slots:
    bool setDarkModeFromSystem();

signals:
    void darkModeChanged(bool);
    void experimentFunctionsChanged(bool);
    void visibilityChanged();

private:
    Settings *settings() const;
    bool darkModeFromSystem() const;

};

#endif // APPEARANCE_H
