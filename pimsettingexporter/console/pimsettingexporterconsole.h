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
class LogInfo;
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

    QString importExportFileName() const;
    void setImportExportFileName(const QString &importFileName);

Q_SIGNALS:
    void finished();

private Q_SLOTS:
    void slotAddEndLine();
    void slotAddError(const QString &message);
    void slotAddInfo(const QString &message);
    void slotAddTitle(const QString &message);
    void slotJobFailed();
    void slotBackupDone();
    void slotJobFinished();
    void slotRestoreDone();
private:
    void initializeLogInFile();
    void closeLogFile();
    QString mTemplateFileName;
    QString mImportExportFileName;
    PimSettingsBackupRestore *mPimSettingsBackupRestore;
    LogInFile *mLogInFile;
    LogInfo *mLogInfo;
    Mode mMode;
    bool mInProgress;
};

#endif // PIMSETTINGEXPORTERCONSOLE_H
