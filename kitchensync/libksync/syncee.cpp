/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002,2004 Holger Hans Peter Freyther <freyther@kde.org>

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

#include "syncee.h"
#include "merger.h"

#include <kdebug.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include <qregexp.h>




using namespace KSync;

Syncee::Syncee( Merger* merger )
  : mMerger( merger )
{}

Syncee::~Syncee()
{
}

QString Syncee::identifier()const {
  return mIdentifier;
}

void Syncee::setIdentifier( const QString &identifier )
{
  mIdentifier = identifier;
}

bool Syncee::isValid()
{
  return !identifier().isEmpty();
}

SyncEntry *Syncee::findEntry( const QString &id )
{

  SyncEntry *entry = firstEntry();
  while ( entry ) {
    if ( entry->id() == id ) return entry;
    entry = nextEntry();
  }

  return 0;
}

void Syncee::replaceEntry( SyncEntry *oldEntry, SyncEntry *newEntry )
{
  removeEntry( oldEntry );
  addEntry( newEntry );

  /* no risk no fun */
  delete oldEntry;
}


int Syncee::modificationState( SyncEntry *entry ) const
{
  return entry->state();
}

SyncEntry::PtrList Syncee::added() {
  return find( SyncEntry::Added );
}

SyncEntry::PtrList Syncee::modified() {
  return find( SyncEntry::Modified );
}

SyncEntry::PtrList Syncee::removed() {
  return find( SyncEntry::Removed );
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
  if ( it != mMaps.end() ) id = it.data();
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

QString Syncee::generateNewId() const
{
  return QString::null;
}

Merger* Syncee::merger()const
{
  return mMerger;
}

void Syncee::setMerger( Merger *merger )
{
  mMerger = merger;
}

void Syncee::setTitle( const QString& str )
{
  mTitle = str;
}

QString Syncee::title() const
{
  return mTitle;
}


void Syncee::setType(const QString& type )
{
  mType = type;
}

QString Syncee::type()const
{
  return mType;
}

SyncEntry::PtrList Syncee::find( int state )
{
  QPtrList<SyncEntry> found;
  SyncEntry* entry;
  for ( entry = firstEntry(); entry != 0; entry = nextEntry() )
    if ( entry->state() == state )
      found.append( entry );


  return found;
}



