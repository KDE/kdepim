
#ifndef CLIENTSIDEENTITYSTORAGE_H
#define CLIENTSIDEENTITYSTORAGE_H


#include <QStringList>

#include <akonadi/collection.h>
#include <akonadi/item.h>

#include "entityupdateadapter.h"

namespace Akonadi
{
class CollectionStatistics;
class Monitor;

class ClientSideEntityStoragePrivate;

/**
  This class wraps Akonadi::Monitor, and takes care of emitting Entities in heirarchical order.
*/
class ClientSideEntityStorage : public QObject
{
  Q_OBJECT
  public:

  enum EntityType
  {
    UnknownType,
    CollectionType,
    ItemType
  };

  /**
  What to fetch and represent in the model.
  */
  enum EntitiesToFetch {
    FetchNothing = 0,                     /// Fetch nothing. This creates an empty model.
    FetchItems = 1,                       /// Fetch items in the rootCollection
    FetchFirstLevelChildCollections = 2,  /// Fetch first level collections in the root collection.
    FetchCollectionsRecursive = 4         /// Fetch collections in the root collection recursively. This implies FetchFirstLevelChildCollections.
  };


  ClientSideEntityStorage( Monitor *monitor, EntityUpdateAdapter *entityUpdateAdapter,
              QStringList mimetypes = QStringList(),
              Collection m_rootCollection = Collection::root(),
              QObject *parent = 0,
              int entitiesToFetch = ClientSideEntityStorage::FetchCollectionsRecursive );

  virtual ~ClientSideEntityStorage();

  Collection getCollection( Collection::Id id );
  Collection rootCollection();
  Item getItem( qint64 );
  int indexOf(Collection::Id parent, Collection::Id col);
  int entityTypeForInternalIdentifier( qint64 );
  qint64 internalEntityIdentifier( Item item );
  qint64 internalEntityIdentifier( Collection col );
  int childEntitiesCount( Collection::Id id );
  int childItemsCount( Collection::Id id );
  int childCollectionsCount( Collection::Id id );
  qint64 childAt( Collection::Id id, int row );
  Collection getParentCollection( Collection );
  Collection getParentCollection( Item );

Q_SIGNALS:
  void beginInsertEntities( Collection::Id parent, int startRow, int endRow);
  void endInsertEntities();
  void beginRemoveEntities( Collection::Id parent, int startRow, int endRow);
  void endRemoveEntities();

  void collectionChanged( const Akonadi::Collection& );

  void itemChanged( const Akonadi::Item&, const QSet<QByteArray>& );

  void collectionStatisticsChanged( Akonadi::Collection::Id, const Akonadi::CollectionStatistics & );

private:
  Q_DECLARE_PRIVATE( ClientSideEntityStorage )
  //@cond PRIVATE
  ClientSideEntityStoragePrivate *d_ptr;

  Q_PRIVATE_SLOT( d_func(), void startFirstListJob() )

  Q_PRIVATE_SLOT( d_func(), void collectionsReceived( const Akonadi::Collection::List& ) )
  Q_PRIVATE_SLOT( d_func(), void itemsReceived( const Akonadi::Item::List&, Collection::Id ) )

  Q_PRIVATE_SLOT( d_func(), void monitoredCollectionAdded( const Akonadi::Collection&, const Akonadi::Collection& ) )
  Q_PRIVATE_SLOT( d_func(), void monitoredCollectionRemoved( const Akonadi::Collection& ) )
  Q_PRIVATE_SLOT( d_func(), void monitoredCollectionChanged( const Akonadi::Collection& ) )
  Q_PRIVATE_SLOT( d_func(), void monitoredCollectionMoved( const Akonadi::Collection&, const Akonadi::Collection&, const Akonadi::Collection&) )

  Q_PRIVATE_SLOT( d_func(), void monitoredItemAdded( const Akonadi::Item&, const Akonadi::Collection& ) )
  Q_PRIVATE_SLOT( d_func(), void monitoredItemRemoved( const Akonadi::Item& ) )
  Q_PRIVATE_SLOT( d_func(), void monitoredItemChanged( const Akonadi::Item&, const QSet<QByteArray>& ) )
  Q_PRIVATE_SLOT( d_func(), void monitoredItemMoved( const Akonadi::Item&,
                  const Akonadi::Collection&, const Akonadi::Collection& ) )

  Q_PRIVATE_SLOT( d_func(), void monitoredCollectionStatisticsChanged(
                    Akonadi::Collection::Id, const Akonadi::CollectionStatistics& ) )

  //@endcond

};

}

#endif

