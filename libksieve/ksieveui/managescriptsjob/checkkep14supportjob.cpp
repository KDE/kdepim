/*
  Copyright (c) 2015 Sandro Knauß <knauss@kolabsys.com>

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

#include "checkkep14supportjob.h"
#include <util/util.h>
#include <kmanagesieve/sievejob.h>

#include <KMessageBox>
#include <KLocalizedString>
#include <KDebug>

using namespace KSieveUi;

CheckKep14SupportJob::CheckKep14SupportJob(QObject *parent)
    : QObject(parent),
      mSieveJob(0)
{

}

CheckKep14SupportJob::~CheckKep14SupportJob()
{

}

void CheckKep14SupportJob::start()
{
    if (mUrl.isEmpty()) {
        qDebug() << " server url is empty";
        deleteLater();
        return;
    }
    mSieveJob = KManageSieve::SieveJob::list(mUrl);
    connect(mSieveJob, SIGNAL(gotList(KManageSieve::SieveJob*,bool,QStringList,QString)),
         this, SLOT(slotCheckKep14Support(KManageSieve::SieveJob*,bool,QStringList,QString)));
}

void CheckKep14SupportJob::setServerUrl(const KUrl &url)
{
    mUrl = url;
}

KUrl CheckKep14SupportJob::serverUrl()
{
    return mUrl;
}

void CheckKep14SupportJob::setServerName(const QString &name)
{
    mServerName = name;
}

QString CheckKep14SupportJob::serverName()
{
    return mServerName;
}


QStringList CheckKep14SupportJob::availableScripts()
{
    return mAvailableScripts;
}

bool CheckKep14SupportJob::hasKep14Support()
{
    return mKep14Support;
}

void CheckKep14SupportJob::slotCheckKep14Support(KManageSieve::SieveJob *job, bool success, const QStringList &availableScripts, const QString &activeScript)
{
    if (!success) {
        emit result(this, false);
        return;
    }

    mKep14Support = Util::hasKep14Support(job->sieveCapabilities(), availableScripts, activeScript);
    mAvailableScripts = availableScripts;
    emit result(this, true);
}
