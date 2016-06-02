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

#ifndef LOGINFILE_H
#define LOGINFILE_H

#include <QObject>
#include <QTextStream>
class QFile;
class LogInFile : public QObject
{
    Q_OBJECT
public:
    explicit LogInFile(QObject *parent = Q_NULLPTR);
    ~LogInFile();

    QString fileName() const;
    void setFileName(const QString &fileName);

public Q_SLOTS:
    void addEndLine();

    void addError(const QString &message);
    void addInfo(const QString &message);
    void addTitle(const QString &message);
private:
    enum LogType {
        AddEndLine = 0,
        AddInfo,
        AddError,
        AddTitle
    };
    void addLogLine(const QString &message, LogType type);
    QString mFileName;
    QTextStream mTextStream;
    QFile *mFile;
};

#endif // LOGINFILE_H
