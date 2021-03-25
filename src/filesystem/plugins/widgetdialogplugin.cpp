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

    QString directory = {};
    switch (folder()) {
    case Dialog::Home:
        directory = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        break;
    case Dialog::Documents:
        directory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        break;
    case Dialog::Images:
        directory = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        break;
    }
    fileDialog->setDirectory(directory);

    auto code = fileDialog->exec();
    switch (code) {
    case QDialog::Accepted:
        emit accepted(fileDialog->selectedFiles(), fileDialog->selectedNameFilter());
        break;
    case QDialog::Rejected:
        emit rejected();
        break;
    }

    fileDialog->deleteLater();
    fileDialog = nullptr;
}

} // namespace filesystem
