/*
    This file is part of libkcal.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Based on the blogging resource:
    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>

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


/*
      void resourceChanged( ResourceCalendar * );
    emit resourceLoaded( ResourceCalendar * );
    void resourceSaved( ResourceCalendar * );
    void resourceLoadError( ResourceCalendar *, const QString &error );
    void resourceSaveError( ResourceCalendar *, const QString &error );
*/
#include "kcal_resourceblogging.h"

#include "api_blogger.h"
#include "bloginterface.h"

#include <qdatetime.h>
#include <klocale.h>

#include <libkcal/journal.h>
#include <kabc/locknull.h>


using namespace KCal;

QString ResourceBlogging::mAppID = QString("20ffffffd7ffffffc5ffffffbdffffff87ffffffb72d39fffffffe5c4bffffffcfffffff80ffffffd4665cfffffff375ffffff88ffffff871a0cffffff8029");

ResourceBlogging::ResourceBlogging( const KConfig *config )
  : ResourceCached( config ), mBlogInterface( 0 )
{
  if ( config ) {
    readConfig( config );
  }

  init();
}

ResourceBlogging::~ResourceBlogging()
{
  close();

  delete mLock;
}

void ResourceBlogging::init()
{
  mBlogInterface = 0;

  mProgress = 0;

  setType( "blogging" );

  mLock = new KABC::LockNull( true );

  enableChangeNotification();
}

void ResourceBlogging::readConfig( const KConfig *config )
{
  QString url = config->readEntry( "URL" );
  mUrl = KURL( url );
  
  mServerAPI = config->readNumEntry( "ServerAPI" );
  mTemplate.categoryTagOpen = config->readEntry( "CategoryTagOpen", "<CATEGORY>" );
  mTemplate.categoryTagClose = config->readEntry( "CategoryTagClose", "</CATEGORY>" );
  mTemplate.titleTagOpen = config->readEntry( "TitleTagOpen", "<TITLE>" );
  mTemplate.titleTagClose = config->readEntry( "TitleTagClose", "</TITLE>" );

  ResourceCached::readConfig( config );
}

void ResourceBlogging::writeConfig( KConfig *config )
{
  kdDebug(5800) << "ResourceBlogging::writeConfig()" << endl;

  ResourceCalendar::writeConfig( config );

  config->writeEntry( "URL", mUrl.url() );
  config->writeEntry( "ServerAPI", mServerAPI );
  config->writeEntry( "CategoryTagOpen", mTemplate.categoryTagOpen );
  config->writeEntry( "CategoryTagClose", mTemplate.categoryTagClose );
  config->writeEntry( "TitleTagOpen", mTemplate.titleTagOpen );
  config->writeEntry( "TitleTagClose", mTemplate.titleTagClose );

  ResourceCached::writeConfig( config );
}

void ResourceBlogging::setURL( const KURL &url )
{
  mUrl = url;
}

KURL ResourceBlogging::url() const
{
  return mUrl;
}

void ResourceBlogging::setServerAPI( int api )
{
  mServerAPI = api;
}
int ResourceBlogging::serverAPI() const
{
  return mServerAPI;
}

void ResourceBlogging::setTitleOpen( const QString &tag )
{
  mTemplate.categoryTagOpen = tag;
}
QString ResourceBlogging::titleOpen() const
{
  return mTemplate.categoryTagOpen;
}
void ResourceBlogging::setTitleClose( const QString &tag )
{
  mTemplate.categoryTagClose = tag;
}
QString ResourceBlogging::titleClose() const
{
  return mTemplate.categoryTagClose;
}
void ResourceBlogging::setCategoryOpen( const QString &tag )
{
  mTemplate.titleTagOpen = tag;
}
QString ResourceBlogging::categoryOpen() const
{
  return mTemplate.titleTagOpen;
}
void ResourceBlogging::setCategoryClose( const QString &tag )
{
  mTemplate.titleTagClose = tag;
}
QString ResourceBlogging::categoryClose() const
{
  return mTemplate.titleTagClose;
}

