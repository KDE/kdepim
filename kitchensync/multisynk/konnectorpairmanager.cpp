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

#include <kstandarddirs.h>

#include "konnectorpairmanager.h"


KonnectorPairManager::KonnectorPairManager( QObject *parent )
  : QObject( parent, "KonnectorPairManager" )
{
}

KonnectorPairManager::~KonnectorPairManager()
{
  KonnectorPair::Map::Iterator it;
  for ( it = mPairs.begin(); it != mPairs.end(); ++it )
    delete it.data();

  mPairs.clear();
}

void KonnectorPairManager::load()
{
  qDebug( "do loading" );

  KConfig config( configFile() );

  config.setGroup( "General" );
  QStringList pairUids = config.readListEntry( "PairUids" );

  QStringList::ConstIterator it;
  for ( it = pairUids.begin(); it != pairUids.end(); ++it ) {
    KonnectorPair *pair = new KonnectorPair;
    pair->setUid( *it );
    pair->load();

    qDebug( "loaded %s", pair->uid().latin1() );

    mPairs.insert( pair->uid(), pair );
  }

  emit changed();
}

void KonnectorPairManager::save()
{
  KConfig config( configFile() );

  config.setGroup( "General" );
  config.writeEntry( "PairUids", mPairs.keys() );

  KonnectorPair::Map::Iterator it;
  for ( it = mPairs.begin(); it != mPairs.end(); ++it )
    it.data()->save();
}

void KonnectorPairManager::add( KonnectorPair *pair )
{
  mPairs.insert( pair->uid(), pair );

  emit changed();
}

void KonnectorPairManager::change( KonnectorPair *pair )
{
  mPairs.insert( pair->uid(), pair );

  emit changed();
}

void KonnectorPairManager::remove( const QString &uid )
{
  delete mPairs[ uid ];
  mPairs.remove( uid );

  emit changed();
}

KonnectorPair* KonnectorPairManager::pair( const QString &uid ) const
{
  return mPairs[ uid ];
}

KonnectorPair::List KonnectorPairManager::pairs() const
{
  return mPairs.values();
}

QString KonnectorPairManager::configFile() const
{
  return locateLocal( "config", "multisynk_konnectorpairsrc" );
}

#include "konnectorpairmanager.moc"
