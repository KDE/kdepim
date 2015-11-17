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

#include "multiimapvacationmanager.h"
#include "vacationcheckjob.h"
#include "util/util.h"
#include <managescriptsjob/checkkolabkep14supportjob.h>
#include <managescriptsjob/parseuserscriptjob.h>
#include <kmanagesieve/sievejob.h>

#include <AgentInstance>

#include "libksieve_debug.h"

using namespace KSieveUi;
MultiImapVacationManager::MultiImapVacationManager(QObject *parent)
    : QObject(parent),
      mNumberOfJobs(0),
      mCheckInProgress(false)
{
}

MultiImapVacationManager::~MultiImapVacationManager()
{

}

QMap <QString, QUrl> MultiImapVacationManager::serverList() const
{
    QMap <QString, QUrl> list;
    const Akonadi::AgentInstance::List instances = KSieveUi::Util::imapAgentInstances();
    foreach (const Akonadi::AgentInstance &instance, instances) {
        if (instance.status() == Akonadi::AgentInstance::Broken) {
            continue;
        }

        const QUrl url = KSieveUi::Util::findSieveUrlForAccount(instance.identifier());
        if (!url.isEmpty()) {
            list.insert(instance.name(), url);
        }
    }
    return list;
}

void MultiImapVacationManager::checkVacation(const QString &serverName, const QUrl &url)
{
    ++mNumberOfJobs;
    if (!mKep14Support.contains(serverName)) {
        CheckKolabKep14SupportJob *checkKep14Job = new CheckKolabKep14SupportJob(this);
        checkKep14Job->setProperty("triggerScript", true);
        checkKep14Job->setServerName(serverName);
        checkKep14Job->setServerUrl(url);
        connect(checkKep14Job, &CheckKolabKep14SupportJob::result, this, &MultiImapVacationManager::slotCheckKep14Ended);
        checkKep14Job->start();
    }

    VacationCheckJob *job = new VacationCheckJob(url, serverName, this);
    job->setKep14Support(mKep14Support[serverName]);
    connect(job, &VacationCheckJob::scriptActive, this, &MultiImapVacationManager::slotScriptActive);
    job->start();
}

void MultiImapVacationManager::checkVacation()
{
    if (mCheckInProgress) {
        return;
    }
    mNumberOfJobs = 0;
    mCheckInProgress = true;

    QMap <QString, QUrl> list = serverList();
    foreach (const QString &serverName, list.keys()) {
        const QUrl url = list.value(serverName);
        checkVacation(serverName, url);
    }
}

void MultiImapVacationManager::slotScriptActive(VacationCheckJob *job, const QString &scriptName, bool active)
{
    --mNumberOfJobs;
    if (mNumberOfJobs == 0) {
        mCheckInProgress = false;
    }

    job->deleteLater();

    if (job->noScriptFound()) {
        emit scriptActive(false, job->serverName());
        return;
    }
    emit scriptActive(active, job->serverName());
    emit scriptAvailable(job->serverName(), job->sieveCapabilities(), scriptName, job->script(), active);
}

void MultiImapVacationManager::slotCheckKep14Ended(CheckKolabKep14SupportJob *job, bool success)
{
    job->deleteLater();
    if (!success) {
        --mNumberOfJobs;
        return;
    }

    mKep14Support.insert(job->serverName(), job->hasKep14Support());

    VacationCheckJob *checkJob = new VacationCheckJob(job->serverUrl(), job->serverName(), this);
    checkJob->setKep14Support(job->hasKep14Support());
    connect(checkJob, &VacationCheckJob::scriptActive, this, &MultiImapVacationManager::slotScriptActive);
    checkJob->start();
}

bool MultiImapVacationManager::kep14Support(const QString &serverName) const
{
    if (mKep14Support.contains(serverName)) {
        return mKep14Support[serverName];
    } else {
        qCWarning(LIBKSIEVE_LOG) << "We don't know the KEP:14 support for this server." << serverName;
    }
    return  false;
}
