/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "konnector.h"

#include "konnectorinfo.h"

#include <kmdcodec.h>
#include <kdebug.h>
#include <libkdepim/progressmanager.h>

#include <qdir.h>

using namespace KPIM;
using namespace KSync;

Konnector::Konnector( const KConfig *config )
    : KRES::Resource( config )
{
  /* default storage path */
  m_sPath = QDir::homeDirPath() + "/.kitchensync/meta/";
}

Konnector::~Konnector()
{
}

void Konnector::writeConfig( KConfig *config )
{
  KRES::Resource::writeConfig( config );
}

void Konnector::add( const QString& res )
{
    m_resources << res;
}

void Konnector::remove( const QString& res )
{
    m_resources.remove( res );
}

QStringList Konnector::resources() const
{
    return m_resources;
}

bool Konnector::isConnected() const
{
    return info().isConnected();
}

QStringList Konnector::builtIn() const
{
    return QStringList();
}

QString Konnector::storagePath()const
{
  return m_sPath;
}

void Konnector::setStoragePath( const QString& path )
{
  m_sPath = path;
  emit storagePathChanged( m_sPath );
}

KPIM::ProgressItem* Konnector::progressItem( const QString &msg )
{
  ProgressItem *item = ProgressManager::instance()->createProgressItem(
                       ProgressManager::getUniqueID(), msg );

  connect( item, SIGNAL( progressItemCanceled( ProgressItem* ) ),
           SLOT( progressItemCanceled( ProgressItem* ) ) );

  return item;
}

void Konnector::progressItemCanceled( ProgressItem *item )
{
  item->setComplete();
}

/**
   Append the Syncee to the internal list of Syncees.
   If the Konnector implements this method and can
   handle the format it will upload the Syncee
   on writeSyncees.
   The ownership of \par ap is transfered to the Konnector.
   The default implementation deletes \par ap.

   @param ap The Syncee to append to the list of Syncees
   @see writeSyncees
*/
void Konnector::appendSyncee( Syncee* ap )
{
  delete ap;
/* DEFAULT NO IMPLEMENTATION */
}

/**
 * Generate a MD5SUM from a QString. The intended use is with
 * with storagePath() + "/" + generateMD5Sum(path) + "some_name.log"
 * to really have unique identifiers
 *
 * @return a MD5SUM for the name
 */
QString Konnector::generateMD5Sum( const QString& base) {
  KMD5 sum(base.local8Bit() );
  QString str = QString::fromLatin1( sum.hexDigest().data() );

  return str;
}

/**
 * Remove SyncEntry::wasRemoved() itrems from the Syncee
 *
 * @param sync The Syncee to manipulate
 */
void Konnector::purgeRemovedEntries( Syncee* sync) {
  QPtrList<SyncEntry> lst = sync->removed();
  SyncEntry* entry;

  for (entry = lst.first(); entry; entry = lst.next() ) {
    kdDebug() << "purgeRemoved Entries " << entry->id() << " " << entry->name() << endl;
    sync->removeEntry( entry );
  }


  lst.setAutoDelete( true );
  lst.clear();
}

#include "konnector.moc"
