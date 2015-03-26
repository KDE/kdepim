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

#include "vacationcheckjob.h"
#include "vacationutils.h"
#include <managescriptsjob/parseuserscriptjob.h>
#include <util/util.h>

#include <kmanagesieve/sievejob.h>

#include <KDebug>

using namespace KSieveUi;
VacationCheckJob::VacationCheckJob(const KUrl &url, const QString &serverName, QObject *parent)
    : QObject(parent),
      mServerName(serverName),
      mUrl(url)
    , mKep14Support(false)
    , mSieveJob(0)
    , mParseJob(0)
    , mNoScriptFound(0)
{
}

VacationCheckJob::~VacationCheckJob()
{
    kill();
}

void VacationCheckJob::kill()
{
    if ( mSieveJob )
        mSieveJob->kill();
    mSieveJob = 0;

    if (mParseJob) {
        mParseJob->kill();
    }
    mParseJob = 0;
}


void VacationCheckJob::setKep14Support(bool kep14Support)
{
  mKep14Support = kep14Support;
}

void VacationCheckJob::start()
{
    if (mKep14Support) {
        KUrl url = mUrl;
        url.setFileName(QLatin1String("USER"));
        mParseJob = new ParseUserScriptJob(url);
        connect(mParseJob, SIGNAL(finished(ParseUserScriptJob*)), SLOT(slotGotActiveScripts(ParseUserScriptJob*)));
        mParseJob->start();
        mSieveJob = KManageSieve::SieveJob::list(url);
        connect(mSieveJob, SIGNAL(gotList(KManageSieve::SieveJob*,bool,QStringList,QString)),
         this, SLOT(slotGotList(KManageSieve::SieveJob*,bool,QStringList,QString)));
    } else {
        mSieveJob = KManageSieve::SieveJob::get(mUrl);
        mSieveJob->setInteractive(false);
        connect(mSieveJob, SIGNAL(gotScript(KManageSieve::SieveJob*,bool,QString,bool)),
                 SLOT(slotGetResult(KManageSieve::SieveJob*,bool,QString,bool)));
    }
}

void VacationCheckJob::slotGetResult(KManageSieve::SieveJob */*job*/, bool success, const QString &script, bool active)
{
    mScript = script;
    mSieveCapabilities = mSieveJob->sieveCapabilities();
    mSieveJob = 0;

    if (mKep14Support) {
        VacationUtils::Vacation vacation = VacationUtils::parseScript(script);
        if (vacation.isValid()) {
            const QString &scriptName = mAvailableScripts[mScriptPos-1];
            emit scriptActive(this, scriptName, mActiveScripts.contains(scriptName) && vacation.active);
            kDebug() << "vacation script found :)";
        } else if (isLastScript()) {
            mNoScriptFound = true;
            emit scriptActive(this, QString(), false);
            kDebug() << "no vacation script found :(";
        } else {
            getNextScript();
        }
    } else {
        if ( !success ) {
            active = false; // default to inactive
            mNoScriptFound = true;
        }
        if (active) {
            mActiveScripts << mUrl.fileName();
        }
        emit scriptActive(this, mUrl.fileName(), active);
    }
}

void VacationCheckJob::slotGotActiveScripts(ParseUserScriptJob *job)
{
    mParseJob = 0;
    if (!job->error().isEmpty()) {
        emitError(QLatin1String("ParseUserScriptJob failed:")+job->error());
        return;
    }
    mActiveScripts = job->activeScriptList();

    if (!mSieveJob) {
        searchVacationScript();
    }
}

void VacationCheckJob::slotGotList(KManageSieve::SieveJob *job, bool success, const QStringList &availableScripts, const QString &activeScript)
{
    mSieveJob = 0;
    if (!success) {
        emitError(QLatin1String("SieveJob list failed."));
        return;
    }

    mAvailableScripts = availableScripts;

    if (!mParseJob) {
        searchVacationScript();
    }
}

void VacationCheckJob::emitError(const QString &errorMessage)
{
    qWarning() << errorMessage;
    //TODO: emit error
}

void VacationCheckJob::searchVacationScript()
{
    QStringList scriptList = mActiveScripts;

    // Reorder script list
    foreach(const QString &script, mAvailableScripts) {
        if (!scriptList.contains(script)) {
            scriptList.append(script);
        }
    }

    mAvailableScripts = scriptList;
    mScriptPos = 0;
    getNextScript();
}

void VacationCheckJob::getNextScript()
{
    if (isLastScript()) {
        //TODO: no script found
        mNoScriptFound = true;
        emit scriptActive(this, QString(), false);
        kDebug() << "no vacation script found :(";
    }
    KUrl url = mUrl;
    url.setFileName(mAvailableScripts[mScriptPos]);
    mScriptPos += 1;
    if (Util::isKep14ProtectedName(url.fileName())) {
        getNextScript();
    }
    mSieveJob = KManageSieve::SieveJob::get(url);
    mSieveJob->setInteractive(false);
    connect(mSieveJob, SIGNAL(gotScript(KManageSieve::SieveJob*,bool,QString,bool)),
             SLOT(slotGetResult(KManageSieve::SieveJob*,bool,QString,bool)));
}

bool VacationCheckJob::isLastScript() const
{
    return mScriptPos >= mAvailableScripts.count();
}

bool VacationCheckJob::noScriptFound()
{
    return mNoScriptFound;
}

QString VacationCheckJob::serverName()
{
    return mServerName;
}

QString VacationCheckJob::script()
{
    return mScript;
}

QStringList VacationCheckJob::sieveCapabilities()
{
    return mSieveCapabilities;
}

