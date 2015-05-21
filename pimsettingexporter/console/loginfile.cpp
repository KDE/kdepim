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

#include "loginfile.h"

LogInFile::LogInFile(QObject *parent)
    : QObject(parent)
{

}

LogInFile::~LogInFile()
{

}

void LogInFile::save()
{
    //TODO
}

QString LogInFile::fileName() const
{
    return mFileName;
}

void LogInFile::setFileName(const QString &fileName)
{
    mFileName = fileName;
}

void LogInFile::slotAddEndLine()
{
    addLogLine(QString(), AddEndLine);
}


void LogInFile::slotAddError(const QString &message)
{
    addLogLine(message, AddError);
}

void LogInFile::slotAddInfo(const QString &message)
{
    addLogLine(message, AddInfo);
}

void LogInFile::slotAddTitle(const QString &message)
{
    addLogLine(message, AddTitle);
}

void LogInFile::addLogLine(const QString &message, LogType type)
{
    QString newMessage;
    switch(type) {
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
    mTextStream << newMessage;
}
