/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "libksieve/kmanagesieve/sievejob.h"

#include <KLocalizedString>

using namespace KSieveUi;
GenerateGlobalScriptJob::GenerateGlobalScriptJob(const KUrl &url, QObject *parent)
    : QObject(parent),
      mCurrentUrl(url),
      mMasterjob(0),
      mUserJob(0)
{
}

GenerateGlobalScriptJob::~GenerateGlobalScriptJob()
{
    if (mMasterjob)
        mMasterjob->kill();
    if (mUserJob)
        mUserJob->kill();
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
    const QString masterScript = QLatin1String("# MASTER\n"
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

    KUrl url(mCurrentUrl);
    url.setFileName(QLatin1String("MASTER"));
    mMasterjob = KManageSieve::SieveJob::put(url, masterScript, true, true );
    connect( mMasterjob, SIGNAL(result(KManageSieve::SieveJob*,bool,QString,bool)),
             this, SLOT(slotPutMasterResult(KManageSieve::SieveJob*,bool)) );
}

void GenerateGlobalScriptJob::slotPutMasterResult( KManageSieve::SieveJob *, bool success )
{
    if (!success) {
        Q_EMIT error(i18n("Error when we wrote \"MASTER\" script on server."));
        return;
    }
    mMasterjob = 0;
    writeUserScript();
}

void GenerateGlobalScriptJob::writeUserScript()
{
    QString userScript = QLatin1String("# USER Management Script\n"
                                       "#\n"
                                       "# This script includes the various active sieve scripts\n"
                                       "# it is AUTOMATICALLY GENERATED. DO NOT EDIT MANUALLY!\n"
                                       "# \n"
                                       "# For more information, see http://wiki.kolab.org/KEP:14#USER\n"
                                       "#\n"
                                       "\n"
                                       "require [\"include\"];\n");

    Q_FOREACH (const QString &activeScript, mListUserActiveScripts) {
        userScript += QString::fromLatin1("\ninclude :personal \"%1\"").arg(activeScript);
    }

    KUrl url(mCurrentUrl);
    url.setFileName(QLatin1String("USER"));
    mUserJob = KManageSieve::SieveJob::put(url, userScript, false, false );
    connect( mUserJob, SIGNAL(result(KManageSieve::SieveJob*,bool,QString,bool)),
             this, SLOT(slotPutUserResult(KManageSieve::SieveJob*,bool)) );
}

void GenerateGlobalScriptJob::slotPutUserResult( KManageSieve::SieveJob *, bool success )
{
    mUserJob = 0;
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


#include "moc_generateglobalscriptjob.cpp"
