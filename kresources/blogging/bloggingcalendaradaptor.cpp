/*
    This file is part of kdepim.

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

#include "bloggingcalendaradaptor.h"
#include "bloggingglobals.h"
#include "webdavhandler.h"
#include <libemailfunctions/idmapper.h>
#include "xmlrpcjob.h"
#include <folderlister.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/icalformat.h>
#include <libkcal/resourcecached.h>

#include <kdebug.h>

using namespace KCal;

BloggingCalendarAdaptor::BloggingCalendarAdaptor()
{
}

KIO::Job *BloggingCalendarAdaptor::createListFoldersJob( const KURL &url )
{
  kdDebug() << "Fetch Blogs..." << endl;
  KURL u( url );
  QString user( u.user() );
  QString pw( u.pass() );
  u.setUser( QString::null );
  u.setPass( QString::null );

  QValueList<QVariant> args( BloggingGlobals::defaultArgs( user, pw ) );
  return KIO::xmlrpcCall( u, BloggingGlobals::getFunctionName( BloggingGlobals::bloggerGetUsersBlogs ),
                          args, false );
}



KIO::TransferJob *BloggingCalendarAdaptor::createDownloadItemJob( const KURL &/*url*/, KPIM::GroupwareJob::ContentType /*ctype*/ )
{
// TODO:
return 0;
//   return BloggingGlobals::createDownloadItemJob( this, url,ctype );
}

KIO::TransferJob *BloggingCalendarAdaptor::createListItemsJob( const KURL &/*url*/ )
{
// TODO:
return 0;
//  return BloggingGlobals::createListItemsJob( url );
}

KIO::Job *BloggingCalendarAdaptor::createRemoveItemsJob( const KURL &/*uploadurl*/, KPIM::GroupwareUploadItem::List /*deletedItems*/ )
{
// TODO:
return 0;
//  return BloggingGlobals::createRemoveItemsJob( uploadurl, deletedItems );
}

void BloggingCalendarAdaptor::interpretListFoldersJob( KIO::Job *job, KPIM::FolderLister */*folderLister*/ )
{
  KIO::XmlrpcJob *trfjob = dynamic_cast<KIO::XmlrpcJob*>(job);
  if ( job->error() ) {
    // TODO: Error handling
  } else if ( trfjob ) {
    QValueList<QVariant> message( trfjob->response() );
    kdDebug () << "TOP: " << message[ 0 ].typeName() << endl;
    
    const QValueList<QVariant> posts = message[ 0 ].toList();
    QValueList<QVariant>::ConstIterator it = posts.begin();
    QValueList<QVariant>::ConstIterator end = posts.end();
    for ( ; it != end; ++it ) {
      kdDebug () << "MIDDLE: " << ( *it ).typeName() << endl;
      const QMap<QString, QVariant> postInfo = ( *it ).toMap();
    
      const QString id( postInfo[ "blogid" ].toString() );
      const QString name( postInfo[ "blogName" ].toString() );
      const QString url( postInfo[ "url" ].toString() );
      
      // Use the Blog ID instead of the URL. The ID already indicates the correct blog, and the 
      // URL for all calls will be the XML-RPC interface, anyway.
      if ( !id.isEmpty() && !name.isEmpty() ) 
        emit folderInformationRetrieved( id, name, KPIM::FolderLister::JournalsFolder );
    }
  }
}

bool BloggingCalendarAdaptor::interpretListItemsJob( KIO::Job */*job*/, QStringList &/*currentlyOnServer*/, QMap<QString,KPIM::GroupwareJob::ContentType> &/*itemsForDownload*/ )
{
// TODO:
return 0;
//  return BloggingGlobals::itemsForDownloadFromList( this, job, currentlyOnServer, itemsForDownload );
}

