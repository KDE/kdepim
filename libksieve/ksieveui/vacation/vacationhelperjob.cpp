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

VacationHelperJob::VacationHelperJob(const KUrl &url, QObject *parent)
    : QObject(parent),
      mUrl(url),
      mSieveJob(0)
{
}

VacationHelperJob::~VacationHelperJob()
{
    killJob();
}

void VacationHelperJob::killJob()
{
    if ( mSieveJob ) {
        mSieveJob->kill();
        mSieveJob = 0;
    }
}

void VacationHelperJob::searchActiveJob()
{
    killJob();

    if ( !mUrl.isValid() ) {
        Q_EMIT resourceHasNotSieveSupport();
    } else {
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
        Q_EMIT scriptListResult(scriptList, activeScript, caps.contains(QLatin1String("include")));
    } else {
        Q_EMIT canNotGetScriptList();
    }
}

#include "vacationhelperjob.moc"
