/*
 * Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
 * Copyright (c) 2010 Tobias Koenig <tokoe@kdab.com>
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
 */

#include "aclmanager.h"

#include "aclentrydialog_p.h"
#include "aclutils_p.h"
#include "imapaclattribute.h"
#include "imapresourcesettings.h"
#include "pimutil.h"

#include <Collection>
#include <CollectionFetchJob>
#include <CollectionModifyJob>
#include <Akonadi/Contact/ContactGroupExpandJob>
#include <Akonadi/Contact/ContactGroupSearchJob>

#include <KPIMUtils/Email>

#include <KLocalizedString>
#include <KMessageBox>

#include <QAbstractListModel>
#include <QAction>
#include <QItemSelectionModel>

using namespace PimCommon;

class AclModel : public QAbstractListModel
{
public:
    enum Role {
        UserIdRole = Qt::UserRole + 1,
        PermissionsRole,
        PermissionsTextRole
    };

    AclModel( QObject *parent = 0 )
        : QAbstractListModel( parent )
    {
    }

    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const
    {
        if ( index.row() < 0 || index.row() >= mRights.count() ) {
            return QVariant();
        }

        const QPair<QByteArray, KIMAP::Acl::Rights> right = mRights.at( index.row() );
        switch ( role ) {
        case Qt::DisplayRole:
            return QString::fromLatin1( "%1: %2" ).
                    arg( QString::fromLatin1( right.first ) ).
                    arg( AclUtils::permissionsToUserString( right.second ) );
        case UserIdRole:
            return QString::fromLatin1( right.first );
        case PermissionsRole:
            return QVariant( static_cast<int>( right.second ) );
        case PermissionsTextRole:
            return AclUtils::permissionsToUserString( right.second );
        default:
            return QVariant();
        }

    }

    virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole )
    {
        if ( index.row() < 0 || index.row() >= mRights.count() ) {
            return false;
        }

        QPair<QByteArray, KIMAP::Acl::Rights> &right = mRights[ index.row() ];
        switch ( role ) {
        case UserIdRole:
            right.first = value.toByteArray();
            emit dataChanged( index, index );
            return true;
            break;
        case PermissionsRole:
            right.second = static_cast<KIMAP::Acl::Rights>( value.toInt() );
            emit dataChanged( index, index );
            return true;
            break;
        default:
            return false;
            break;
        }

        return false;
    }

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const
    {
        if ( parent.isValid() ) {
            return 0;
        } else {
            return mRights.count();
        }
    }

    void setRights( const QMap<QByteArray, KIMAP::Acl::Rights> &rights )
    {
        mRights.clear();

        QMapIterator<QByteArray, KIMAP::Acl::Rights> it( rights );
        while ( it.hasNext() ) {
            it.next();
            mRights.append( qMakePair( it.key(), it.value() ) );
        }

        reset();
    }

    QMap<QByteArray, KIMAP::Acl::Rights> rights() const
    {
        QMap<QByteArray, KIMAP::Acl::Rights> result;

        typedef QPair<QByteArray, KIMAP::Acl::Rights> RightPair;
        foreach ( const RightPair &right, mRights ) {
            result.insert( right.first, right.second );
        }

        return result;
    }

protected:
    virtual bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() )
    {
        beginInsertRows( parent, row, row + count - 1 );
        for ( int i = 0; i < count; ++i ) {
            mRights.insert( row, qMakePair( QByteArray(), KIMAP::Acl::Rights() ) );
        }
        endInsertRows();

        return true;
    }

    virtual bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() )
    {
        beginRemoveRows( parent, row, row + count - 1 );
        for ( int i = 0; i < count; ++i ) {
            mRights.remove( row, count );
        }
        endRemoveRows();

        return true;
    }

private:
    QVector<QPair<QByteArray, KIMAP::Acl::Rights> > mRights;
};

class PimCommon::AclManager::Private
{
public:
    Private( AclManager *qq )
        : q( qq ),
          mChanged( false )
    {
        mAddAction = new QAction( i18n( "Add Entry..." ), q );
        q->connect( mAddAction, SIGNAL(triggered(bool)),
                    q, SLOT(addAcl()) );

        mEditAction = new QAction( i18n( "Edit Entry..." ), q );
        mEditAction->setEnabled( false );
        q->connect( mEditAction, SIGNAL(triggered(bool)),
                    q, SLOT(editAcl()) );

        mDeleteAction = new QAction( i18n( "Remove Entry" ), q );
        mDeleteAction->setEnabled( false );
        q->connect( mDeleteAction, SIGNAL(triggered(bool)),
                    q, SLOT(deleteAcl()) );

        mModel = new AclModel( q );

        mSelectionModel = new QItemSelectionModel( mModel );
        q->connect( mSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                    q, SLOT(selectionChanged()) );
    }

