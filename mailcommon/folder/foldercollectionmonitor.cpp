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
#include "collectionpage/expirecollectionattribute.h"

#include <ChangeRecorder>
#include <Collection>
#include <CollectionFetchScope>
#include <CollectionModel>
#include <EntityTreeModel>
#include <Item>
#include <ItemDeleteJob>
#include <ItemFetchJob>
#include <ItemFetchScope>
#include <Akonadi/KMime/MessageParts>
#include <AkonadiCore/entityannotationsattribute.h>
#include <QDebug>
#include <KMime/KMimeMessage>

namespace MailCommon
{

FolderCollectionMonitor::FolderCollectionMonitor(Akonadi::Session *session, QObject *parent)
    : QObject(parent)
{
    // monitor collection changes
    mMonitor = new Akonadi::ChangeRecorder(this);
    mMonitor->setSession(session);
    mMonitor->setCollectionMonitored(Akonadi::Collection::root());
    mMonitor->fetchCollectionStatistics(true);
    mMonitor->collectionFetchScope().setIncludeStatistics(true);
    mMonitor->fetchCollection(true);
    mMonitor->setAllMonitored(true);
    mMonitor->setMimeTypeMonitored(KMime::Message::mimeType());
#ifdef MERGE_KNODE_IN_KMAIL
    mMonitor->setMimeTypeMonitored(QString::fromLatin1("message/news"));
#endif
    mMonitor->setResourceMonitored("akonadi_search_resource", true);
    mMonitor->itemFetchScope().fetchPayloadPart(Akonadi::MessagePart::Envelope);
    mMonitor->itemFetchScope().setFetchModificationTime(false);
    mMonitor->itemFetchScope().setFetchRemoteIdentification(false);
    mMonitor->itemFetchScope().setFetchTags(true);
    mMonitor->itemFetchScope().fetchAttribute<Akonadi::EntityAnnotationsAttribute>(true);
}

FolderCollectionMonitor::~FolderCollectionMonitor()
{
}

Akonadi::ChangeRecorder *FolderCollectionMonitor::monitor() const
{
    return mMonitor;
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
                index, Akonadi::CollectionModel::CollectionRole).value<Akonadi::Collection>();

        if (!collection.isValid() || Util::isVirtualCollection(collection)) {
            continue;
        }

        bool mustDeleteExpirationAttribute = false;
        MailCommon::ExpireCollectionAttribute *attr =
            MailCommon::ExpireCollectionAttribute::expirationCollectionAttribute(
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
        qDebug() << " Try to expunge an invalid collection :" << col;
    }
}

void FolderCollectionMonitor::slotDeleteJob(KJob *job)
{
    Util::showJobErrorMessage(job);
}

}

