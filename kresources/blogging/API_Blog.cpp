/***************************************************************************
*   Copyright (C) 2004-05 Reinhold Kainhofer <reinhold@kainhofer.com>     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#include "API_Blog.h"
#include <kdebug.h>
#include <qvariant.h>

using namespace KBlog;

APIBlog::APIBlog( const KURL &url, QObject *parent, const char *name ) :
    QObject( parent, name ),
    mServerURL( url ), mAppID( QString::null ), mDownloadCount( 20 )
{}

APIBlog::~APIBlog()
{}

void APIBlog::dumpBlog( BlogPosting *blog )
{
  kdDebug() << "-----------------------------------" << endl;
  kdDebug() << "Post " << blog->postID() << " by \"" <<
               blog->userID() << "\" on " <<
               blog->dateTime().toString() << endl;
  kdDebug() << "Title: " << blog->title() << endl;
  kdDebug() << blog->content() <<endl;
  kdDebug() << "-----------------------------------" << endl;
}



/*void APIBlog::setTemplateTags( const BlogTemplate& Template )
{
  mTemplate = Template;
}
BlogTemplate APIBlog::templateTags() const
{
  return mTemplate;
}*/

/*void APIBlog::deletePost( const QString &postID )
{
  BlogPosting *post = new BlogPosting();
  post->setPostID( postID );
  deletePost( post );
  delete post;
}*/

QValueList<QVariant> APIBlog::defaultArgs( const QString &id )
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


KCal::Journal *APIBlog::journalFromPosting( KBlog::BlogPosting *blog )
{
  if ( !blog ) return 0;
  KCal::Journal *j = new KCal::Journal();
  QDateTime dt = blog->dateTime();
  QDateTime creationDt = blog->creationDateTime();
  QDateTime modificationDt = blog->modificationDateTime();
kdDebug() << "dt            ="<<dt.toString( Qt::ISODate ) << endl;
kdDebug() << "creationDt    ="<<creationDt.toString( Qt::ISODate ) << endl;
kdDebug() << "modificationDt="<<modificationDt.toString( Qt::ISODate ) << endl;
  if ( dt.isValid() && !dt.isNull() ) {
    j->setDtStart( dt );
  } else if ( creationDt.isValid() && !creationDt.isNull() ) {
    j->setDtStart( creationDt );
  } else if ( modificationDt.isValid() && !modificationDt.isNull() ) {
    j->setDtStart( modificationDt );
  }
  
  j->setCreated( blog->creationDateTime() );
  j->setLastModified( blog->modificationDateTime() );
  j->setFloats( false );
  kdDebug() << "Date for blog " << blog->title() << " is "
            << blog->dateTime().toString()<<endl;
  j->setSummary( blog->title() );
  j->setDescription( blog->content() );
  j->setCategories( QStringList( blog->category() ) );
  j->setOrganizer( blog->userID() );
  j->setCustomProperty( "KCalBloggerRes", "UserID", blog->userID() );
  j->setCustomProperty( "KCalBloggerRes", "BlogID", blog->blogID() );
  j->setCustomProperty( "KCalBloggerRes", "PostID", blog->postID() );

  // TODO: Set the read-only flag in the resource!
//   j->setReadOnly( readOnly() );

  return j;
}

KBlog::BlogPosting *APIBlog::postingFromJournal( KCal::Journal *journal )
{
  KBlog::BlogPosting *item = new KBlog::BlogPosting();
  if ( journal && item ) {
    item->setContent( journal->description() );
    item->setTitle( journal->summary() );
    item->setCategory( journal->categories().first() );
    item->setDateTime( journal->dtStart() );
    item->setModificationDateTime( journal->lastModified() );
    item->setCreationDateTime( journal->created() );
    item->setUserID( journal->customProperty( "KCalBloggerRes", "UserID" ) );
    item->setBlogID( journal->customProperty( "KCalBloggerRes", "BlogID" ) );
    item->setPostID( journal->customProperty( "KCalBloggerRes", "PostID" ) );
  }
  return item;
}


#include "API_Blog.moc"
