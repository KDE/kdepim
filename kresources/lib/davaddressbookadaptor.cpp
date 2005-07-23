/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>
    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "davaddressbookadaptor.h"
// #include "ogoglobals.h"
// #include "davgroupwareglobals.h"
// #include "webdavhandler.h"

// #include <kabc/addressee.h>
// #include <kabc/vcardconverter.h>
// #include <kabcresourcecached.h>

#include <kio/davjob.h>
// #include <kio/job.h>
#include <kdebug.h>

using namespace KABC;

void DavAddressBookAdaptor::interpretListFoldersJob( KIO::Job *job, KPIM::FolderLister */*folderLister*/ )
{
  KIO::DavJob *davjob = dynamic_cast<KIO::DavJob*>( job );
  Q_ASSERT( davjob );
  if ( !davjob ) return;

  QDomDocument doc = davjob->response();
  kdDebug(7000) << " Doc: " << doc.toString() << endl;

  QDomElement docElement = doc.documentElement();
  QDomNode n;
  for( n = docElement.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    QDomNode n2 = n.namedItem( "propstat" );
    QDomNode n3 = n2.namedItem( "prop" );
    
    KURL href( n.namedItem( "href" ).toElement().text() );
    QString displayName = n3.namedItem( "displayname" ).toElement().text();
    KPIM::FolderLister::ContentType type = getContentType( n3 );

    emit folderInfoRetrieved( href, displayName, type );
    emit folderSubitemRetrieved( href, getFolderHasSubs( n3 ) );
  }
}
