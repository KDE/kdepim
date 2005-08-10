/*
    Copyright (c) 2005 by Volker Krause <volker.krause@rwth-aachen.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


#include <kiconloader.h>
#include <kguiitem.h>
#include <klistview.h>
#include <klocale.h>

#include "sloxfolderdialog.h"
#include "sloxfoldermanager.h"

SloxFolderDialog::SloxFolderDialog( SloxFolderManager *manager, FolderType type, QWidget *parent, const char *name ) :
  KDialogBase( parent, name, true, i18n("Select Folder"), Ok|Cancel|User1, Ok, false, KGuiItem( i18n("Reload"), "reload" ) ),
  mManager( manager ),
  mFolderType( type )
{
  mListView = new KListView( this );
  mListView->setRootIsDecorated( true );
  mListView->setShowSortIndicator( true );
  mListView->addColumn( i18n("Folder") );
  mListView->addColumn( i18n("Folder ID"), 0 );
  setMainWidget( mListView );
  updateFolderView();
  connect( manager, SIGNAL( foldersUpdated() ), SLOT( updateFolderView() ) );
}

SloxFolderDialog::~SloxFolderDialog()
{
  QMap<QString, SloxFolder*> folders = mManager->folders();
  QMap<QString, SloxFolder*>::Iterator it;
  for ( it = folders.begin(); it != folders.end(); ++it )
    (*it)->item = 0;
}

void SloxFolderDialog::updateFolderView()
{
  QString selected = selectedFolder();
  mListView->clear();
  QMap<QString, SloxFolder*> folders = mManager->folders();
  QMap<QString, SloxFolder*>::Iterator it;
  for ( it = folders.begin(); it != folders.end(); ++it )
    createFolderViewItem( *it );
  setSelectedFolder( selected );
}

void SloxFolderDialog::slotUser1( )
{
  mManager->requestFolders();
}

void SloxFolderDialog::createFolderViewItem( SloxFolder *folder )
{
  if ( folder->item )
    return;
  if ( folder->type() != mFolderType && folder->type() != Unbound )
    return;
  if( mManager->folders().contains( folder->parentId() ) ) {
    SloxFolder *parent = mManager->folders()[folder->parentId()];
    createFolderViewItem( parent );
    if ( parent->item )
      folder->item = new KListViewItem( parent->item );
    else
      folder->item = new KListViewItem( mListView );
  } else {
    folder->item = new KListViewItem( mListView );
  }
  folder->item->setText( 0, folder->name() );
  folder->item->setText( 1, folder->id() );
  KGlobal::instance()->iconLoader()->addAppDir( "kmail" );
  switch ( folder->type() ) {
    case Calendar:
      folder->item->setPixmap( 0, SmallIcon( "kmgroupware_folder_calendar" ) );
      break;
    case Tasks:
      folder->item->setPixmap( 0, SmallIcon( "kmgroupware_folder_tasks" ) );
      break;
    case Contacts:
      folder->item->setPixmap( 0, SmallIcon( "kmgroupware_folder_contacts" ) );
      break;
    default:
      folder->item->setPixmap( 0, SmallIcon( "folder" ) );
      break;
  }
}

QString SloxFolderDialog::selectedFolder() const
{
  QListViewItem *item = mListView->selectedItem();
  if ( item )
    return item->text( 1 );
  return "-1"; // OX default folder
}

void SloxFolderDialog::setSelectedFolder( const QString &id )
{
  QMap<QString, SloxFolder*> folders = mManager->folders();
  QMap<QString, SloxFolder*>::Iterator it;
  for ( it = folders.begin(); it != folders.end(); ++it ) {
    if ( !(*it)->item )
      continue;
    if ( (*it)->id() == id || ( ( id.isEmpty() || id == "-1" ) && (*it)->isDefault() ) ) {
      mListView->setSelected( (*it)->item, true );
      mListView->ensureItemVisible( (*it)->item );
      break;
    }
  }
}

#include "sloxfolderdialog.moc"
