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

#include "synchistory.h"

#include <kconfig.h>

namespace KSync {

/**
 * Construct an empty MetaMap. You need to call
 * load to retrive the information from the permanent
 * storage.
 *
 * \code
 * SyncHistoryMap *map =  new SyncHistoryMap(metaData);
 * map->load();
 * \endcode
 *
 * @param file The file where the information is stored
 */
SyncHistoryMap::SyncHistoryMap( const QString& file )
  : mFile( file ), mConf( 0l )
{}


/**
 * Destructor, cleans up and deletes the internal
 * Config object
 */
SyncHistoryMap::~SyncHistoryMap()
{
  delete mConf;
}

/**
 *
 * @return the FileName that was supplied on instantiation
 */
QString SyncHistoryMap::fileName()const
{
  return mFile;
}

/**
 * \brief load the information
 *
 * This method possible creates a KConfig object
 * if mFile is a valid path and tries to load
 * the saved MetaData and fills the internal map.
 */
void SyncHistoryMap::load()
{
  if ( mFile.isEmpty() )
    return;

  KConfig* conf = config();
  QStringList list = conf->groupList();

  for (QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
    conf->setGroup( (*it) );
    insert( (*it), conf->readEntry("sum") );
  }
}


/**
 * Saves the current Map to a permanent storage and
 * wipes out any previous data.
 * Calling without a prior call to load() works as well
 * as the Config object is created on demand
 */
void SyncHistoryMap::save()
{
  KConfig* conf = config();

  QStringList groups = conf->groupList();
  for (QStringList::Iterator it = groups.begin(); it != groups.end(); ++it )
    conf->deleteGroup( (*it) );


  Iterator it;
  for ( it = mMap.begin(); it != mMap.end(); ++it ) {
    conf->setGroup( it.key() );
    conf->writeEntry( "sum", it.data() );
  }
  conf->sync();
}

/**
 * @param id_key The ID to retrieve the text for
 * @return Returns the to the id_key associated text or
 *         an empty string in case of the id_key is
 *         not contained.
 */
QString SyncHistoryMap::text( const QString& id_key )const
{
  return mMap[id_key];
}

/**
 * @return True if the @param id_key is inside the internal map
 */
bool SyncHistoryMap::contains( const QString& id_key )const
{
  return mMap.contains( id_key );
}

/**
 * If the id_key already existed before the old incarnation
 * is shadowed.
 *
 * @param id_key the Key for the inernal map
 * @param text_data The text associated with the id_key
 */
void SyncHistoryMap::insert( const QString& id_key, const QString& text_data )
{
  mMap.insert( id_key, text_data );
}

/**
 * replace the internal map with the parameter
 *
 * @param map Replace the internal map with this one
 */
void SyncHistoryMap::set( const SyncHistoryMap::Map& map )
{
  mMap = map;
}


/**
 * @return Returns the internal used map
 */
SyncHistoryMap::Map SyncHistoryMap::map()const
{
  return mMap;
}

/**
 * Clears the permanent storage and also
 * the internal map
 */
void SyncHistoryMap::clear()
{
  mMap.clear();
  KConfig* conf = config();
  QStringList groups = conf->groupList();
  for (QStringList::Iterator it = groups.begin(); it != groups.end(); ++it ) {
    conf->deleteGroup( (*it) );
  }
}


/**
 * creates the kconfig object on demand
 */
KConfig* SyncHistoryMap::config()
{
  if (!mConf )
    mConf = new KConfig( mFile, false, false );

  return mConf;
}


}