    ~Private()
    {
    }

    void selectionChanged()
    {
        const bool itemSelected = !mSelectionModel->selectedIndexes().isEmpty();

        bool canAdmin = ( mUserRights & KIMAP::Acl::Admin );

        bool canAdminThisItem = canAdmin;
        if ( canAdmin && itemSelected ) {
            const QModelIndex index = mSelectionModel->selectedIndexes().first();
            const QString userId = index.data( AclModel::UserIdRole ).toString();
            const KIMAP::Acl::Rights rights =
                    static_cast<KIMAP::Acl::Rights>( index.data( AclModel::PermissionsRole ).toInt() );

            // Don't allow users to remove their own admin permissions - there's no way back
            if ( mImapUserName == userId && ( rights & KIMAP::Acl::Admin ) ) {
                canAdminThisItem = false;
            }
        }

        mAddAction->setEnabled( canAdmin );
        mEditAction->setEnabled( itemSelected && canAdminThisItem );
        mDeleteAction->setEnabled( itemSelected && canAdminThisItem );
    }

    void addAcl()
    {
        AclEntryDialog dlg;
        dlg.setWindowTitle( i18n( "Add ACL" ) );

        if ( !dlg.exec() ) {
            return;
        }

        if ( mModel->insertRow( mModel->rowCount() ) ) {
            const QModelIndex index = mModel->index( mModel->rowCount() - 1, 0 );
            mModel->setData( index, dlg.userId(), AclModel::UserIdRole );
            mModel->setData( index, static_cast<int>( dlg.permissions() ), AclModel::PermissionsRole );

            mChanged = true;
        }
    }

    void editAcl()
    {
        const QModelIndex index = mSelectionModel->selectedIndexes().first();
        const QString userId = index.data( AclModel::UserIdRole ).toString();
        const KIMAP::Acl::Rights permissions =
                static_cast<KIMAP::Acl::Rights>( index.data( AclModel::PermissionsRole ).toInt() );

        AclEntryDialog dlg;
        dlg.setWindowTitle( i18n( "Edit ACL" ) );
        dlg.setUserId( userId );
        dlg.setPermissions( permissions );

        if ( !dlg.exec() ) {
            return;
        }

        mModel->setData( index, dlg.userId(), AclModel::UserIdRole );
        mModel->setData( index, static_cast<int>( dlg.permissions() ), AclModel::PermissionsRole );
        mChanged = true;
    }

    void deleteAcl()
    {
        const QModelIndex index = mSelectionModel->selectedIndexes().first();
        const QString userId = index.data( AclModel::UserIdRole ).toString();

        if ( mImapUserName == userId ) {
            if ( KMessageBox::Cancel == KMessageBox::warningContinueCancel(
                     0,
                     i18n( "Do you really want to remove your own permissions for this folder? "
                           "You will not be able to access it afterwards." ),
                     i18n( "Remove" ) ) )
                return;
        }

        mModel->removeRow( index.row(), QModelIndex() );
        mChanged = true;
    }

    /**
     * We call this method if our first try to get the ACLs for the user fails.
     * That's the case if the ACLs use a different user id than the login name.
     *
     * Examples:
     *   login: testuser                acls: testuser@mydomain.org
     *   login: testuser@mydomain.org   acls: testuser
     */
    static QString guessUserName( const QString &loginName, const QString &serverName )
    {
        if ( loginName.contains( QLatin1Char( '@' ) ) ) {
            // strip of the domain part and use user name only
            return loginName.left( loginName.indexOf( QLatin1Char( '@' ) ) );
        } else {
            int pos = serverName.lastIndexOf( QLatin1Char( '.' ) );
            if ( pos == -1 ) { // no qualified domain name, only hostname
                return QString::fromLatin1( "%1@%2" ).arg( loginName ).arg( serverName );
            }

            pos = serverName.lastIndexOf( QLatin1Char( '.' ), pos - 1 );
            if ( pos == -1 ) { // a simple domain name e.g. mydomain.org
                return QString::fromLatin1( "%1@%2" ).arg( loginName ).arg( serverName );
            } else {
                return QString::fromLatin1( "%1@%2" ).arg( loginName ).arg( serverName.mid( pos + 1 ) );
            }
        }
    }

