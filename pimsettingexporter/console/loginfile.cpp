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
#include "pimsettingexportconsole_debug.h"

#include <QFile>

LogInFile::LogInFile(QObject *parent)
    : QObject(parent),
      mFile(Q_NULLPTR)
{
    qDebug()<<" LogInFile::LogInFile(QObject *parent)"<<this;
}

LogInFile::~LogInFile()
{
    qDebug()<<" LogInFile::~LogInFile"<<this;
    if (mFile) {
        mFile->close();
        delete mFile;
    }
}

QString LogInFile::fileName() const
{
    return mFileName;
}

void LogInFile::setFileName(const QString &fileName)
{
    if (!fileName.isEmpty()) {
        mFileName = fileName;
        if (!mFile) {
            mFile = new QFile(mFileName);
            if (!mFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
                return;
            }
        }
        mTextStream.setDevice(mFile);
    }
}

void LogInFile::addEndLine()
{
    addLogLine(QString(), AddEndLine);
}

void LogInFile::addError(const QString &message)
{
    addLogLine(message, AddError);
}

void LogInFile::addInfo(const QString &message)
{
    addLogLine(message, AddInfo);
}

void LogInFile::addTitle(const QString &message)
{
    addLogLine(message, AddTitle);
}

void LogInFile::addLogLine(const QString &message, LogType type)
{
    QString newMessage;
    switch (type) {
    case AddEndLine:
        newMessage = QLatin1Char('\n');
        break;
    case AddInfo:
        newMessage = QStringLiteral("INFO: %1\n").arg(message);
        break;
    case AddError:
        newMessage = QStringLiteral("ERROR: %1\n").arg(message);
        break;
    case AddTitle:
        newMessage = message + QLatin1Char('\n');
        break;
    }
    mTextStream << newMessage;
}
