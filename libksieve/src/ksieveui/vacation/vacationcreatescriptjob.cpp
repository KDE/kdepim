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

#include "vacationcreatescriptjob.h"
#include <kmanagesieve/sievejob.h>

#include <KMessageBox>
#include <KLocalizedString>
#include "libksieve_debug.h"

using namespace KSieveUi;

VacationCreateScriptJob::VacationCreateScriptJob(QObject *parent)
    : QObject(parent),
      mActivate(false),
      mWasActive(false),
      mSieveJob(Q_NULLPTR)
{

}

VacationCreateScriptJob::~VacationCreateScriptJob()
{

}

void VacationCreateScriptJob::setStatus(bool activate, bool wasActive)
{
    mActivate = activate;
    mWasActive = wasActive;
}

void VacationCreateScriptJob::setServerName(const QString &servername)
{
    mServerName = servername;
}

void VacationCreateScriptJob::start()
{
    if (mUrl.isEmpty()) {
        qCDebug(LIBKSIEVE_LOG) << " server url is empty";
        deleteLater();
        return;
    }
    mSieveJob = KManageSieve::SieveJob::put(mUrl, mScript, mActivate, mWasActive);
    if (mActivate) {
        connect(mSieveJob, &KManageSieve::SieveJob::gotScript, this, &VacationCreateScriptJob::slotPutActiveResult);
    } else {
        connect(mSieveJob, &KManageSieve::SieveJob::gotScript, this, &VacationCreateScriptJob::slotPutInactiveResult);
    }
}

void VacationCreateScriptJob::setServerUrl(const QUrl &url)
{
    mUrl = url;
}

void VacationCreateScriptJob::setScript(const QString &script)
{
    mScript = script;
}

void VacationCreateScriptJob::slotPutActiveResult(KManageSieve::SieveJob *job, bool success)
{
    handlePutResult(job, success, true);
}

void VacationCreateScriptJob::slotPutInactiveResult(KManageSieve::SieveJob *job, bool success)
{
    handlePutResult(job, success, false);
}

void VacationCreateScriptJob::handlePutResult(KManageSieve::SieveJob *, bool success, bool activated)
{
    if (success)
        KMessageBox::information(Q_NULLPTR, activated
                                 ? i18n("Sieve script installed successfully on the server \'%1\'.\n"
                                        "Out of Office reply is now active.", mServerName)
                                 : i18n("Sieve script installed successfully on the server \'%1\'.\n"
                                        "Out of Office reply has been deactivated.", mServerName));

    qCDebug(LIBKSIEVE_LOG) << "( ???," << success << ", ? )";
    mSieveJob = Q_NULLPTR; // job deletes itself after returning from this slot!
    Q_EMIT result(success);
    Q_EMIT scriptActive(activated, mServerName);
    deleteLater();
}
