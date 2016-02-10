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

#ifndef INDEXERUTILS_H
#define INDEXERUTILS_H

#include <KJob>
#include "pimcommon_export.h"

#include <Akonadi/Collection>

class QDBusPendingCallWatcher;

namespace PimCommon
{

PIMCOMMON_EXPORT QString indexerServiceName();


class PIMCOMMON_EXPORT CollectionIndexStatusJob : public KJob
{
    Q_OBJECT
public:
    CollectionIndexStatusJob(const Akonadi::Collection::List &collections, QObject *parent = 0);
    ~CollectionIndexStatusJob();

    void start();

    QMap<qint64, qint64> resultStats() const;

private Q_SLOTS:
    void replyReceived(QDBusPendingCallWatcher *watcher);

private:
    QList<QDBusPendingCallWatcher*> mWatchers;
    Akonadi::Collection::List mCollections;
    QMap<qint64 /* collectionId */, qint64 /* indexed cnt */> mResult;
};

}

#endif
