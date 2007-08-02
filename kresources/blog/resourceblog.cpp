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

#include <kcal/journal.h>
#include <kcal/calendarlocal.h>

#include <kresources/configwidget.h>

#include <libkdepim/progressmanager.h>

#include <kblog/blogposting.h>
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

  delete mLock;
}

void ResourceBlog::init()
{
  mProgress = 0;

  mBlog = 0;

  setType( "blog" );

  mLock = new KABC::Lock( cacheFile() );

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
    kError() << "ResourceBlog::setAPI(): Unrecognised API:" << API;
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
  kDebug() << "ResourceBlog::doLoad()";

  if ( mUseCacheFile ) {
    disableChangeNotification();
    loadFromCache();
    enableChangeNotification();
  }

  clearChanges();

  if ( mBlog ) {
    if ( mLock->lock() ) {
      connect ( mBlog, SIGNAL( listedRecentPostings(
                const QList<KBlog::BlogPosting*> & ) ),
                this, SLOT( slotListedPostings(
                const QList<KBlog::BlogPosting*> & ) ) );
      connect ( mBlog, SIGNAL( error( const KBlog::Blog::ErrorType &,
                const QString & ) ),
                this, SLOT( slotError( const KBlog::Blog::ErrorType &,
                const QString & ) ) );

      if ( mUseProgressManager ) {
        mProgress = KPIM::ProgressManager::createProgressItem(
            KPIM::ProgressManager::getUniqueID(),
            i18n("Downloading blog posts") );
        mProgress->setProgress( 0 );
      }
      mBlog->listRecentPostings( downloadCount() );
      mLock->unlock();
      return true;
    } else {
      kDebug() << "ResourceBlog::doLoad(): cache file is locked"
          << " - something else must be loading the file";
    }
  }
  kError() << "ResourceBlog::doLoad(): Blog not initialised";
  return false;
}

