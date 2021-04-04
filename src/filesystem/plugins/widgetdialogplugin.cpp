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
#include "widgetdialogplugin.h"
#include <QStandardPaths>
#include <QQuickWindow>
#include <QFileDialog>

namespace filesystem {

WidgetDialogPlugin::WidgetDialogPlugin(QObject *parent) : DialogPlugin(parent)
{
}

void WidgetDialogPlugin::open(const QQuickWindow *window)
{
    Q_UNUSED(window);
    auto fileDialog = new QFileDialog(nullptr, Qt::Popup);
    switch (mode()) {
    case Dialog::OpenFile:
        fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
        break;
    case Dialog::SaveFile:
        fileDialog->setAcceptMode(QFileDialog::AcceptSave);
        break;
    }

    fileDialog->setFileMode(QFileDialog::AnyFile);
    fileDialog->setNameFilters(nameFilters());
    fileDialog->setDefaultSuffix(defaultSuffix());
    fileDialog->setWindowTitle(title());

    QString directory = folderURL().toLocalFile();
    fileDialog->setDirectory(directory);

    auto code = fileDialog->exec();
    switch (code) {
    case QDialog::Accepted: {
        QStringList paths = fileDialog->selectedFiles();
        QList<QUrl> files = {};
        for (auto &&path : paths) {
            files.push_back(QUrl::fromLocalFile(path));
        }
        emit accepted(files, fileDialog->selectedNameFilter());
    }
    break;
    case QDialog::Rejected:
        emit rejected();
        break;
    }

    fileDialog->deleteLater();
    fileDialog = nullptr;
}

} // namespace filesystem
