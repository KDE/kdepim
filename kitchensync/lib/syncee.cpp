/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

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

#include <qregexp.h>

#include <kdebug.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include "syncee.h"

using namespace KSync;

Syncee::Syncee( uint size )
  : mStatusLog( 0 ), mSupport( size )
{
    mSyncMode = MetaLess;
    mFirstSync = false;
    mSupport.fill( true );
    kdDebug(5230) << "Size is " << size << " " << mSupport.size() << endl;
}

Syncee::~Syncee()
{
  delete mStatusLog;
}

void Syncee::setFilename( const QString &filename )
{
  mFilename = filename;
}

QString Syncee::filename()
{
  return mFilename;
}

SyncEntry *Syncee::findEntry( const QString &id )
{
  kdDebug(5231) << "Syncee::findEntry() '" << id << "'" << endl;

  SyncEntry *entry = firstEntry();
  while (entry) {
    if (entry->id() == id) return entry;
    entry = nextEntry();
  }

  return 0;
}

void Syncee::replaceEntry( SyncEntry *oldEntry, SyncEntry *newEntry )
{
  removeEntry(oldEntry);
  addEntry(newEntry);
}

bool Syncee::hasChanged( SyncEntry *entry )
{
  if ( entry->state() != SyncEntry::Undefined ) return true;
  if ( entry->timestamp().isEmpty() ) return false; // sure -zecke

  if (!mStatusLog ) return false;
  mStatusLog->setGroup(entry->id());
  QString timestamp = mStatusLog->readEntry("Timestamp");

  return (timestamp != entry->timestamp());
}

bool Syncee::loadLog()
{
  delete mStatusLog;
  mStatusLog = new KSimpleConfig( locateLocal( "appdata", statusLogName() ) );

  return true;
}

bool Syncee::saveLog()
{
  writeLog();
  return true;
}

void Syncee::writeLog()
{
  if ( !mStatusLog ) return;
  for ( SyncEntry *entry = firstEntry(); entry; entry = nextEntry() ) {
    mStatusLog->setGroup( entry->id() );
    mStatusLog->writeEntry( "Name",entry->name() );
    mStatusLog->writeEntry( "Timestamp",entry->timestamp() );
  }

  mStatusLog->sync();
}

QString Syncee::statusLogName()
{
  QString name = filename();

  name.replace(QRegExp("/"),"_");
  name.replace(QRegExp(":"),"_");

  name += ".syncee";

  return name;
}

int Syncee::modificationState( SyncEntry* entry ) const
{
    return entry->state();
}

int Syncee::syncMode() const
{
    return mSyncMode;
}

void Syncee::setSyncMode( int mode )
{
    mSyncMode = mode;
}

bool Syncee::firstSync() const
{
    return mFirstSync;
}

void Syncee::setFirstSync( bool first )
{
    mFirstSync = first;
}

void Syncee::insertId( const QString &type,
                       const QString &konnectorId,
                       const QString &kdeId )
{
    QMap<QString,  Kontainer::ValueList>::Iterator it;
    it = mMaps.find( type );
    if ( it == mMaps.end() ) { // not inserted yet anything
        Kontainer::ValueList list;
        list.append( Kontainer(konnectorId,  kdeId) );
        mMaps.replace( type, list);
    } else {
        it.data().append(Kontainer( konnectorId,  kdeId) );
    }
}

Kontainer::ValueList Syncee::ids( const QString &type ) const
{
    Kontainer::ValueList id;
    QMap<QString,  Kontainer::ValueList >::ConstIterator it;
    it = mMaps.find( type );
    if ( it != mMaps.end() )
        id = it.data();
    return id;
}

QMap<QString, Kontainer::ValueList> Syncee::ids() const
{
    return mMaps;
}

bool Syncee::trustIdsOnFirstSync() const
{
    return false;
}

QString Syncee::newId() const
{
    return QString::null;
}

void Syncee::setSupports( const QBitArray& ar )
{
    mSupport = ar;
    mSupport.detach();
    kdDebug(5230) << "setSupports count is " << ar.size() << endl;
}

QBitArray Syncee::bitArray() const
{
    return mSupport;
}

bool Syncee::isSupported( uint attr ) const
{
    if ( attr >= mSupport.size() )
        return false;
    return mSupport.testBit( attr );
}

void Syncee::setSource( const QString& str )
{
    mName = str;
}

QString Syncee::source() const
{
    return mName;
}
