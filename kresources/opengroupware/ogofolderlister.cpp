/*
    This file is part of libkcal.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>

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

#include "ogofolderlister.h"
#include "webdavhandler.h"
#include <kdebug.h>
#include <klocale.h>
#include <kio/davjob.h>

using namespace KPIM;

OGoFolderLister::OGoFolderLister( FolderLister::Type type ) : FolderLister( type )
{
}

KURL OGoFolderLister::adjustUrl( const KURL &u )
{
  KURL url = u;
  if ( getType() == Calendar ) {
    url.addPath( "Groups" );
  } else {
    url.addPath( "public" );
  }
  return url;
}

KIO::DavJob *OGoFolderLister::createJob( const KURL &url )
{
  QDomDocument props = WebdavHandler::createAllPropsRequest();
  kdDebug(7000) << "props: " << props.toString() << endl;
  return KIO::davPropFind( url, props, "1", false );
}

FolderLister::Entry::List OGoFolderLister::defaultFolders()
{
  Entry::List newFolders;

  // Personal calendar/addressbook
  Entry personal;
  KURL url = getUrl();
  url.setUser( QString::null );
  url.setPass( QString::null );
  if ( getType() == Calendar ) {
    personal.name = i18n("Personal Calendar");
    personal.id = url.url();
  } else {
    personal.name = i18n("Personal Addressbook");
    url.addPath( "Contacts" );
    personal.id = url.url();
  }
  newFolders.append( personal );
  return newFolders;
}

FolderLister::FolderType OGoFolderLister::getFolderType( const QDomNode &folderNode )
{
  QDomNode n4;
  for( n4 = folderNode.firstChild(); !n4.isNull(); n4 = n4.nextSibling() ) {
    QDomElement e = n4.toElement();
        
    if ( e.tagName() == "resourcetype" ) {
      if ( !e.namedItem( "vevent-collection" ).isNull() )
        return CalendarFolder;
      if ( !e.namedItem( "vtodo-collection" ).isNull() )
        return TasksFolder;
      if ( !e.namedItem( "vcard-collection" ).isNull() )
        return ContactsFolder;
      if ( !e.namedItem( "collection" ).isNull() ) 
        return Folder;
    }
  }
  return Unknown;

}

#include "ogofolderlister.moc"
