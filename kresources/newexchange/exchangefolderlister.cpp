/*
    This file is part of libkcal.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>


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

#include "exchangefolderlister.h"
#include <webdavhandler.h>
#include <kdebug.h>
#include <klocale.h>
#include <qdom.h>
#include <kio/davjob.h>

using namespace KPIM;

ExchangeFolderLister::ExchangeFolderLister( Type type ) : FolderLister( type )
{
}
KIO::DavJob *FolderLister::createJob( const KURL &url )
{
  QDomDocument doc;
  QDomElement root = WebdavHandler::addDavElement(  doc, doc, "d:propfind" );
  QDomElement prop = WebdavHandler::addElement(  doc, root, "d:prop" );
  WebdavHandler::addElement( doc, prop, "d:displayname" );
  WebdavHandler::addElement( doc, prop, "d:contentclass" );
  WebdavHandler::addElement( doc, prop, "d:hassubs" );

  kdDebug(7000) << "props: " << doc.toString() << endl;
  return KIO::davPropFind( url, doc, "1", false );
}

bool ExchangeFolderLister::getFolderHasSubs( const QDomNode &folderNode )
{
  QString hassubs = folderNode.namedItem( "hassubs" ).toElement().text();
  return hassubs == "1";
}

FolderLister::FolderType ExchangeFolderLister::getFolderType( const QDomNode &folderNode )
{
kdDebug()<<"ExchangeFolderLister::getFolderType(...)"<<endl;
  QDomNode n4;
  for( n4 = folderNode.firstChild(); !n4.isNull(); n4 = n4.nextSibling() ) {
    QDomElement e = n4.toElement();

    if ( e.tagName() == "contentclass" ) {
      QString contentclass( e.text() );
      if ( contentclass == "urn:content-classes:contactfolder" )
        return ContactsFolder;
      if ( contentclass == "urn:content-classes:calendarfolder" )
        return CalendarFolder;
      if ( contentclass == "urn:content-classes:taskfolder" )
        return TasksFolder;
      if ( contentclass == "urn:content-classes:journalfolder" )
        return JournalsFolder;
      if ( contentclass == "urn:content-classes:folder" )
        return Folder;
    }
  }
  return Unknown;

}

#include "exchangefolderlister.moc"
