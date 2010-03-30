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

#include <messagecore/stringutil.h>

#include <akonadi/attributefactory.h>
#include <akonadi/collection.h>
#include <akonadi/collectionstatistics.h>
#include <akonadi/entitymimetypefiltermodel.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/item.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/kmime/messagefolderattribute.h>
#include <akonadi/selectionproxymodel.h>

#include <KDE/KCodecs>
#include <KDE/KLocale>
#include <Nepomuk/ResourceManager>
#include <Soprano/Statement>
#include <soprano/signalcachemodel.h>
#include <soprano/nao.h>

#include "core/messageitem.h"
#include "core/settings.h"
#include "core/subjectutils_p.h"

#include <QtCore/QAbstractItemModel>
#include <QtCore/QAtomicInt>
#include <QtCore/QScopedPointer>
#include <QtGui/QItemSelectionModel>

namespace MessageList
{

class StorageModel::Private
{
public:
  void onSourceDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight );
  void onSelectionChanged();
  void loadSettings();
  void statementChanged( const Soprano::Statement &statement );

  StorageModel * const q;

  QAbstractItemModel *mModel;
  QItemSelectionModel *mSelectionModel;

  QScopedPointer<Soprano::Util::SignalCacheModel> mSopranoModel;

  Private( StorageModel *owner )
    : q( owner ),
      mSopranoModel( new Soprano::Util::SignalCacheModel( Nepomuk::ResourceManager::instance()->mainModel() ) )
  {}
};

} // namespace MessageList

using namespace Akonadi;
using namespace MessageList;

static QAtomicInt _k_attributeInitialized;

StorageModel::StorageModel( QAbstractItemModel *model, QItemSelectionModel *selectionModel, QObject *parent )
  : Core::StorageModel( parent ), d( new Private( this ) )
{
  d->mModel = 0;
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
  itemFilter->addMimeTypeInclusionFilter( "message/rfc822" );
  itemFilter->setHeaderGroup( EntityTreeModel::ItemListHeaders );

  d->mModel = itemFilter;

  connect( d->mSopranoModel.data(), SIGNAL(statementAdded(Soprano::Statement)),
           SLOT(statementChanged(Soprano::Statement)) );
  connect( d->mSopranoModel.data(), SIGNAL(statementRemoved(Soprano::Statement)),
           SLOT(statementChanged(Soprano::Statement)) );

  connect( d->mModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
           this, SLOT(onSourceDataChanged(QModelIndex, QModelIndex)) );

  connect( d->mModel, SIGNAL(layoutAboutToBeChanged()),
           this, SIGNAL(layoutAboutToBeChanged()) );
  connect( d->mModel, SIGNAL(layoutChanged()),
           this, SIGNAL(layoutChanged()) );
  connect( d->mModel, SIGNAL(modelAboutToBeReset()),
           this, SIGNAL(modelAboutToBeReset()) );
  connect( d->mModel, SIGNAL(modelReset()),
           this, SIGNAL(modelReset()) );

  //Here we assume we'll always get QModelIndex() in the parameters
  connect( d->mModel, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)),
           this, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)) );
  connect( d->mModel, SIGNAL(rowsInserted(QModelIndex, int, int)),
           this, SIGNAL(rowsInserted(QModelIndex, int, int)) );
  connect( d->mModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)),
           this, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)) );
  connect( d->mModel, SIGNAL(rowsRemoved(QModelIndex, int, int)),
           this, SIGNAL(rowsRemoved(QModelIndex, int, int)) );

  connect( d->mSelectionModel, SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
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
  return ids.join(":");
}

