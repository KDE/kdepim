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

#include <libkcal/icalformat.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/confirmsavedialog.h>

#include <qapplication.h>
#include <qdatetime.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qtimer.h>

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
  : ResourceCached( 0 ), mLock( true )
{
  init();

  mPrefs->addGroupPrefix( identifier() );
}

ResourceGroupwise::ResourceGroupwise( const KConfig *config )
  : ResourceCached( config ), mLock( true )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  if ( config ) readConfig( config );
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

void ResourceGroupwise::readConfig( const KConfig *config )
{
  kdDebug() << "KCal::ResourceGroupwise::readConfig()" << endl;

  mPrefs->readConfig();

  ResourceCached::readConfig( config );
}

void ResourceGroupwise::writeConfig( KConfig *config )
{
  kdDebug() << "KCal::ResourceGroupwise::writeConfig()" << endl;

  ResourceCalendar::writeConfig( config );

  mPrefs->writeConfig();

  ResourceCached::writeConfig( config );
}

bool ResourceGroupwise::doOpen()
{
  return true;
}

void ResourceGroupwise::doClose()
{
  ResourceCached::doClose();
}

bool ResourceGroupwise::doLoad()
{
  kdDebug() << "ResourceGroupwise::load()" << endl;

  if ( mIsShowingError ) {
    kdDebug() << "Still showing error" << endl;
    return true;
  }

  if ( mDownloadJob ) {
    kdDebug() << "Download still in progress" << endl;
    return true;
  }

  mCalendar.close();

  disableChangeNotification();
  loadCache();
  enableChangeNotification();

  emit resourceChanged( this );

  clearChanges();

  KURL url( prefs()->url() );
  if ( url.protocol() == "http" ) url.setProtocol( "groupwise" );
  else url.setProtocol( "groupwises" );
  url.setPath( "/calendar/" );
  url.setUser( prefs()->user() );
  url.setPass( prefs()->password() );

  kdDebug() << "Download URL: " << url << endl;

  mJobData = QString::null;

  mDownloadJob = KIO::get( url, false, false );
  connect( mDownloadJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotJobResult( KIO::Job * ) ) );
  connect( mDownloadJob, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
           SLOT( slotJobData( KIO::Job *, const QByteArray & ) ) );

  mProgress = KPIM::ProgressManager::instance()->createProgressItem(
    KPIM::ProgressManager::getUniqueID(), i18n("Downloading calendar") );
  connect( mProgress,
           SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
           SLOT( cancelLoad() ) );

  return true;
}

void ResourceGroupwise::slotJobResult( KIO::Job *job )
{
  kdDebug() << "ResourceGroupwise::slotJobResult(): " << endl;

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
      for( it = incidences.begin(); it != incidences.end(); ++it ) {
//        kdDebug() << "INCIDENCE: " << (*it)->summary() << endl;
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
    saveCache();
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
//  kdDebug() << "ResourceGroupwise::slotJobData()" << endl;

  mJobData.append( data.data() );
}

bool ResourceGroupwise::doSave()
{
  kdDebug() << "KCal::ResourceGroupwise::doSave()" << endl;

  saveCache();

  if ( !hasChanges() ) {
    kdDebug() << "No changes" << endl;
    return true;
  }

  if ( !confirmSave() ) return false;

  GroupwiseServer server( mPrefs->url(), mPrefs->user(), mPrefs->password(),
    0 );

  if ( !server.login() ) {
    kdError() << "Unable to login to server" << endl;
    emit resourceSaveError( this, i18n( "Unable to login to server: " ) + server.errorText() );
    return false;
  }

  Incidence::List::ConstIterator it;

  Incidence::List added = addedIncidences();
  for( it = added.begin(); it != added.end(); ++it ) {
    if ( server.addIncidence( *it, this ) ) {
      clearChange( *it );
      saveCache();
    }
  }
  Incidence::List changed = changedIncidences();
  for( it = changed.begin(); it != changed.end(); ++it ) {
    if ( server.changeIncidence( *it ) ) clearChange( *it );
  }
  Incidence::List deleted = deletedIncidences();
  for( it = deleted.begin(); it != deleted.end(); ++it ) {
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
  kdDebug() << "ResourceGroupwise::userSettings()" << endl;

  GroupwiseServer server( prefs()->url(),
                          prefs()->user(),
                          prefs()->password(), this );

  server.login();
  // get these out again, once we discover their format.
  bool success = server.readUserSettings( settings );
  server.logout();
  return success;
}

bool ResourceGroupwise::modifyUserSettings( QMap<QString, QString> & settings )
{
  kdDebug() << "ResourceGroupwise::modifyUserSettings()" << endl;

  if ( settings.isEmpty() ) 
  {
   kdDebug() << "ResourceGroupwise::modifyUserSettings(): no changed settings" << endl;
    return false;
  }


  GroupwiseServer server( prefs()->url(),
                          prefs()->user(),
                          prefs()->password(), this );

  server.login();
  // get these out again, once we discover their format.
  bool success = server.modifyUserSettings( settings );
  server.logout();
  return success;
}

#include "kcal_resourcegroupwise.moc"
