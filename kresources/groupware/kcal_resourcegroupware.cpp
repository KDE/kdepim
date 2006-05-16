/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "kcal_resourcegroupware.h"

#include "kcal_groupwareprefsbase.h"
#include "kcal_resourcegroupwareconfig.h"

#include <libkcal/confirmsavedialog.h>
#include <libkcal/icalformat.h>
#include <libkcal/calendarlocal.h>

#include <QApplication>
#include <QDateTime>
#include <Q3PtrList>
#include <QStringList>
#include <QTimer>

#include <kabc/locknull.h>
#include <kdebug.h>
#include <klocale.h>
#include <kresources/configwidget.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <kurl.h>
#include <libkdepim/kpimprefs.h>
#include <libkdepim/groupwarejob.h>

using namespace KCal;

ResourceGroupware::ResourceGroupware()
  : ResourceCached( 0 ), mLock( true ),
    mProgress( 0 )
{
  init();

  mPrefs->addGroupPrefix( identifier() );
}

ResourceGroupware::ResourceGroupware( const KConfig *config )
  : ResourceCached( config ), mLock( true )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  if ( config ) readConfig( config );
}

ResourceGroupware::~ResourceGroupware()
{
  disableChangeNotification();

  delete mPrefs;
  mPrefs = 0;
}

void ResourceGroupware::init()
{
  mDownloadJob = 0;
  mProgress = 0;
  
  mIsShowingError = false;

  mPrefs = new GroupwarePrefsBase();
  
  setType( "groupware" );

  enableChangeNotification();
}

GroupwarePrefsBase *ResourceGroupware::prefs()
{
  return mPrefs;
}

void ResourceGroupware::readConfig( const KConfig *config )
{
  kDebug() << "KCal::ResourceGroupware::readConfig()" << endl;

  mPrefs->readConfig();

  ResourceCached::readConfig( config );
}

void ResourceGroupware::writeConfig( KConfig *config )
{
  kDebug() << "KCal::ResourceGroupware::writeConfig()" << endl;

  ResourceCalendar::writeConfig( config );

  mPrefs->writeConfig();

  ResourceCached::writeConfig( config );
}

bool ResourceGroupware::doOpen()
{
  return true;
}

void ResourceGroupware::doClose()
{
  ResourceCached::doClose();
}

bool ResourceGroupware::doLoad()
{
  kDebug() << "ResourceGroupware::load()" << endl;

  if ( mIsShowingError ) {
    kDebug() << "Still showing error" << endl;
    return true;
  }

  if ( mDownloadJob ) {
    kWarning() << "Download still in progress" << endl;
    return false;
  }

  mCalendar.close();

  disableChangeNotification();
  loadFromCache();
  enableChangeNotification();

  emit resourceChanged( this );

  clearChanges();

  KUrl url( prefs()->url() );
  url.setUser( prefs()->user() );
  url.setPass( prefs()->password() );

  kDebug() << "Download URL: " << url << endl;

  mJobData.clear();

  mDownloadJob = KPIM::GroupwareJob::getCalendar( url );
  connect( mDownloadJob, SIGNAL( result( KJob * ) ),
           SLOT( slotJobResult( KJob * ) ) );
  connect( mDownloadJob, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
           SLOT( slotJobData( KIO::Job *, const QByteArray & ) ) );

  mProgress = KPIM::ProgressManager::instance()->createProgressItem(
    KPIM::ProgressManager::getUniqueID(), i18n("Downloading calendar") );
  connect( mProgress,
           SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
           SLOT( cancelLoad() ) );

  return true;
}

void ResourceGroupware::slotJobResult( KJob *job )
{
  kDebug() << "ResourceGroupware::slotJobResult(): " << endl;

  if ( job->error() ) {
    mIsShowingError = true;
    loadError( job->errorString() );
    mIsShowingError = false;
  } else {
    disableChangeNotification();

    clearCache();

     // FIXME: This does not take into account the time zone!
    CalendarLocal calendar;
    ICalFormat ical;
    if ( !ical.fromString( &calendar, mJobData ) ) {
      loadError( i18n("Error parsing calendar data.") );
    } else {
      Incidence::List incidences = calendar.incidences();
      Incidence::List::ConstIterator it;
      for( it = incidences.begin(); it != incidences.end(); ++it ) {
//        kDebug() << "INCIDENCE: " << (*it)->summary() << endl;
        Incidence *i = (*it)->clone();
        QString remote = (*it)->customProperty( "GWRESOURCE", "UID" );
        QString local = idMapper().localId( remote );
        if ( local.isEmpty() ) {
          idMapper().setRemoteId( i->uid(), remote );
        } else {
          i->setUid( local );
        }
        addIncidence( i );
      }
    }
    saveToCache();
    enableChangeNotification();

    clearChanges();

    emit resourceChanged( this );
    emit resourceLoaded( this );
  }

  mDownloadJob = 0;
  if ( mProgress ) mProgress->setComplete();
  mProgress = 0;
}

void ResourceGroupware::slotJobData( KIO::Job *, const QByteArray &data )
{
//  kDebug() << "ResourceGroupware::slotJobData()" << endl;

  mJobData.append( data.data() );
}

void ResourceGroupware::loadFinished()
{
  saveToCache();
  enableChangeNotification();

  emit resourceChanged( this );
  emit resourceLoaded( this );
}

bool ResourceGroupware::doSave()
{
  kDebug() << "KCal::ResourceGroupware::doSave()" << endl;

  saveToCache();

  if ( !hasChanges() ) {
    kDebug() << "No changes" << endl;
    return true;
  }
  
  if ( !confirmSave() ) return false;

#if 0
  Incidence::List::ConstIterator it;

  Incidence::List added = addedIncidences();
  for( it = added.begin(); it != added.end(); ++it ) {
    if ( mServer->addIncidence( *it, this ) ) {
      clearChange( *it );
      saveToCache();
    }
  }
  Incidence::List changed = changedIncidences();
  for( it = changed.begin(); it != changed.end(); ++it ) {
    if ( mServer->changeIncidence( *it ) ) clearChange( *it );
  }
  Incidence::List deleted = deletedIncidences();
  for( it = deleted.begin(); it != deleted.end(); ++it ) {
    if ( mServer->deleteIncidence( *it ) ) clearChange( *it );
  }
#endif

  return true;
}

// FIXME: Put this into ResourceCached
bool ResourceGroupware::confirmSave()
{
  if ( !hasChanges() ) return true;

  ConfirmSaveDialog dlg( resourceName(), 0 );

  dlg.addIncidences( addedIncidences(), i18n("Added") );
  dlg.addIncidences( changedIncidences(), i18n("Changed") );
  dlg.addIncidences( deletedIncidences(), i18n("Deleted") );

  int result = dlg.exec();
  return result == QDialog::Accepted;
}

KABC::Lock *ResourceGroupware::lock()
{
  return &mLock;
}

void ResourceGroupware::cancelLoad()
{
  if ( mDownloadJob ) mDownloadJob->kill();
  mDownloadJob = 0;
  if ( mProgress ) mProgress->setComplete();
  mProgress = 0;
}

#include "kcal_resourcegroupware.moc"
