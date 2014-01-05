/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#include <kmanagesieve/sievejob.h>

using namespace KSieveUi;
VacationCheckJob::VacationCheckJob(const KUrl &url, const QString &serverName, QObject *parent)
    : QObject(parent),
      mServerName(serverName),
      mUrl(url)
{
    mSieveJob = KManageSieve::SieveJob::get( mUrl );
    mSieveJob->setInteractive( false );
    connect( mSieveJob, SIGNAL(gotScript(KManageSieve::SieveJob*,bool,QString,bool)),
             SLOT(slotGetResult(KManageSieve::SieveJob*,bool,QString,bool)) );
}

VacationCheckJob::~VacationCheckJob()
{
    if ( mSieveJob )
        mSieveJob->kill();
    mSieveJob = 0;
}

void VacationCheckJob::slotGetResult(KManageSieve::SieveJob */*job*/, bool success, const QString &/*script*/, bool active)
{
    mSieveJob = 0;
    if ( !success )
        active = false; // default to inactive
    emit scriptActive( active, mServerName );
}
