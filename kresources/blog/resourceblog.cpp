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

#include "resourceblog.h"

#include <QDateTime>
#include <QString>

#include <KUrl>
#include <KLocale>
#include <KConfigGroup>
#include <kcal/journal.h>
#include <kcal/calendarlocal.h>

#include <kresources/configwidget.h>

#include <libkdepim/progressmanager.h>

#include <kblog/blogpost.h>
#include <kblog/blogmedia.h>
#include <kblog/movabletype.h>
#include <kblog/livejournal.h>
#include <kblog/gdata.h>

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

  if ( mLock ) {
    delete mLock;
  }
  if ( mPostMap ) {
    delete mPostMap;
  }
}

void ResourceBlog::init()
{
  mProgress = 0;

  mBlog = 0;

  mDownloadCount = 10;

  setType( "blog" );

  mLock = new KABC::Lock( cacheFile() );
  mPostMap = new QMap<QString,KBlog::BlogPost*>();

  enableChangeNotification();
}

void ResourceBlog::readConfig( const KConfigGroup &group )
{
  QString url = group.readEntry( "URL" );
  mUrl = KUrl( url );
  mUsername = group.readEntry( "Username" );
  mPassword = group.readEntry( "Password" );
  setAPI( group.readEntry( "API" ) );
  mBlogID = group.readEntry( "BlogID" );
  mBlog->setBlogId( mBlogID );
  mBlogName = group.readEntry( "BlogName" );
  mDownloadCount = group.readEntry( "DownloadCount" ).toInt();

  ResourceCached::readConfig( group );
}

void ResourceBlog::writeConfig( KConfigGroup &group )
{
  group.writeEntry( "URL", mUrl.url() );
  group.writeEntry( "Username", mUsername );
  group.writeEntry( "Password", mPassword );
  group.writeEntry( "API", API() );
  group.writeEntry( "BlogID", mBlogID );
  group.writeEntry( "BlogName", mBlogName );
  group.writeEntry( "DownloadCount", mDownloadCount );

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
  if ( API == "Google Blogger Data" ) {
    mBlog = new KBlog::GData( mUrl, this );
  } else if ( API == "LiveJournal" ) {
    mBlog = new KBlog::LiveJournal( mUrl, this );
  } else if ( API == "Movable Type" ) {
    mBlog = new KBlog::MovableType( mUrl, this );
  } else if ( API == "MetaWeblog" ) {
    mBlog = new KBlog::MetaWeblog( mUrl, this );
  } else if ( API == "Blogger 1.0" ) {
    mBlog = new KBlog::Blogger1( mUrl, this );
  } else {
    kError( 5650 ) << "ResourceBlog::setAPI(): Unrecognised API:" << API;
    return;
  }
  mBlog->setUsername( mUsername );
  mBlog->setPassword( mPassword );
}

QString ResourceBlog::API() const
{
  if ( mBlog ) {
    if ( qobject_cast<KBlog::GData*>( mBlog ) ) {
      return "Google Blogger Data";
    }
    else if ( qobject_cast<KBlog::LiveJournal*>( mBlog ) ) {
      return "LiveJournal";
    }
    else if ( qobject_cast<KBlog::MovableType*>( mBlog ) ) {
      return "Movable Type";
    }
    else if ( qobject_cast<KBlog::MetaWeblog*>( mBlog ) ) {
      return "MetaWeblog";
    }
    else if ( qobject_cast<KBlog::Blogger1*>( mBlog ) ) {
      return "Blogger 1.0";
    }
  }
  return "Unknown";
}

void ResourceBlog::setDownloadCount( int downloadCount )
{
  mDownloadCount = downloadCount;
}

