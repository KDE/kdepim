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
#include "vacationutils.h"
#include <managescriptsjob/parseuserscriptjob.h>
#include <managescriptsjob/generateglobalscriptjob.h>
#include <kmanagesieve/sievejob.h>

#include <KMessageBox>
#include <KLocalizedString>
#include "libksieve_debug.h"

using namespace KSieveUi;

VacationCreateScriptJob::VacationCreateScriptJob(QObject *parent)
    : QObject(parent)
    , mActivate(false)
    , mScriptActive(false)
    , mKep14Support(false)
    , mUserJobRunning(false)
    , mScriptJobRunning(false)
    , mSuccess(true)
    , mSieveJob(Q_NULLPTR)
    , mParseUserJob(Q_NULLPTR)
    , mCreateJob(Q_NULLPTR)
{

}

VacationCreateScriptJob::~VacationCreateScriptJob()
{
    kill();
}

void VacationCreateScriptJob::kill()
{
    if (mSieveJob) {
        mSieveJob->kill();
    }
    mSieveJob = Q_NULLPTR;

    if (mParseUserJob) {
        mParseUserJob->kill();
    }
    mParseUserJob = Q_NULLPTR;

    if (mCreateJob) {
        mCreateJob->kill();
    }
    mParseUserJob = Q_NULLPTR;
}

void VacationCreateScriptJob::setStatus(bool activate, bool wasActive)
{
    mActivate = activate;
    mScriptActive = wasActive;
}

void VacationCreateScriptJob::setServerName(const QString &servername)
{
    mServerName = servername;
}

const QString &VacationCreateScriptJob::serverName() const
{
    return mServerName;
}

void VacationCreateScriptJob::setKep14Support(bool kep14Support)
{
    mKep14Support = kep14Support;
}

void VacationCreateScriptJob::setServerUrl(const QUrl &url)
{
    mUrl = url;
}

void VacationCreateScriptJob::setScript(const QString &script)
{
    mScript = script;
}

void VacationCreateScriptJob::start()
{
    if (mUrl.isEmpty()) {
        qCDebug(LIBKSIEVE_LOG) << " server url is empty";
        deleteLater();
        return;
    }

    mUserJobRunning = false;
    mScriptJobRunning = true;
    mSieveJob = KManageSieve::SieveJob::get(mUrl);
    mSieveJob->setInteractive(false);
    connect(mSieveJob, &KManageSieve::SieveJob::gotScript, this, &VacationCreateScriptJob::slotGetScript);

    if (mKep14Support && mActivate && !mScriptActive) {
        mUserJobRunning = true;
        QUrl url = mUrl;
        url = url.adjusted(QUrl::RemoveFilename);
        url.setPath(url.path() + QLatin1String("USER"));
        mParseUserJob = new ParseUserScriptJob(url, this);
        connect(mParseUserJob, &ParseUserScriptJob::finished, this, &VacationCreateScriptJob::slotGotActiveScripts);
        mParseUserJob->start();
    }
}

void VacationCreateScriptJob::slotGetScript(KManageSieve::SieveJob *job, bool success, const QString &oldScript, bool active)
{
    Q_UNUSED(active)
    Q_ASSERT(job == mSieveJob);
    mSieveJob = Q_NULLPTR;
    QString script = mScript;
    if (success || !oldScript.trimmed().isEmpty()) {
        script = VacationUtils::mergeRequireLine(oldScript, mScript);
        script = VacationUtils::updateVacationBlock(oldScript, mScript);
    }
    if (mKep14Support) {
        mSieveJob = KManageSieve::SieveJob::put(mUrl, mScript, false, false);
    } else {
        mSieveJob = KManageSieve::SieveJob::put(mUrl, mScript, mActivate, false);           //Never deactivate
    }
    connect(mSieveJob, &KManageSieve::SieveJob::gotScript, this, &VacationCreateScriptJob::slotPutResult);
}

void VacationCreateScriptJob::slotPutResult(KManageSieve::SieveJob *job, bool success)
{
    Q_ASSERT(job == mSieveJob);
    mSieveJob = Q_NULLPTR;
    mScriptJobRunning = false;
    if (!success) {
        mSuccess = false;
    }
    handleResult();
}

void VacationCreateScriptJob::handleResult()
{
    if (mUserJobRunning || mScriptJobRunning) {                 // Not both jobs are done
        return;
    }

    if (mSuccess)
        KMessageBox::information(Q_NULLPTR,  mActivate
                                 ? i18n("Sieve script installed successfully on the server \'%1\'.\n"
                                        "Out of Office reply is now active.", mServerName)
                                 : i18n("Sieve script installed successfully on the server \'%1\'.\n"
                                        "Out of Office reply has been deactivated.", mServerName));

    qCDebug(LIBKSIEVE_LOG) << "( ???," << mSuccess << ", ? )";
    mSieveJob = Q_NULLPTR; // job deletes itself after returning from this slot!
    Q_EMIT result(mSuccess);
    Q_EMIT scriptActive(mActivate, mServerName);
    deleteLater();
}

void VacationCreateScriptJob::slotGotActiveScripts(ParseUserScriptJob *job)
{
    Q_ASSERT(job == mParseUserJob);
    mParseUserJob = Q_NULLPTR;
    if (!job->error().isEmpty()) {
        slotGenerateDone(job->error());
        return;
    }

    QStringList list = job->activeScriptList();

    if (!list.contains(mUrl.fileName())) {
        list.prepend(mUrl.fileName());
        mCreateJob = new GenerateGlobalScriptJob(mUrl, this);
        mCreateJob->addUserActiveScripts(list);
        connect(mCreateJob, &GenerateGlobalScriptJob::success, [ = ]() {
            this->slotGenerateDone();
        });
        connect(mCreateJob, &GenerateGlobalScriptJob::error, this, &VacationCreateScriptJob::slotGenerateDone);
        mCreateJob->start();
    }
}

void VacationCreateScriptJob::slotGenerateDone(const QString &error)
{
    mCreateJob = Q_NULLPTR;
    mUserJobRunning = false;
    if (!error.isEmpty()) {
        qCWarning(LIBKSIEVE_LOG) << error;
        mSuccess = false;
    }
    handleResult();
}
