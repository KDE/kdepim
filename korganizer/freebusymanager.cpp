/*
  This file is part of the Groupware/KOrganizer integration.

  Requires the Qt and KDE widget libraries, available at no cost at
  http://www.trolltech.com and http://www.kde.org respectively

  Copyright (c) 2002-2004 Klar�vdalens Datakonsult AB
        <info@klaralvdalens-datakonsult.se>
  Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA  02110-1301, USA.

  In addition, as a special exception, the copyright holders give
  permission to link the code of this program with any edition of
  the Qt library by Trolltech AS, Norway (or with modified versions
  of Qt that use the same license as Qt), and distribute linked
  combinations including the two.  You must obey the GNU General
  Public License in all respects for all of the code used other than
  Qt.  If you modify this file, you may extend this exception to
  your version of the file, but you are not obligated to do so.  If
  you do not wish to do so, delete this exception statement from
  your version.
*/

#include "freebusymanager.h"

#include "koprefs.h"
#include "mailscheduler.h"
#include "actionmanager.h"
#include "korganizer.h"

#include <libkcal/incidencebase.h>
#include <libkcal/attendee.h>
#include <libkcal/freebusy.h>
#include <libkcal/journal.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/icalformat.h>

#include <kio/job.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kio/jobclasses.h>
#include <kio/netaccess.h>
#include <kio/scheduler.h>
#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kabc/stdaddressbook.h>
#include <kabc/addressee.h>

#include <tqfile.h>
#include <tqbuffer.h>
#include <tqregexp.h>
#include <tqdir.h>

#define DEBUG_5850 kdDebug(5850)

using namespace KCal;

FreeBusyDownloadJob::FreeBusyDownloadJob( const TQString &email, const KURL &url,
                                          FreeBusyManager *manager,
                                          const char *name )
  : TQObject( manager, name ), mManager( manager ), mEmail( email )
{
  KIO::TransferJob *job = KIO::get( url, false, false );
  //pass the mainwindow to the job so any prompts are active
  KOrg::MainWindow *korg = ActionManager::findInstance( KURL() );
  job->setWindow( korg->topLevelWidget() );

  connect( job, TQT_SIGNAL( result( KIO::Job * ) ),
           TQT_SLOT( slotResult( KIO::Job * ) ) );
  connect( job, TQT_SIGNAL( data( KIO::Job *, const TQByteArray & ) ),
           TQT_SLOT( slotData( KIO::Job *, const TQByteArray & ) ) );
  KIO::Scheduler::scheduleJob( job );
}

FreeBusyDownloadJob::~FreeBusyDownloadJob()
{
}


void FreeBusyDownloadJob::slotData( KIO::Job *, const TQByteArray &data )
{
  TQByteArray tmp = data;
  tmp.resize( tmp.size() + 1 );
  tmp[tmp.size()-1] = 0;
  mFreeBusyData += tmp;
}

void FreeBusyDownloadJob::slotResult( KIO::Job *job )
{
  DEBUG_5850 << "FreeBusyDownloadJob::slotResult() " << mEmail << endl;

  if( job->error() ) {
    DEBUG_5850 << "FreeBusyDownloadJob::slotResult() job error for " << mEmail << endl;
    emit freeBusyDownloadError( mEmail );
  } else {
    FreeBusy *fb = mManager->iCalToFreeBusy( mFreeBusyData );
    if ( fb ) {
      Person p = fb->organizer();
      p.setEmail( mEmail );
      mManager->saveFreeBusy( fb, p );
    }
    emit freeBusyDownloaded( fb, mEmail );
  }
  deleteLater();
}

////

FreeBusyManager::FreeBusyManager( TQObject *parent, const char *name )
  : TQObject( parent, name ),
    mCalendar( 0 ), mTimerID( 0 ), mUploadingFreeBusy( false ),
    mBrokenUrl( false )
{
}

void FreeBusyManager::setCalendar( KCal::Calendar *c )
{
  mCalendar = c;
  if ( mCalendar ) {
    mFormat.setTimeZone( mCalendar->timeZoneId(), true );
  }
}

