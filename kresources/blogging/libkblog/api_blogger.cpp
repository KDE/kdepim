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
#include "bloggerwrapper.h"
#include <kdebug.h>
#include <klocale.h>

#include <qregexp.h>

//using namespace KBlog;



namespace KBlog {


bloggerAPI::bloggerAPI( const KURL &kurl, QObject *parent, const char *name ) : blogInterface( kurl, parent, name )
{
  mXMLRPCServer = new KXMLRPC::Server( "", this );
  mXMLRPCServer->setUrl( kurl );
  isValid = false;
}


bloggerAPI::~bloggerAPI()
{}

QString bloggerAPI::getFunctionName( blogFunctions type )
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


/*****************************************************
 *     Helper methods
 *****************************************************/
 
void bloggerAPI::setDefaultBlogID( const QString &blogID )
{
  mDefaultBlogID = blogID;
}

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

QString bloggerAPI::formatContents( BlogPosting *blog )
{
  if ( !blog ) return QString::null;
  QString content = blog->content();

  QString post = "";
  if ( !blog->title().isEmpty() ) {
    post += mTemplate.titleTagOpen() + blog->title() + mTemplate.titleTagClose() + "\n";
  }
  if ( !blog->category().isEmpty() ) {
    post += mTemplate.categoryTagOpen() + blog->category() + mTemplate.categoryTagClose() + "\n";
  }
  post += content;
  return escapeContent( post );
}

bool bloggerAPI::readPostingFromMap( BlogPosting *post, 
        const QMap<QString, QVariant> &postInfo )
{
  // FIXME:
  if ( !post ) return false;
  QStringList mapkeys = postInfo.keys();
  kdDebug() << endl << "Keys: " << mapkeys.join(", ") << endl << endl;

  QDateTime dt( postInfo[ "dateCreated" ].toDateTime() );
  if ( dt.isValid() ) post->setCreationDateTime( dt );
  dt = postInfo[ "lastModified" ].toDateTime();
  if ( dt.isValid() ) post->setModificationDateTime( dt );
  dt = postInfo[ "postDate" ].toDateTime();
  if ( dt.isValid() ) post->setDateTime( dt );
  
  post->setUserID( postInfo[ "userid" ].toString() );
  post->setPostID( postInfo[ "postid" ].toString() );
  
  QString title( postInfo[ "title" ].toString() );
  QString description( postInfo[ "description" ].toString() );
  QString contents( postInfo[ "content" ].toString() );
  QString category;
  
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
  
  post->setTitle( title );
  post->setContent( contents );
  if ( !category.isEmpty() )
    post->setCategory( category );
  return true;
}

bool bloggerAPI::readBlogInfoFromMap( KBlog::BlogListItem &blog, 
        const QMap<QString, QVariant> &postInfo )
{
kdDebug() << "BlogID=" << postInfo["blogid"].toString() <<
             ", blogName=" << postInfo["blogName"].toString() <<
             ", URL=" << postInfo[ "url" ].toString() << endl;
kdDebug() << "bloggerAPI::readBlogInfoFromMap, keys = " << QStringList(postInfo.keys()).join(" - ") << endl;
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
  mXMLRPCServer->call( getFunctionName(bloggerGetUserInfo), args,
                        this, SLOT( userInfoFinished( const QValueList<QVariant> & ) ),
                        this, SLOT( fault( int, const QString& ) ) );
}

void bloggerAPI::getBlogs()
{
  if ( isValid ) {
    kdDebug() << "Fetch Blogs..." << endl;
    QValueList<QVariant> args( defaultArgs() );

    mXMLRPCServer->call( getFunctionName(bloggerGetUsersBlogs), args,
                          this, SLOT( blogListFinished( const QValueList<QVariant> & ) ),
                          this, SLOT( fault( int, const QString& ) ) );
  } else {
    warningNotInitialized();
  }

}

BloggerPostingWrapper *bloggerAPI::createWrapper( BlogPosting *posting )
{
  BloggerPostingWrapper *postwrapper = new BloggerPostingWrapper( posting );
  connect( postwrapper, SIGNAL( errorSignal( const QString & ) ),
           this, SIGNAL( error( const QString & ) ) );
  connect( postwrapper, SIGNAL( postFinishedSignal( bool ) ),
           this, SIGNAL( postFinishedSignal( bool ) ) );
  connect( postwrapper, SIGNAL( editFinishedSignal( bool ) ),
           this, SIGNAL( editFinishedSignal( bool ) ) );
  connect( postwrapper, SIGNAL( deleteFinishedSignal( bool ) ),
           this, SIGNAL( deleteFinishedSignal( bool ) ) );
  return postwrapper;
}

void bloggerAPI::post( BlogPosting *posting, bool publish )
{
  if ( isValid ) {
// FIXME: Use the wrapper here for posting-specific slots
    kdDebug() << "Posting data..." << endl;

    QString blogID( posting->blogID() );
    if ( blogID.isEmpty() ) blogID = mDefaultBlogID;
    QValueList<QVariant> args( defaultArgs( blogID ) );
    args << QVariant( formatContents( posting ) )
         << QVariant( publish, 0 );
         
kdDebug()<<"type name of last variant: "<< args.last().typeName() << endl;

    BloggerPostingWrapper *postwrap = createWrapper( posting );
    mXMLRPCServer->call( getFunctionName(bloggerNewPost), args,
                          postwrap, SLOT( postFinished( const QValueList<QVariant> & ) ),
                          postwrap, SLOT( postFault( int, const QString& ) ) );
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

    mXMLRPCServer->call( getFunctionName(bloggerGetRecentPosts), args,
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

    mXMLRPCServer->call( getFunctionName(bloggerGetPost), args,
                         this, SLOT( getFinished( const QValueList<QVariant> & ) ),
                         this, SLOT( fault( int, const QString& ) ) );
  } else {
    warningNotInitialized();
  }

}

void bloggerAPI::editPost( BlogPosting *posting, bool publish )
{
  if ( isValid ) {
    kdDebug() << "bloggerAPI::Posting data..." << endl;
    QString postID( posting->postID() );
    if ( postID.isEmpty() ) {
      kdDebug() << "Posting has no PostID, so assume it has never been on the "
                   "server. Posting it as new item." << endl;
      post( posting, publish );
    } else {
      QValueList<QVariant> args( defaultArgs( posting->postID() ) );
      args << QVariant( formatContents( posting ) )
           << QVariant( publish, 0 );

      BloggerPostingWrapper *postwrap = createWrapper( posting );
      mXMLRPCServer->call( getFunctionName(bloggerEditPost), args,
                           postwrap, SLOT( editFinished( const QValueList<QVariant> & ) ),
                           postwrap, SLOT( postFault( int, const QString& ) ) );
    }
  } else {
    warningNotInitialized();
  }
}

void bloggerAPI::deletePost( BlogPosting *posting )
{
  if ( !posting || posting->postID().isEmpty() ) return;
  if ( isValid )
  {
    kdDebug() << "delete post..." << endl;
    QValueList<QVariant> args( defaultArgs( posting->postID() ) );
    args << QVariant( true, 0 );

    BloggerPostingWrapper *postwrap = createWrapper( posting );
    mXMLRPCServer->call( getFunctionName(bloggerDeletePost), args,
                         postwrap, SLOT( deleteFinished( const QValueList<QVariant> & ) ),
                         postwrap, SLOT( deleteFault( int, const QString& ) ) );
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
  QValueList<BlogPosting*> fullposts;
        
  const QValueList<QVariant> postReceived = message[ 0 ].toList();
  QValueList<QVariant>::ConstIterator it = postReceived.begin();
  QValueList<QVariant>::ConstIterator end = postReceived.end();
  for ( ; it != end; ++it ) {
    BlogPosting *posting = new BlogPosting();
    kdDebug () << "MIDDLE: " << ( *it ).typeName() << endl;
    const QMap<QString, QVariant> postInfo = ( *it ).toMap();
    
    if ( readPostingFromMap( posting, postInfo ) ) {
      dumpBlog( posting );
      posts.append( posting->postID() );
      fullposts.append( posting );
    }
  }

  emit recentPostsSignal( posts );
  emit recentPostsSignal( fullposts );
  // TODO: Does this work, i.e. does emit recentPostsSignal really return after the signal is processed???
  // TODO: Add warning that the QValueList<BlogPosting*> argument to the signal is just a temporary object!
  for ( QValueList<BlogPosting*>::Iterator it = fullposts.begin(); it != fullposts.end(); ++it ) {
    delete (*it);
  }
}

void bloggerAPI::blogListFinished( const QValueList<QVariant> &message )
{
  kdDebug () << "TOP: " << message[ 0 ].typeName() << endl;
  QValueList<KBlog::BlogListItem> blogs;
  const QValueList<QVariant> posts = message[ 0 ].toList();
  QValueList<QVariant>::ConstIterator it = posts.begin();
  QValueList<QVariant>::ConstIterator end = posts.end();
  for ( ; it != end; ++it ) {
    kdDebug () << "MIDDLE: " << ( *it ).typeName() << endl;
    const QMap<QString, QVariant> postInfo = ( *it ).toMap();

    KBlog::BlogListItem blog;
    if ( readBlogInfoFromMap( blog, postInfo ) ) {
      blogs.append( blog );
    }
  }
  if ( blogs.count()>0 ) {
kdDebug()<<endl<<endl<<endl<<endl<<"BlogID of the first blog is: "<<blogs.first().id()<<", using that as default"<<endl;
    setDefaultBlogID( blogs.first().id() );
  }
  emit blogListSignal( blogs );
}

void bloggerAPI::getFinished( const QValueList<QVariant> &message )
{
  kdDebug () << "TOP: " << message[ 0 ].typeName() << endl;
  BlogPosting *post = new BlogPosting();
  const QMap<QString, QVariant> postInfo = message[ 0 ].toMap();
  if ( readPostingFromMap( post, postInfo ) ) {
    // TODO: Add warning that post is just temporary!
    emit newPostSignal( post );
  } else {
    emit error( i18n("Unable to interpret posting returned by the server" ) );
  }
  delete post;
}

void bloggerAPI::fault( int code, const QString &message )
{
  kdDebug() << "Error " << code << " " << message << endl;
  emit error( message );
}

}

#include "api_blogger.moc"


