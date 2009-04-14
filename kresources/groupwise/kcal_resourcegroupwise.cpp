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

#include "kcal_resourcegroupwise.h"

#include "kcal_groupwiseprefsbase.h"
#include "kcal_resourcegroupwiseconfig.h"

#include "soap/groupwiseserver.h"

#include <kcal/icalformat.h>
#include <kcal/calendarlocal.h>
#include <kcal/confirmsavedialog.h>

#include <QApplication>
#include <QDateTime>
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

using namespace KCal;

ResourceGroupwise::ResourceGroupwise()
  : ResourceCached(), mLock( true )
{
  init();

  mPrefs->addGroupPrefix( identifier() );
}

ResourceGroupwise::ResourceGroupwise( const KConfigGroup &group )
  : ResourceCached( group ), mLock( true )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  readConfig( group );
}

ResourceGroupwise::~ResourceGroupwise()
{
  disableChangeNotification();

  delete mPrefs;
  mPrefs = 0;
}

void ResourceGroupwise::init()
{
  mDownloadJob = 0;
  mProgress = 0;

  mIsShowingError = false;

  mPrefs = new GroupwisePrefsBase();

  setType( "groupwise" );

  enableChangeNotification();
}

GroupwisePrefsBase *ResourceGroupwise::prefs()
{
  return mPrefs;
}

void ResourceGroupwise::readConfig( const KConfigGroup &group )
{
  kDebug() <<"KCal::ResourceGroupwise::readConfig()";

  mPrefs->readConfig();

  ResourceCached::readConfig( group );
}

void ResourceGroupwise::writeConfig( KConfigGroup &group )
{
  kDebug() <<"KCal::ResourceGroupwise::writeConfig()";

  ResourceCalendar::writeConfig( group );

  mPrefs->writeConfig();

  ResourceCached::writeConfig( group );
}

bool ResourceGroupwise::doOpen()
{
  return true;
}

void ResourceGroupwise::doClose()
{
  ResourceCached::doClose();
}

