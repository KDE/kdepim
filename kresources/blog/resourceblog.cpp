/*
  This file is part of the blog resource.

  Copyright (c) 2007 Mike Arthur <mike@mikearthur.co.uk>

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

#include <QDateTime>
#include <QString>

#include <KDebug>
#include <KUrl>
#include <KLocale>

#include <kcal/journal.h>

#include <kresources/configwidget.h>

#include <libkdepim/progressmanager.h>

#include <kblog/blogposting.h>
#include <kblog/metaweblog.h>

#include "resourceblog.h"

using namespace KCal;

ResourceBlog::ResourceBlog()
    : ResourceCached(), mUseProgressManager( true ), mUseCacheFile( true )
{
  init();
}

ResourceBlog::ResourceBlog( const KConfigGroup &group )
    : ResourceCached( group ), mUseProgressManager( true ),
    mUseCacheFile( true )
{
  init();
  readConfig( group );
}

ResourceBlog::~ResourceBlog()
{
  close();

  delete mLock;
}

void ResourceBlog::init()
{
  mProgress = 0;

  mAPI = 0;

  setType( "blog" );

  mLock = new KABC::Lock( cacheFile() );

  enableChangeNotification();
}

void ResourceBlog::readConfig( const KConfigGroup &group )
{
  kDebug( 5800 ) << "ResourceBlog::readConfig()";

  QString url = group.readEntry( "URL" );
  mUrl = KUrl( url );
  mUsername = group.readEntry( "Username" );
  mPassword = group.readEntry( "Password" );
  setAPI( group.readEntry( "API" ) );
  mBlogID = group.readEntry( "BlogID" );
  kDebug( 5800 ) << "ResourceBlog::readConfig(): ID: " << mBlogID;
  mBlogName = group.readEntry( "BlogName" );

  ResourceCached::readConfig( group );
}

void ResourceBlog::writeConfig( KConfigGroup &group )
{
  kDebug( 5800 ) << "ResourceBlog::writeConfig()";

  group.writeEntry( "URL", mUrl.url() );
  group.writeEntry( "Username", mUsername );
  group.writeEntry( "Password", mPassword );
  group.writeEntry( "API", API() );
  group.writeEntry( "BlogID", mBlogID );
  group.writeEntry( "BlogName", mBlogName );

  ResourceCalendar::writeConfig( group );
  ResourceCached::writeConfig( group );
}

void ResourceBlog::setUrl( const KUrl &url )
{
  mUrl = url;
}

KUrl ResourceBlog::url() const
{
  return mUrl;
}

void ResourceBlog::setUsername( const QString &username )
{
  mUsername = username;
}

QString ResourceBlog::username() const
{
  return mUsername;
}

void ResourceBlog::setPassword( const QString &password )
{
  mPassword = password;
}

QString ResourceBlog::password() const
{
  return mPassword;
}

void ResourceBlog::setAPI( const QString &API )
{
  kDebug( 5800 ) << "ResourceBlog::setAPI(): " << API;
  if ( API == "MetaWeblog" ) {
    mAPI = new KBlog::MetaWeblog( mUrl, this );
  } else if ( API == "Blogger 1.0" ) {
    mAPI = new KBlog::Blogger1( mUrl, this );
  } else {
    kError() << "ResourceBlog::setAPI(): Unrecognised API: " << API;
    return;
  }
  mAPI->setUsername( mUsername );
  mAPI->setPassword( mPassword );
}

QString ResourceBlog::API() const
{
  if ( mAPI ) {
    if ( qobject_cast<KBlog::MetaWeblog*>( mAPI ) ) {
      return "MetaWeblog";
    }
    else if ( qobject_cast<KBlog::Blogger1*>( mAPI ) ) {
      return "Blogger 1.0";
    }
  }
  return "Unknown";
}

void ResourceBlog::setUseProgressManager( bool useProgressManager )
{
  mUseProgressManager = useProgressManager;
}

bool ResourceBlog::useProgressManager() const
{
  return mUseProgressManager;
}

void ResourceBlog::setUseCacheFile( bool useCacheFile )
{
  mUseCacheFile = useCacheFile;
}

bool ResourceBlog::useCacheFile() const
{
  return mUseCacheFile;
}

bool ResourceBlog::doLoad( bool fullReload )
{
  kDebug( 5800 ) << "ResourceBlog::load()";

  if ( mAPI ) {
    if ( mLock->lock() ) {
      int downloadCount = 100;
      //FIXME Actually do something? Calculate posts for non-full reload.
      if ( fullReload ) {
        // TODO: downloadCount = math;
        mJournalsMap.clear();
        ResourceCached::deleteAllJournals();
      }
      mAPI->setBlogId( mBlogID );
      connect ( mAPI, SIGNAL( listedPosting( KBlog::BlogPosting & ) ),
                this, SLOT( slotListedPosting( KBlog::BlogPosting & ) ) );
      connect ( mAPI, SIGNAL( listPostingsFinished() ),
                this, SLOT( slotListPostingsFinished() ));
      connect ( mAPI, SIGNAL( error( const KBlog::Blog::ErrorType &, const QString & ) ),
                this, SLOT( slotError( const KBlog::Blog::ErrorType &, const QString & ) ) );

      if ( mUseProgressManager ) {
        mProgress = KPIM::ProgressManager::createProgressItem(
            KPIM::ProgressManager::getUniqueID(),
            i18n("Downloading blog posts") );
        mProgress->setProgress( 0 );
      }
      mAPI->listRecentPostings( downloadCount );
      mLock->unlock();
      return true;
    } else {
      kDebug( 5800 ) << "ResourceBlog::load(): cache file is locked"
          << " - something else must be loading the file";
    }
  }
  kError( 5800 ) << "ResourceBlog::load(): null mAPI";
  return false;
}

void ResourceBlog::slotListedPosting( KBlog::BlogPosting &blogPosting )
{
  kDebug( 5800 ) << "ResourceBlog::slotListedPosting()";
  Journal *journalBlog = new Journal();
  QString id = "kblog-" + mUrl.url() + "-" + mUsername + "-" +
      blogPosting.postingId();
  if ( mJournalsMap.value( id ) == 0 ) {
    connect ( mAPI, SIGNAL( createdPosting( QString & ) ),
      this, SLOT( slotCreatedPosting( QString & ) ) );
    connect ( mAPI, SIGNAL( error( const KBlog::Blog::ErrorType &, const QString & ) ),
      this, SLOT( slotError( const KBlog::Blog::ErrorType &, const QString & ) ) );
    journalBlog->setUid( id );
    journalBlog->setSummary( blogPosting.title() );
    journalBlog->setCategories( blogPosting.categories() );
    journalBlog->setDescription( blogPosting.content() );
    journalBlog->setDtStart( blogPosting.creationDateTime() );
    if ( ResourceCached::addJournal( journalBlog ) ) {
      kDebug( 5800 ) << "ResourceBlog::slotListedPosting(): Journal added"
         ;
      mJournalsMap.insert( id, journalBlog );
    }
  }
}

void ResourceBlog::slotListPostingsFinished()
{
  kDebug( 5800 ) << "ResourceBlog::slotListPostingsFinished()";

  emit resourceChanged( this );

  if ( mProgress ) {
    mProgress->setComplete();
    mProgress = 0;
  }

  emit resourceLoaded( this );
}

void ResourceBlog::slotError( const KBlog::Blog::ErrorType &type,
                              const QString &errorMessage )
{
  kError( 5800 ) << "ResourceBlog: " << type << ": " << errorMessage;
  //Q_ASSERT(false);
}

void ResourceBlog::slotCreatedPosting( const QString &id )
{
  kDebug( 5800 ) << "ResourceBlog: Posting created with id " << id;
  mPostID = id.toInt();
}

void ResourceBlog::slotBlogInfoRetrieved( const QString &id,
                                          const QString &name )
{
  kDebug( 5800 ) << "ResourceBlog::slotBlogInfoRetrieved( id=" << id <<
      ", name=" << name;
  mBlogsMap.insert( name, id );
  emit signalBlogInfoRetrieved( id, name );
}

bool ResourceBlog::doSave( bool )
{
  kDebug( 5800 ) << "ResourceBlog::save()";

  if ( readOnly() || !hasChanges() ) {
    emit resourceSaved( this );
    return true;
  }

  saveToCache();
  emit resourceSaved( this );

  return true;
}

KABC::Lock *ResourceBlog::lock ()
{
  return mLock;
}

void ResourceBlog::dump() const
{
  ResourceCalendar::dump();
  kDebug( 5800 ) << "  URL: " << mUrl.url();
  kDebug( 5800 ) << "  Username: " << mUsername;
  kDebug( 5800 ) << "  API: " << API();
  kDebug( 5800 ) << "  ReloadPolicy: " << reloadPolicy();
}

void ResourceBlog::addInfoText( QString &txt ) const
{
  txt += "<br>";
  txt += i18n( "URL: %1", mUrl.prettyUrl() );
  txt += i18n( "API: %1", API() );
}

bool ResourceBlog::setValue( const QString &key, const QString &value )
{
  if ( key == "URL" ) {
    setUrl( KUrl( value ) );
    return true;
  } else if ( key == "Username" ) {
    setUsername( value );
    return true;
  } else if ( key == "Password" ) {
    setPassword( value );
    return true;
  } else if ( key == "API" ) {
    setAPI( value );
    return true;
  } else {
    return ResourceCached::setValue( key, value );
  }
}

bool ResourceBlog::addJournal( Journal *journal )
{
  kDebug( 5800 ) << "ResourceBlog::addJournal()";
  QString title = journal->summary();
  QString content = journal->description();
  KDateTime date = journal->dtStart();
  QStringList categories = journal->categories();
  //TODO: Delete
  KBlog::BlogPosting *post = new KBlog::BlogPosting();
  post->setTitle( title );
  post->setContent( content );
  post->setCategories( categories );
  if ( mAPI ) {
    mAPI->setBlogId( mBlogID );
    post->setCreationDateTime( date );
    connect ( mAPI, SIGNAL( error( const KBlog::Blog::ErrorType &, const QString & ) ),
              this, SLOT( slotError( const KBlog::Blog::ErrorType &, const QString & ) ) );
    mAPI->createPosting( post );
    return true;
  }
  kError() << "ResourceBlog::addJournal(): Journal not initialised.";
  return false;
}

bool ResourceBlog::fetchBlogs() {
  if ( mAPI ) {
    connect ( mAPI, SIGNAL( blogInfoRetrieved( const QString &,
                                               const QString & ) ),
        this, SLOT( slotBlogInfoRetrieved( const QString &,
                                           const QString & ) ) );
    //Check if we're using Blogger or higher
    //mAPI->listBlogs();
    return true;
  }
  kError( 5800 ) << "ResourceBlog::fetchBlogs(): Journal not initialised.";
  return false;
}

bool ResourceBlog::setBlog( const QString &name ) {
  if ( mBlogsMap.contains( name ) ) {
    mBlogID = mBlogsMap.value( name );
    mBlogName = name;
    kDebug( 5800 ) << "ResourceBlog::setBlog( id=" << mBlogID <<
      ", name=" << mBlogName;
    return true;
  }
  return false;
}

QPair<QString, QString> ResourceBlog::blog() {
  return qMakePair( mBlogID, mBlogName );
}

#include "resourceblog.moc"
