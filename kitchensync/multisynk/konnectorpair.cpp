#include <kapplication.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include "konnectorpair.h"

KonnectorPair::KonnectorPair()
  : mManager( new KonnectorManager ), mConfig( 0 )
{
  mUid = KApplication::randomString( 10 );
}

KonnectorPair::~KonnectorPair()
{
  delete mManager;
  mManager = 0;

  delete mConfig;
  mConfig = 0;

  qDebug( "****************connector %s destroyed", mUid.latin1() );
}

QString KonnectorPair::uid() const
{
  return mUid;
}

void KonnectorPair::setUid( const QString &uid )
{
  mUid = uid;
}

QString KonnectorPair::name() const
{
  return mName;
}

void KonnectorPair::setName( const QString &name )
{
  mName = name;
}

int KonnectorPair::resolveStrategy() const
{
  return mStrategy;
}

void KonnectorPair::setResolveStrategy( int strategy )
{
  mStrategy = strategy;
}

void KonnectorPair::load()
{
  if ( !mConfig )
    mConfig = new KConfig( configFile() );

  mManager->readConfig( mConfig );
  mManager->connectSignals();

  mConfig->setGroup( "General" );
  mName = mConfig->readEntry( "Name" );
  mStrategy = mConfig->readNumEntry( "ResolveStrategy", ResolveManually );
}

void KonnectorPair::save()
{
  if ( !mConfig )
    mConfig = new KConfig( configFile() );

  mManager->writeConfig( mConfig );

  mConfig->setGroup( "General" );
  mConfig->writeEntry( "Name", mName );
  mConfig->writeEntry( "ResolveStrategy", mStrategy );
}

QString KonnectorPair::configFile() const
{
  return locateLocal( "config", "multisynk/konnectorpair_" + mUid );
}

KonnectorManager* KonnectorPair::manager()
{
  return mManager;
}
