/*
    Copyright (c) 2006 Till Adam <adam@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "localbookmarksresource.h"
#include <libakonadi/collectionlistjob.h>
#include <libakonadi/collectionmodifyjob.h>
#include <libakonadi/itemappendjob.h>
#include <libakonadi/itemfetchjob.h>
#include <libakonadi/itemstorejob.h>
#include <libakonadi/session.h>

#include <kfiledialog.h>
#include <klocale.h>
#include <kbookmarkmanager.h>
#include <kbookmark.h>

#include <QtCore/QDebug>
#include <QtDBus/QDBusConnection>

using namespace Akonadi;


LocalBookmarksResource::LocalBookmarksResource( const QString &id )
    :ResourceBase( id )
{
}

LocalBookmarksResource::~ LocalBookmarksResource()
{
}

bool LocalBookmarksResource::requestItemDelivery( const Akonadi::DataReference &ref, const QStringList &parts, const QDBusMessage &msg )
{
  Q_UNUSED( parts );
  // TODO use remote id to retrieve the item in the file
  return true;
}

void LocalBookmarksResource::aboutToQuit()
{
  // TODO save to the backend
}

void LocalBookmarksResource::configure()
{
  QString oldFile = settings()->value( "General/Path" ).toString();
  KUrl url;
  if ( !oldFile.isEmpty() )
    url = KUrl::fromPath( oldFile );
  else
    url = KUrl::fromPath( QDir::homePath() );
  QString newFile = KFileDialog::getOpenFileName( url, "*.xml |" + i18nc("Filedialog filter for *.xml", "XML Bookmark file"), 0, i18n("Select Bookmarks File") );
  if ( newFile.isEmpty() )
    return;
  if ( oldFile == newFile )
    return;
  settings()->setValue( "General/Path", newFile );

  mBookmarkManager = KBookmarkManager::managerForFile( newFile, name() );

  synchronize();
}

void LocalBookmarksResource::itemAdded( const Akonadi::Item & item, const Akonadi::Collection& col )
{
  if ( item.mimeType() != QLatin1String( "application/x-xbel" ) )
    return;

  KBookmark bk = item.payload<KBookmark>();
  KBookmark bkg = mBookmarkManager->findByAddress( col.remoteId() );
  if ( !bkg.isGroup() )
    return;

  KBookmarkGroup group = bkg.toGroup();
  group.addBookmark( bk );

  // saves the file
  mBookmarkManager->emitChanged( group );
}

void LocalBookmarksResource::itemChanged( const Akonadi::Item& item, const QStringList& )
{
  KBookmark bk = item.payload<KBookmark>();

  // saves the file
  mBookmarkManager->emitChanged( bk.parentGroup() );
}

void LocalBookmarksResource::itemRemoved(const Akonadi::DataReference & ref)
{
  KBookmark bk = mBookmarkManager->findByAddress( ref.remoteId() );
  KBookmarkGroup bkg = bk.parentGroup();

  if ( !bk.isNull() )
    return;

  bkg.deleteBookmark( bk );

  // saves the file
  mBookmarkManager->emitChanged( bkg );
}

Collection::List listRecursive( const KBookmarkGroup& parent, const Collection& parentCol )
{
  Collection::List list;
  const QStringList mimeTypes = QStringList() << "message/rfc822" << Collection::collectionMimeType();

  for ( KBookmark it = parent.first(); !it.isNull(); it = parent.next( it ) ) {
    if ( !it.isGroup() )
      continue;

    KBookmarkGroup bkg = it.toGroup();
    Collection col;
    col.setName( bkg.fullText() + '(' + bkg.address() + ')' ); // has to be unique
    col.setRemoteId( bkg.address() );
    col.setParent( parentCol );
    col.setContentTypes( mimeTypes ); // ###
    list << col;
    list << listRecursive( bkg, col );
  }

  return list;
}

void LocalBookmarksResource::retrieveCollections()
{
  Collection root;
  root.setParent( Collection::root() );
  root.setRemoteId( settings()->value( "General/Path" ).toString() );
  root.setName( name() );
  QStringList mimeTypes;
  mimeTypes << "application/x-xbel" << Collection::collectionMimeType(); // ###
  root.setContentTypes( mimeTypes );

  Collection::List list;
  list << root;
  list << listRecursive( mBookmarkManager->root(), root );

  collectionsRetrieved( list );
}

void LocalBookmarksResource::synchronizeCollection(const Akonadi::Collection & col)
{
  if ( !col.isValid() )
  {
    qDebug() << "Collection not valid";
    return;
  }

  changeProgress( 0 );

  KBookmarkGroup bkg;
  if ( col.remoteId() == settings()->value( "General/Path" ).toString() ) {
    bkg = mBookmarkManager->root();
  } else {

    KBookmark bk = mBookmarkManager->findByAddress( col.remoteId() );
    if ( bk.isNull() || !bk.isGroup() )
      return;

    bkg = bk.toGroup();
  }

  for ( KBookmark it = bkg.first(); !it.isNull(); it = bkg.next( it ) ) {

    if ( it.isGroup() || it.isSeparator() || it.isNull() )
      continue;

    Item item( DataReference( -1, it.address() ) );
    item.setMimeType( "application/x-xbel" );
    item.setPayload<KBookmark>( it );
    ItemAppendJob *job = new ItemAppendJob( item, col, this );

    if ( !job->exec() ) {
      qDebug() << "Error while appending bookmark to storage: " << job->errorString();
      continue;
    }
  }

  changeProgress( 100 );
  collectionSynchronized();
}

#include "localbookmarksresource.moc"
