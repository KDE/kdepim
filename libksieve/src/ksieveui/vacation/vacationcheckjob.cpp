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

#include "vacationcheckjob.h"
#include "vacationutils.h"
#include "managescriptsjob/parseuserscriptjob.h"
#include "util/util.h"

#include <kmanagesieve/sievejob.h>

#include <QDate>
#include <klocalizedstring.h>

#include "libksieve_debug.h"

using namespace KSieveUi;
VacationCheckJob::VacationCheckJob(const QUrl &url, const QString &serverName, QObject *parent)
    : QObject(parent),
      mServerName(serverName),
      mUrl(url)
    , mSieveJob(Q_NULLPTR)
    , mParseJob(Q_NULLPTR)
    , mScriptPos(-1)
    , mKep14Support(false)
    , mNoScriptFound(false)
{
}

VacationCheckJob::~VacationCheckJob()
{
    kill();
}

void VacationCheckJob::kill()
{
    if (mSieveJob) {
        mSieveJob->kill();
    }
    mSieveJob = Q_NULLPTR;

    if (mParseJob) {
        mParseJob->kill();
    }
    mParseJob = Q_NULLPTR;
}

void VacationCheckJob::setKep14Support(bool kep14Support)
{
    mKep14Support = kep14Support;
}

void VacationCheckJob::start()
{
    if (mKep14Support) {
        QUrl url = mUrl;
        url = url.adjusted(QUrl::RemoveFilename);
        url.setPath(url.path() + QLatin1String("USER"));
        mParseJob = new ParseUserScriptJob(url);
        connect(mParseJob, &ParseUserScriptJob::finished, this, &VacationCheckJob::slotGotActiveScripts);
        mParseJob->start();
        mSieveJob = KManageSieve::SieveJob::list(url);
        connect(mSieveJob, &KManageSieve::SieveJob::gotList, this, &VacationCheckJob::slotGotList);
    } else {
        mSieveJob = KManageSieve::SieveJob::get(mUrl);
        mSieveJob->setInteractive(false);
        connect(mSieveJob, &KManageSieve::SieveJob::gotScript, this, &VacationCheckJob::slotGetResult);
    }
}

void VacationCheckJob::slotGetResult(KManageSieve::SieveJob *job, bool success, const QString &script, bool active)
{
    Q_ASSERT(job == mSieveJob);
    mScript = script;
    mSieveCapabilities = mSieveJob->sieveCapabilities();
    mSieveJob = Q_NULLPTR;

    if (mKep14Support) {
        VacationUtils::Vacation vacation = VacationUtils::parseScript(script);
        if (vacation.isValid()) {
            const QString &scriptName = mAvailableScripts[mScriptPos - 1];
            bool active = mActiveScripts.contains(scriptName) && vacation.active;
            if (active && vacation.startDate.isValid() && vacation.endDate.isValid()) {
                active = (vacation.startDate <= QDate::currentDate() && vacation.endDate >= QDate::currentDate());
            }
            Q_EMIT scriptActive(this, scriptName, active);
            qCDebug(LIBKSIEVE_LOG) << "vacation script found :)";
        } else if (isLastScript()) {
            mNoScriptFound = true;
            Q_EMIT scriptActive(this, QString(), false);
            qCDebug(LIBKSIEVE_LOG) << "no vacation script found :(";
        } else {
            getNextScript();
        }
    } else {
        if (!success) {
            active = false; // default to inactive
            mNoScriptFound = true;
        }
        if (active) {
            mActiveScripts << mUrl.fileName();
        }
        Q_EMIT scriptActive(this, mUrl.fileName(), active);
    }
}

void VacationCheckJob::slotGotActiveScripts(ParseUserScriptJob *job)
{
    Q_ASSERT(job == mParseJob);
    mParseJob = Q_NULLPTR;
    if (!job->error().isEmpty()) {
        emitError(i18n("ParseUserScriptJob failed: %1", job->error()));
        return;
    }
    mActiveScripts = job->activeScriptList();

    if (!mSieveJob) {
        searchVacationScript();
    }
}

void VacationCheckJob::slotGotList(KManageSieve::SieveJob *job, bool success, const QStringList &availableScripts, const QString &activeScript)
{
    Q_UNUSED(activeScript)
    Q_ASSERT(job == mSieveJob);
    mSieveJob = Q_NULLPTR;
    if (!success) {
        emitError(i18n("SieveJob list failed."));
        return;
    }

    mAvailableScripts = availableScripts;

    if (!mParseJob) {
        searchVacationScript();
    }
}

void VacationCheckJob::emitError(const QString &errorMessage)
{
    qCWarning(LIBKSIEVE_LOG) << errorMessage;
    //TODO: Q_EMIT error
}

void VacationCheckJob::searchVacationScript()
{
    QStringList scriptList = mActiveScripts;

    // Reorder script list
    foreach (const QString &script, mAvailableScripts) {
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
        Q_EMIT scriptActive(this, QString(), false);
        qCDebug(LIBKSIEVE_LOG) << "no vacation script found :(";
    }
    QUrl url = mUrl;
    url = url.adjusted(QUrl::RemoveFilename);
    url.setPath(url.path() + mAvailableScripts[mScriptPos]);
    mScriptPos += 1;
    if (Util::isKep14ProtectedName(url.fileName())) {
        getNextScript();
    }
    mSieveJob = KManageSieve::SieveJob::get(url);
    mSieveJob->setInteractive(false);
    connect(mSieveJob, &KManageSieve::SieveJob::gotScript, this, &VacationCheckJob::slotGetResult);
}

bool VacationCheckJob::isLastScript() const
{
    return mScriptPos >= mAvailableScripts.count();
}

bool VacationCheckJob::noScriptFound() const
{
    return mNoScriptFound;
}

QString VacationCheckJob::serverName() const
{
    return mServerName;
}

QString VacationCheckJob::script() const
{
    return mScript;
}

QStringList VacationCheckJob::sieveCapabilities() const
{
    return mSieveCapabilities;
}

