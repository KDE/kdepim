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

#include <AgentInstance>

#include <KMessageBox>
#include <KLocalizedString>

using namespace KSieveUi;
MultiImapVacationManager::MultiImapVacationManager(QObject *parent)
    : QObject(parent),
      mNumberOfJobs(0),
      mQuestionAsked(false),
      mCheckInProgress(false)
{
}

MultiImapVacationManager::~MultiImapVacationManager()
{

}

void MultiImapVacationManager::checkVacation()
{
    if (mCheckInProgress) {
        return;
    }
    mNumberOfJobs = 0;
    mCheckInProgress = true;
    mQuestionAsked = false;

    const Akonadi::AgentInstance::List instances = KSieveUi::Util::imapAgentInstances();
    foreach (const Akonadi::AgentInstance &instance, instances) {
        if (instance.status() == Akonadi::AgentInstance::Broken) {
            continue;
        }

        const QUrl url = KSieveUi::Util::findSieveUrlForAccount(instance.identifier());
        if (!url.isEmpty()) {
            const QString serverName = instance.name();
            ++mNumberOfJobs;
            VacationCheckJob *job = new VacationCheckJob(url, serverName, this);
            connect(job, &VacationCheckJob::scriptActive, this, &MultiImapVacationManager::slotScriptActive);
        }
    }
}

void MultiImapVacationManager::slotScriptActive(bool active, const QString &serverName)
{
    --mNumberOfJobs;
    Q_EMIT scriptActive(active, serverName);

    if (active) {
        if (!mQuestionAsked) {
            mQuestionAsked = true;
            if (KMessageBox::questionYesNo(0, i18n("There is still an active out-of-office reply configured.\n"
                                                   "Do you want to edit it?"), i18n("Out-of-office reply still active"),
                                           KGuiItem(i18n("Edit"), QStringLiteral("document-properties")),
                                           KGuiItem(i18n("Ignore"), QStringLiteral("dialog-cancel")))
                    == KMessageBox::Yes) {
                Q_EMIT requestEditVacation();
            }
        }
    }

    if (mNumberOfJobs == 0) {
        mCheckInProgress = false;
    }
}
