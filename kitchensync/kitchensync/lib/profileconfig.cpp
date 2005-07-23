/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "profileconfig.h"

#include <kconfig.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>

using namespace KSync;

ProfileConfig::ProfileConfig()
{
  QString configPath = locateLocal( "appdata", "profiles" );
  
  mConfig = new KConfig( configPath );
}

ProfileConfig::~ProfileConfig()
{
  delete mConfig;
}

/*
 * saves Profils
 */
void ProfileConfig::save( const QValueList<Profile> &profiles )
{
  kdDebug() << "ProfileConfig::save()" << endl;

  clear( mConfig );

  QStringList ids;
  Profile::List::ConstIterator it;
  for ( it = profiles.begin(); it != profiles.end(); ++it ) {
    ids << (*it).uid();
    saveProfile( mConfig, (*it) );
  }
  mConfig->setGroup( "General" );
  mConfig->writeEntry( "Keys", ids );
  
  mConfig->sync();
}

/*
 * loads one from file
 */
Profile::List ProfileConfig::load()
{
  mConfig->setGroup( "General" );
  QStringList keys = mConfig->readListEntry( "Keys" );

  Profile::List profiles;
  QStringList::Iterator it;
  for ( it = keys.begin(); it != keys.end(); ++it ) {
    mConfig->setGroup( *it );
    profiles.append( readProfile( mConfig ) );
  }

  if ( profiles.isEmpty() ) profiles = defaultProfiles();

  return profiles;
}

Profile::List ProfileConfig::defaultProfiles()
{
  Profile::List profiles;

  Profile profSyncing;
  profSyncing.setName( i18n("Syncing") );
  ActionPartService::List partsSyncing;
  addPart( "overview", partsSyncing );
  addPart( "backup", partsSyncing );
  addPart( "syncerpart", partsSyncing );
  profSyncing.setActionParts( partsSyncing );
  profiles.append( profSyncing );

  Profile profRestore;
  profRestore.setName( i18n("Restore Backup") );
  ActionPartService::List partsRestore;
  addPart( "backup", partsRestore );
  profRestore.setActionParts( partsRestore );
  profiles.append( profRestore );
    
  return profiles;
}

void ProfileConfig::addPart( const QString &id, ActionPartService::List &parts )
{
  ActionPartService overview = ActionPartService::partForId( id );
  if ( !overview.id().isEmpty() ) {
    parts.append( overview );
  }
}

void ProfileConfig::clear( KConfig *conf )
{
  QStringList list = conf->groupList();
  QStringList::Iterator it;
  for ( it = list.begin(); it != list.end(); ++it ) {
    conf->deleteGroup( *it );
  }
}

void ProfileConfig::saveProfile( KConfig *conf, const Profile &prof )
{
  conf->setGroup( prof.uid() );
  conf->writeEntry( "Name", prof.name() );
  conf->writeEntry( "Pixmap", prof.pixmap() );
  conf->writeEntry( "ConfirmDelete", prof.confirmDelete() );
  conf->writeEntry( "ConfirmSync", prof.confirmSync() );

  QMap<QString,QString> paths = prof.paths();
  QMap<QString,QString>::Iterator pathIt;
  QStringList pathlist;
  for ( pathIt = paths.begin(); pathIt != paths.end(); ++pathIt ) {
    pathlist << pathIt.key();
    conf->writePathEntry( "Path" + pathIt.key(), pathIt.data() );
  }
  conf->writeEntry("LocationPath", pathlist );

  ActionPartService::List parts = prof.actionParts();
  QStringList ids;
  ActionPartService::List::ConstIterator it;
  for( it = parts.begin(); it != parts.end(); ++it ) {
    ids.append( (*it).id() );
  }
  conf->writeEntry( "ActionParts", ids );
}

void ProfileConfig::saveActionPart( KConfig *conf,
                                    const ActionPartService &s )
{
  conf->writeEntry( "Id", s.id() );
}

Profile ProfileConfig::readProfile( KConfig *conf )
{
  Profile prof;
  prof.setUid( conf->group() );
  prof.setName( conf->readEntry( "Name" ) );
  prof.setPixmap( conf->readEntry( "Pixmap" ) );
  prof.setConfirmSync( conf->readBoolEntry( "ConfirmSync",  true ) );
  prof.setConfirmDelete( conf->readBoolEntry( "ConfirmDelete", true ) );

  QStringList locationPath = conf->readListEntry( "LocationPath" );
  QStringList::ConstIterator it;
  QMap<QString,QString> map;
  for ( it = locationPath.begin(); it != locationPath.end(); ++it ) {
    QString val = conf->readPathEntry( "Path" + (*it) );
    map.insert( (*it),  val );
  }
  prof.setPaths( map );

  ActionPartService::List parts;

  QStringList ids = conf->readListEntry( "ActionParts" );
  for( it = ids.begin(); it != ids.end(); ++it ) {
    addPart( *it, parts );
  }

  prof.setActionParts( parts );

  return prof;
}
