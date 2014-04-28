/*
    Copyright (c) 2009 Kevin Ottens <ervin@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "storagemodel.h"

#include <messagecore/utils/stringutil.h>
#include <messagecore/settings/globalsettings.h>
#include <messagecore/helpers/nodehelper.h>

#include <attributefactory.h>
#include <collection.h>
#include <collectionstatistics.h>
#include <entitymimetypefiltermodel.h>
#include <entitytreemodel.h>
#include <item.h>
#include <itemmodifyjob.h>
#include <akonadi/kmime/messagefolderattribute.h>
#include <selectionproxymodel.h>

#include <QDebug>
#include <KLocale>
#include "core/messageitem.h"
#include "core/settings.h"
#include "messagelistutil.h"


#include <QtCore/QAbstractItemModel>
#include <QtCore/QAtomicInt>
#include <QtCore/QScopedPointer>
#include <QItemSelectionModel>
#include <QtCore/QMimeData>
#include <QtCore/QCryptographicHash>

namespace MessageList
{

class StorageModel::Private
{
public:
    void onSourceDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight );
    void onSelectionChanged();
    void loadSettings();

    StorageModel * const q;

    QAbstractItemModel *mModel;
    QItemSelectionModel *mSelectionModel;

    Private( StorageModel *owner )
        : q( owner ),
          mModel(0),
          mSelectionModel(0)
    {}
};

} // namespace MessageList

using namespace Akonadi;
using namespace MessageList;

namespace {

KMime::Message::Ptr messageForItem( const Akonadi::Item &item )
{
    if ( !item.hasPayload<KMime::Message::Ptr>() ) {
        qWarning() << "Not a message" << item.id() << item.remoteId() << item.mimeType();
        return KMime::Message::Ptr();
    }
    return item.payload<KMime::Message::Ptr>();
}

}

static QAtomicInt _k_attributeInitialized;

StorageModel::StorageModel( QAbstractItemModel *model, QItemSelectionModel *selectionModel, QObject *parent )
    : Core::StorageModel( parent ), d( new Private( this ) )
{
    d->mSelectionModel = selectionModel;
    if ( _k_attributeInitialized.testAndSetAcquire( 0, 1 ) ) {
        AttributeFactory::registerAttribute<MessageFolderAttribute>();
    }

    Akonadi::SelectionProxyModel *childrenFilter = new Akonadi::SelectionProxyModel( d->mSelectionModel, this );
    childrenFilter->setSourceModel( model );
    childrenFilter->setFilterBehavior( KSelectionProxyModel::ChildrenOfExactSelection );

    EntityMimeTypeFilterModel *itemFilter = new EntityMimeTypeFilterModel( this );
    itemFilter->setSourceModel( childrenFilter );
    itemFilter->addMimeTypeExclusionFilter( Collection::mimeType() );
    itemFilter->addMimeTypeInclusionFilter( QLatin1String( "message/rfc822" ) );
    itemFilter->setHeaderGroup( EntityTreeModel::ItemListHeaders );

    d->mModel = itemFilter;

    qDebug() << "Using model:" << model->metaObject()->className();

    connect( d->mModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
             this, SLOT(onSourceDataChanged(QModelIndex,QModelIndex)) );

    connect( d->mModel, SIGNAL(layoutAboutToBeChanged()),
             this, SIGNAL(layoutAboutToBeChanged()) );
    connect( d->mModel, SIGNAL(layoutChanged()),
             this, SIGNAL(layoutChanged()) );
    connect( d->mModel, SIGNAL(modelAboutToBeReset()),
             this, SIGNAL(modelAboutToBeReset()) );
    connect( d->mModel, SIGNAL(modelReset()),
             this, SIGNAL(modelReset()) );

    //Here we assume we'll always get QModelIndex() in the parameters
    connect( d->mModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
             this, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)) );
    connect( d->mModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
             this, SIGNAL(rowsInserted(QModelIndex,int,int)) );
    connect( d->mModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
             this, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)) );
    connect( d->mModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
             this, SIGNAL(rowsRemoved(QModelIndex,int,int)) );

    connect( d->mSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
             this, SLOT(onSelectionChanged()) );

    d->loadSettings();
    connect( Core::Settings::self(), SIGNAL(configChanged()),
             this, SLOT(loadSettings()) );
}

StorageModel::~StorageModel()
{
    delete d;
}

Collection::List StorageModel::displayedCollections() const
{
    Collection::List collections;
    QModelIndexList indexes = d->mSelectionModel->selectedRows();

    foreach ( const QModelIndex &index, indexes ) {
        Collection c = index.data( EntityTreeModel::CollectionRole ).value<Collection>();
        if ( c.isValid() ) {
            collections << c;
        }
    }

    return collections;
}

QString StorageModel::id() const
{
    QStringList ids;
    QModelIndexList indexes = d->mSelectionModel->selectedRows();

    foreach ( const QModelIndex &index, indexes ) {
        Collection c = index.data( EntityTreeModel::CollectionRole ).value<Collection>();
        if ( c.isValid() ) {
            ids << QString::number( c.id() );
        }
    }

    ids.sort();
    return ids.join(QLatin1String( ":" ));
}

bool StorageModel::isOutBoundFolder( const Akonadi::Collection& c ) const
{
    if ( c.hasAttribute<MessageFolderAttribute>()
         && c.attribute<MessageFolderAttribute>()->isOutboundFolder() ) {
        return true;
    }
    return false;
}

bool StorageModel::containsOutboundMessages() const
{
    QModelIndexList indexes = d->mSelectionModel->selectedRows();

    foreach ( const QModelIndex &index, indexes ) {
        Collection c = index.data( EntityTreeModel::CollectionRole ).value<Collection>();
        if ( c.isValid() ) {
            return isOutBoundFolder( c );
        }
    }

    return false;
}

int StorageModel::initialUnreadRowCountGuess() const
{
    QModelIndexList indexes = d->mSelectionModel->selectedRows();

    int unreadCount = 0;

    foreach ( const QModelIndex &index, indexes ) {
        Collection c = index.data( EntityTreeModel::CollectionRole ).value<Collection>();
        if ( c.isValid() ) {
            unreadCount+= c.statistics().unreadCount();
        }
    }

    return unreadCount;
}

bool StorageModel::initializeMessageItem( MessageList::Core::MessageItem *mi,
                                          int row, bool bUseReceiver ) const
{
    const Item item = itemForRow( row );
    const KMime::Message::Ptr mail = messageForItem( item );
    if ( !mail ) {
        return false;
    }

    QString sender;
    if ( mail->from() ) {
        sender = mail->from()->asUnicodeString();
    }
    QString receiver;
    if ( mail->to() ) {
        receiver = mail->to()->asUnicodeString();
    }

    // Static for speed reasons
    static const QString noSubject = i18nc( "displayed as subject when the subject of a mail is empty", "No Subject" );
    static const QString unknown( i18nc( "displayed when a mail has unknown sender, receiver or date", "Unknown" ) );

    if ( sender.isEmpty() ) {
        sender = unknown;
    }
    if ( receiver.isEmpty() ) {
        receiver = unknown;
    }

    mi->initialSetup( mail->date()->dateTime().toTime_t(),
                      item.size(),
                      sender, receiver,
                      bUseReceiver );
    mi->setItemId( item.id() );

    QString subject = mail->subject()->asUnicodeString();
    if ( subject.isEmpty() ) {
        subject = QLatin1Char( '(' ) + noSubject + QLatin1Char( ')' );
    }

    mi->setSubject( subject );

    updateMessageItemData( mi, row );

    return true;
}

static QByteArray md5Encode( const QByteArray &str )
{
    if ( str.trimmed().isEmpty() ) return QByteArray();

    QCryptographicHash c( QCryptographicHash::Md5 );
    c.addData( str.trimmed() );
    return c.result();
}

void StorageModel::fillMessageItemThreadingData( MessageList::Core::MessageItem *mi,
                                                 int row, ThreadingDataSubset subset ) const
{
    const KMime::Message::Ptr mail = messageForRow( row );
    Q_ASSERT( mail ); // We ASSUME that initializeMessageItem has been called successfully...

    switch ( subset ) {
    case PerfectThreadingReferencesAndSubject:
    {
        const QString subject = mail->subject()->asUnicodeString();
        const QString strippedSubject = MessageCore::StringUtil::stripOffPrefixes( subject );
        mi->setStrippedSubjectMD5( md5Encode( strippedSubject.toUtf8() ) );
        mi->setSubjectIsPrefixed( subject != strippedSubject );
        // fall through
    }
    case PerfectThreadingPlusReferences:
        if ( !mail->references()->identifiers().isEmpty() ) {
            mi->setReferencesIdMD5( md5Encode( mail->references()->identifiers().last() ) );
        }
        // fall through
    case PerfectThreadingOnly:
        mi->setMessageIdMD5( md5Encode( mail->messageID()->identifier() ) );
        if ( !mail->inReplyTo()->identifiers().isEmpty() ) {
            mi->setInReplyToIdMD5( md5Encode( mail->inReplyTo()->identifiers().first() ) );
        }
        break;
    default:
        Q_ASSERT( false ); // should never happen
        break;
    }
}

void StorageModel::updateMessageItemData( MessageList::Core::MessageItem *mi,
                                          int row ) const
{
    const Item item = itemForRow( row );

    Akonadi::MessageStatus stat;
    stat.setStatusFromFlags( item.flags() );

    mi->setAkonadiItem( item );
    mi->setStatus( stat );


    if ( stat.isEncrypted() )
        mi->setEncryptionState( Core::MessageItem::FullyEncrypted );
    else
        mi->setEncryptionState( Core::MessageItem::EncryptionStateUnknown );


    if ( stat.isSigned() )
        mi->setSignatureState( Core::MessageItem::FullySigned );
    else
        mi->setSignatureState( Core::MessageItem::SignatureStateUnknown );

    mi->invalidateTagCache();
    mi->invalidateAnnotationCache();
}

void StorageModel::setMessageItemStatus( MessageList::Core::MessageItem *mi,
                                         int row, const Akonadi::MessageStatus &status )
{
    Q_UNUSED( mi );
    Item item = itemForRow( row );
    item.setFlags( status.statusFlags() );
    ItemModifyJob *job = new ItemModifyJob( item, this );
    job->disableRevisionCheck();
    job->setIgnorePayload( true );
}

QVariant StorageModel::data( const QModelIndex &index, int role ) const
{
    // We don't provide an implementation for data() in No-Akonadi-KMail.
    // This is because StorageModel must be a wrapper anyway (because columns
    // must be re-mapped and functions not available in a QAbstractItemModel
    // are strictly needed. So when porting to Akonadi this class will
    // either wrap or subclass the MessageModel and implement initializeMessageItem()
    // with appropriate calls to data(). And for No-Akonadi-KMail we still have
    // a somewhat efficient implementation.

    Q_UNUSED( index );
    Q_UNUSED( role );

    return QVariant();
}

int StorageModel::columnCount( const QModelIndex &parent ) const
{
    if ( !parent.isValid() )
        return 1;
    return 0; // this model is flat.
}

QModelIndex StorageModel::index( int row, int column, const QModelIndex &parent ) const
{
//QT5
#if 0
    if ( !parent.isValid() )
        return createIndex( row, column, 0 );
#endif
    return QModelIndex(); // this model is flat.
}

QModelIndex StorageModel::parent( const QModelIndex &index ) const
{
    Q_UNUSED( index );
    return QModelIndex(); // this model is flat.
}

int StorageModel::rowCount( const QModelIndex &parent ) const
{
    if ( !parent.isValid() )
        return d->mModel->rowCount();
    return 0; // this model is flat.
}

QMimeData* StorageModel::mimeData( const QList< MessageList::Core::MessageItem* >& items ) const
{
    QMimeData *data = new QMimeData();
    KUrl::List urls;
    foreach ( MessageList::Core::MessageItem* mi, items ) {
        Akonadi::Item item = itemForRow( mi->currentModelIndexRow() );
        urls << item.url( Item::UrlWithMimeType );
    }

    urls.populateMimeData( data );

    return data;
}


void StorageModel::prepareForScan()
{

}

void StorageModel::Private::onSourceDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight )
{
    emit q->dataChanged( q->index( topLeft.row(), 0 ),
                         q->index( bottomRight.row(), 0 ) );
}

void StorageModel::Private::onSelectionChanged()
{
    emit q->headerDataChanged( Qt::Horizontal, 0, q->columnCount()-1 );
}

void StorageModel::Private::loadSettings()
{
    // Custom/System colors
    Core::Settings *settings = Core::Settings::self();

    if ( MessageCore::GlobalSettings::self()->useDefaultColors() ) {
        Core::MessageItem::setUnreadMessageColor( MessageList::Util::unreadDefaultMessageColor() );
        Core::MessageItem::setImportantMessageColor( MessageList::Util::importantDefaultMessageColor() );
        Core::MessageItem::setToDoMessageColor( MessageList::Util::todoDefaultMessageColor() );
    } else {
        Core::MessageItem::setUnreadMessageColor( settings->unreadMessageColor() );
        Core::MessageItem::setImportantMessageColor( settings->importantMessageColor() );
        Core::MessageItem::setToDoMessageColor( settings->todoMessageColor() );
    }

    if ( MessageCore::GlobalSettings::self()->useDefaultFonts() ) {
        Core::MessageItem::setGeneralFont( KGlobalSettings::generalFont() );
        Core::MessageItem::setUnreadMessageFont( KGlobalSettings::generalFont() );
        Core::MessageItem::setImportantMessageFont( KGlobalSettings::generalFont() );
        Core::MessageItem::setToDoMessageFont( KGlobalSettings::generalFont() );
    } else {
        Core::MessageItem::setGeneralFont( settings->messageListFont() );
        Core::MessageItem::setUnreadMessageFont( settings->unreadMessageFont() );
        Core::MessageItem::setImportantMessageFont( settings->importantMessageFont() );
        Core::MessageItem::setToDoMessageFont( settings->todoMessageFont() );
    }
}

Item StorageModel::itemForRow( int row ) const
{
    return d->mModel->data( d->mModel->index( row, 0 ), EntityTreeModel::ItemRole ).value<Item>();
}

KMime::Message::Ptr StorageModel::messageForRow( int row ) const
{
    return messageForItem( itemForRow( row ) );
}

void StorageModel::resetModelStorage()
{
    reset();
}

#include "moc_storagemodel.cpp"