blogInterface *ResourceBlogging::initBlogAPI( int serverAPI, const KURL &url, QObject*parent ) 
{
  blogInterface *blogIf = 0;
  switch ( serverAPI ) {
    case blogAPIBlogger2:
// TODO:      blogIf =
      break;
    case blogAPIFile:
// TODO:      blogIf = new fileAPI( url, parent, "FileAPI" );
      break;
    case blogAPIDrupal:
// TODO:      blogIf = 
      break;
    case blogAPIKDEDevelopers:
// TODO:      blogIf = 
      break;
    case blogAPIBlogger:
    default:
      blogIf = new bloggerAPI( url, parent, "BloggerAPI" );
      break;
  }
  if ( !blogIf ) 
    return 0;
    
  blogIf->setUsername( url.user() );
  blogIf->setPassword( url.pass() );
  
  connect( blogIf, SIGNAL( serverInfo( const QString &, const QString &, const QString & ) ),
           this, SLOT( serverInfo( const QString &, const QString &, const QString & ) ) );
  connect( blogIf, SIGNAL( blogList( QValueList<blogListItem> ) ),
           this, SLOT( blogList( QValueList<blogListItem> ) ) );
//  connect( blogIf, SIGNAL( recentPosts( QStringList ) ),
//           this, SLOT( recentPosts( QStringList ) ) ) ;
  connect( blogIf, SIGNAL( recentPosts( const QValueList<blogPost> & ) ),
           this, SLOT( recentPosts( const QValueList<blogPost> & ) ) );
  

  connect( blogIf, SIGNAL( postFinished( bool ) ),
           this, SLOT( postFinished( bool ) ) );
  connect( blogIf, SIGNAL( publishFinished( bool ) ),
           this, SLOT( publishFinished( bool ) ) );
  connect( blogIf, SIGNAL( editFinished( bool ) ),
           this, SLOT( editFinished( bool ) ) );
  connect( blogIf, SIGNAL( deleteFinished( bool ) ),
           this, SLOT( deleteFinished( bool ) ) );
  connect( blogIf, SIGNAL( newPost( const blogPost & ) ),
           this, SLOT( newPost( const blogPost & ) ) );

  // Error message
  connect( blogIf, SIGNAL( error( const QString & ) ),
           this, SLOT( error( const QString & ) ) );

  return blogIf;
}

void ResourceBlogging::decrementDownloadCount()
{
  bool finished = false;
  if ( mDownloading > 0 ) {
    --mDownloading;
    if ( mDownloading == 0 && (mConnectionStatus & blogLoading) ) {
      mConnectionStatus &= ~blogLoading;
      finished = true;
      emit resourceLoaded( this );
    }
  }
  if ( mProgress ) {
    mProgress->incCompletedItems( 1 );
    if ( finished ) mProgress->setComplete();
  }
}

void ResourceBlogging::decrementUploadCount()
{
  bool finished = false;
  if ( mUploading > 0 ) {
    --mUploading;
    if ( mUploading == 0 && (mConnectionStatus & blogSaving) ) {
      mConnectionStatus &= ~blogSaving;
      finished = true;
      emit resourceSaved( this );
    }
  }
  if ( mProgress ) {
    mProgress->incCompletedItems( 1 );
    if ( finished ) mProgress->setComplete();
  }
}

void ResourceBlogging::newPost( const blogPost &newblog )
{
kdDebug() << "ResourceBlogging::newPost(): Downloaded post with ID " << 
newblog.postInfo.postID << endl;
  if ( mProgress )
    mProgress->setStatus( i18n("Received blog #%1").arg( newblog.postInfo.postID ) );
  Journal *j = journalFromBlog( newblog );
  if ( j ) addJournal( j );
  decrementDownloadCount();
}

bool ResourceBlogging::doLoad()
{
kdDebug(5800) << "ResourceBlogging::doOpen()" << endl;
  if ( mBlogInterface ) {
    delete mBlogInterface;
    mBlogInterface=0;
  }
  mBlogInterface = initBlogAPI( mServerAPI, mUrl, this );
  mBlogList.clear();
  mUploading = 0;
  mDownloading = 0;
  mConnectionStatus = blogConnecting;
  
  mProgress = KPIM::ProgressManager::createProgressItem(
      KPIM::ProgressManager::getUniqueID(), resourceName() );
  mProgress->setProgress( 0 );
  // We have three steps for sure: connecting, loading the cache and obtaining 
  // the list of blogs from the server.
  // Everything else will be added later.
  mProgress->setTotalItems( 3 );

  mBlogInterface->initServer();
  return true;
}



