#include "resourcedump.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <akonadi/agenttype.h>
#include <akonadi/agentinstancecreatejob.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionfetchscope.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kconfiggroup.h>

using namespace Akonadi;

ResourceDump::ResourceDump( const QDir &path, QObject *parent ) :
    AbstractDump( path, parent ), m_instance(), m_remainingJobs( 0 )
{
  m_name = path.dirName();
  m_config = KSharedConfig::openConfig( QString( "%1/%2" ).arg( path.absolutePath() ).arg( "resourceinfo" ),
                                        KSharedConfig::SimpleConfig);
}

ResourceDump::ResourceDump( const QDir &path, const Akonadi::AgentInstance &instance, QObject *parent ) :
    AbstractDump( path, parent ), m_instance( instance ), m_remainingJobs( 0 )
{
  m_name = path.dirName();
  m_config = KSharedConfig::openConfig( QString( "%1/%2" ).arg( path.absolutePath() ).arg( "resourceinfo" ),
                                        KSharedConfig::SimpleConfig);
}

ResourceDump::~ResourceDump()
{
  m_config->sync();
}

Akonadi::AgentInstance ResourceDump::instance() const
{
  return m_instance;
}

void ResourceDump::dump()
{
  // check if m_instance is a valid instance
  if ( !m_instance.isValid() ) {
    kError() << "ResourceDump::dump(): m_instance is invalid";
    return ;
  }

  // copy resource configuration file if there is one
  KStandardDirs stdDirs;
  QString configPath = stdDirs.findResource( "config", QString( "%1rc" ).arg( m_instance.identifier() ) );
  if ( !configPath.isEmpty() ) {
    QFile file( configPath );
    QString configDest = QString( "%1/%2" ).arg( path().absolutePath() ).arg( "resourcerc" );
    if ( !file.copy( configDest ) ) {
      kError() << "ResourceDump::dump(): unable to copy file " << file.fileName()
          << " to " << configDest;
      return;
    }
  }

  // write resourece type to info file
  KConfigGroup cfgGroup( m_config, "General" );
  cfgGroup.writeEntry( "type", m_instance.type().identifier() );

  // create config group for root collection
  KConfigGroup *collectionsConfig = new KConfigGroup( m_config, "Collection" );
  m_configGroups.insert( Akonadi::Collection::root().id(),
                         QSharedPointer< KConfigGroup >( collectionsConfig ) );

  // recursively dump all collections
  m_remainingJobs = 1;
  dumpSubCollections( Akonadi::Collection::root() );
}

void ResourceDump::restore()
{
  // get resource type from info file
  KConfigGroup cfgGroup( m_config, "General" );
  QString type = cfgGroup.readEntry( "type", QString() );

  // create resource
  Akonadi::AgentInstanceCreateJob *job = new Akonadi::AgentInstanceCreateJob( type, this);
  connect( job, SIGNAL( result( KJob* ) ), this, SLOT( resourceCreated( KJob* ) ) );
  job->start();
}

void ResourceDump::resourceCreated( KJob *job )
{
  if ( job->error() ) {
    kError() << "restoring " << m_name << " failed: " << job->errorText();
    return;
  }

  m_instance = static_cast< Akonadi::AgentInstanceCreateJob* >( job )->instance();
  restoreResource();
}

void ResourceDump::dumpSubCollectionsResult( KJob *job )
{
  if ( job->error() )
    kError() << "ResourceDump::fetchResult(): " << job->errorString();

  const Akonadi::CollectionFetchJob *fetchJob = static_cast<Akonadi::CollectionFetchJob*>( job );

  foreach ( const Akonadi::Collection &collection, fetchJob->collections() ) {
    ++m_remainingJobs;

    // Create config group for the collection
    Collection parentCollection = collection.parentCollection();
    if ( !m_configGroups.contains( parentCollection.id() ) )
      kError() << "ResourceDump::fetchResult(): no config for collection " << parentCollection;
    const KConfigGroup *parentConfig = m_configGroups[ parentCollection.id() ].data();
    KConfigGroup *collectionConfig = new KConfigGroup( parentConfig, QString::number( collection.id() ) );
    collectionConfig->writeEntry( "id", collection.id() );
    collectionConfig->writeEntry( "parent", parentCollection.id() );
    collectionConfig->writeEntry( "name", collection.name() );
    collectionConfig->writeEntry( "contentMimeTypes", collection.contentMimeTypes() );
    m_configGroups.insert( collection.id(), QSharedPointer< KConfigGroup >( collectionConfig ) );

    // Find all sub-collections
    dumpSubCollections( collection );
  }

  // decrease remaining jobs count and check exit condition
  if ( --m_remainingJobs == 0 )
    emit finished();
}

void ResourceDump::restoreResource()
{
  // copy resource's config file if there is one
  QFile cfgFile( QString( "%1/%2" ).arg( path().absolutePath() ).arg( "resourcerc" ) );
  if ( cfgFile.exists() ) {
    KStandardDirs dirs;
    QString configDir = dirs.saveLocation( "config" );
    QString dest = QString( "%1/%2%3" ).arg( configDir ).arg(m_instance.identifier() ).arg( "rc" );
    bool result = cfgFile.copy( dest );
    if ( !result )
      kError() << "ResourceDump::restore(): copying file failed: " << cfgFile.fileName() << " to "
          << dest;
    m_instance.reconfigure();
  }

  kError() << "restored " << m_name;

  emit finished();
}

void ResourceDump::dumpSubCollections( const Akonadi::Collection &base )
{
  Akonadi::CollectionFetchJob *fetchJob = new Akonadi::CollectionFetchJob(
      base, Akonadi::CollectionFetchJob::FirstLevel, this );
  connect( fetchJob, SIGNAL( result( KJob* ) ), this, SLOT( dumpSubCollectionsResult( KJob* ) ) );
  fetchJob->fetchScope().setResource( m_instance.identifier() );
  fetchJob->start();
}
