/*
   Copyright (C) 2015-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef LOGINFO_H
#define LOGINFO_H

#include <QObject>

class LogInfo : public QObject
{
    Q_OBJECT
public:
    explicit LogInfo(QObject *parent = Q_NULLPTR);
    ~LogInfo();

    void addInfoLogEntry(const QString &log);
    void addErrorLogEntry(const QString &log);
    void addTitleLogEntry(const QString &log);
    void addEndLineLogEntry();
private:
    enum LogType {
        AddEndLine = 0,
        AddInfo,
        AddError,
        AddTitle
    };
    void addLogLine(const QString &message, LogType type);
};

#endif // LOGINFO_H
