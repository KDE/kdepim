/*
    This file is part of KAddressbook.
    Copyright (c) 2003  Alexander Kellett <lypanov@kde.org>
                        Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kbookmark.h>
#include <kbookmarkmanager.h>
#include <kbookmarkmenu.h>
#include <kbookmarkdombuilder.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "bookmark_xxport.h"

K_EXPORT_KADDRESSBOOK_XXFILTER( libkaddrbk_bookmark_xxport, BookmarkXXPort )

BookmarkXXPort::BookmarkXXPort( KABC::AddressBook *ab, QWidget *parent, const char *name )
  : KAB::XXPort( ab, parent, name )
{
  createExportAction( i18n( "Export Bookmarks Menu..." ) );
}

bool BookmarkXXPort::exportContacts( const KABC::AddresseeList &list, const QString& )
{
  QString fileName = locateLocal( "data", "kabc/bookmarks.xml" );

  KBookmarkManager *mgr = KBookmarkManager::managerForFile( fileName );
  KBookmarkDomBuilder *builder = new KBookmarkDomBuilder( mgr->root(), mgr );
  builder->connectImporter( this );

  KABC::AddresseeList::ConstIterator it;
  emit newFolder( i18n( "AddressBook" ), false, "" );
  for ( it = list.begin(); it != list.end(); ++it ) {
    if ( !(*it).url().isEmpty() ) {
      QString name = (*it).givenName() + " " + (*it).familyName();
      emit newBookmark( name, (*it).url().url().latin1(), QString( "" ) );
    }
  }
  emit endFolder();
  delete builder;
  mgr->save();

  KBookmarkMenu::DynMenuInfo menu;
  menu.name = i18n( "Addressbook Bookmarks" );
  menu.location = fileName;
  menu.type = "xbel";
  menu.show = true;
  KBookmarkMenu::setDynamicBookmarks( "kabc", menu );

  return true;
}

#include "bookmark_xxport.moc"