KCal::FreeBusy *FreeBusyManager::ownerFreeBusy()
{
  TQDateTime start = TQDateTime::currentDateTime();
  TQDateTime end = start.addDays( KOPrefs::instance()->mFreeBusyPublishDays );

  FreeBusy *freebusy = new FreeBusy( mCalendar, start, end );
  freebusy->setOrganizer( Person( KOPrefs::instance()->fullName(),
                          KOPrefs::instance()->email() ) );

  return freebusy;
}

TQString FreeBusyManager::ownerFreeBusyAsString()
{
  FreeBusy *freebusy = ownerFreeBusy();

  TQString result = freeBusyToIcal( freebusy );

  delete freebusy;

  return result;
}

TQString FreeBusyManager::freeBusyToIcal( KCal::FreeBusy *freebusy )
{
  return mFormat.createScheduleMessage( freebusy, Scheduler::Publish );
}

void FreeBusyManager::slotPerhapsUploadFB()
{
  // user has automatic uploading disabled, bail out
  if ( !KOPrefs::instance()->freeBusyPublishAuto() ||
       KOPrefs::instance()->freeBusyPublishUrl().isEmpty() )
     return;
  if( mTimerID != 0 )
    // A timer is already running, so we don't need to do anything
    return;

  int now = static_cast<int>( TQDateTime::currentDateTime().toTime_t() );
  int eta = static_cast<int>( mNextUploadTime.toTime_t() ) - now;

  if( !mUploadingFreeBusy ) {
    // Not currently uploading
    if( mNextUploadTime.isNull() ||
        TQDateTime::currentDateTime() > mNextUploadTime ) {
      // No uploading have been done in this session, or delay time is over
      publishFreeBusy();
      return;
    }

    // We're in the delay time and no timer is running. Start one
    if( eta <= 0 ) {
      // Sanity check failed - better do the upload
      publishFreeBusy();
      return;
    }
  } else {
    // We are currently uploading the FB list. Start the timer
    if( eta <= 0 ) {
      DEBUG_5850 << "This shouldn't happen! eta <= 0\n";
      eta = 10; // whatever
    }
  }

  // Start the timer
  mTimerID = startTimer( eta * 1000 );

  if( mTimerID == 0 )
    // startTimer failed - better do the upload
    publishFreeBusy();
}

// This is used for delayed Free/Busy list uploading
void FreeBusyManager::timerEvent( TQTimerEvent* )
{
  publishFreeBusy();
}

void FreeBusyManager::setBrokenUrl( bool isBroken )
{
  mBrokenUrl = isBroken;
}

