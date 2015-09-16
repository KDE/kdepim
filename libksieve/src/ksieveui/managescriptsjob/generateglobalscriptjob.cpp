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

#include "generateglobalscriptjob.h"

#include "kmanagesieve/sievejob.h"

#include <KLocalizedString>

using namespace KSieveUi;
GenerateGlobalScriptJob::GenerateGlobalScriptJob(const QUrl &url, QObject *parent)
    : QObject(parent),
      mCurrentUrl(url),
      mMasterjob(Q_NULLPTR),
      mUserJob(Q_NULLPTR)
{
}

GenerateGlobalScriptJob::~GenerateGlobalScriptJob()
{
    if (mMasterjob) {
        mMasterjob->kill();
    }
    if (mUserJob) {
        mUserJob->kill();
    }
}

void GenerateGlobalScriptJob::addUserActiveScripts(const QStringList &lstScript)
{
    mListUserActiveScripts = lstScript;
}

void GenerateGlobalScriptJob::start()
{
    if (mCurrentUrl.isEmpty()) {
        Q_EMIT error(i18n("Path is not specified."));
        return;
    }
    writeMasterScript();
}

void GenerateGlobalScriptJob::writeMasterScript()
{
    const QString masterScript = QStringLiteral("# MASTER\n"
                                 "#\n"
                                 "# This file is authoritative for your system and MUST BE KEPT ACTIVE.\n"
                                 "#\n"
                                 "# Altering it is likely to render your account dysfunctional and may\n"
                                 "# be violating your organizational or corporate policies.\n"
                                 "# \n"
                                 "# For more information on the mechanism and the conventions behind\n"
                                 "# this script, see http://wiki.kolab.org/KEP:14\n"
                                 "#\n"
                                 "\n"
                                 "require [\"include\"];\n"
                                 "\n"
                                 "# OPTIONAL: Includes for all or a group of users\n"
                                 "# include :global \"all-users\";\n"
                                 "# include :global \"this-group-of-users\";\n"
                                 "\n"
                                 "# The script maintained by the general management system\n"
                                 "include :personal :optional \"MANAGEMENT\";\n"
                                 "\n"
                                 "# The script(s) maintained by one or more editors available to the user\n"
                                 "include :personal :optional \"USER\";\n");

    QUrl url(mCurrentUrl);
    url = url.adjusted(QUrl::RemoveFilename);
    url.setPath(url.path() + QLatin1String("MASTER"));
    mMasterjob = KManageSieve::SieveJob::put(url, masterScript, true, true);
    connect(mMasterjob, &KManageSieve::SieveJob::result, this, &GenerateGlobalScriptJob::slotPutMasterResult);
}

void GenerateGlobalScriptJob::slotPutMasterResult(KManageSieve::SieveJob *, bool success)
{
    if (!success) {
        Q_EMIT error(i18n("Error when we wrote \"MASTER\" script on server."));
        return;
    }
    mMasterjob = Q_NULLPTR;
    writeUserScript();
}

void GenerateGlobalScriptJob::writeUserScript()
{
    QString userScript = QStringLiteral("# USER Management Script\n"
                                        "#\n"
                                        "# This script includes the various active sieve scripts\n"
                                        "# it is AUTOMATICALLY GENERATED. DO NOT EDIT MANUALLY!\n"
                                        "# \n"
                                        "# For more information, see http://wiki.kolab.org/KEP:14#USER\n"
                                        "#\n"
                                        "\n"
                                        "require [\"include\"];\n");

    Q_FOREACH (const QString &activeScript, mListUserActiveScripts) {
        userScript += QStringLiteral("\ninclude :personal \"%1\"").arg(activeScript);
    }

    QUrl url(mCurrentUrl);
    url = url.adjusted(QUrl::RemoveFilename);
    url.setPath(url.path() + QLatin1String("USER"));
    mUserJob = KManageSieve::SieveJob::put(url, userScript, false, false);
    connect(mUserJob, &KManageSieve::SieveJob::result, this, &GenerateGlobalScriptJob::slotPutUserResult);
}

void GenerateGlobalScriptJob::slotPutUserResult(KManageSieve::SieveJob *, bool success)
{
    mUserJob = Q_NULLPTR;
    if (!success) {
        Q_EMIT error(i18n("Error when we wrote \"User\" script on server."));
        return;
    }
    disableAllOtherScripts();
}

void GenerateGlobalScriptJob::disableAllOtherScripts()
{
    //TODO
    Q_EMIT success();
}

