/*
    Copyright (c) 2005 by Volker Krause <vkrause@kde.org>

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

#include <QPixmap>

#include <kiconloader.h>
#include <kguiitem.h>
#include <kcomponentdata.h>
#include <k3listview.h>
#include <klocale.h>

#include "sloxfolderdialog.h"
#include "sloxfoldermanager.h"

SloxFolderDialog::SloxFolderDialog( SloxFolderManager *manager, FolderType type, QWidget *parent ) :
  KDialog( parent),
  mManager( manager ),
  mFolderType( type )
{
  setCaption( i18n("Select Folder") );
  setButtons( Ok|Cancel|User1 );
  setDefaultButton( Ok );
  setButtonGuiItem( User1, KGuiItem( i18n("Reload"), "view-refresh" ) );
  mListView = new K3ListView( this );
  mListView->setRootIsDecorated( true );
  mListView->setShowSortIndicator( true );
  mListView->addColumn( i18n("Folder") );
  mListView->addColumn( i18n("Folder ID"), 0 );
  setMainWidget( mListView );
  updateFolderView();
  connect( manager, SIGNAL( foldersUpdated() ), SLOT( updateFolderView() ) );
  connect(this,SIGNAL(user1Clicked()),this,SLOT(slotUser1( )));
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
      folder->item = new K3ListViewItem( parent->item );
    else
      folder->item = new K3ListViewItem( mListView );
  } else {
    folder->item = new K3ListViewItem( mListView );
  }
  folder->item->setText( 0, folder->name() );
  folder->item->setText( 1, folder->id() );
  KIconLoader::global()->addAppDir( "kmail" );
  KIconLoader::global()->addAppDir( "kdepim" );
  switch ( folder->type() ) {
    case Calendar:
      folder->item->setPixmap( 0, SmallIcon( "view-pim-calendar" ) );
      break;
    case Tasks:
      folder->item->setPixmap( 0, SmallIcon( "view-pim-tasks" ) );
      break;
    case Contacts:
      folder->item->setPixmap( 0, SmallIcon( "view-pim-contacts" ) );
      break;
    default:
      folder->item->setPixmap( 0, SmallIcon( "folder" ) );
      break;
  }
}

QString SloxFolderDialog::selectedFolder() const
{
  Q3ListViewItem *item = mListView->selectedItem();
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
