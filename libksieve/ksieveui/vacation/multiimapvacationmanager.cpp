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

#include "multiimapvacationmanager.h"
#include "vacationcheckjob.h"
#include "util/util.h"
#include <managescriptsjob/checkkep14supportjob.h>
#include <managescriptsjob/parseuserscriptjob.h>
#include <kmanagesieve/sievejob.h>

#include <Akonadi/AgentInstance>

#include <KMessageBox>
#include <KLocalizedString>

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

QMap <QString, KUrl> MultiImapVacationManager::serverList()
{
    QMap <QString, KUrl> list;
    const Akonadi::AgentInstance::List instances = KSieveUi::Util::imapAgentInstances();
    foreach ( const Akonadi::AgentInstance &instance, instances ) {
        if ( instance.status() == Akonadi::AgentInstance::Broken )
            continue;

        const KUrl url = KSieveUi::Util::findSieveUrlForAccount( instance.identifier() );
        if ( !url.isEmpty() ) {
            list.insert(instance.name(), url);
        }
    }
    return list;
}

void MultiImapVacationManager::checkVacation(const QString &serverName, const KUrl &url)
{
    ++mNumberOfJobs;
    if (!mKep14Support.contains(serverName)) {
        CheckKep14SupportJob *checkKep14Job = new CheckKep14SupportJob(this);
        checkKep14Job->setProperty(QLatin1String("triggerScript").latin1(), true);
        checkKep14Job->setServerName(serverName);
        checkKep14Job->setServerUrl(url);
        connect(checkKep14Job, SIGNAL(result(CheckKep14SupportJob*,bool)), SLOT(slotCheckKep14Ended(CheckKep14SupportJob*,bool)));
        checkKep14Job->start();
    }

    VacationCheckJob *job = new VacationCheckJob(url, serverName, this);
    job->setKep14Support(mKep14Support[serverName]);
    connect(job, SIGNAL(scriptActive(VacationCheckJob*,QString,bool)), this, SLOT(slotScriptActive(VacationCheckJob*,QString,bool)));
    job->start();
}

void MultiImapVacationManager::checkVacation()
{
    if (mCheckInProgress)
        return;
    mNumberOfJobs = 0;
    mCheckInProgress = true;

    QMap <QString, KUrl> list = serverList();
    foreach ( const QString &serverName, list.keys() ) {
        const KUrl url = list.value(serverName);
        checkVacation(serverName, url);
    }
}

void MultiImapVacationManager::slotScriptActive(VacationCheckJob* job, QString scriptName, bool active)
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

void MultiImapVacationManager::slotCheckKep14Ended(CheckKep14SupportJob *job, bool success)
{
    job->deleteLater();
    if (!success) {
        --mNumberOfJobs;
        return;
    }

    mKep14Support.insert(job->serverName(), job->hasKep14Support());

    VacationCheckJob *checkJob = new VacationCheckJob(job->serverUrl(), job->serverName(), this);
    checkJob->setKep14Support(job->hasKep14Support());
    connect(checkJob, SIGNAL(scriptActive(VacationCheckJob*,QString,bool)),
            SLOT(slotScriptActive(VacationCheckJob*,QString,bool)));
    checkJob->start();
}
