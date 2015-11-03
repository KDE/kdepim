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

#include "vacationhelperjob.h"
#include "kmanagesieve/sievejob.h"
#include "ksieveui/util.h"

using namespace KSieveUi;

VacationHelperJob::VacationHelperJob(const QUrl &url, QObject *parent)
    : QObject(parent),
      mUrl(url),
      mSieveJob(Q_NULLPTR)
{
}

VacationHelperJob::~VacationHelperJob()
{
    killJob();
}

void VacationHelperJob::killJob()
{
    if (mSieveJob) {
        mSieveJob->kill();
        mSieveJob = Q_NULLPTR;
    }
}

void VacationHelperJob::searchActiveJob()
{
    killJob();

    if (!mUrl.isValid()) {
        Q_EMIT resourceHasNotSieveSupport();
    } else {
        mSieveJob = KManageSieve::SieveJob::list(mUrl);

        connect(mSieveJob, &KManageSieve::SieveJob::gotList, this, &VacationHelperJob::slotGetScriptList);
    }
}

void VacationHelperJob::slotGetScriptList(KManageSieve::SieveJob *job, bool success, const QStringList &scriptList, const QString &activeScript)
{
    mSieveJob = Q_NULLPTR;
    if (success) {
        const QStringList caps = job->sieveCapabilities();
        Q_EMIT scriptListResult(scriptList, activeScript, caps.contains(QStringLiteral("include")));
    } else {
        Q_EMIT canNotGetScriptList();
    }
}