/*!
  This method is called when the user has selected to publish its
  free/busy list or when the delay have passed.
*/
void FreeBusyManager::publishFreeBusy()
{
  // Already uploading? Skip this one then.
  if ( mUploadingFreeBusy )
    return;
  KURL targetURL( KOPrefs::instance()->freeBusyPublishUrl() );
  if ( targetURL.isEmpty() )  {
    KMessageBox::sorry( 0,
      i18n( "<qt>No URL configured for uploading your free/busy list. Please "
            "set it in KOrganizer's configuration dialog, on the \"Free/Busy\" page. "
            "<br>Contact your system administrator for the exact URL and the "
            "account details."
            "</qt>" ), i18n("No Free/Busy Upload URL") );
    return;
  }
  if ( mBrokenUrl ) // Url is invalid, don't try again
    return;
  if ( !targetURL.isValid() ) {
     KMessageBox::sorry( 0,
      i18n( "<qt>The target URL '%1' provided is invalid."
            "</qt>" ).arg( targetURL.prettyURL() ), i18n("Invalid URL") );
    mBrokenUrl = true;
    return;
  }

//   // Substitute %u and %d [FIXME]
//   TQString defaultEmail = KOCore()::self()->email();
//   int emailpos = defaultEmail.find( '@' );
//   if (emailpos != -1) {
//     const TQString emailName = defaultEmail.left( emailpos );
//     const TQString emailHost = defaultEmail.mid( emailpos + 1 );
//     targetURL = targetURL.url().replace("%25u", emailName, true);
//     targetURL = targetURL.url().replace("%25d", emailHost, true);
//   }
  targetURL.setUser( KOPrefs::instance()->mFreeBusyPublishUser );
  targetURL.setPass( KOPrefs::instance()->mFreeBusyPublishPassword );

  mUploadingFreeBusy = true;

  // If we have a timer running, it should be stopped now
  if( mTimerID != 0 ) {
    killTimer( mTimerID );
    mTimerID = 0;
  }

  // Save the time of the next free/busy uploading
  mNextUploadTime = TQDateTime::currentDateTime();
  if( KOPrefs::instance()->mFreeBusyPublishDelay > 0 )
    mNextUploadTime = mNextUploadTime.addSecs(
        KOPrefs::instance()->mFreeBusyPublishDelay * 60 );

  TQString messageText = ownerFreeBusyAsString();

  // We need to massage the list a bit so that Outlook understands
  // it.
  messageText = messageText.replace( TQRegExp( "ORGANIZER\\s*:MAILTO:" ),
                                     "ORGANIZER:" );

  // Create a local temp file and save the message to it
  KTempFile tempFile;
  TQTextStream *textStream = tempFile.textStream();
  if( textStream ) {
    *textStream << messageText;
    tempFile.close();

#if 0
    TQString defaultEmail = KOCore()::self()->email();
    TQString emailHost = defaultEmail.mid( defaultEmail.find( '@' ) + 1 );

    // Put target string together
    KURL targetURL;
    if( KOPrefs::instance()->mPublishKolab ) {
      // we use Kolab
      TQString server;
      if( KOPrefs::instance()->mPublishKolabServer == "%SERVER%" ||
	  KOPrefs::instance()->mPublishKolabServer.isEmpty() )
	server = emailHost;
      else
	server = KOPrefs::instance()->mPublishKolabServer;

      targetURL.setProtocol( "webdavs" );
      targetURL.setHost( server );

      TQString fbname = KOPrefs::instance()->mPublishUserName;
      int at = fbname.find('@');
      if( at > 1 && fbname.length() > (uint)at ) {
	fbname = fbname.left(at);
      }
      targetURL.setPath( "/freebusy/" + fbname + ".ifb" );
      targetURL.setUser( KOPrefs::instance()->mPublishUserName );
      targetURL.setPass( KOPrefs::instance()->mPublishPassword );
    } else {
      // we use something else
      targetURL = KOPrefs::instance()->mPublishAnyURL.replace( "%SERVER%",
                                                               emailHost );
      targetURL.setUser( KOPrefs::instance()->mPublishUserName );
      targetURL.setPass( KOPrefs::instance()->mPublishPassword );
    }
#endif


    KURL src;
    src.setPath( tempFile.name() );

    DEBUG_5850 << "FreeBusyManager::publishFreeBusy(): " << targetURL << endl;

    KIO::Job * job = KIO::file_copy( src, targetURL, -1,
                                     true /*overwrite*/,
                                     false /*don't resume*/,
                                     false /*don't show progress info*/ );
    //pass the mainwindow to the job so any prompts are active
    KOrg::MainWindow *korg = ActionManager::findInstance( KURL() );
    job->setWindow( korg->topLevelWidget() );

    connect( job, TQT_SIGNAL( result( KIO::Job * ) ),
             TQT_SLOT( slotUploadFreeBusyResult( KIO::Job * ) ) );
  }
}

void FreeBusyManager::slotUploadFreeBusyResult(KIO::Job *_job)
{
    KIO::FileCopyJob* job = static_cast<KIO::FileCopyJob *>(_job);
    if ( job->error() )
        KMessageBox::sorry( 0,
          i18n( "<qt>The software could not upload your free/busy list to the "
                "URL '%1'. There might be a problem with the access rights, or "
                "you specified an incorrect URL. The system said: <em>%2</em>."
                "<br>Please check the URL or contact your system administrator."
                "</qt>" ).arg( job->destURL().prettyURL() )
                         .arg( job->errorString() ) );
    // Delete temp file
    KURL src = job->srcURL();
    Q_ASSERT( src.isLocalFile() );
    if( src.isLocalFile() )
        TQFile::remove(src.path());
    mUploadingFreeBusy = false;
}

