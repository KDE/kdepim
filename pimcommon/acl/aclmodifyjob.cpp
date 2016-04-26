/*
  Copyright (c) 2016 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "aclmodifyjob.h"
#include "imapresourcesettings.h"
#include "pimutil.h"
#include "aclutils_p.h"

#include "imapaclattribute.h"
#include "job/fetchrecursivecollectionsjob.h"
#include <kdebug.h>
#include <KMessageBox>
#include <KLocalizedString>
#include <QDBusInterface>
#include <akonadi/collectionmodifyjob.h>

using namespace PimCommon;

AclModifyJob::AclModifyJob(QObject *parent)
    : QObject(parent),
      mRecursive(false),
      mCurrentIndex(-1)
{
}

AclModifyJob::~AclModifyJob()
{

}

void AclModifyJob::start()
{
    if (!mTopLevelCollection.isValid()) {
        deleteLater();
        return;
    }
    mCurrentIndex = 0;
    if (mRecursive) {
        PimCommon::FetchRecursiveCollectionsJob *fetchJob = new PimCommon::FetchRecursiveCollectionsJob(this);
        fetchJob->setTopCollection(mTopLevelCollection);
        connect(fetchJob, SIGNAL(fetchCollectionFailed()), this, SLOT(slotFetchCollectionFailed()));
        connect(fetchJob, SIGNAL(fetchCollectionFinished(Akonadi::Collection::List)), this, SLOT(slotFetchCollectionFinished(Akonadi::Collection::List)));
        fetchJob->start();
    } else {
        changeAcl(mTopLevelCollection);
    }
}

bool AclModifyJob::canAdministrate(PimCommon::ImapAclAttribute *attribute, const Akonadi::Collection &collection) const
{
    if (!attribute || !collection.isValid()) {
        return false;
    }
    const QMap<QByteArray, KIMAP::Acl::Rights> rights = attribute->rights();

    QString resource = collection.resource();
    if (resource.contains(QLatin1String("akonadi_kolabproxy_resource"))) {
        QDBusInterface interface( QLatin1String("org.freedesktop.Akonadi.Agent.akonadi_kolabproxy_resource"), QLatin1String("/KolabProxy") );
        if (interface.isValid()) {
            QDBusReply<QString> reply = interface.call(QLatin1String("imapResourceForCollection"), collection.remoteId().toLongLong());
            if (reply.isValid()) {
                resource = reply;
            }
        }
    }
    OrgKdeAkonadiImapSettingsInterface *imapSettingsInterface =
            PimCommon::Util::createImapSettingsInterface( resource );

    QString loginName;
    QString serverName;
    if ( imapSettingsInterface->isValid() ) {
        QDBusReply<QString> reply = imapSettingsInterface->userName();
        if ( reply.isValid() ) {
            loginName = reply;
        }

        reply = imapSettingsInterface->imapServer();
        if ( reply.isValid() ) {
            serverName = reply;
        }
    } else {
        kDebug()<<"AclModifyJob:  collection has not imap as resources: "<<collection.resource();
    }
    delete imapSettingsInterface;

    QString imapUserName = loginName;
    if ( !rights.contains( loginName.toUtf8() ) ) {
        const QString guessedUserName = AclUtils::guessUserName( loginName, serverName );
        if ( rights.contains( guessedUserName.toUtf8() ) ) {
            imapUserName = guessedUserName;
        }
    }

    return rights[ imapUserName.toUtf8() ] & KIMAP::Acl::Admin;
}

void AclModifyJob::changeAcl(Akonadi::Collection collection)
{
    if (collection.hasAttribute<PimCommon::ImapAclAttribute>()) {
        PimCommon::ImapAclAttribute *attribute = collection.attribute<PimCommon::ImapAclAttribute>();
        if (canAdministrate(attribute, collection)) {
            attribute->setRights( mNewRight );
            Akonadi::CollectionModifyJob *modifyJob = new Akonadi::CollectionModifyJob( collection );
            connect(modifyJob, SIGNAL(result(KJob*)), this, SLOT(slotModifyDone(KJob*)));
            kDebug() << "AclModifyJob: modifiying acl for: " << collection.name();
        } else {
            kDebug() << "AclModifyJob: Can't administratrate ACL for this folder.";
        }
    } else {
        checkNewCollection();
    }
}

void AclModifyJob::checkNewCollection()
{
    mCurrentIndex++;
    if (mCurrentIndex < mRecursiveCollection.count()) {
        changeAcl(mRecursiveCollection.at(mCurrentIndex));
    } else {
        deleteLater();
    }
}

void AclModifyJob::slotModifyDone(KJob *job)
{
    if (job->error()) {
        kDebug() << "AclModifyJob: Error during modify collection " << job->errorString();
    }
    checkNewCollection();
}

void AclModifyJob::slotFetchCollectionFinished(const Akonadi::Collection::List &collectionList)
{
    QStringList folderNames;
    Q_FOREACH (const Akonadi::Collection col, collectionList) {
        if (col.hasAttribute<PimCommon::ImapAclAttribute>()) {
            PimCommon::ImapAclAttribute *attribute = col.attribute<PimCommon::ImapAclAttribute>();
            if (canAdministrate(attribute, col)) {
                QString fullName;
                bool parentFound;
                Akonadi::Collection cur = col;
                do {
                    parentFound = false;
                    // parentCollection() only returns the ID, but we have the
                    // collection in our list already because we've recursively
                    // fetched. So look it up in our list.
                    Q_FOREACH (const Akonadi::Collection it, collectionList) {
                        if (it.id() == cur.id()) {
                            fullName = QLatin1String("/") + it.displayName() + fullName;
                            parentFound = true;
                            cur = cur.parentCollection();
                            break;
                        }
                    }
                } while (parentFound);
                // Remove leading slash
                folderNames << fullName.right(fullName.size() - 1);
            } else {
                kDebug() << "AclModifyJob: No rights to administer " << col.name();
            }
        } else {
            kDebug() << "AclModifyJob: Collection " << col.name() << "has no ACL.";
        }
    }
    folderNames.sort();
    if ( KMessageBox::Continue != KMessageBox::warningContinueCancelList(0,
                                                                         i18n( "Do you really want to apply this folders permissions on the subdirectories:" ),
                                                                         folderNames,
                                                                         i18n( "Apply Permissions" ) ) ) {
        deleteLater();
        kDebug() << "AclModifyJob: User canceled .";
        return;
    }
    mRecursiveCollection = collectionList;
    changeAcl(mTopLevelCollection);
}

void AclModifyJob::slotFetchCollectionFailed()
{
    kDebug() << "AclModifyJob: fetch collection failed";
    deleteLater();
}

void AclModifyJob::setTopLevelCollection(const Akonadi::Collection &topLevelCollection)
{
    mTopLevelCollection = topLevelCollection;
}

void AclModifyJob::setRecursive(bool recursive)
{
    mRecursive = recursive;
}

void AclModifyJob::setNewRights(const QMap<QByteArray, KIMAP::Acl::Rights> &right)
{
    mNewRight = right;
}
