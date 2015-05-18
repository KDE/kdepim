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

#ifndef PIMSETTINGEXPORTERCONSOLE_H
#define PIMSETTINGEXPORTERCONSOLE_H

#include <QObject>
class PimSettingsBackupRestore;
class LogInFile;
class PimSettingExporterConsole : public QObject
{
    Q_OBJECT
public:
    enum Mode {
        Import = 0,
        Export = 1
    };

    explicit PimSettingExporterConsole(QObject *parent = Q_NULLPTR);
    ~PimSettingExporterConsole();

    Mode mode() const;
    void setMode(const Mode &mode);

    void setLogFileName(const QString &logFileName);

    void setTemplateFileName(const QString &templateFileName);

    void start();
Q_SIGNALS:
    void finished();

private:
    QString mTemplateFileName;
    PimSettingsBackupRestore *mPimSettingsBackupRestore;
    LogInFile *mLogInFile;
    Mode mMode;
    bool mInProgress;
};

#endif // PIMSETTINGEXPORTERCONSOLE_H