bool FreeBusyManager::retrieveFreeBusy( const TQString &email, bool forceDownload )
{
  DEBUG_5850 << "FreeBusyManager::retrieveFreeBusy(): " << email << endl;
  if ( email.isEmpty() ) return false;

  // Check for cached copy of free/busy list
  KCal::FreeBusy *fb = loadFreeBusy( email );
  if ( fb ) {
    emit freeBusyRetrieved( fb, email );
  }

  // Don't download free/busy if the user does not want it.
  if( !KOPrefs::instance()->mFreeBusyRetrieveAuto && !forceDownload) {
    slotFreeBusyDownloadError( email ); // fblist
    return false;
  }

  mRetrieveQueue.append( email );

  if ( mRetrieveQueue.count() > 1 ) return true;

  return processRetrieveQueue();
}

bool FreeBusyManager::processRetrieveQueue()
{
  if ( mRetrieveQueue.isEmpty() ) return true;

  TQString email = mRetrieveQueue.first();
  mRetrieveQueue.pop_front();

  KURL sourceURL = freeBusyUrl( email );

  kdDebug(5850) << "FreeBusyManager::processRetrieveQueue(): url: " << sourceURL << endl;

  if ( !sourceURL.isValid() ) {
    kdDebug(5850) << "Invalid FB URL\n";
    slotFreeBusyDownloadError( email );
    return false;
  }

  FreeBusyDownloadJob *job = new FreeBusyDownloadJob( email, sourceURL, this,
                                                      "freebusy_download_job" );
  connect( job, TQT_SIGNAL( freeBusyDownloaded( KCal::FreeBusy *,
                                            const TQString & ) ),
	   TQT_SIGNAL( freeBusyRetrieved( KCal::FreeBusy *, const TQString & ) ) );
  connect( job, TQT_SIGNAL( freeBusyDownloaded( KCal::FreeBusy *,
                                            const TQString & ) ),
           TQT_SLOT( processRetrieveQueue() ) );

  connect( job, TQT_SIGNAL( freeBusyDownloadError( const TQString& ) ),
           this, TQT_SLOT( slotFreeBusyDownloadError( const TQString& ) ) );

  return true;
}

void FreeBusyManager::slotFreeBusyDownloadError( const TQString& email )
{
  if( KOPrefs::instance()->thatIsMe( email ) ) {
    // We tried to download our own free-busy list from the net, but it failed
    // so use local version instead.
    // The reason we try to download even our own free-busy list is that
    // this allows to avoid showing as busy the folders that are "fb relevant for nobody"
    // like shared resources (meeting rooms etc.)
    DEBUG_5850 << "freebusy of owner, falling back to local list" << endl;
    emit freeBusyRetrieved( ownerFreeBusy(), email );
  }

}

void FreeBusyManager::cancelRetrieval()
{
  mRetrieveQueue.clear();
}

KURL replaceVariablesURL( const KURL &url, const TQString &email )
{
  TQString emailName, emailHost;
  int emailpos = email.find( '@' );
  if( emailpos >= 0 ) {
    emailName = email.left( emailpos );
    emailHost = email.mid( emailpos + 1 );
  }

  TQString saveStr = url.path();
  saveStr.replace( TQRegExp( "%[Ee][Mm][Aa][Ii][Ll]%" ), email );
  saveStr.replace( TQRegExp( "%[Nn][Aa][Mm][Ee]%" ), emailName );
  saveStr.replace( TQRegExp( "%[Ss][Ee][Rr][Vv][Ee][Rr]%" ), emailHost );

  KURL retUrl( url );
  retUrl.setPath( saveStr );
  return retUrl;
}

bool fbExists( const KURL &url )
{
  // We need this function because using KIO::NetAccess::exists()
  // is useless for the http and https protocols. And getting back
  // arbitrary data is also useless because a server can respond back
  // with a "no such document" page.  So we need smart checking.

  KIO::Job *job = KIO::get( url, false, false );
  TQByteArray data;
  if ( KIO::NetAccess::synchronousRun( job, 0, &data ) ) {
    TQString dataStr ( data );
    if ( dataStr.contains( "BEGIN:VCALENDAR" ) ) {
      return true;
    }
  }
  return false;
}