bool ResourceBlogging::doDownLoad()
{
kdDebug(5800) << "ResourceBlogging::load()" << endl;
  mProgress->setStatus( i18n("Connection established") );
  if ( !isOpen() ) 
    return false;
  mCalendar.close();
  mUploading = 0;
  mDownloading = 0;
  mConnectionStatus &= ~(blogLoading|blogSaving);
  mProgress->incCompletedItems( 1 );

  disableChangeNotification();
  loadCache();
  enableChangeNotification();

  clearChanges();

  emit resourceChanged( this );

//  mProgress->setProgress( 20 );
  
  kdDebug() << "Downloading blogs from: " << mUrl << endl;
  mConnectionStatus |= blogLoading;
  mBlogInterface->getBlogs();  
  
  mProgress->incCompletedItems( 1 );

  mProgress->setStatus( i18n("Downloading list of blogs") );
  return true;
}

//    void serverInfo( const QString &nickname, const QString & m_userid, const QString & email );

Journal *ResourceBlogging::journalFromBlog( const blogPost &blog ) 
{
  Journal *j = new Journal();
  j->setDtStart( blog.postInfo.date );
kdDebug()<<"Date for blog "<<blog.postInfo.title<<" is "<< blog.postInfo.date.toString()<<endl;
  j->setSummary( blog.postInfo.title );
  j->setDescription( blog.postInfo.description );
  j->setCategories( QStringList( blog.category ) );
  j->setOrganizer( blog.postInfo.userID );
  j->setCustomProperty( "KCalBloggerRes", "UserID", blog.postInfo.userID );
  j->setCustomProperty( "KCalBloggerRes", "BlogID", blog.postInfo.blogID );
  j->setCustomProperty( "KCalBloggerRes", "PostID", blog.postInfo.postID );
  
  return j;
}

blogPost ResourceBlogging::blogFromJournal( Journal *journal ) 
{
  blogPost item;
  if ( journal ) {
    item.content = journal->description();
    item.category = journal->categories().first();
    item.postInfo.date = journal->dtStart();
    item.postInfo.userID = journal->customProperty( "KCalBloggerRes", "UserID" );
    item.postInfo.blogID = journal->customProperty( "KCalBloggerRes", "BlogID" );
    item.postInfo.postID = journal->customProperty( "KCalBloggerRes", "PostID" );
  }
  return item;
}

void ResourceBlogging::blogList( QValueList<blogListItem> blogs )
{
  kdDebug(5800) << "ResourceBlogging::blogList() success" << endl;

  mCalendar.close();
  mBlogList = blogs;
  disableChangeNotification();
  // read in the blogs from the list of blogPostItems
  
  if ( mProgress )
    mProgress->setTotalItems( mProgress->totalItems() + blogs.count() );
  mDownloading += blogs.count();
  QValueList<blogListItem>::Iterator it;
  for ( it = blogs.begin(); it != blogs.end(); ++it ) {
kdDebug()<<"Fetching List of posts for blog with ID="<<(*it).ID<<endl;
    // FIXME: Implement "last N blogs":
    mBlogInterface->fetchPosts( (*it).ID, 50 );
//    mBlogInterface->fetchPost( (*it).ID );
  }
  enableChangeNotification();
  if ( blogs.count() == 0 ) 
    emit resourceLoaded( this );
  else 
    emit resourceChanged( this );

  if ( mProgress ) {
    mProgress->incCompletedItems( 1 );
    mProgress->setStatus( i18n("Downloading list of posts in the blog(s)") );
  }

}

bool ResourceBlogging::doSave()
{
  kdDebug(5800) << "ResourceBlogging::save()" << endl;

  // FIXME: Not yet implemented!!!
  if ( readOnly() || !hasChanges() ) {
    emit resourceSaved( this );
    return true;
  }
  mConnectionStatus |= blogSaving;

  Incidence::List newBlogs = addedIncidences();
  Incidence::List changedBlogs = changedIncidences();
  Incidence::List deletedBlogs = deletedIncidences();
  
  mUploading += newBlogs.count() + changedBlogs.count() + deletedBlogs.count();
  // TODO: post the new blogs, update the changed blogs, and remove the 
  // deleted blogs (if they have ever been on the server)
/*  if ( mDownloadJob ) {
    kdWarning() << "ResourceBlogging::save(): download still in progress."
                << endl;
    return false;
  }
  if ( mUploadJob ) {
    kdWarning() << "ResourceBlogging::save(): upload still in progress."
                << endl;
    return false;
  }

  mChangedIncidences = allChanges();

  saveCache();

  mUploadJob = KIO::file_copy( KURL( cacheFile() ), mUploadUrl, -1, true );
  connect( mUploadJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotSaveJobResult( KIO::Job * ) ) );
*/
  return true;
}


