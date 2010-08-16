#include "collectionrestorejob.h"

#include "kconfig.h"
#include "kconfiggroup.h"
#include <akonadi/cachepolicy.h>
#include <akonadi/collectioncreatejob.h>
#include <akonadi/item.h>
#include <akonadi/itemcreatejob.h>

using namespace Akonadi;

CollectionRestoreJob::CollectionRestoreJob(const Akonadi::Collection &parentCollection, const QDir &path, QObject *parent) :
    KJob( parent ), m_parent( parentCollection ), m_path( path ), m_session( 0 ), m_subjobsNo( 1 )
{
}

CollectionRestoreJob::CollectionRestoreJob(const Akonadi::Collection &parentCollection, const QDir &path, Session *session) :
    KJob( session ), m_parent( parentCollection ), m_path( path ), m_session( session ), m_subjobsNo( 1 )
{
}

void CollectionRestoreJob::start()
{
  // check config file
  if ( !m_path.exists( "collectioninfo" ) ) {
    setError( 1 );
    setErrorText( QString( "File %1 does not exist" ).arg( m_path.absoluteFilePath( "collectioninfo" ) ) );
    emitResult();
    return;
  }

  // read config file
  KConfig config( m_path.absoluteFilePath( "collectioninfo" ) );
  KConfigGroup configGroup( &config, "General" );
  Collection collection;
  collection.setName( configGroup.readEntry( "name", QString() ) );
  collection.setParentCollection( m_parent );

  // restore cache policy
  KConfigGroup cacheGroup( &configGroup, "CachePolicy" );
  CachePolicy policy;
  policy.setInheritFromParent( cacheGroup.readEntry( "inheritFromParent", true ) );
  policy.setCacheTimeout( cacheGroup.readEntry( "cacheTimeout", policy.cacheTimeout() ) );
  policy.setIntervalCheckTime( cacheGroup.readEntry( "intervalCheckTime", policy.intervalCheckTime() ) );
  policy.setLocalParts( cacheGroup.readEntry( "localParts", policy.localParts() ) );
  policy.setSyncOnDemand( cacheGroup.readEntry( "syncOnDemand", policy.syncOnDemand() ) );

  // restore collection
  CollectionCreateJob *job;
  if ( m_session )
    job = new CollectionCreateJob( collection, m_session);
  else
    job = new CollectionCreateJob( collection, this);

  connect( job, SIGNAL( result( KJob* ) ), this, SLOT( createResult( KJob* ) ) );
  job->start();
}

void CollectionRestoreJob::createResult( KJob *job )
{
  if ( job->error() ) {
    setError( job->error() );
    setErrorText( job->errorText() );
    kError() << job->errorString();
    emitResult();
    return;
  }

  const CollectionCreateJob *createJob = static_cast< CollectionCreateJob* >( job );
  m_collection = createJob->collection();

  restoreItems();

  // restore subcollections or emit result if there are none
  Collection collection = createJob->collection();
  QStringList subcollections = m_path.entryList( QDir::Dirs | QDir::NoDotAndDotDot );
  m_subjobsNo += subcollections.size();
  foreach ( const QString &subcollection, subcollections ) {
    CollectionRestoreJob *restoreJob = new CollectionRestoreJob( collection,
                                                                 m_path.absoluteFilePath( subcollection ),
                                                                 this );
    connect( restoreJob, SIGNAL( result( KJob* ) ), this, SLOT( checkJobResult( KJob* ) ) );
    restoreJob->start();
  }

  decreaseSubjobs();
}

void CollectionRestoreJob::checkJobResult( KJob *job )
{
  if ( job->error() ) {
    setError( job->error() );
    setErrorText( job->errorText() );
    kError() << job->errorString();
    emitResult();
    return;
  }

  decreaseSubjobs();
}

void CollectionRestoreJob::restoreItems()
{
  KConfig config( m_path.absoluteFilePath( "collectioninfo" ), KConfig::SimpleConfig );
  KConfigGroup itemsGroup( &config, "Items" );
  m_subjobsNo += itemsGroup.groupList().size();
  foreach ( const QString &id, itemsGroup.groupList() ) {
    KConfigGroup itemGroup( &itemsGroup, id );
    Item item;
    QList< Item::Flag > flags = itemGroup.readEntry( "flags", QList< Item::Flag >() );
    item.setFlags( Item::Flags::fromList( flags ) );
    item.setMimeType( itemGroup.readEntry( "mimeType", QString() ) );
    item.setModificationTime( itemGroup.readEntry( "modificationTime", QDateTime() ) );
    item.setPayloadFromData( itemGroup.readEntry( "payloadData", QByteArray() ) );

    ItemCreateJob *job = new ItemCreateJob( item, m_collection, this );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( checkJobResult( KJob* ) ) );
    job->start();
  }
}

void CollectionRestoreJob::decreaseSubjobs()
{
  // decrease remaining subjobs
  if ( --m_subjobsNo == 0 )
    emitResult();
}