KURL FreeBusyManager::freeBusyUrl( const TQString &email )
{
  DEBUG_5850 << "FreeBusyManager::freeBusyUrl(): " << email << endl;

  // First check if there is a specific FB url for this email
  TQString configFile = locateLocal( "data", "korganizer/freebusyurls" );
  KConfig cfg( configFile );

  cfg.setGroup( email );
  TQString url = cfg.readEntry( "url" );
  if ( !url.isEmpty() ) {
    kdDebug(5850) << "found cached url: " << url << endl;
    KURL cachedURL( url );
    if ( KOPrefs::instance()->thatIsMe( email ) ) {
      cachedURL.setUser( KOPrefs::instance()->mFreeBusyRetrieveUser );
      cachedURL.setPass( KOPrefs::instance()->mFreeBusyRetrievePassword );
    }
    return replaceVariablesURL( cachedURL, email );
  }

  // Try with the url configurated by preferred email in kaddressbook
  KABC::Addressee::List list= KABC::StdAddressBook::self( true )->findByEmail( email );
  KABC::Addressee::List::Iterator it;
  TQString pref;
  for ( it = list.begin(); it != list.end(); ++it ) {
    pref = (*it).preferredEmail();
    if ( !pref.isEmpty() && pref != email ) {
      kdDebug(5850) << "FreeBusyManager::freeBusyUrl():"
                    << "Preferred email of " << email << " is " << pref << endl;
      cfg.setGroup( pref );
      url = cfg.readEntry ( "url" );
      if ( !url.isEmpty() ) {
        kdDebug(5850) << "FreeBusyManager::freeBusyUrl():"
                      << "Taken url from preferred email:" << url << endl;
        return replaceVariablesURL( KURL( url ), email );
      }
    }
  }
  // None found. Check if we do automatic FB retrieving then
  if ( !KOPrefs::instance()->mFreeBusyRetrieveAuto ) {
    kdDebug( 5850 ) << "no auto retrieving" << endl;
    // No, so no FB list here
    return KURL();
  }

  // Sanity check: Don't download if it's not a correct email
  // address (this also avoids downloading for "(empty email)").
  int emailpos = email.find( '@' );
  if( emailpos == -1 ) {
    return KURL();
  }

  // Cut off everything left of the @ sign to get the user name.
  const TQString emailName = email.left( emailpos );
  const TQString emailHost = email.mid( emailpos + 1 );

  // Build the URL
  KURL sourceURL;
  sourceURL = KOPrefs::instance()->mFreeBusyRetrieveUrl;

  if ( KOPrefs::instance()->mFreeBusyCheckHostname ) {
    // Don't try to fetch free/busy data for users not on the specified servers
    // This tests if the hostnames match, or one is a subset of the other
    const TQString hostDomain = sourceURL.host();
    if ( hostDomain != emailHost &&
         !hostDomain.endsWith( '.' + emailHost ) &&
         !emailHost.endsWith( '.' + hostDomain ) ) {
      // Host names do not match
      kdDebug(5850) << "Host '" << sourceURL.host() << "' doesn't match email '" << email << endl;
      return KURL();
    }
  }

  if ( sourceURL.url().contains( TQRegExp( "\\.[xiv]fb$" ) ) ) { // user specified a fullpath
    // do variable string replacements to the URL (MS Outlook style)
    KURL fullpathURL = replaceVariablesURL( sourceURL, email );

    // This should work with anything thrown at it, not just Kolab
    // Notice that Kolab URLs are just entered as the base address, e.g. http://server.com/mykolab/
    // This means that if the trailing slash is not entered, we can treat this as a custom, non-Kolab URL!
    // In that case, just pass it on through with substitution for %u and %d
    // TODO: May want an explicit configuration option in kogroupwareprefspage.ui for this
    if ((fullpathURL.url().endsWith("/", true) == false) || (fullpathURL.url().contains("%25u", true)) || (fullpathURL.url().contains("%25d", true))) {
      // A generic URL, substitute %u and %d
      fullpathURL = fullpathURL.url().replace("%25u", emailName, true);
      fullpathURL = fullpathURL.url().replace("%25d", emailHost, true);
    }
    else {
      // This is (probably) a Kolab URL!
    }

    // set the User and Password part of the URL
    fullpathURL.setUser( KOPrefs::instance()->mFreeBusyRetrieveUser );
    fullpathURL.setPass( KOPrefs::instance()->mFreeBusyRetrievePassword );

    // no need to cache this URL as this is pretty fast to get from the config value.

    // return the fullpath URL
    return fullpathURL;
  }

  // else we search for a fb file in the specified URL with known possible extensions

  TQStringList extensions;
  extensions << "xfb" << "ifb" << "vfb";
  TQStringList::ConstIterator ext;
  for ( ext = extensions.constBegin(); ext != extensions.constEnd(); ++ext ) {
    // build a url for this extension
    sourceURL = KOPrefs::instance()->mFreeBusyRetrieveUrl;
    KURL dirURL = replaceVariablesURL( sourceURL, email );
    if ( KOPrefs::instance()->mFreeBusyFullDomainRetrieval ) {
      dirURL.addPath( email + '.' + (*ext) );
    } else {
      dirURL.addPath( emailName + '.' + (*ext ) );
    }
    dirURL.setUser( KOPrefs::instance()->mFreeBusyRetrieveUser );
    dirURL.setPass( KOPrefs::instance()->mFreeBusyRetrievePassword );
    if ( fbExists( dirURL ) ) {
      // write the URL to the cache
      cfg.setGroup( email );
      cfg.writeEntry( "url", dirURL.prettyURL() ); // prettyURL() does not write user nor password
      return dirURL;
    }
  }

  return KURL();
}