void ResourceBlog::slotListedPostings(
    const QList<KBlog::BlogPosting*> &postings )
{
  kDebug() << "ResourceBlog::slotListedPostings()";
  QList<KBlog::BlogPosting*>::const_iterator i;
  for (i = postings.constBegin(); i != postings.constEnd(); ++i) {
    Journal* newJournal = (**i).journal( *mBlog );
    delete *i;
    Journal* existingJournal = journal( newJournal->uid() );
    if ( existingJournal ) {
      existingJournal->setSummary( newJournal->summary() );
      existingJournal->setCategories( newJournal->categories() );
      existingJournal->setDescription( newJournal->description() );
      existingJournal->setDtStart( newJournal->dtStart() );
      delete newJournal;
    }
    else {
      addJournal( newJournal );
      clearChange( newJournal );
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
  kError() << "ResourceBlog::slotError " << type << ": " << errorMessage;
  Q_ASSERT(false);
}

void ResourceBlog::slotSavedPosting( KBlog::BlogPosting *posting )
{
  kDebug() << "ResourceBlog::slotSavedPosting: Posting saved with ID"
      << posting->postingId();
  if ( posting->status() == KBlog::BlogPosting::Created ) {
    mLastKnownPostID = posting->postingId().toInt();
  }
  clearChange( (*posting).journalId() );
  delete posting;

  Incidence::List changes = allChanges();
  if ( changes.begin() == changes.end() ) {
    saveToCache();
    emit resourceSaved( this );
  }
}

void ResourceBlog::slotBlogInfoRetrieved( const QMap<QString,QString> &blogs )
{
  emit signalBlogInfoRetrieved( blogs );
}

bool ResourceBlog::doSave( bool )
{
  kDebug() << "ResourceBlog::doSave()";

  if ( readOnly() || !hasChanges() ) {
    emit resourceSaved( this );
    return true;
  }

  if ( !mBlog ) {
    kError() << "ResourceBlog::addJournal(): Blog not initialised.";
    return false;
  }

  Incidence::List::Iterator i;
  Incidence::List added = addedIncidences();
  for ( i = added.begin(); i != added.end(); ++i ) {
    Journal* journal = dynamic_cast<Journal*>( *i );
    if ( !journal ) {
      return false;
    }
    KBlog::BlogPosting *posting = new KBlog::BlogPosting( *journal );
    connect ( mBlog, SIGNAL( createdPosting( KBlog::BlogPosting * ) ),
              this, SLOT( slotSavedPosting( KBlog::BlogPosting * ) ) );
    connect ( mBlog, SIGNAL( error( const KBlog::Blog::ErrorType &,
              const QString & ) ),
              this, SLOT( slotError( const KBlog::Blog::ErrorType &,
                          const QString & ) ) );
    mBlog->createPosting( posting );
    //FIXME: Fix memory leak on error
    kDebug() << "ResourceBlog::doSave(): adding " << journal->uid();
  }

  Incidence::List changed = changedIncidences();
  for( i = changed.begin(); i != changed.end(); ++i ) {
    Journal* journal = dynamic_cast<Journal*>( *i );
    if ( !journal ) {
      return false;
    }
    KBlog::BlogPosting *posting = new KBlog::BlogPosting( *journal );
    connect ( mBlog, SIGNAL( modifiedPosting( KBlog::BlogPosting * ) ),
              this, SLOT( slotSavedPosting( KBlog::BlogPosting * ) ) );
    connect ( mBlog, SIGNAL( error( const KBlog::Blog::ErrorType &,
              const QString & ) ),
              this, SLOT( slotError( const KBlog::Blog::ErrorType &,
                          const QString & ) ) );
    mBlog->modifyPosting( posting );
    //FIXME: Fix memory leak on error
    kDebug() << "ResourceBlog::doSave(): changing " << journal->uid();
  }

  Incidence::List deleted = deletedIncidences();
  for( i = deleted.begin(); i != deleted.end(); ++i ) {
    Journal* journal = dynamic_cast<Journal*>( *i );
    if ( !journal ) {
      return false;
    }
    KBlog::BlogPosting *posting = new KBlog::BlogPosting( *journal );
    connect ( mBlog, SIGNAL( removedPosting( KBlog::BlogPosting * ) ),
              this, SLOT( slotSavedPosting( KBlog::BlogPosting * ) ) );
    connect ( mBlog, SIGNAL( error( const KBlog::Blog::ErrorType &,
              const QString & ) ),
              this, SLOT( slotError( const KBlog::Blog::ErrorType &,
                          const QString & ) ) );
    mBlog->removePosting( posting );
    //FIXME: Fix memory leak on error
    kDebug() << "ResourceBlog::doSave(): removing " << journal->uid();
    //FIXME: Just a hack at the moment until we have slotRemovePosting
    clearChange( journal );
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
  kDebug() << "  URL: " << mUrl.url();
  kDebug() << "  Username: " << mUsername;
  kDebug() << "  API: " << API();
  kDebug() << "  ReloadPolicy: " << reloadPolicy();
  kDebug() << "  BlogID: " << mBlogID;
  kDebug() << "  BlogName: " << mBlogName;
  kDebug() << "  DownloadCount: " << mDownloadCount;
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
    connect ( blogger, SIGNAL( listedBlogs( const QMap<QString,QString> & ) ),
              this, SLOT( slotBlogInfoRetrieved(
                    const QMap<QString,QString> & ) ) );
    blogger->listBlogs();
    return true;
  }
  KBlog::GData* gdata = qobject_cast<KBlog::GData*>( mBlog );
  if ( gdata ) {
    connect ( gdata, SIGNAL( listedBlogs( const QMap<QString,QString> & ) ),
              this, SLOT( slotBlogInfoRetrieved(
                          const QMap<QString,QString> & ) ) );
    gdata->listBlogs();
    return true;
  }
  kError() << "ResourceBlog::listBlogs(): "
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
