/*
 * Copyright (c) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "indexerutils.h"

#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

#include <Akonadi/ServerManager>

using namespace PimCommon;


QString PimCommon::indexerServiceName()
{
    QLatin1String basename("org.freedesktop.Akonadi.Agent.akonadi_baloo_indexer");
    if (Akonadi::ServerManager::hasInstanceIdentifier()) {
        return basename + QLatin1Char('.') + Akonadi::ServerManager::instanceIdentifier();
    } else {
        return basename;
    }
}

CollectionIndexStatusJob::CollectionIndexStatusJob(const Akonadi::Collection::List &collections,
                                                   QObject* parent)
    : KJob(parent)
    , mCollections(collections)
{
}

CollectionIndexStatusJob::~CollectionIndexStatusJob()
{
}

QMap<qint64, qint64> CollectionIndexStatusJob::resultStats() const
{
    return mResult;
}

void CollectionIndexStatusJob::start()
{
    QDBusInterface indexer(indexerServiceName(), QLatin1String("/"),
                           QLatin1String("org.freedesktop.Akonadi.BalooIndexer"),
                           QDBusConnection::sessionBus(), this);

    Q_FOREACH (const Akonadi::Collection &col, mCollections) {
        QDBusPendingCall call = indexer.asyncCall(QLatin1String("indexedItems"), col.id());
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
        watcher->setProperty("collectionId", col.id());
        connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                this, SLOT(replyReceived(QDBusPendingCallWatcher*)));
        mWatchers.push_back(watcher);
    }

    if (mWatchers.isEmpty()) {
        emitResult();
    }
}

void CollectionIndexStatusJob::replyReceived(QDBusPendingCallWatcher *watcher)
{
    Q_ASSERT(mWatchers.contains(watcher));

    QDBusPendingReply<qint64> reply = *watcher;
    if (reply.isError()) {
        kWarning() << "DBus error:" << reply.error().message();
    } else {
        mResult.insert(watcher->property("collectionId").toLongLong(),
                       reply.argumentAt<0>());
    }

    watcher->deleteLater();
    mWatchers.removeOne(watcher);

    if (mWatchers.isEmpty()) {
        emitResult();
    }
}


