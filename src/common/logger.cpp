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
#include "logger.h"
#include <iostream>
#include <QFile>
#include <workingfolder.h>

void logger::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    QString formatedMessage = qFormatLogMessage(type, context, message);
    static QString lastMessage = "";
    static QFile outFile(workingfolder::logFilePath());
    if (!outFile.isOpen()) {
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    }
    QTextStream textStream(&outFile);

    if (outFile.size() > 1024 * 1024) {
        outFile.resize(0);
    }

    QString preLog = context.file;
    preLog += "(" + QString::number(context.line) + "):";

    switch (type) {
    case QtDebugMsg:
        preLog += "Debug: ";
        break;
    case QtWarningMsg:
        preLog += "Warning: ";
        break;
    case QtCriticalMsg:
        preLog += "Critical: ";
        break;
    case QtFatalMsg:
        preLog += "Fatal: ";
        break;
    case QtInfoMsg:
        preLog += "Info: ";
        break;
    }
    if (formatedMessage != lastMessage && outFile.isOpen()) {
        textStream << preLog << formatedMessage << Qt::endl;
    }
    lastMessage = formatedMessage;

    std::cout << formatedMessage.toStdString() << std::endl;
}
