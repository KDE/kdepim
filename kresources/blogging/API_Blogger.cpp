/*
    This file is part of kdepim.

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

#include "API_Blogger.h"
#include "xmlrpcjob.h"
#include <kdebug.h>

using namespace KBlog;

QString APIBlogger::getFunctionName( blogFunctions type )
{
  switch ( type ) {
    case bloggerGetUserInfo:    return "blogger.getUserInfo";
    case bloggerGetUsersBlogs:  return "blogger.getUsersBlogs";
    case bloggerGetRecentPosts: return "blogger.getRecentPosts";
    case bloggerNewPost:        return "blogger.newPost";
    case bloggerEditPost:       return "blogger.editPost";
    case bloggerDeletePost:     return "blogger.deletePost";
    case bloggerGetPost:        return "blogger.getPost";
    case bloggerGetTemplate:    return "blogger.getTemplate";
    case bloggerSetTemplate:    return "blogger.setTemplate";
    default: return QString::null;
  }
}




KIO::Job *APIBlogger::createUserInfoJob()
{
  kdDebug() << "read user info..." << endl;
  QValueList<QVariant> args( defaultArgs() );
  return KIO::xmlrpcCall( mServerURL, getFunctionName( bloggerGetUserInfo ), args, false );
}

KIO::Job *APIBlogger::createListFoldersJob()
{
  // TODO: Check if we're already authenticated. If not, do it!
//   if ( isValid() ) {
    kdDebug() << "Fetch List of Blogs..." << endl;
    QValueList<QVariant> args( defaultArgs() );
    return KIO::xmlrpcCall( mServerURL, getFunctionName( bloggerGetUsersBlogs ), args, false );
//   } else {
//     warningNotInitialized();
//     return 0;
//   }
}

KIO::TransferJob *APIBlogger::createListItemsJob( const KURL &url )
{
  // TODO: Check if we're already authenticated. If not, do it!
//   if ( isValid() ) {
    kdDebug() << "Fetch List of Posts..." << endl;
    QValueList<QVariant> args( defaultArgs( url.url() ) );
    args << QVariant( mDownloadCount );
    return KIO::xmlrpcCall( mServerURL, getFunctionName( bloggerGetRecentPosts ), args, false );
//   } else {
//     warningNotInitialized();
//     return 0;
//   }
}

KIO::TransferJob *APIBlogger::createDownloadJob( const KURL &url )
{
//   if ( isValid() ){
    kdDebug() << "Fetch Posting with url " << url.url() << endl;
    QValueList<QVariant> args( defaultArgs( url.url() ) );
    return KIO::xmlrpcCall( mServerURL, getFunctionName( bloggerGetPost ), args, false );
//   } else {
//     warningNotInitialized();
//     return 0;
//   }
}






bool APIBlogger::interpretUserInfoJob( KIO::Job *job )
{
  // TODO: Implement user authentication
//   isValid = true;
  KIO::XmlrpcJob *trfjob = dynamic_cast<KIO::XmlrpcJob*>(job);
  if ( job->error() || !trfjob ) {
    // TODO: Error handling
    return false;
  } else if ( trfjob ) {
    QValueList<QVariant> message( trfjob->response() );

    kdDebug () << "TOP: " << message[ 0 ].typeName() << endl;
    const QValueList<QVariant> posts = message;
    QValueList<QVariant>::ConstIterator it = posts.begin();
    QValueList<QVariant>::ConstIterator end = posts.end();
    for ( ; it != end; ++it ) {
      kdDebug () << "MIDDLE: " << ( *it ).typeName() << endl;
      const QMap<QString, QVariant> postInfo = ( *it ).toMap();
      const QString nickname = postInfo[ "nickname" ].toString();
      const QString userid = postInfo[ "userid" ].toString();
      const QString email = postInfo[ "email" ].toString();
      kdDebug() << "Post " << nickname << " " << userid << " " << email << endl;
      // FIXME: How about a BlogUserInfo class???
      emit userInfoRetrieved( nickname, userid, email );
    }
    return true;
  }
  return false;
}

void APIBlogger::interpretListFoldersJob( KIO::Job *job )
{
kdDebug() << "APIBlogger::interpretListFoldersJob" << endl;
  KIO::XmlrpcJob *trfjob = dynamic_cast<KIO::XmlrpcJob*>(job);
  if ( job->error() || !trfjob ) {
    // TODO: Error handling
  } else {
kdDebug() << "APIBlogger::interpretListFoldersJob, no error!" << endl;
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
      if ( !id.isEmpty() && !name.isEmpty() ) {
        emit folderInfoRetrieved( id, name );
kdDebug()<< "Emitting folderInfoRetrieved( id=" << id << ", name=" << name << "); " << endl;
      }
    }
  }
}

bool APIBlogger::interpretListItemsJob( KIO::Job *job )
{
  return interpretDownloadItemsJob( job );
}

bool APIBlogger::interpretDownloadItemsJob( KIO::Job *job )
{
  kdDebug(5800)<<"APIBlogger::interpretDownloadItemJob"<<endl;
  KIO::XmlrpcJob *trfjob = dynamic_cast<KIO::XmlrpcJob*>(job);
  bool success = false;
  if ( job->error() || !trfjob ) {
    // TODO: Error handling
    success = false;
  } else {
    //array of structs containing ISO.8601 dateCreated, String userid, String postid, String content;
    // TODO: Time zone for the dateCreated!
    QValueList<QVariant> message( trfjob->response() );
    kdDebug () << "TOP: " << message[ 0 ].typeName() << endl;

    const QValueList<QVariant> postReceived = message[ 0 ].toList();
    QValueList<QVariant>::ConstIterator it = postReceived.begin();
    QValueList<QVariant>::ConstIterator end = postReceived.end();
    success = true;
    for ( ; it != end; ++it ) {
      BlogPosting posting;
      kdDebug () << "MIDDLE: " << ( *it ).typeName() << endl;
      const QMap<QString, QVariant> postInfo = ( *it ).toMap();
      if ( readPostingFromMap( &posting, postInfo ) ) {
        KCal::Journal *j = journalFromPosting( &posting );
//         dumpBlog( &posting );
        kdDebug() << "Emitting itemOnServer( posting.postID()="<<posting.postID() << "); " << endl;
        emit itemOnServer( posting.postID() );
        kdDebug() << "Emitting itemDownloaded( j=" << j << ", uid=" << j->uid()
                  << ", postID=" << posting.postID() << ", fpr="
                  << posting.fingerprint() << "); " << endl;
        emit itemDownloaded( j, j->uid(), posting.postID(),
                             posting.fingerprint(), posting.postID() );
      } else {
        kdDebug() << "readPostingFromMap failed! " << endl;
        success = false;
        // TODO: Error handling
      }
    }
  }
  return success;
}


bool APIBlogger::readPostingFromMap( BlogPosting *post, const QMap<QString, QVariant> &postInfo )
{
  // FIXME:
  if ( !post ) return false;
  QStringList mapkeys = postInfo.keys();
  kdDebug() << endl << "Keys: " << mapkeys.join(", ") << endl << endl;

  QString fp( QString::null );
  
  QDateTime dt( postInfo[ "dateCreated" ].toDateTime() );
  if ( dt.isValid() && !dt.isNull() ) {
    post->setCreationDateTime( dt );
    QString fp = dt.toString( Qt::ISODate );
  }
  dt = postInfo[ "postDate" ].toDateTime();
  if ( dt.isValid() && !dt.isNull() ) {
    post->setDateTime( dt );
    fp = dt.toString( Qt::ISODate );
  }
  dt = postInfo[ "lastModified" ].toDateTime();
  if ( dt.isValid() && !dt.isNull() ) {
    post->setModificationDateTime( dt );
    fp = dt.toString( Qt::ISODate );
  }
  post->setFingerprint( fp );

  post->setUserID( postInfo[ "userid" ].toString() );
  post->setPostID( postInfo[ "postid" ].toString() );

  QString title( postInfo[ "title" ].toString() );
  QString description( postInfo[ "description" ].toString() );
  QString contents( postInfo[ "content" ].toString() );
  QString category;

    // TODO: Extract title and cats from the old-style blogger api without extensions
/*
  if ( (title.isEmpty() || description.isEmpty() ) && !contents.isEmpty()  ) {
    // we don't have both title and description, so use the content (ie. it's an
    // old-style blogger api, not the extended drupal api.

    kdDebug() << "No title and description given, so it's an old-style "
                 "Blogger API without extensions" << endl;
    QString catTagOpen = mTemplate.categoryTagOpen();
    QString catTagClose = mTemplate.categoryTagClose();
    QString titleTagOpen = mTemplate.titleTagOpen();
    QString titleTagClose = mTemplate.titleTagClose();

    int catStart = contents.find( catTagOpen, 0, false ) + catTagOpen.length();
    int catEnd = contents.find( catTagClose, 0, false );
kdDebug() << "  catTagOpen = " << catTagOpen << ", catTagClose = " << catTagClose << ", start - end : " << catStart <<" - " << catEnd << endl;
    if ( catEnd > catStart ) {
      category = contents.mid( catStart, catEnd - catStart );
      kdDebug() << "Found a category \"" << category << "\"" << endl;
      contents = contents.remove( catStart - catTagOpen.length(),
              catEnd - catStart + catTagClose.length() + catTagOpen.length() );
    }
    int titleStart = contents.find( titleTagOpen, 0, false ) + titleTagOpen.length();
    int titleEnd = contents.find( titleTagClose, 0, false );
kdDebug() << "  titleTagOpen = " << titleTagOpen << ", titleTagClose = " << titleTagClose << ", start - end : " << titleStart <<" - " << titleEnd << endl;
    kdDebug() << "Title start and end: " << titleStart << ", " << titleEnd << endl;
    if ( titleEnd > titleStart ) {
      title = contents.mid( titleStart, titleEnd - titleStart );
      contents = contents.remove( titleStart - titleTagOpen.length(),
              titleEnd - titleStart + titleTagClose.length() + titleTagOpen.length() );
    }
    kdDebug() << endl << endl << endl << "After treatment of the special tags, we have a content of: "<< endl << contents << endl;
  }
*/

  post->setTitle( title );
  post->setContent( contents );
  if ( !category.isEmpty() )
    post->setCategory( category );
  return true;
}



