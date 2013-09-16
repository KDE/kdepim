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

#include "vacationhelperjob.h"
#include "kmanagesieve/sievejob.h"
#include "ksieveui/util.h"

using namespace KSieveUi;

VacationHelperJob::VacationHelperJob(const QString &accountName, QObject *parent)
    : QObject(parent),
      mAccountName(accountName),
      mSieveJob(0)
{
}

VacationHelperJob::~VacationHelperJob()
{
    if ( mSieveJob ) {
        mSieveJob->kill();
        mSieveJob = 0;
    }
}

void VacationHelperJob::searchActiveJob()
{
    if (mSieveJob) {
        mSieveJob->kill();
        mSieveJob = 0;
    }

    const KUrl url = KSieveUi::Util::findSieveUrlForAccount( mAccountName );
    if ( !url.isValid() ) {
        Q_EMIT resourceHasNotSieveSupport();
    } else {
        mUrl = url;

        mSieveJob = KManageSieve::SieveJob::list( mUrl );

        connect( mSieveJob, SIGNAL(gotList(KManageSieve::SieveJob*,bool,QStringList,QString)),
                 SLOT(slotGetScriptList(KManageSieve::SieveJob*,bool,QStringList,QString)) );
    }
}

void VacationHelperJob::slotGetScriptList( KManageSieve::SieveJob *job, bool success, const QStringList &scriptList, const QString &activeScript )
{
    mSieveJob = 0;
    if (success) {
        const QStringList caps = job->sieveCapabilities();
        if (!activeScript.isEmpty()) {
            Q_EMIT hasActiveScript(activeScript);
        }
        //TODO
    } else {
        Q_EMIT canNotGetScriptList();
    }
}

#include "vacationhelperjob.moc"
