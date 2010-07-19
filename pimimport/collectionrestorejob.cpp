#include "collectionrestorejob.h"

#include "kconfig.h"
#include "kconfiggroup.h"
#include <akonadi/collectioncreatejob.h>

using namespace Akonadi;

CollectionRestoreJob::CollectionRestoreJob(const Akonadi::Collection &parentCollection, const QDir &path, QObject *parent) :
    KJob( parent ), m_parent( parentCollection ), m_path( path ), m_subcollectionsNo( 0 )
{
}

void CollectionRestoreJob::start()
{
  // check config file
  QDir configFile( m_path.absoluteFilePath( "collectioninfo" ) );
  if ( !configFile.exists() ) {
    setError( 1 );
    setErrorText( QString( "File %1 does not exist" ).arg( configFile.absolutePath() ) );
  }

  // read config file
  KConfig config( configFile.absolutePath() );
  KConfigGroup configGroup( &config, "General" );
  Collection collection;
  collection.setName( configGroup.readEntry( "name", QString() ) );
  collection.setContentMimeTypes( configGroup.readEntry( "contentMimeTypes", QStringList() ) );
  collection.setParentCollection( m_parent );

  // restore collection
  CollectionCreateJob *job = new CollectionCreateJob( collection, this );
  connect( job, SIGNAL( result( KJob* ) ), this, SLOT( createResult( KJob* ) ) );
  job->start();
}

void CollectionRestoreJob::createResult( KJob *job )
{
  if ( job->error() ) {
    setError( job->error() );
    setErrorText( job->errorText() );
    emitResult();
  }

  // restore subcollections or emit result if there are none
  const CollectionCreateJob *createJob = static_cast< CollectionCreateJob* >( job );
  Collection collection = createJob->collection();
  QStringList subcollections = m_path.entryList( QDir::Dirs | QDir::NoDotAndDotDot );
  m_subcollectionsNo = subcollections.size();
  if ( m_subcollectionsNo > 0 ) {
    foreach ( const QString &subcollection, subcollections ) {
      CollectionRestoreJob *restoreJob = new CollectionRestoreJob( collection,
                                                                   m_path.absoluteFilePath( subcollection ),
                                                                   this );
      connect( restoreJob, SIGNAL( result( KJob* ) ), this, SLOT( restoreResult( KJob* ) ) );
      restoreJob->start();
    }
  }
  else {
    emitResult();
  }
}

void CollectionRestoreJob::restoreResult( KJob *job )
{
  if ( job->error() ) {
    setError( job->error() );
    setErrorText( job->errorText() );
    emitResult();
  }

  // check exit condition
  if ( --m_subcollectionsNo == 0 )
    emitResult();
}