int ResourceBlog::downloadCount() const
{
  return mDownloadCount;
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

bool ResourceBlog::doLoad( bool )
{
  kDebug( 5650 ) << "ResourceBlog::doLoad()";

  if ( mUseCacheFile ) {
    disableChangeNotification();
    loadFromCache();
    enableChangeNotification();
  }

  clearChanges();

  if ( mBlog ) {
    if ( mLock->lock() ) {
      connect ( mBlog, SIGNAL( listedRecentPosts(
                const QList<KBlog::BlogPost> & ) ),
                this, SLOT( slotListedPosts(
                const QList<KBlog::BlogPost> & ) ) );
      connect ( mBlog, SIGNAL( errorPost( const KBlog::Blog::ErrorType &,
                const QString &, KBlog::BlogPost * ) ),
                this, SLOT( slotErrorPost( const KBlog::Blog::ErrorType &,
                const QString &, KBlog::BlogPost * ) ) );

      if ( mUseProgressManager ) {
        mProgress = KPIM::ProgressManager::createProgressItem(
            KPIM::ProgressManager::getUniqueID(),
            i18n("Downloading blog posts") );
        mProgress->setProgress( 0 );
      }
      mBlog->listRecentPosts( downloadCount() );
      mLock->unlock();
      return true;
    } else {
      kError( 5650 ) << "ResourceBlog::doLoad(): cache file is locked"
          << " - something else must be loading the file";
    }
  }
  kError( 5650 ) << "ResourceBlog::doLoad(): Blog not initialised";
  return false;
}

void ResourceBlog::slotListedPosts(
    const QList<KBlog::BlogPost> &posts )
{
  kDebug( 5650 ) << "ResourceBlog::slotListedPosts()";
  QList<KBlog::BlogPost>::const_iterator i;
  for (i = posts.constBegin(); i != posts.constEnd(); ++i) {
    Journal* newJournal = (*i).journal( *mBlog );
    if ( newJournal ) {
      Journal* existingJournal = journal( newJournal->uid() );
      if ( existingJournal ) {
        existingJournal->setSummary( newJournal->summary() );
        existingJournal->setCategories( newJournal->categories() );
        existingJournal->setDescription( newJournal->description() );
        existingJournal->setDtStart( newJournal->dtStart() );
        delete newJournal;
        clearChange( existingJournal );
      }
      else {
        addJournal( newJournal );
        clearChange( newJournal );
      }
    }
  }
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
  kError( 5650 ) << "ResourceBlog::slotError " << type << ": " << errorMessage;
  mProgress->setComplete();
  mProgress = 0;
  //Q_ASSERT(false);
}

void ResourceBlog::slotErrorPost( const KBlog::Blog::ErrorType &type,
                              const QString &errorMessage,
                              KBlog::BlogPost *post )
{
  kError( 5650 ) << "ResourceBlog::slotErrorPost " << type << ": "
      << errorMessage;
  mProgress->setComplete();
  mProgress = 0;
  if ( post ) {
    delete post;
  }
  //Q_ASSERT(false);
}

void ResourceBlog::slotErrorMedia( const KBlog::Blog::ErrorType &type,
                                     const QString &errorMessage,
                                     KBlog::BlogMedia *media )
{
  kError( 5650 ) << "ResourceBlog::slotErrorMedia " << type << ": "
      << errorMessage;
  mProgress->setComplete();
  mProgress = 0;
  if ( media ) {
    delete media;
  }
  //Q_ASSERT(false);
}

void ResourceBlog::slotSavedPost( KBlog::BlogPost *post )
{
  if ( post ) {
    kDebug( 5650 ) << "ResourceBlog::slotSavedPost: Post saved with ID"
        << post->postId();
    if ( post->status() == KBlog::BlogPost::Created ) {
      mLastKnownPostID = post->postId().toInt();
      // Instead of modifying the existing we journal we create a new one and
      // use that instead to make use of the metadata provided in KBlog.
      Journal* existingJournal = journal( post->journalId() );
      if ( existingJournal ) {
        deleteJournal( existingJournal );
        clearChange( existingJournal );
        delete existingJournal;
      }
      Journal* newJournal = post->journal( *mBlog );
      if ( newJournal ) {
        addJournal ( newJournal );
        clearChange( newJournal );
      }
    }
    else {
      clearChange( post->journalId() );
    }
    delete post;

    Incidence::List changes = allChanges();
    if ( changes.begin() == changes.end() ) {
      saveToCache();
      emit resourceSaved( this );
    }
    }
  else {
    kDebug( 5650 ) << "ResourceBlog::slotSavedPost: Post saved was null";
  }
}

void ResourceBlog::slotBlogInfoRetrieved(
      const QList<QMap<QString,QString> > &blogs )
{
  emit signalBlogInfoRetrieved( blogs );
}

bool ResourceBlog::doSave( bool )
{
  kDebug( 5650 ) << "ResourceBlog::doSave()";

  if ( readOnly() || !hasChanges() ) {
    emit resourceSaved( this );
    return true;
  }

  if ( !mBlog ) {
    kError( 5650 ) << "ResourceBlog::addJournal(): Blog not initialised.";
    return false;
  }

  Incidence::List::Iterator i;
  Incidence::List added = addedIncidences();
  for ( i = added.begin(); i != added.end(); ++i ) {
    Journal* journal = dynamic_cast<Journal*>( *i );
    if ( journal ) {
      KBlog::BlogPost *post = new KBlog::BlogPost( *journal );
      if ( post ) {
        connect ( mBlog, SIGNAL( createdPost( KBlog::BlogPost * ) ),
                  this, SLOT( slotSavedPost( KBlog::BlogPost * ) ) );
        connect ( mBlog, SIGNAL( errorPost( const KBlog::Blog::ErrorType &,
                  const QString &, KBlog::BlogPost * ) ),
                  this, SLOT( slotErrorPost( const KBlog::Blog::ErrorType &,
                              const QString &, KBlog::BlogPost * ) ) );
        mBlog->createPost( post );
        kDebug( 5650 ) << "ResourceBlog::doSave(): adding " << journal->uid();
      }
    }
  }

  Incidence::List changed = changedIncidences();
  for( i = changed.begin(); i != changed.end(); ++i ) {
    Journal* journal = dynamic_cast<Journal*>( *i );
    if ( journal ) {
      KBlog::BlogPost *post = new KBlog::BlogPost( *journal );
      if ( post ) {
        connect ( mBlog, SIGNAL( modifiedPost( KBlog::BlogPost * ) ),
                  this, SLOT( slotSavedPost( KBlog::BlogPost * ) ) );
        connect ( mBlog, SIGNAL( errorPost( const KBlog::Blog::ErrorType &,
                  const QString &, KBlog::BlogPost * ) ),
                  this, SLOT( slotErrorPost( const KBlog::Blog::ErrorType &,
                              const QString &, KBlog::BlogPost * ) ) );
        mBlog->modifyPost( post );
        kDebug( 5650 ) << "ResourceBlog::doSave(): changing " << journal->uid();
      }
    }
  }

  Incidence::List deleted = deletedIncidences();
  for( i = deleted.begin(); i != deleted.end(); ++i ) {
    Journal* journal = dynamic_cast<Journal*>( *i );
    if ( journal ) {
      KBlog::BlogPost *post = new KBlog::BlogPost( *journal );
      if ( post ) {
        connect ( mBlog, SIGNAL( removedPost( KBlog::BlogPost * ) ),
                  this, SLOT( slotSavedPost( KBlog::BlogPost * ) ) );
        connect ( mBlog, SIGNAL( errorPost( const KBlog::Blog::ErrorType &,
                  const QString &, KBlog::BlogPost * ) ),
                  this, SLOT( slotErrorPost( const KBlog::Blog::ErrorType &,
                              const QString &, KBlog::BlogPost * ) ) );
        mBlog->removePost( post );
        kDebug( 5650 ) << "ResourceBlog::doSave(): removing " << journal->uid();
      }
    }
  }
  return true;
}

KABC::Lock *ResourceBlog::lock ()
{
  return mLock;
}

void ResourceBlog::dump() const
{
  ResourceCalendar::dump();
  kDebug( 5650 ) << "  URL: " << mUrl.url();
  kDebug( 5650 ) << "  Username: " << mUsername;
  kDebug( 5650 ) << "  API: " << API();
  kDebug( 5650 ) << "  ReloadPolicy: " << reloadPolicy();
  kDebug( 5650 ) << "  BlogID: " << mBlogID;
  kDebug( 5650 ) << "  BlogName: " << mBlogName;
  kDebug( 5650 ) << "  DownloadCount: " << mDownloadCount;
}

void ResourceBlog::addInfoText( QString &txt ) const
{
  txt += "<br>";
  txt += i18n( "URL: %1", mUrl.prettyUrl() );
  txt += i18n( "Username: %1", mUsername );
  txt += i18n( "API: %1", API() );
  txt += i18n( "BlogName: %1", mBlogName );
  txt += i18n( "DownloadCount: %1", mDownloadCount );
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

bool ResourceBlog::listBlogs() {
  // Only children of Blogger 1.0 and Google Blogger Data support listBlogs()
  KBlog::Blogger1* blogger = qobject_cast<KBlog::Blogger1*>( mBlog );
  if ( blogger ) {
    connect ( blogger,
              SIGNAL( listedBlogs( const QList<QMap<QString,QString> > & ) ),
              this, SLOT( slotBlogInfoRetrieved(
                          const QList<QMap<QString,QString> > & ) ) );
    //TODO: Error handling
    blogger->listBlogs();
    return true;
  }
  KBlog::GData* gdata = qobject_cast<KBlog::GData*>( mBlog );
  if ( gdata ) {
    connect ( gdata,
              SIGNAL( listedBlogs( const QList<QMap<QString,QString> > & ) ),
              this, SLOT( slotBlogInfoRetrieved(
                          const QList<QMap<QString,QString> > & ) ) );
    //TODO: Error handling
    gdata->listBlogs();
    return true;
  }
  kDebug( 5650 ) << "ResourceBlog::listBlogs(): "
      << "API does not support multiple blogs.";
  return false;
}

void ResourceBlog::setBlog( const QString &id, const QString &name ) {
  mBlogID = id;
  mBlogName = name;
}

QPair<QString, QString> ResourceBlog::blog() const {
  return qMakePair( mBlogID, mBlogName );
}

#include "resourceblog.moc"
