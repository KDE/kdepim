/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef VACATIONCREATESCRIPTJOB_H
#define VACATIONCREATESCRIPTJOB_H

#include <QObject>

#include <QUrl>

namespace KManageSieve
{
class SieveJob;
}

namespace KSieveUi
{
class ParseUserScriptJob;
class GenerateGlobalScriptJob;
class VacationCreateScriptJob : public QObject
{
    Q_OBJECT
public:
    explicit VacationCreateScriptJob(QObject *parent = Q_NULLPTR);
    ~VacationCreateScriptJob();

    void start();
    void kill();

    void setServerUrl(const QUrl &url);
    void setScript(const QString &script);
    void setServerName(const QString &servername);
    const QString &serverName() const;
    void setStatus(bool activate, bool wasActive);
    void setKep14Support(bool kep14Support);

Q_SIGNALS:
    void result(bool);
    void scriptActive(bool activated, const QString &serverName);

private Q_SLOTS:
    void slotPutResult(KManageSieve::SieveJob *job, bool success);
    void slotGetScript(KManageSieve::SieveJob *job, bool success, const QString &oldScript, bool active);
    void slotGotActiveScripts(ParseUserScriptJob *job);
    void slotGenerateDone(const QString &error = QString());

private:
    void handleResult();
    QUrl mUrl;
    QString mScript;
    QString mServerName;
    bool mActivate;
    bool mScriptActive;
    bool mKep14Support;
    bool mUserJobRunning;
    bool mScriptJobRunning;
    bool mSuccess;
    KManageSieve::SieveJob *mSieveJob;
    ParseUserScriptJob *mParseUserJob;
    GenerateGlobalScriptJob *mCreateJob;
};
}

#endif // VACATIONCREATESCRIPTJOB_H
