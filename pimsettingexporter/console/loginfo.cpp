/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "loginfo.h"
#include <QDebug>

LogInfo::LogInfo(QObject *parent)
    : QObject(parent)
{

}

LogInfo::~LogInfo()
{

}

void LogInfo::addInfoLogEntry(const QString &log)
{
    addLogLine(log, AddInfo);
}

void LogInfo::addErrorLogEntry(const QString &log)
{
    addLogLine(log, AddError);
}

void LogInfo::addTitleLogEntry(const QString &log)
{
    addLogLine(log, AddTitle);
}

void LogInfo::addEndLineLogEntry()
{
    addLogLine(QString(), AddEndLine);
}

void LogInfo::addLogLine(const QString &message, LogType type)
{
    QString newMessage;
    switch (type) {
    case AddEndLine:
        newMessage = QLatin1Char('\n');
        break;
    case AddInfo:
        newMessage = QStringLiteral("INFO: %1").arg(message);
        break;
    case AddError:
        newMessage = QStringLiteral("ERROR: %1").arg(message);
        break;
    case AddTitle:
        newMessage = message;
        break;
    }
    //Laurent: Don't use qCDebug here.
    qDebug() << newMessage;
}