KCal::FreeBusy *FreeBusyManager::iCalToFreeBusy( const TQCString &data )
{
  kdDebug(5850) << "FreeBusyManager::iCalToFreeBusy()" << endl;
  kdDebug(5850) << data << endl;

  TQString freeBusyVCal = TQString::fromUtf8( data );
  KCal::FreeBusy *fb = mFormat.parseFreeBusy( freeBusyVCal );
  if ( !fb ) {
    kdDebug(5850) << "FreeBusyManager::iCalToFreeBusy(): Error parsing free/busy"
              << endl;
    kdDebug(5850) << freeBusyVCal << endl;
  }
  return fb;
}

TQString FreeBusyManager::freeBusyDir()
{
  return locateLocal( "data", "korganizer/freebusy" );
}

FreeBusy *FreeBusyManager::loadFreeBusy( const TQString &email )
{
  DEBUG_5850 << "FreeBusyManager::loadFreeBusy(): " << email << endl;

  TQString fbd = freeBusyDir();

  TQFile f( fbd + "/" + email + ".ifb" );
  if ( !f.exists() ) {
    DEBUG_5850 << "FreeBusyManager::loadFreeBusy() " << f.name()
                  << " doesn't exist." << endl;
    return 0;
  }

  if ( !f.open( IO_ReadOnly ) ) {
    DEBUG_5850 << "FreeBusyManager::loadFreeBusy() Unable to open file "
              << f.name() << endl;
    return 0;
  }

  TQTextStream ts( &f );
  TQString str = ts.read();

  return iCalToFreeBusy( str.utf8() );
}

bool FreeBusyManager::saveFreeBusy( FreeBusy *freebusy, const Person &person )
{
  DEBUG_5850 << "FreeBusyManager::saveFreeBusy(): " << person.fullName() << endl;

  TQString fbd = freeBusyDir();

  TQDir freeBusyDirectory( fbd );
  if ( !freeBusyDirectory.exists() ) {
    DEBUG_5850 << "Directory " << fbd << " does not exist!" << endl;
    DEBUG_5850 << "Creating directory: " << fbd << endl;

    if( !freeBusyDirectory.mkdir( fbd, true ) ) {
      DEBUG_5850 << "Could not create directory: " << fbd << endl;
      return false;
    }
  }

  TQString filename( fbd );
  filename += "/";
  filename += person.email();
  filename += ".ifb";
  TQFile f( filename );

  DEBUG_5850 << "FreeBusyManager::saveFreeBusy(): filename: " << filename
            << endl;

  freebusy->clearAttendees();
  freebusy->setOrganizer( person );

  TQString messageText = mFormat.createScheduleMessage( freebusy,
                                                       Scheduler::Publish );

  if ( !f.open( IO_ReadWrite ) ) {
    DEBUG_5850 << "acceptFreeBusy: Can't open:" << filename << " for writing"
              << endl;
    return false;
  }
  TQTextStream t( &f );
  t << messageText;
  f.close();

  return true;
}

#include "freebusymanager.moc"
