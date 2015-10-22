/*
 * Copyright (c) 2015 Sandro Knauß <knauss@kolabsys.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "checkkolabkep14supportjob.h"
#include <util/util.h>
#include <kmanagesieve/sievejob.h>

#include <KMessageBox>
#include <KLocalizedString>

#include "libksieve_debug.h"

using namespace KSieveUi;

class KSieveUi::CheckKolabKep14SupportJobPrivate
{
public:
    CheckKolabKep14SupportJobPrivate()
        : mSieveJob(Q_NULLPTR)
        , mKolabKep14Support(false)
    {
    }

    QUrl mUrl;
    KManageSieve::SieveJob *mSieveJob;
    QStringList mAvailableScripts;
    QString mServerName;
    bool mKolabKep14Support;
};

CheckKolabKep14SupportJob::CheckKolabKep14SupportJob(QObject *parent)
    : QObject(parent)
    , d(new CheckKolabKep14SupportJobPrivate)
{

}

CheckKolabKep14SupportJob::~CheckKolabKep14SupportJob()
{
    delete d;
}

void CheckKolabKep14SupportJob::start()
{
    if (d->mUrl.isEmpty()) {
        qCWarning(LIBKSIEVE_LOG) << " server url is empty";
        deleteLater();
        return;
    }
    d->mSieveJob = KManageSieve::SieveJob::list(d->mUrl);
    connect(d->mSieveJob, &KManageSieve::SieveJob::gotList, this, &CheckKolabKep14SupportJob::slotCheckKep14Support);
}

void CheckKolabKep14SupportJob::setServerUrl(const QUrl &url)
{
    d->mUrl = url;
}

QUrl CheckKolabKep14SupportJob::serverUrl() const
{
    return d->mUrl;
}

void CheckKolabKep14SupportJob::setServerName(const QString &name)
{
    d->mServerName = name;
}

QString CheckKolabKep14SupportJob::serverName() const
{
    return d->mServerName;
}

QStringList CheckKolabKep14SupportJob::availableScripts() const
{
    return d->mAvailableScripts;
}

bool CheckKolabKep14SupportJob::hasKep14Support() const
{
    return d->mKolabKep14Support;
}

void CheckKolabKep14SupportJob::slotCheckKep14Support(KManageSieve::SieveJob *job, bool success, const QStringList &availableScripts, const QString &activeScript)
{
    if (!success) {
        Q_EMIT result(this, false);
        return;
    }

    d->mKolabKep14Support = Util::hasKep14Support(job->sieveCapabilities(), availableScripts, activeScript);
    d->mAvailableScripts = availableScripts;
    Q_EMIT result(this, true);
}
