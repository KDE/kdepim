/***************************************************************************
*   Copyright (C) 2003 by ian reinhart geiser <geiseri@kde.org>           *
*   Copyright (C) 2004 by Reinhold Kainhofer <reinhold@kainhofer.com>     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#include "api_blogger.h"
#include "xmlrpciface.h"
#include <kdebug.h>
#include <klocale.h>

#include <qregexp.h>

using namespace KBlog;

bloggerAPI::bloggerAPI( const KURL &kurl, QObject *parent, const char *name ) : blogInterface( kurl, parent, name )
{
  mXMLRPCServer = new KXMLRPC::Server( "", this );
  mXMLRPCServer->setUrl( kurl );
  isValid = false;
}


bloggerAPI::~bloggerAPI()
{}


/*****************************************************
 *     Helper methods
 *****************************************************/

QValueList<QVariant> bloggerAPI::defaultArgs( const QString &id )
{
  QValueList<QVariant> args;
  args << QVariant( mAppID );
  if ( !id.isNull() ) {
    args << QVariant( id );
  }
  args << QVariant( mUsername )
       << QVariant( mPassword );
  return args;
}

void bloggerAPI::warningNotInitialized()
{
  kdDebug() << "bloggerAPI::getBlogs(): Server not yet initialized. "
                "Call initServer first (and wait until userInfoFinished is emited)!"<<endl;
}

QString bloggerAPI::escapeContent( const QString &content )
{
  QString post( content );
  post = post.replace( QRegExp( "&" ), "&amp;" );
  post = post.replace( QRegExp( "<" ), "&lt;" );
  post = post.replace( QRegExp( ">" ), "&gt;" );
  post = post.replace( QRegExp( "\"" ), "&quot;" );
  return post;
}

QString bloggerAPI::formatContents( const BlogPosting &blog )
{
  QString content = blog.content();

  QString post = "";
  if ( !blog.title().isEmpty() ) {
    post += mTemplate.titleTagOpen() + blog.title() + mTemplate.titleTagClose() + "\n";
  }
  if ( !blog.category().isEmpty() ) {
    post += mTemplate.categoryTagOpen() + blog.category() + mTemplate.categoryTagClose() + "\n";
  }
  post += content;
  return escapeContent( post );
}

void bloggerAPI::dumpBlog( const BlogPosting &blog ) 
{
  kdDebug() << "-----------------------------------" << endl;
  kdDebug() << "Post " << blog.postID() << " by \"" << 
               blog.userID() << "\" on " << 
               blog.dateTime().toString() << endl;
  kdDebug() << "Title: " << blog.title() << endl;
  kdDebug() << blog.content() <<endl;
  kdDebug() << "-----------------------------------" << endl;
}

bool bloggerAPI::readPostingFromMap( BlogPosting &post, 
        const QMap<QString, QVariant> &postInfo )
{
  // FIXME:
  QStringList mapkeys = postInfo.keys();
  kdDebug() << endl << "Keys: " << mapkeys.join(", ") << endl << endl;

  post.setDateTime( postInfo[ "dateCreated" ].toDateTime() );
  post.setUserID( postInfo[ "userid" ].toString() );
  post.setPostID( postInfo[ "postid" ].toString() );
  post.setTitle( postInfo[ "title" ].toString() );
//   post.description = postInfo[ "description" ].toString();
  QString postContent = postInfo[ "content" ].toString();
  
/////

  QString catTagOpen = mTemplate.categoryTagOpen();
  QString catTagClose = mTemplate.categoryTagClose();
  QString titleTagOpen = mTemplate.titleTagOpen();
  QString titleTagClose = mTemplate.titleTagClose();

  int catStart = postContent.find( catTagOpen, 0, false ) + catTagOpen.length();
  int catEnd = postContent.find( catTagClose, 0, false );
  if ( catEnd > catStart )
  {
    QString cat = postContent.mid( catStart, catEnd - catStart );
    postContent = postContent.remove( catStart - catTagOpen.length(), catEnd - catStart + catTagClose.length() + catTagOpen.length() );
    post.setCategory( cat );
  }
  int titleStart = postContent.find( titleTagOpen, 0, false ) + titleTagOpen.length();
  int titleEnd = postContent.find( titleTagClose, 0, false );
  kdDebug() << titleStart << titleEnd << endl;
  if ( titleEnd > titleStart )
  {
    QString title = postContent.mid( titleStart, titleEnd - titleStart );
    postContent = postContent.remove( titleStart - titleTagOpen.length(), titleEnd - titleStart + titleTagClose.length() + titleTagOpen.length() );
    post.setTitle( title );
  }
  post.setContent( postContent );

  return true;
}

