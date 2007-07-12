/*
  This file is part of the blog resource.

  Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>
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

#include <typeinfo>
#include <stdlib.h>

#include <QDateTime>
#include <QString>
#include <q3ptrlist.h>

#include <kdebug.h>
#include <kurl.h>
#include <kio/job.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <kcal/incidence.h>
#include <kcal/event.h>
#include <kcal/todo.h>
#include <kcal/journal.h>

#include <kabc/locknull.h>

#include <kresources/configwidget.h>

#include <libkdepim/progressmanager.h>

#include <kblog/blogger.h>
#include <kblog/metaweblog.h>

#include "resourceblog.h"

using namespace KCal;

KCAL_RESOURCEBLOG_EXPORT ResourceBlog::ResourceBlog()
    : ResourceCached(), mUseProgressManager( true ), mUseCacheFile( true )
{
  init();
}

KCAL_RESOURCEBLOG_EXPORT ResourceBlog::ResourceBlog( const KConfigGroup &group )
    : ResourceCached( group ), mUseProgressManager( true ),
    mUseCacheFile( true )
{
  init();
  readConfig( group );
}

KCAL_RESOURCEBLOG_EXPORT ResourceBlog::ResourceBlog( const KUrl &url )
    : ResourceCached(), mUseProgressManager( false ), mUseCacheFile( false )
{
  init();
  mUrl = url;
}

ResourceBlog::~ResourceBlog()
{
  close();

  delete mLock;

  qDeleteAll( *mJournalList );
  mJournalList->clear();
  delete mJournalList;
}

void ResourceBlog::init()
{
  mProgress = 0;

  mAPI = 0;

  mJournalList = new Journal::List();

  setType( "blog" );

  mLock = new KABC::Lock( cacheFile() );

  enableChangeNotification();
}

void ResourceBlog::readConfig( const KConfigGroup &group )
{
  kDebug( 5800 ) << "ResourceBlog::readConfig()" << endl;

  QString url = group.readEntry( "Url" );
  mUrl = KUrl( url );
  mUser = group.readEntry( "User" );
  mPassword = group.readEntry( "Password" );
  QString api = group.readEntry( "API" );
  setAPI( api );

  ResourceCached::readConfig( group );
}

void ResourceBlog::writeConfig( KConfigGroup &group )
{
  kDebug( 5800 ) << "ResourceBlog::writeConfig()" << endl;

  group.writeEntry( "Url", mUrl.url() );
  group.writeEntry( "User", mUser );
  group.writeEntry( "Password", mPassword );
  group.writeEntry( "API", API() );

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

void ResourceBlog::setUser( const QString &user )
{
  mUser = user;
}

QString ResourceBlog::user() const
{
  return mUser;
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
  if ( API == "Blogger" ) {
    mAPI = new KBlog::APIBlogger( mUrl, this );
  } else if ( API == "MetaWeblog" ) {
    mAPI = new KBlog::APIMetaWeblog( mUrl, this );
  } else {
    kError() << "ResourceBlog::setAPI(): Unrecognised API: " << API << endl;
    return;
  }
  mAPI->setUsername( mUser );
  mAPI->setPassword( mPassword );
}

QString ResourceBlog::API() const
{
  if ( mAPI ) {
    if ( qobject_cast<KBlog::APIBlogger*>( mAPI ) ) {
      return "Blogger";
    }
    if ( qobject_cast<KBlog::APIMetaWeblog*>( mAPI ) ) {
      return "MetaWeblog";
    }
  }
  return 0;
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
  kDebug( 5800 ) << "ResourceBlog::load()" << endl;


  mCalendar.close();

  /*
  if ( mUseCacheFile ) {
    disableChangeNotification();
    loadFromCache();
    enableChangeNotification();
  }

  clearChanges();
  */

  emit resourceChanged( this );


  if ( mAPI ) {
    if ( mLock->lock() ) {
      kDebug( 5800 ) << "Downloading blog posts from: " << mUrl << endl;
      mAPI->setBlogId( "1" ); //FIXME Set the correct blogid
      mAPI->setDownloadCount( 0 ); // Download ALL the posts
      connect ( mAPI, SIGNAL( listedPosting( KBlog::BlogPosting & ) ),
                this, SLOT( slotListedPosting( KBlog::BlogPosting & ) ));
      connect ( mAPI, SIGNAL( listPostingsFinished() ),
                this, SLOT( slotListPostingsFinished() ));

      if ( mUseProgressManager ) {
        mProgress = KPIM::ProgressManager::createProgressItem(
            KPIM::ProgressManager::getUniqueID(),
            i18n("Downloading blog posts") );
        mProgress->setProgress( 0 );
      }

      mAPI->listPostings();
      return true;
    } else {
      kDebug( 5800 ) << "ResourceBlog::load(): cache file is locked"
          << " - something else must be loading the file" << endl;
    }
  }
  return false;
}

void ResourceBlog::slotPercent( KJob *, unsigned long percent )
{
  kDebug( 5800 ) << "ResourceBlog::slotPercent(): " << percent << endl;

  mProgress->setProgress( percent );
}

void ResourceBlog::slotListedPosting( KBlog::BlogPosting &blogPosting )
{
  kDebug( 5800 ) << "ResourceBlog::slotListedPosting()" << endl;
  Journal *journalBlog = new Journal();
  journalBlog->setUid("kblog-" + mUrl.url() + "-" + mUser + "-" +
      blogPosting.postingId() );
  journalBlog->setSummary(blogPosting.title());
  journalBlog->setDescription(blogPosting.content());
  journalBlog->setDtStart(blogPosting.creationDateTime());
  if (ResourceCached::addJournal( journalBlog )) {
    kDebug( 5800 ) << "ResourceBlog::slotListedPosting(): Journal added"
        << endl;
    *mJournalList << journalBlog;
  }
}

void ResourceBlog::slotListPostingsFinished()
{
  kDebug( 5800 ) << "ResourceBlog::slotListPostingsFinished()" << endl;

  emit resourceChanged( this );

  if ( mProgress ) {
    mProgress->setComplete();
    mProgress = 0;
  }

  mLock->unlock();
  emit resourceLoaded( this );
}


bool ResourceBlog::doSave( bool )
{
  kDebug( 5800 ) << "ResourceBlog::save()" << endl;

  if ( readOnly() || !hasChanges() ) {
    emit resourceSaved( this );
    return true;
  }

  //mChangedIncidences = allChanges();

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
  kDebug( 5800 ) << "  Url: " << mUrl.url() << endl;
  kDebug( 5800 ) << "  User: " << mUser << endl;
  kDebug( 5800 ) << "  API: " << API() << endl;
  kDebug( 5800 ) << "  ReloadPolicy: " << reloadPolicy() << endl;
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
  } else if ( key == "User" ) {
    setUser( value );
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
  kDebug( 5800 ) << "ResourceBlog::addJournal()" << endl;
  QString title = journal->summary();
  QString content = journal->description();
  KDateTime date = journal->dtStart();
  QStringList categories = journal->categories();
  KBlog::BlogPosting *post;
  post = new KBlog::BlogPosting( title, content );
  if ( mAPI ) {
    //TODO: Categories
    mAPI->setBlogId( "1" );
    post->setCreationDateTime( date );
    mAPI->createPosting( post );
    return true;
  }
  kError() << "ResourceBlog::addJournal(): Journal not initialised." << endl;
  return false;
}

#include "resourceblog.moc"
