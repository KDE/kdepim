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

#ifndef GENERATEGLOBALSCRIPTJOB_H
#define GENERATEGLOBALSCRIPTJOB_H

#include <QObject>
#include <QStringList>
#include <QUrl>
namespace KManageSieve
{
class SieveJob;
}
namespace KSieveUi
{
class GenerateGlobalScriptJob : public QObject
{
    Q_OBJECT
public:
    explicit GenerateGlobalScriptJob(const QUrl &url, QObject *parent = Q_NULLPTR);
    ~GenerateGlobalScriptJob();

    void start();

    void addUserActiveScripts(const QStringList &lstScript);

Q_SIGNALS:
    void success();
    void error(const QString &msgError);

private Q_SLOTS:
    void slotPutMasterResult(KManageSieve::SieveJob *, bool success);
    void slotPutUserResult(KManageSieve::SieveJob *, bool success);

private:
    void disableAllOtherScripts();
    void writeMasterScript();
    void writeUserScript();
    QStringList mListUserActiveScripts;
    QUrl mCurrentUrl;
    KManageSieve::SieveJob *mMasterjob;
    KManageSieve::SieveJob *mUserJob;
};
}

#endif // GENERATEGLOBALSCRIPTJOB_H
