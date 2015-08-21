/*
 * Copyright (C) 2014  Sandro Knauß <knauss@kolabsys.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "autoconfigkolabmail.h"
#include <QDomDocument>
#include "accountwizard_debug.h"

AutoconfigKolabMail::AutoconfigKolabMail(QObject *parent)
    : Ispdb(parent)
{

}

void AutoconfigKolabMail::startJob(const QUrl &url)
{
    mData.clear();
    QMap< QString, QVariant > map;
    map[QStringLiteral("errorPage")] = false;
    map[QStringLiteral("no-auth-promt")] = true;
    map[QStringLiteral("no-www-auth")] = true;

    KIO::TransferJob *job = KIO::get(url, KIO::NoReload, KIO::HideProgressInfo);
    job->setMetaData(map);
    connect(job, &KIO::TransferJob::result, this, &AutoconfigKolabMail::slotResult);
    connect(job, &KIO::TransferJob::data, this, &AutoconfigKolabMail::dataArrived);
}

void AutoconfigKolabMail::slotResult(KJob *job)
{
    if (job->error()) {
        if (job->error() == KIO::ERR_INTERNAL_SERVER ||   // error 500
                job->error() == KIO::ERR_UNKNOWN_HOST ||  // unknown host
                job->error() == KIO::ERR_COULD_NOT_CONNECT ||
                job->error() == KIO::ERR_DOES_NOT_EXIST) {    // error 404
            if (serverType() == DataBase) {
                setServerType(IspAutoConfig);
                lookupInDb(false, false);
            } else if (serverType() == IspAutoConfig) {
                setServerType(IspWellKnow);
                lookupInDb(false, false);
            } else {
                Q_EMIT finished(false);
            }
        } else {
            //qCDebug(ACCOUNTWIZARD_LOG) << "Fetching failed" << job->error() << job->errorString();
            Q_EMIT finished(false);
        }
        return;
    }

    KIO::TransferJob *tjob = qobject_cast<KIO::TransferJob *>(job);

    int responsecode = tjob->queryMetaData(QStringLiteral("responsecode")).toInt();

    if (responsecode == 401) {
        lookupInDb(true, true);
        return;
    } else if (responsecode != 200  && responsecode != 0 && responsecode != 304) {
        //qCDebug(ACCOUNTWIZARD_LOG) << "Fetching failed with" << responsecode;
        Q_EMIT finished(false);
        return;
    }

    QDomDocument document;
    bool ok = document.setContent(mData);
    if (!ok) {
        //qCDebug(ACCOUNTWIZARD_LOG) << "Could not parse xml" << mData;
        Q_EMIT finished(false);
        return;
    }
    parseResult(document);
}