void ResourceBlogging::serverInfo( const QString &nickname, 
    const QString & m_userid, const QString & email )
{
kdDebug()<<"resourceBlogging::serverInfo( nickname="<<nickname<<", m_userid="<<m_userid<<", email="<<email<<")"<<endl;
  mConnectionStatus &= ~blogConnecting;
  mConnectionStatus |= blogConnected;
  mNick = nickname;
  mUserID = m_userid;
  mEmail = email;
  doDownLoad();
}

void ResourceBlogging::recentPosts( const QValueList<blogPost> &blogs )
{
kdDebug() << "resourceBlogging::recentPosts(), #=" << blogs.count() << endl;
  disableChangeNotification();
  
  if ( mProgress ) {
    mProgress->setTotalItems( mProgress->totalItems() + blogs.count() );
  }
  mDownloading += blogs.count();
  --mDownloading; 
  QValueList<blogPost>::ConstIterator it;
  for ( it = blogs.constBegin(); it != blogs.constEnd(); ++it ) {
kdDebug()<<"Fetching blog with ID=" << (*it).postInfo.postID << endl;
    newPost( *it );
//    mBlogInterface->fetchPost( *it );
  }
  enableChangeNotification();

  if ( mProgress ) {
    mProgress->incCompletedItems( 1 );
    mProgress->setTotalItems( mProgress->totalItems() + blogs.count() );
  }
}


//void ResourceBlogging::post( const blogPost &post );
void ResourceBlogging::handleUploadResult( bool success )
{
kdDebug()<<"resourceBlogging::handleUploadResult( success="<<success<<")"<<endl;
  decrementUploadCount();
  if ( !success ) {
    kdWarning() << "Upload failed!" << endl;
  }
}

void ResourceBlogging::postFinished( bool success )
{
kdDebug()<<"resourceBlogging::postFinished( success="<<success<<")"<<endl;
  handleUploadResult( success );
}
void ResourceBlogging::publishFinished( bool success )
{
kdDebug()<<"resourceBlogging::publishFinished( success="<<success<<")"<<endl;
  handleUploadResult( success );
}
void ResourceBlogging::editFinished( bool success )
{
kdDebug()<<"resourceBlogging::editFinished( success="<<success<<")"<<endl;
  handleUploadResult( success );
}
void ResourceBlogging::deleteFinished( bool success )
{
kdDebug()<<"resourceBlogging::deleteFinished( success="<<success<<")"<<endl;
  handleUploadResult( success );
}

// Error message
void ResourceBlogging::error( const QString &faultMessage )
{
kdWarning()<<"ResourceBlogging::error: " << faultMessage << 
    ", Connection status=" << mConnectionStatus << endl;
  if (  mConnectionStatus & blogConnecting ) {
    kdWarning()<<"Connection error while trying to establish connection:" <<
         faultMessage << endl;
    mConnectionStatus &= ~blogConnecting;
  } else if ( mConnectionStatus & blogLoading ) {
    decrementDownloadCount();
    loadError( faultMessage );
  } else if ( mConnectionStatus & blogSaving ) {
    decrementUploadCount();
    saveError( faultMessage );
  }
}


KABC::Lock *ResourceBlogging::lock()
{
  return mLock;
}

void ResourceBlogging::dump() const
{
  ResourceCalendar::dump();
  kdDebug(5800) << "  Url: " << mUrl.url() << endl;
  kdDebug(5800) << "  ReloadPolicy: " << reloadPolicy() << endl;
}

void ResourceBlogging::addInfoText( QString &txt ) const
{
  txt += "<br>";
  txt += i18n("URL: %1").arg( mUrl.prettyURL() );
  txt += i18n("Username: %1").arg( mUrl.user() );
}

#include "kcal_resourceblogging.moc"
