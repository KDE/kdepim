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

#include "vacationcreatescriptjob.h"
#include "vacationutils.h"
#include <managescriptsjob/parseuserscriptjob.h>
#include <managescriptsjob/generateglobalscriptjob.h>
#include <kmanagesieve/sievejob.h>

#include <KMessageBox>
#include <KLocalizedString>
#include <KDebug>

using namespace KSieveUi;

VacationCreateScriptJob::VacationCreateScriptJob(QObject *parent)
    : QObject(parent),
      mActivate(false),
      mScriptActive(false)
    , mKep14Support(false)
    , mUserJobRunning(false)
    , mScriptJobRunning(false)
    , mSuccess(true)
    , mSieveJob(0)
    , mParseUserJob(0)
    , mCreateJob(0)
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
    mSieveJob = 0;

    if (mParseUserJob) {
        mParseUserJob->kill();
    }
    mParseUserJob = 0;

    if (mCreateJob) {
        mCreateJob->kill();
    }
    mParseUserJob = 0;
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

void VacationCreateScriptJob::setServerUrl(const KUrl &url)
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
        qDebug()<<" server url is empty";
        deleteLater();
        return;
    }

    mUserJobRunning = false;
    mScriptJobRunning = true;
    mSieveJob = KManageSieve::SieveJob::get(mUrl);
    mSieveJob->setInteractive(false);
    connect(mSieveJob, SIGNAL(gotScript(KManageSieve::SieveJob*,bool,QString,bool)),
            SLOT(slotGetScript(KManageSieve::SieveJob*,bool,QString,bool)));

    if (mKep14Support && mActivate && !mScriptActive) {
        mUserJobRunning = true;
        KUrl url = mUrl;
        url.setFileName(QLatin1String("USER"));
        mParseUserJob = new ParseUserScriptJob(url, this);
        connect(mParseUserJob, SIGNAL(finished(ParseUserScriptJob*)), SLOT(slotGotActiveScripts(ParseUserScriptJob*)));
        mParseUserJob->start();
    }
}

void VacationCreateScriptJob::slotGetScript(KManageSieve::SieveJob *job, bool success, const QString &oldScript, bool active)
{
    mSieveJob = 0;
    QString script = mScript;
    if (success || !oldScript.trimmed().isEmpty()) {
        script = VacationUtils::mergeRequireLine(oldScript, mScript);
        script = VacationUtils::updateVacationBlock(oldScript,mScript);
    }
    if (mKep14Support) {
        mSieveJob = KManageSieve::SieveJob::put( mUrl, mScript, false, false );
    } else {
        mSieveJob = KManageSieve::SieveJob::put( mUrl, mScript, mActivate, false );         //Never deactivate
    }
    connect( mSieveJob, SIGNAL(gotScript(KManageSieve::SieveJob*,bool,QString,bool)),
             SLOT(slotPutResult(KManageSieve::SieveJob*,bool)) );
}

void VacationCreateScriptJob::slotPutResult( KManageSieve::SieveJob * job, bool success )
{
    mSieveJob = 0;
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

    if ( mSuccess )
        KMessageBox::information( 0, mActivate
                                  ? i18n("Sieve script installed successfully on the server \'%1\'.\n"
                                         "Out of Office reply is now active.", mServerName)
                                  : i18n("Sieve script installed successfully on the server \'%1\'.\n"
                                         "Out of Office reply has been deactivated.", mServerName) );

    kDebug() << "( ???," << mSuccess << ", ? )";
    Q_EMIT result( mSuccess );
    Q_EMIT scriptActive( mActivate, mServerName );
    deleteLater();
}

void VacationCreateScriptJob::slotGotActiveScripts(ParseUserScriptJob *job)
{
    mParseUserJob = 0;
    if (!job->error().isEmpty()) {
        slotGenerateDone(job->error());
        return;
    }

    QStringList list = job->activeScriptList();

    if (!list.contains(mUrl.fileName())) {
        list.prepend(mUrl.fileName());
        mCreateJob = new GenerateGlobalScriptJob(mUrl, this);
        mCreateJob->addUserActiveScripts(list);
        connect( mCreateJob, SIGNAL(success()), SLOT(slotGenerateDone()));
        connect( mCreateJob, SIGNAL(error(QString)), SLOT(slotGenerateDone(QString)));
        mCreateJob->start();
    }
}

void VacationCreateScriptJob::slotGenerateDone(const QString &error)
{
    mCreateJob = 0;
    mUserJobRunning = false;
    if (!error.isEmpty()) {
        qWarning() << error;
        mSuccess = false;
    }
    handleResult();
}
