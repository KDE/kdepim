/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

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
