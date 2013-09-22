/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "vacationmanager.h"
#include "vacation.h"
#include "util.h"

#include <akonadi/agentinstance.h>


using namespace KSieveUi;
VacationManager::VacationManager(QObject *parent)
    : QObject(parent),
      mWasInitialized(false)
{
}

VacationManager::~VacationManager()
{

}

void VacationManager::findImapResourceWithVacationSupport()
{
    const Akonadi::AgentInstance::List instances = Util::imapAgentInstances();
    foreach ( const Akonadi::AgentInstance &instance, instances ) {
        if ( instance.status() == Akonadi::AgentInstance::Broken )
            continue;

        const KUrl url = Util::findSieveUrlForAccount( instance.identifier() );
        if ( !url.isEmpty() ) {
            vacationInfo info;
            info.displayName = instance.name();
            info.url = url;
            mImapUrl.insert(instance.identifier(), info);
        }
    }
    mWasInitialized = true;
}


void VacationManager::checkVacation()
{
    if (!mWasInitialized) {
        findImapResourceWithVacationSupport();
    }

    QHash<QString, vacationInfo>::const_iterator i = mImapUrl.constBegin();
     while (i != mImapUrl.constEnd()) {
         Vacation *vacationJob = new Vacation(this, true, i.value().url);
         connect( vacationJob, SIGNAL(scriptActive(bool,QString)), SIGNAL(updateVacationScriptStatus(bool,QString)) );
         connect( vacationJob, SIGNAL(requestEditVacation()), SIGNAL(editVacation()) );
         ++i;
     }

    //TODO check vacation
}

#include "vacationmanager.moc"
