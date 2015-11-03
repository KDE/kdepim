/*
  Copyright (c) 2009, 2010, 2011 Montel Laurent <montel@kde.org>

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

#include "foldercollectionmonitor.h"
#include "util/mailutil.h"
#include "foldercollection.h"
#include "mailcommon_debug.h"
#include <ChangeRecorder>
#include <Collection>
#include <CollectionFetchScope>
#include <EntityTreeModel>
#include <Item>
#include <ItemDeleteJob>
#include <ItemFetchJob>
#include <ItemFetchScope>
#include "collectionpage/attributes/expirecollectionattribute.h"
#include <Akonadi/KMime/MessageParts>
#include <AkonadiCore/entityannotationsattribute.h>

#include <KMime/KMimeMessage>

namespace MailCommon
{

class FolderCollectionMonitorPrivate
{
public:
    FolderCollectionMonitorPrivate()
        : mMonitor(Q_NULLPTR)
    {

    }

    Akonadi::ChangeRecorder *mMonitor;
};

FolderCollectionMonitor::FolderCollectionMonitor(Akonadi::Session *session, QObject *parent)
    : QObject(parent),
      d(new MailCommon::FolderCollectionMonitorPrivate)
{
    // monitor collection changes
    d->mMonitor = new Akonadi::ChangeRecorder(this);
    d->mMonitor->setSession(session);
    d->mMonitor->setCollectionMonitored(Akonadi::Collection::root());
    d->mMonitor->fetchCollectionStatistics(true);
    d->mMonitor->collectionFetchScope().setIncludeStatistics(true);
    d->mMonitor->fetchCollection(true);
    d->mMonitor->setAllMonitored(true);
    d->mMonitor->setMimeTypeMonitored(KMime::Message::mimeType());
    d->mMonitor->setResourceMonitored("akonadi_search_resource", true);
    d->mMonitor->itemFetchScope().fetchPayloadPart(Akonadi::MessagePart::Envelope);
    d->mMonitor->itemFetchScope().setFetchModificationTime(false);
    d->mMonitor->itemFetchScope().setFetchRemoteIdentification(false);
    d->mMonitor->itemFetchScope().setFetchTags(true);
    d->mMonitor->itemFetchScope().fetchAttribute<Akonadi::EntityAnnotationsAttribute>(true);
}

FolderCollectionMonitor::~FolderCollectionMonitor()
{
    delete d;
}

Akonadi::ChangeRecorder *FolderCollectionMonitor::monitor() const
{
    return d->mMonitor;
}

void FolderCollectionMonitor::expireAllFolders(bool immediate,
        QAbstractItemModel *collectionModel)
{
    if (collectionModel) {
        expireAllCollection(collectionModel, immediate);
    }
}

void FolderCollectionMonitor::expireAllCollection(const QAbstractItemModel *model,
        bool immediate,
        const QModelIndex &parentIndex)
{
    const int rowCount = model->rowCount(parentIndex);
    for (int row = 0; row < rowCount; ++row) {
        const QModelIndex index = model->index(row, 0, parentIndex);
        const Akonadi::Collection collection =
            model->data(
                index, Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();

        if (!collection.isValid() || Util::isVirtualCollection(collection)) {
            continue;
        }

        bool mustDeleteExpirationAttribute = false;
        MailCommon::ExpireCollectionAttribute *attr =
            MailCommon::Util::expirationCollectionAttribute(
                collection, mustDeleteExpirationAttribute);

        if (attr->isAutoExpire()) {
            MailCommon::Util::expireOldMessages(collection, immediate);
        }

        if (model->rowCount(index) > 0) {
            expireAllCollection(model, immediate, index);
        }

        if (mustDeleteExpirationAttribute) {
            delete attr;
        }
    }
}

void FolderCollectionMonitor::expunge(const Akonadi::Collection &col, bool sync)
{
    if (col.isValid()) {
        Akonadi::ItemDeleteJob *job = new Akonadi::ItemDeleteJob(col, this);
        connect(job, &Akonadi::ItemDeleteJob::result, this, &FolderCollectionMonitor::slotDeleteJob);
        if (sync) {
            job->exec();
        }
    } else {
        qCDebug(MAILCOMMON_LOG) << " Try to expunge an invalid collection :" << col;
    }
}

void FolderCollectionMonitor::slotDeleteJob(KJob *job)
{
    Util::showJobErrorMessage(job);
}

}