bool StorageModel::containsOutboundMessages() const
{
  QModelIndexList indexes = d->mSelectionModel->selectedRows();

  foreach ( const QModelIndex &index, indexes ) {
    Collection c = index.data( EntityTreeModel::CollectionRole ).value<Collection>();
    if ( c.isValid()
      && c.hasAttribute<MessageFolderAttribute>()
      && c.attribute<MessageFolderAttribute>()->isOutboundFolder() ) {
      return true;
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
  Item item = itemForRow( row );
  const KMime::Message::Ptr mail = messageForRow( row );
  if ( !mail ) return false;

  QString sender = MessageCore::StringUtil::stripEmailAddr( mail->from()->asUnicodeString() );
  QString receiver = MessageCore::StringUtil::stripEmailAddr( mail->to()->asUnicodeString() );

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
                    bUseReceiver ? receiver : sender );

  mi->setUniqueId( item.id() );

  // Storing the Nepomuk resource URI could be avoided, since we already store the item ID, but that
  // would mean that MessageItem needs to depend on Akonadi, which we don't want (yet?)
  mi->setNepomukResourceURI( item.url() );

  QString subject = mail->subject()->asUnicodeString();
  if ( subject.isEmpty() ) {
    subject = '(' + noSubject + ')';
  }

  mi->setSubject( subject );

  updateMessageItemData( mi, row );

  return true;
}

static QString md5Encode( const QString &str )
{
  if ( str.trimmed().isEmpty() ) return QString();

  KMD5 md5( str.trimmed().toUtf8() );
  static const int Base64EncodedMD5Len = 22;
  return md5.base64Digest().left( Base64EncodedMD5Len );
}

void StorageModel::fillMessageItemThreadingData( MessageList::Core::MessageItem *mi,
                                                 int row, ThreadingDataSubset subset ) const
{
  const KMime::Message::Ptr mail = messageForRow( row );
  Q_ASSERT( mail ); // We ASSUME that initializeMessageItem has been called successfully...

  switch ( subset ) {
  case PerfectThreadingReferencesAndSubject:
    mi->setStrippedSubjectMD5( md5Encode( Core::SubjectUtils::stripOffPrefixes( mail->subject()->asUnicodeString() ) ) );
    mi->setSubjectIsPrefixed( mail->subject()->asUnicodeString() != Core::SubjectUtils::stripOffPrefixes( mail->subject()->asUnicodeString() ) );
    // fall through
  case PerfectThreadingPlusReferences:
    if ( !mail->references()->identifiers().isEmpty() ) {
      mi->setReferencesIdMD5( md5Encode( mail->references()->identifiers().first() ) );
    }
    // fall through
  case PerfectThreadingOnly:
    mi->setMessageIdMD5( md5Encode( mail->messageID()->asUnicodeString() ) );
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
  Item item = itemForRow( row );
  const KMime::Message::Ptr mail = messageForRow( row );
  Q_ASSERT( mail );

  KPIM::MessageStatus stat;
  stat.setStatusFromFlags( item.flags() );

  mi->setStatus( stat );

  mi->setEncryptionState( Core::MessageItem::EncryptionStateUnknown );
  if ( mail->contentType()->isSubtype( "encrypted" )
    || mail->contentType()->isSubtype( "pgp-encrypted" )
    || mail->contentType()->isSubtype( "pkcs7-mime" ) ) {
      mi->setEncryptionState( Core::MessageItem::FullyEncrypted );
  } else if ( mail->mainBodyPart( "multipart/encrypted" )
           || mail->mainBodyPart( "application/pgp-encrypted" )
           || mail->mainBodyPart( "application/pkcs7-mime" ) ) {
    mi->setEncryptionState( Core::MessageItem::PartiallyEncrypted );
  }

  mi->setSignatureState( Core::MessageItem::SignatureStateUnknown );
  if ( mail->contentType()->isSubtype( "signed" )
    || mail->contentType()->isSubtype( "pgp-signature" )
    || mail->contentType()->isSubtype( "pkcs7-signature" )
    || mail->contentType()->isSubtype( "x-pkcs7-signature" ) ) {
      mi->setSignatureState( Core::MessageItem::FullySigned );
  } else if ( mail->mainBodyPart( "multipart/signed" )
           || mail->mainBodyPart( "application/pgp-signature" )
           || mail->mainBodyPart( "application/pkcs7-signature" )
           || mail->mainBodyPart( "application/x-pkcs7-signature" ) ) {
    mi->setSignatureState( Core::MessageItem::PartiallySigned );
  }

  mi->invalidateTagCache();
}

void StorageModel::setMessageItemStatus( MessageList::Core::MessageItem *mi,
                                         int row, const KPIM::MessageStatus &status )
{
  Item item = itemForRow( row );
  item.setFlags( status.getStatusFlags() );
  new ItemModifyJob( item, this );
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
  if ( !parent.isValid() )
    return createIndex( row, column, 0 );

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

void StorageModel::prepareForScan()
{

}

void StorageModel::Private::statementChanged( const Soprano::Statement &statement )
{
  if ( statement.predicate() == Soprano::Vocabulary::NAO::hasTag() ) {
    const Akonadi::Item item = Item::fromUrl( statement.subject().uri() );
    if ( !item.isValid() ) {
      return;
    }

    const QModelIndexList list = mModel->match( QModelIndex(), EntityTreeModel::ItemIdRole, item.id() );
    if ( list.isEmpty() ) {
      return;
    }
    emit q->dataChanged( q->index( list.first().row(), 0 ),
                         q->index( list.first().row(), 0 ) );
  }
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

  if ( settings->useDefaultColors() ) {
    Core::MessageItem::setNewMessageColor( QColor( "red" ) );
    Core::MessageItem::setUnreadMessageColor( QColor( "blue" ) );
    Core::MessageItem::setImportantMessageColor( QColor( 0x0, 0x7F, 0x0 ) );
    Core::MessageItem::setToDoMessageColor( QColor( 0x0, 0x98, 0x0 ) );
  } else {
    Core::MessageItem::setNewMessageColor( settings->newMessageColor() );
    Core::MessageItem::setUnreadMessageColor( settings->unreadMessageColor() );
    Core::MessageItem::setImportantMessageColor( settings->importantMessageColor() );
    Core::MessageItem::setToDoMessageColor( settings->todoMessageColor() );
  }

  if ( settings->useDefaultFonts() ) {
    Core::MessageItem::setGeneralFont( KGlobalSettings::generalFont() );
    Core::MessageItem::setNewMessageFont( KGlobalSettings::generalFont() );
    Core::MessageItem::setUnreadMessageFont( KGlobalSettings::generalFont() );
    Core::MessageItem::setImportantMessageFont( KGlobalSettings::generalFont() );
    Core::MessageItem::setToDoMessageFont( KGlobalSettings::generalFont() );
  } else {
    Core::MessageItem::setGeneralFont( settings->messageListFont() );
    Core::MessageItem::setNewMessageFont( settings->newMessageFont() );
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
  Item item = itemForRow( row );

  if ( !item.hasPayload<KMime::Message::Ptr>() ) {
    kWarning() << "Not a message" << item.id() << item.remoteId() << item.mimeType();
    return KMime::Message::Ptr();
  }

  return item.payload<KMime::Message::Ptr>();
}

#include "storagemodel.moc"