bool bloggerAPI::readBlogInfoFromMap( BlogListItem &blog, 
        const QMap<QString, QVariant> &postInfo )
{
  blog.setId( postInfo[ "blogid" ].toString() );
  blog.setName( postInfo[ "blogName" ].toString() );
  blog.setUrl( postInfo[ "url" ].toString() );
  kdDebug() << "Message: " << blog.id() << " " << blog.name() << " " << blog.url() << endl;
  return true;
}


/*****************************************************
 *     Calling methods
 *****************************************************/

void bloggerAPI::initServer()
{
  QValueList<QVariant> args( defaultArgs() );

  mXMLRPCServer->setUrl( mServerURL );
  mXMLRPCServer->call( "blogger.getUserInfo", args,
                        this, SLOT( userInfoFinished( const QValueList<QVariant> & ) ),
                        this, SLOT( fault( int, const QString& ) ) );
}

void bloggerAPI::getBlogs()
{
  if ( isValid ) {
    kdDebug() << "Fetch Blogs..." << endl;
    QValueList<QVariant> args( defaultArgs() );

    mXMLRPCServer->call( "blogger.getUsersBlogs", args,
                          this, SLOT( blogListFinished( const QValueList<QVariant> & ) ),
                          this, SLOT( fault( int, const QString& ) ) );
  } else {
    warningNotInitialized();
  }

}

void bloggerAPI::post( const BlogPosting &posting, bool publish )
{
  if ( isValid ) {

    kdDebug() << "Posting data..." << endl;

    QValueList<QVariant> args( defaultArgs( posting.blogID() ) );
    args << QVariant( formatContents( posting ) )
         << QVariant( publish );

    mXMLRPCServer->call( "blogger.newPost", args,
                          this, SLOT( postFinished( const QValueList<QVariant> & ) ),
                          this, SLOT( fault( int, const QString& ) ) );
  } else {
    warningNotInitialized();
  }
}


void bloggerAPI::fetchPosts( const QString &blogID, int maxPosts )
{
  if ( isValid ) {
    kdDebug() << "Fetch Posts..." << endl;
    QValueList<QVariant> args( defaultArgs( blogID ) );
    args << QVariant( maxPosts );

    mXMLRPCServer->call( "blogger.getRecentPosts", args,
                          this, SLOT( listFinished( const QValueList<QVariant> & ) ),
                          this, SLOT( fault( int, const QString& ) ) );
  } else {
    warningNotInitialized();
  }
}

void bloggerAPI::fetchPost( const QString &postID )
{
  if ( isValid ){
    kdDebug() << "Fetch Post..." << endl;
    QValueList<QVariant> args( defaultArgs( postID ) );

    mXMLRPCServer->call( "blogger.getPost", args,
                          this, SLOT( getFinished( const QValueList<QVariant> & ) ),
                          this, SLOT( fault( int, const QString& ) ) );
  } else {
    warningNotInitialized();
  }

}

// void bloggerAPI::fetchTemplates(const QString &username, const QString &password)
//{
//}

