/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003,2004 Holger Freyther <freyther@kde.org>

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

#ifndef KSYNC_SYNC_HISTORY_H
#define KSYNC_SYNC_HISTORY_H

#include <qmap.h>

#include <addressbooksyncee.h>
#include <bookmarksyncee.h>
#include <calendarsyncee.h>
#include <kdepimmacros.h>

class KConfig;

namespace KSync {

/**
 * A small helper class to map a string (timestamp,md5sum) from and to a uid
 * and save this map on permanent storage like KConfig.
 */
class KDE_EXPORT SyncHistoryMap 
{
public:
  typedef QMap<QString, QString> Map;
  typedef QMap<QString, QString>::Iterator Iterator;
  SyncHistoryMap( const QString& fileName = QString::null );
  virtual ~SyncHistoryMap();
  
  void setFileName( const QString& fileName );
  QString fileName()const;
  
  void load();
  void save();
  
  QString  text( const QString& id_key )const;
  bool contains( const QString& id_key )const;
  void insert(   const QString& id_key, const QString& text_data );
  void set( const SyncHistoryMap::Map& );
  
  SyncHistoryMap::Map map()const;
  
  void clear();
  
protected:
  KConfig* config();
private:
  Map mMap;
  QString mFile;
  KConfig* mConf;
};

/**
 * This is the generic base class for keeping track of additions, deletions,
 * modifications between different syncs. In the purest version it relies on
 * timestamps of \sa KSync::SyncEntry::timestamp for finding the changes, additions
 * and deletions.
 * You can change the way it operates from where it reads the information, on how the
 * control string looks like, on howto safe it.
 *
 * If you want to use MD5 sum instead of a timestamp inherit from this class and implement
 * the string method.
 * If you want to use a different storage reimplement the load and save methods
 */
template <class Syn, class Ent>
class SyncHistory 
{
public:
  SyncHistory( Syn*, const QString& file );
  virtual ~SyncHistory();

  void save ( );
  void load ( );
protected:
  virtual void      save( SyncHistoryMap* );
  virtual SyncHistoryMap*  load( const QString& );
  virtual QString string( Ent * );
  
private:
  SyncHistoryMap* loadAndClear();
  SyncHistoryMap* loadInternal();
  SyncHistoryMap *mMap;
  QString mFile;
  Syn *mSyncee;
};


typedef SyncHistory<KSync::CalendarSyncee,    KSync::CalendarSyncEntry   > CalendarSyncHistory;
typedef SyncHistory<KSync::AddressBookSyncee, KSync::AddressBookSyncEntry> AddressBookSyncHistory;
typedef SyncHistory<KSync::BookmarkSyncee,    KSync::BookmarkSyncEntry   > BookmarkSyncHistory;


/**
 * \brief Construct a new SyncHistory
 *
 * Construct a new SyncHistory instance.
 * @param sy The Syncee to be used
 * @param file The path to the fileto either
          save the information or to read from
 *
 */
template <class Sync, class Ent>
SyncHistory<Sync, Ent>::SyncHistory( Sync* sy, const QString& file )
  : mMap( 0l ), mFile( file ), mSyncee( sy )
{}


/**
 * \brief d'tor
 */
template <class Sync, class Ent>
SyncHistory<Sync, Ent>::~SyncHistory() {
  delete mMap;
}


/**
 * \brief Safe the Metainformation
 * Saves the MetaInformation  of the supplied Syncee
 * to the file specified.
 */
template <class Sync, class Ent>
void SyncHistory<Sync, Ent>::save() 
{
  mMap = loadAndClear();
  
  /* update the state */
  for ( Ent* entry = (Ent*)mSyncee->firstEntry();
        entry != 0; entry = (Ent*)mSyncee->nextEntry() ) {
    
    /* only save meta for not deleted SyncEntries! */
    if ( entry->state() != SyncEntry::Removed )
      mMap->insert( entry->id(), string( entry ) );
    
  }
  
  save( mMap );
}

/**
 * \brief Load and apply MetaInformation
 *
 * Fill the supplied Syncee with the MetaInformation
 * from the file specified.
 */
template <class Sync, class Ent>
void SyncHistory<Sync, Ent>::load() 
{
  mMap = loadInternal();
  
  bool found;
  Ent* entryNew;
  
  /*
   * Now we'll search for some meta info
   * go through all entries
   * check if they exist
   * if exist check if modified
   * else it was added
   */
  for ( entryNew = static_cast<Ent*>(mSyncee->firstEntry());
        entryNew != 0;
        entryNew = static_cast<Ent*>(mSyncee->nextEntry()) ) {
    found = false;
    
    /*
     * check if the Map contains the UID
     * if the string sums are not equal
     * set the modified state
     * ADDED set Added state
     */
    if ( mMap->contains( entryNew->id() ) ) {
      found = true;
      QString str = mMap->text( entryNew->id() );
      QString newStr = string( entryNew );
      
      if ( str != newStr)
        entryNew->setState( SyncEntry::Modified );
      
    }
    if (!found )
      entryNew->setState( SyncEntry::Added );
    
  }
  
  /*
   * Now find the deleted records
   */
  SyncHistoryMap::Map::Iterator it;
  SyncHistoryMap::Map ma = mMap->map();
  for ( it = ma.begin(); it != ma.end(); ++it ) {
    entryNew = static_cast<Ent*>( mSyncee->findEntry( it.key() ) );
    
    /**
     * if we've a UID
     * but we can not find it
     * in the Syncee
     * it was removed
     */
    if (!entryNew) {
      entryNew = new Ent(mSyncee);
      entryNew->setId( it.key() );
      
      kdDebug() << "FOUND deleted record of type " << entryNew->type() << " and ids are " << it.key() << " and " << entryNew->id() << endl;
            /* add entry first and then to setState */
      entryNew->setState( KSync::SyncEntry::Removed );
      mSyncee->addEntry( entryNew );
    }
  }
}

/**
 * @internal
 * responsible for saving the SyncHistoryMap to a KConfig file
 */
template <class Sync, class Ent>
void SyncHistory<Sync, Ent>::save( SyncHistoryMap* m) 
{
  m->save();
}

/**
 * @internal
 *
 * create and load the SyncHistoryMap
 */
template <class Sync, class Ent>
SyncHistoryMap* SyncHistory<Sync, Ent>::load( const QString& f) 
{
  SyncHistoryMap *map  = new SyncHistoryMap( f );
  
  map->load();
  return map;
}


/**
 * @internal
 * Reimplement this if you for example want ot use MD5 sums.
 * This implementation uses the timestamp
 */
template <class Sync, class Ent>
QString SyncHistory<Sync, Ent>::string( Ent* ent) 
{
  return ent->timestamp();
}


/**
 * @internal
 */
template <class Sync, class Ent>
SyncHistoryMap* SyncHistory<Sync, Ent>::loadAndClear() 
{
  if(!mMap )
    mMap = load( mFile );
  mMap->clear();
  
  return mMap;
}

/**
 * @internal
 */
template <class Sync, class Ent>
SyncHistoryMap* SyncHistory<Sync, Ent>::loadInternal() 
{
  if(!mMap )
    mMap = load( mFile );
  
  return mMap;
}

}
#endif