bool ResourceGroupwise::doLoad( bool )
{
  kDebug() <<"ResourceGroupwise::load()";

  if ( mIsShowingError ) {
    kDebug() <<"Still showing error";
    return true;
  }

  if ( mDownloadJob ) {
    kDebug() <<"Download still in progress";
    return true;
  }

  calendar()->close();

  disableChangeNotification();
  loadFromCache();
  enableChangeNotification();

  emit resourceChanged( this );

  clearChanges();

  KUrl url( prefs()->url() );
  if ( url.protocol() == "http" ) url.setProtocol( "groupwise" );
  else url.setProtocol( "groupwises" );
  url.setPath( "/calendar/" );
  url.setUser( prefs()->user() );
  url.setPass( prefs()->password() );

  kDebug() <<"Download URL:" << url;

  mJobData.clear();

  mDownloadJob = KIO::get( url, KIO::NoReload, KIO::HideProgressInfo );
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

void ResourceGroupwise::slotJobResult( KJob *job )
{
  kDebug() <<"ResourceGroupwise::slotJobResult():";

  if ( job->error() ) {
    mIsShowingError = true;
    loadError( job->errorString() );
    mIsShowingError = false;
  } else {
    disableChangeNotification();

    clearCache();

    // FIXME: This does not take into account the time zone!
    CalendarLocal calendar( QString::fromLatin1("UTC") );
    ICalFormat ical;
    if ( !ical.fromString( &calendar, mJobData ) ) {
      loadError( i18n("Error parsing calendar data.") );
    } else {
      Incidence::List incidences = calendar.incidences();
      Incidence::List::ConstIterator it;
      for( it = incidences.constBegin(); it != incidences.constEnd(); ++it ) {
//        kDebug() <<"INCIDENCE:" << (*it)->summary();
        Incidence *i = (*it)->clone();
        QString remote = (*it)->customProperty( "GWRESOURCE", "UID" );
        if ( remote.isEmpty() ) {
          kDebug() <<"INCIDENCE:" << (*it)->summary() << " HAS NO REMOTE UID, REJECTING!";
        } else {
          QString local = idMapper().localId( remote );
          if ( local.isEmpty() ) {
            idMapper().setRemoteId( i->uid(), remote );
          } else {
            i->setUid( local );
          }
          addIncidence( i );
        }
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

void ResourceGroupwise::slotJobData( KIO::Job *, const QByteArray &data )
{
//  kDebug() <<"ResourceGroupwise::slotJobData()";

  mJobData.append( data.data() );
}

bool ResourceGroupwise::doSave( bool )
{
  kDebug() <<"KCal::ResourceGroupwise::doSave()";

  saveToCache();

  if ( !hasChanges() ) {
    kDebug() <<"No changes";
    return true;
  }

  if ( !confirmSave() ) return false;

  GroupwiseServer server( mPrefs->url(), mPrefs->user(), mPrefs->password(),
    timeSpec(), 0 );

  if ( !server.login() ) {
    kError() <<"Unable to login to server" << server.error();
    emit resourceSaveError( this, i18n( "Unable to login to server: " ) +server.errorText() );
    return false;
  }

  Incidence::List::ConstIterator it;

  Incidence::List added = addedIncidences();
  for( it = added.constBegin(); it != added.constEnd(); ++it ) {
    if ( server.addIncidence( *it, this ) ) {
      clearChange( *it );
      saveToCache();
    }
  }
  Incidence::List changed = changedIncidences();
  for( it = changed.constBegin(); it != changed.constEnd(); ++it ) {
    if ( server.changeIncidence( *it ) ) clearChange( *it );
  }
  Incidence::List deleted = deletedIncidences();
  for( it = deleted.constBegin(); it != deleted.constEnd(); ++it ) {
    if ( server.deleteIncidence( *it ) ) clearChange( *it );
  }

  server.logout();

  return true;
}

// FIXME: Put this into ResourceCached
bool ResourceGroupwise::confirmSave()
{
  if ( !hasChanges() ) return true;

  ConfirmSaveDialog dlg( resourceName(), 0 );

  dlg.addIncidences( addedIncidences(), i18n("Added") );
  dlg.addIncidences( changedIncidences(), i18n("Changed") );
  dlg.addIncidences( deletedIncidences(), i18n("Deleted") );

  int result = dlg.exec();
  return result == QDialog::Accepted;
}

KABC::Lock *ResourceGroupwise::lock()
{
  return &mLock;
}

void ResourceGroupwise::cancelLoad()
{
  if ( mDownloadJob ) mDownloadJob->kill();
  mDownloadJob = 0;
  if ( mProgress ) mProgress->setComplete();
  mProgress = 0;
}

bool ResourceGroupwise::userSettings( ngwt__Settings *&settings )
{
  kDebug() <<"ResourceGroupwise::userSettings()";

  GroupwiseServer server( prefs()->url(),
                          prefs()->user(),
                          prefs()->password(), timeSpec(), this );

  server.login();
  // get these out again, once we discover their format.
  bool success = server.readUserSettings( settings );
  server.logout();
  return success;
}

bool ResourceGroupwise::modifyUserSettings( QMap<QString, QString> & settings )
{
  kDebug() <<"ResourceGroupwise::modifyUserSettings()";

  if ( settings.isEmpty() ) 
  {
   kDebug() <<"ResourceGroupwise::modifyUserSettings(): no changed settings";
    return false;
  }


  GroupwiseServer server( prefs()->url(),
                          prefs()->user(),
                          prefs()->password(),  timeSpec(), this );

  server.login();
  // get these out again, once we discover their format.
  bool success = server.modifyUserSettings( settings );
  server.logout();
  return success;
}

#include "kcal_resourcegroupwise.moc"