void bloggerAPI::editPost( const BlogPosting& posting, bool publish )
{
  if ( isValid ) {
    kdDebug() << "Posting data..." << endl;
    QValueList<QVariant> args( defaultArgs( posting.postID() ) );
    args << QVariant( formatContents( posting ) )
         << QVariant( publish );

    mXMLRPCServer->call( "blogger.editPost", args,
                          this, SLOT( postFinished( const QValueList<QVariant> & ) ),
                          this, SLOT( fault( int, const QString& ) ) );
  } else {
    warningNotInitialized();
  }
}

void bloggerAPI::deletePost( const QString &postID )
{
  if ( isValid )
  {
    kdDebug() << "delete post..." << endl;
    QValueList<QVariant> args( defaultArgs( postID ) );
    args << QVariant( true );

    mXMLRPCServer->call( "blogger.getRecentPosts", args,
                          this, SLOT( deleteFinished( const QValueList<QVariant> & ) ),
                          this, SLOT( fault( int, const QString& ) ) );
  } else {
    warningNotInitialized();
  }
}


/*****************************************************
 *     Slots for Results from the server
 *****************************************************/

void bloggerAPI::userInfoFinished( const QValueList<QVariant> &message )
{
  isValid = true;
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
    emit serverInfoSignal( nickname, userid, email );
  }
}

void bloggerAPI::listFinished( const QValueList<QVariant> &message )
{
  //array of structs containing ISO.8601 dateCreated, String userid, String postid, String content;
  kdDebug () << "TOP: " << message[ 0 ].typeName() << endl;
  QStringList posts;
  QValueList<BlogPosting> fullposts;
        
  const QValueList<QVariant> postReceived = message[ 0 ].toList();
  QValueList<QVariant>::ConstIterator it = postReceived.begin();
  QValueList<QVariant>::ConstIterator end = postReceived.end();
  for ( ; it != end; ++it ) {
    BlogPosting posting;
    kdDebug () << "MIDDLE: " << ( *it ).typeName() << endl;
    const QMap<QString, QVariant> postInfo = ( *it ).toMap();
    
    if ( readPostingFromMap( posting, postInfo ) ) {
      dumpBlog( posting );
      posts.append( posting.postID() );
      fullposts.append( posting );
    }
  }

  emit recentPostsSignal( posts );
  emit recentPostsSignal( fullposts );
}

void bloggerAPI::blogListFinished( const QValueList<QVariant> &message )
{
  kdDebug () << "TOP: " << message[ 0 ].typeName() << endl;
  QValueList<BlogListItem> blogs;
  const QValueList<QVariant> posts = message[ 0 ].toList();
  QValueList<QVariant>::ConstIterator it = posts.begin();
  QValueList<QVariant>::ConstIterator end = posts.end();
  for ( ; it != end; ++it ) {
    kdDebug () << "MIDDLE: " << ( *it ).typeName() << endl;
    const QMap<QString, QVariant> postInfo = ( *it ).toMap();
    
    BlogListItem blog;
    if ( readBlogInfoFromMap( blog, postInfo ) ) {
      blogs.append( blog );
    }
  }
  emit blogListSignal( blogs );
}

void bloggerAPI::deleteFinished( const QValueList<QVariant> & /*message */ )
{
  emit deleteFinishedSignal( true );
}

void bloggerAPI::getFinished( const QValueList<QVariant> &message )
{
  kdDebug () << "TOP: " << message[ 0 ].typeName() << endl;
  BlogPosting post;
  const QMap<QString, QVariant> postInfo = message[ 0 ].toMap();
  if ( readPostingFromMap( post, postInfo ) ) {
    emit newPostSignal( post );
  } else {
    emit error( i18n("Unable to interpret posting returned by the server" ) );
  }
}

void bloggerAPI::fault( int code, const QString &message )
{
  kdDebug() << "Error " << code << " " << message << endl;
  emit error( message );
}

void bloggerAPI::postFinished( const QValueList<QVariant> &/*data */)
{
  kdDebug() << "Post successful. " << endl;
  emit postFinishedSignal( true );
}

#include "api_blogger.moc"