    void setCollection( const Akonadi::Collection &collection )
    {
        mCollection = collection;
        mChanged = false;

        const PimCommon::ImapAclAttribute *attribute =
                collection.attribute<PimCommon::ImapAclAttribute>();
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
            qDebug()<<" collection has not imap as resources: "<<collection.resource();
        }
        delete imapSettingsInterface;

        mImapUserName = loginName;
        if ( !rights.contains( loginName.toUtf8() ) ) {
            const QString guessedUserName = guessUserName( loginName, serverName );
            if ( rights.contains( guessedUserName.toUtf8() ) ) {
                mImapUserName = guessedUserName;
            }
        }

        mUserRights = rights[ mImapUserName.toUtf8() ];

        mModel->setRights( rights );
        selectionChanged();
    }

    AclManager *q;
    AclModel *mModel;
    QItemSelectionModel *mSelectionModel;
    QAction *mAddAction;
    QAction *mEditAction;
    QAction *mDeleteAction;

    Akonadi::Collection mCollection;
    QString mImapUserName;
    KIMAP::Acl::Rights mUserRights;
    bool mChanged;
};

AclManager::AclManager( QObject *parent )
    : QObject( parent ), d( new Private( this ) )
{
}

AclManager::~AclManager()
{
    delete d;
}

void AclManager::setCollection( const Akonadi::Collection &collection )
{
    d->setCollection( collection );
    emit collectionChanged( d->mCollection );
}

Akonadi::Collection AclManager::collection() const
{
    return d->mCollection;
}

QAbstractItemModel *AclManager::model() const
{
    return d->mModel;
}

QItemSelectionModel *AclManager::selectionModel() const
{
    return d->mSelectionModel;
}

QAction *AclManager::addAction() const
{
    return d->mAddAction;
}

QAction *AclManager::editAction() const
{
    return d->mEditAction;
}

QAction *AclManager::deleteAction() const
{
    return d->mDeleteAction;
}

void AclManager::save()
{
    if ( !d->mCollection.isValid() || !d->mChanged ) {
        return;
    }

    // refresh the collection, it might be outdated in the meantime
    Akonadi::CollectionFetchJob *job =
            new Akonadi::CollectionFetchJob( d->mCollection, Akonadi::CollectionFetchJob::Base );
    if ( !job->exec() ) {
        return;
    }

    if ( job->collections().isEmpty() ) {
        return;
    }

    d->mCollection = job->collections().first();

    d->mChanged = false;

    PimCommon::ImapAclAttribute *attribute =
            d->mCollection.attribute<PimCommon::ImapAclAttribute>();

    QMap<QByteArray, KIMAP::Acl::Rights> newRights;

    const QMap<QByteArray, KIMAP::Acl::Rights> rights = d->mModel->rights();
    QMapIterator<QByteArray, KIMAP::Acl::Rights> it( rights );
    while ( it.hasNext() ) {
        it.next();

        // we can use job->exec() here, it is not a hot path
        Akonadi::ContactGroupSearchJob *searchJob = new Akonadi::ContactGroupSearchJob( this );
        searchJob->setQuery( Akonadi::ContactGroupSearchJob::Name, QString::fromLatin1(it.key()) );
        searchJob->setLimit( 1 );
        if ( !searchJob->exec() ) {
            continue;
        }

        if ( !searchJob->contactGroups().isEmpty() ) { // it has been a distribution list
            Akonadi::ContactGroupExpandJob *expandJob =
                    new Akonadi::ContactGroupExpandJob( searchJob->contactGroups().first(), this );
            if ( expandJob->exec() ) {
                foreach ( const KABC::Addressee &contact, expandJob->contacts() ) {
                    const QByteArray rawEmail =
                            KPIMUtils::extractEmailAddress( contact.preferredEmail().toUtf8() );
                    if ( !rawEmail.isEmpty() ) {
                        newRights[ rawEmail ] = it.value();
                    }
                }
            }
        } else { // it has been a normal contact
            const QByteArray rawEmail = KPIMUtils::extractEmailAddress( it.key() );
            if ( !rawEmail.isEmpty() ) {
                newRights[ rawEmail ] = it.value();
            }
        }
    }

    attribute->setRights( newRights );

    new Akonadi::CollectionModifyJob( d->mCollection );
}

#include "moc_aclmanager.cpp"
