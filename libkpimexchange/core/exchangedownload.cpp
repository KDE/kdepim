/*
    This file is part of libkpimexchange
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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

#include <qfile.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <qcstring.h>
#include <qregexp.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kaction.h>
#include <kurl.h>
#include <kdebug.h>
#include <krfcdate.h>

#include <kio/slave.h>
#include <kio/scheduler.h>
#include <kio/slavebase.h>
#include <kio/davjob.h>
#include <kio/http.h>
#include <kio/job.h>

#include <libkcal/incidence.h>
#include <libkcal/event.h>
#include <libkcal/recurrence.h>
#include <libkcal/icalformat.h>
#include <libkcal/icalformatimpl.h>
#include <libkcal/calendarlocal.h>

extern "C" {
  #include <ical.h>
}

#include "exchangeclient.h"
#include "exchangeaccount.h"
#include "exchangeprogress.h"
#include "utils.h"

#include "exchangedownload.h"

using namespace KPIM;

ExchangeDownload::ExchangeDownload( ExchangeAccount *account, QWidget *window )
  : mWindow( window )
{
  kdDebug() << "ExchangeDownload()" << endl;

  mAccount = account;
  mDownloadsBusy = 0;
  mProgress = 0;
  mCalendar = 0;
  mFormat = new KCal::ICalFormat();
}

ExchangeDownload::~ExchangeDownload()
{
  kdDebug() << "ExchangeDownload destructor" << endl;
  delete mFormat;
  if ( mEvents ) delete mEvents;
}

void ExchangeDownload::download( KCal::Calendar *calendar, const QDate &start,
                                 const QDate &end, bool showProgress )
{
  mCalendar = calendar;
  mEvents = 0;

  if( showProgress ) {
#if 0
    //kdDebug() << "Creating progress dialog" << endl;
    mProgress = new ExchangeProgress();
    mProgress->show();
  
    connect( this, SIGNAL( startDownload() ), mProgress,
             SLOT( slotTransferStarted() ) );
    connect( this, SIGNAL(finishDownload() ), mProgress,
             SLOT( slotTransferFinished() ) );
#endif
  }

  QString sql = dateSelectQuery( start, end.addDays( 1 ) );
 
  kdDebug() << "Exchange download query: " << endl << sql << endl;

  increaseDownloads();

  kdDebug() << "ExchangeDownload::download() davSearch URL: "
            << mAccount->calendarURL() << endl;

  KIO::DavJob *job = KIO::davSearch( mAccount->calendarURL(), "DAV:", "sql",
                                     sql, false );
  KIO::Scheduler::scheduleJob( job );
  job->setWindow( mWindow );
  connect( job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotSearchResult( KIO::Job *) ) );
}

void ExchangeDownload::download( const QDate& start, const QDate& end, bool showProgress )
{
  mCalendar = 0;
  mEvents = new QPtrList<KCal::Event>;

  if( showProgress ) {
    //kdDebug() << "Creating progress dialog" << endl;
    mProgress = new ExchangeProgress();
    mProgress->show();
  
    connect( this, SIGNAL(startDownload()), mProgress, SLOT(slotTransferStarted()) );
    connect( this, SIGNAL(finishDownload()), mProgress, SLOT(slotTransferFinished()) );
  }

  QString sql = dateSelectQuery( start, end.addDays( 1 ) );
 
  increaseDownloads();

  KIO::DavJob *job = KIO::davSearch( mAccount->calendarURL(), "DAV:", "sql", sql, false );
  KIO::Scheduler::scheduleJob(job);
  job->setWindow( mWindow );
  connect( job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotSearchResult( KIO::Job * ) ) );
}

// Original query TODO: make query configurable
QString ExchangeDownload::dateSelectQuery( const QDate& start, const QDate& end )
{
  QString startString;
  startString.sprintf("%04i/%02i/%02i",start.year(),start.month(),start.day());
  QString endString;
  endString.sprintf("%04i/%02i/%02i",end.year(),end.month(),end.day());
  QString sql = 
        "SELECT \"DAV:href\", \"urn:schemas:calendar:instancetype\", \"urn:schemas:calendar:uid\"\r\n"
        "FROM Scope('shallow traversal of \"\"')\r\n"
        "WHERE \"urn:schemas:calendar:dtend\" > '" + startString + "'\r\n"
        "AND \"urn:schemas:calendar:dtstart\" < '" + endString + "'";
  return sql;
}

#if 0
// That's the "new" code that breaks with Exchange. It was meant for Opengroupware, but that got its own resource anyway
QString ExchangeDownload::dateSelectQuery( const QDate& start, const QDate& end )
{
  QString startString;
  startString.sprintf( "%04i-%02i-%02iT00:00:00Z", start.year(),
                       start.month(), start.day() );
  QString endString;
  endString.sprintf( "%04i-%02i-%02iT23:59:59Z", end.year(), end.month(),
                     end.day() );
  QString sql = 
        "SELECT \"DAV:href\", \"urn:schemas:calendar:instancetype\", "
        "\"urn:schemas:calendar:uid\"\r\n"
        "FROM Scope('shallow traversal of \"\"')\r\n"
        "WHERE \"urn:schemas:calendar:dtend\" > '" + startString + "'\r\n"
        "AND \"urn:schemas:calendar:dtstart\" < '" + endString + "'";
  return sql;
}
#endif

void ExchangeDownload::slotSearchResult( KIO::Job *job )
{
  if ( job->error() ) {
    kdError() << "ExchangeDownload::slotSearchResult() error: "
              << job->error() << endl;
    QString text = i18n("ExchangeDownload\nError accessing '%1': %2")
                   .arg( mAccount->calendarURL().prettyURL() )
                   .arg( job->errorString() );
    KMessageBox::error( 0, text );
    finishUp( ExchangeClient::CommunicationError, job );
    return;
  }
  QDomDocument &response = static_cast<KIO::DavJob *>( job )->response();

  kdDebug() << "Search result: " << endl << response.toString() << endl;

  handleAppointments( response, true );
  
  decreaseDownloads();
}

void ExchangeDownload::slotMasterResult( KIO::Job *job )
{
  if ( job->error() ) {
    kdError() << "Error result for Master search: " << job->error() << endl;
    job->showErrorDialog( 0 );
    finishUp( ExchangeClient::CommunicationError, job );
    return;
  }
  QDomDocument &response = static_cast<KIO::DavJob *>( job )->response();

  kdDebug() << "Search (master) result: " << endl << response.toString() << endl;

  handleAppointments( response, false );
  
  decreaseDownloads();
}

void ExchangeDownload::handleAppointments( const QDomDocument &response,
                                           bool recurrence )
{
  kdDebug() << "Entering handleAppointments" << endl;
  int successCount = 0;

  if ( response.documentElement().firstChild().toElement().isNull() ) {
    // Got an empty response, but no error. This would mean there are
    // no appointments in this time period.
    return;
  }

  for( QDomElement item = response.documentElement().firstChild().toElement();
       !item.isNull();
       item = item.nextSibling().toElement() ) {
    //kdDebug() << "Current item:" << item.tagName() << endl;
    QDomNodeList propstats = item.elementsByTagNameNS( "DAV:", "propstat" );
    // kdDebug() << "Item has " << propstats.count() << " propstat children" << endl; 
    for( uint i=0; i < propstats.count(); i++ ) {
      QDomElement propstat = propstats.item(i).toElement();
      QDomElement prop = propstat.namedItem( "prop" ).toElement();
      if ( prop.isNull() ) {
        kdError() << "Error: no <prop> in response" << endl;
	continue;
      }

      QDomElement instancetypeElement = prop.namedItem( "instancetype" ).toElement();
      if ( instancetypeElement.isNull() ) {
        kdError() << "Error: no instance type in Exchange server reply" << endl;
        continue;
      }
      int instanceType = instancetypeElement.text().toInt();
      //kdDebug() << "Instance type: " << instanceType << endl;
    
      if ( recurrence && instanceType > 0 ) {
        QDomElement uidElement = prop.namedItem( "uid" ).toElement();
        if ( uidElement.isNull() ) {
          kdError() << "Error: no uid in Exchange server reply" << endl;
          continue;
        }
        QString uid = uidElement.text();
        if ( ! m_uids.contains( uid ) ) {
          m_uids[uid] = 1;
          handleRecurrence(uid);
          successCount++;
        }
        continue;
      }

      QDomElement hrefElement = prop.namedItem( "href" ).toElement();
      if ( hrefElement.isNull() ) {
        kdError() << "Error: no href in Exchange server reply" << endl;
        continue;
      }
      QString href = hrefElement.text();
      KURL url(href);
      
      kdDebug() << "Getting appointment from url: " << url.prettyURL() << endl;
      
      readAppointment( toDAV( url ) );
      successCount++;
    }
  }
  if ( !successCount ) {
    finishUp( ExchangeClient::ServerResponseError,
              "WebDAV SEARCH response:\n" + response.toString() );
  }
}

void ExchangeDownload::handleRecurrence( QString uid )
{
  // kdDebug() << "Handling recurrence info for uid=" << uid << endl;
  QString query = 
        "SELECT \"DAV:href\", \"urn:schemas:calendar:instancetype\"\r\n"
        "FROM Scope('shallow traversal of \"\"')\r\n"
        "WHERE \"urn:schemas:calendar:uid\" = '" + uid + "'\r\n"
	" AND (\"urn:schemas:calendar:instancetype\" = 1)\r\n";
//	"      OR \"urn:schemas:calendar:instancetype\" = 3)\r\n" // FIXME: exception are not handled

  // kdDebug() << "Exchange master query: " << endl << query << endl;

  increaseDownloads();
 
  KIO::DavJob* job = KIO::davSearch( mAccount->calendarURL(), "DAV:", "sql",
                                     query, false );
  KIO::Scheduler::scheduleJob( job );
  job->setWindow( mWindow );
  connect( job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotMasterResult( KIO::Job * ) ) );
}

void ExchangeDownload::readAppointment( const KURL& url )
{
  QDomDocument doc;
  QDomElement root = addElement( doc, doc, "DAV:", "propfind" );
  QDomElement prop = addElement( doc, root, "DAV:", "prop" );
  addElement( doc, prop, "urn:schemas:calendar:", "uid" );
  addElement( doc, prop, "urn:schemas:calendar:", "timezoneid" );
  addElement( doc, prop, "urn:schemas:calendar:", "timezone" );
  addElement( doc, prop, "urn:schemas:calendar:", "lastmodified" );
  addElement( doc, prop, "urn:schemas:calendar:", "organizer" );
  addElement( doc, prop, "urn:schemas:calendar:", "contact" );
  addElement( doc, prop, "urn:schemas:httpmail:", "to" );
  addElement( doc, prop, "urn:schemas:calendar:", "attendeestatus" );
  addElement( doc, prop, "urn:schemas:calendar:", "attendeerole" );
  addElement( doc, prop, "DAV:", "isreadonly" );
  addElement( doc, prop, "urn:schemas:calendar:", "instancetype" );
  addElement( doc, prop, "urn:schemas:calendar:", "created" );
  addElement( doc, prop, "urn:schemas:calendar:", "dtstart" );
  addElement( doc, prop, "urn:schemas:calendar:", "dtend" );
  addElement( doc, prop, "urn:schemas:calendar:", "alldayevent" );
  addElement( doc, prop, "urn:schemas:calendar:", "transparent" );
  addElement( doc, prop, "urn:schemas:httpmail:", "textdescription" );
  addElement( doc, prop, "urn:schemas:httpmail:", "subject" );
  addElement( doc, prop, "urn:schemas:calendar:", "location" );
  addElement( doc, prop, "urn:schemas:calendar:", "rrule" );
  addElement( doc, prop, "urn:schemas:calendar:", "exdate" );
  addElement( doc, prop, "urn:schemas:mailheader:", "sensitivity" );
  addElement( doc, prop, "urn:schemas:calendar:", "reminderoffset" );
  
  addElement( doc, prop, "urn:schemas-microsoft-com:office:office",
              "Keywords" );

//  addElement( doc, prop, "", "" );
//  addElement( doc, prop, "DAV:", "" );
//  addElement( doc, prop, "urn:schemas:calendar:", "" );
//  addElement( doc, prop, "urn:content-classes:appointment", "" );
//  addElement( doc, prop, "urn:schemas:httpmail:", "" );

  increaseDownloads();

  KIO::DavJob* job = KIO::davPropFind( url, doc, "0", false );
  KIO::Scheduler::scheduleJob( job );
  job->setWindow( mWindow );
  job->addMetaData( "errorPage", "false" );
  connect( job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotPropFindResult( KIO::Job * ) ) );
}

void ExchangeDownload::slotPropFindResult( KIO::Job *job )
{
  kdDebug() << "slotPropFindResult" << endl;

  int error = job->error(); 
  if ( error ) {
    job->showErrorDialog( 0 );
    finishUp( ExchangeClient::CommunicationError, job );
    return;
  }

  QDomDocument response = static_cast<KIO::DavJob *>( job )->response();
  kdDebug() << "Response: " << endl;
  kdDebug() << response.toString() << endl;

  QDomElement prop = response.documentElement().namedItem( "response" )
                     .namedItem( "propstat" ).namedItem( "prop" ).toElement();
 
  KCal::Event* event = new KCal::Event();

  QDomElement uidElement = prop.namedItem( "uid" ).toElement();
  if ( uidElement.isNull() ) {
    kdError() << "Error: no uid in Exchange server reply" << endl;
    finishUp( ExchangeClient::IllegalAppointmentError,
              "WebDAV server response:\n" + response.toString() );
    return;
  }
  event->setUid( uidElement.text() );
  // kdDebug() << "Got UID: " << uidElement.text() << endl;

  QString timezoneid = prop.namedItem( "timezoneid" ).toElement().text();
  // kdDebug() << "DEBUG: timezoneid = " << timezoneid << endl;

  QString timezone = prop.namedItem( "timezone" ).toElement().text();
  // kdDebug() << "DEBUG: timezone = " << timezone << endl;

  // mFormat is used for parsing recurrence rules.
  QString localTimeZoneId;
  if ( mCalendar ) {
    mFormat->setTimeZone( mCalendar->timeZoneId(), !mCalendar->isLocalTime() );
    localTimeZoneId = mCalendar->timeZoneId();
  }  else {
    localTimeZoneId = "UTC";
    // If no mCalendar, stay in UTC
  }

  QString lastModified = prop.namedItem( "lastmodified" ).toElement().text();
  if ( !lastModified.isEmpty() ) {
    QDateTime dt = utcAsZone( QDateTime::fromString( lastModified, Qt::ISODate ), localTimeZoneId );
    event->setLastModified( dt );
    kdDebug() << "Got lastModified:" << lastModified << ", " << dt.toString() << endl;
  }

  QString organizer = prop.namedItem( "organizer" ).toElement().text();
  // TODO: Does outlook have a common name? Or does the organizer already contain both?
  event->setOrganizer( organizer );
  // kdDebug() << "Got organizer: " << organizer << endl;

  // Trying to find attendees, not working yet
  QString contact = prop.namedItem( "contact" ).toElement().text();
//  event->setOrganizer( organizer );
  // kdDebug() << "DEBUG: Got contact: " << contact << endl;

  // This looks promising for finding attendees
  // FIXME: get this to work
  QString to = prop.namedItem( "to" ).toElement().text();
  // kdDebug() << "DEBUG: Got to: " << to << endl;
  QStringList attn = QStringList::split( ",", to ); // This doesn't work: there can be commas between ""
  QStringList::iterator it;
  for ( it = attn.begin(); it != attn.end(); ++it ) {
    // kdDebug() << "    attendee: " << (*it) << endl;
    QString name = "";
    // KCal::Attendee* a = new KCal::Attendee( name, email );

    // event->addAttendee( a );
  }

  QString readonly = prop.namedItem( "isreadonly" ).toElement().text();
  event->setReadOnly( readonly == "1" );
  kdDebug() << "Got readonly: " << readonly << ":" << (readonly != "0") << endl;

  QString created = prop.namedItem( "created" ).toElement().text();
  if ( !created.isEmpty() ) {
    QDateTime dt = utcAsZone( QDateTime::fromString( created, Qt::ISODate ),
                    localTimeZoneId );
    event->setCreated( dt );
    kdDebug() << "got created: " << dt.toString() << endl;
  }

  QString dtstart = prop.namedItem( "dtstart" ).toElement().text();
  if ( !dtstart.isEmpty() ) {
    QDateTime dt = utcAsZone( QDateTime::fromString( dtstart, Qt::ISODate ),
                              localTimeZoneId );
    event->setDtStart( dt );
    kdDebug() << "got dtstart: " << dtstart << " becomes in timezone " << dt.toString() << endl;
  }

  QString alldayevent = prop.namedItem( "alldayevent" ).toElement().text();
  bool floats = alldayevent.toInt() != 0;
  event->setFloats( floats );
  kdDebug() << "Got alldayevent: \"" << alldayevent << "\":" << floats << endl;

  QString dtend = prop.namedItem( "dtend" ).toElement().text();
  if ( !dtend.isEmpty() ) {
    QDateTime dt = utcAsZone( QDateTime::fromString( dtend, Qt::ISODate ),
                              localTimeZoneId );
    // Outlook thinks differently about floating event timing than libkcal
    if ( floats ) dt = dt.addDays( -1 );
    event->setDtEnd( dt );
    kdDebug() << "got dtend: " << dtend << " becomes in timezone " << dt.toString() << endl;
  }

  QString transparent = prop.namedItem( "transparent" ).toElement().text();
  event->setTransparency( transparent.toInt() > 0 ? KCal::Event::Transparent
			  : KCal::Event::Opaque );
  // kdDebug() << "Got transparent: " << transparent << endl;

  QString description = prop.namedItem( "textdescription" ).toElement().text();
  event->setDescription( description );
  kdDebug() << "Got description: " << description << endl;

  QString subject = prop.namedItem( "subject" ).toElement().text();
  event->setSummary( subject );
  kdDebug() << "Got summary: " << subject << endl;

  QString location =  prop.namedItem( "location" ).toElement().text();
  event->setLocation( location );
  // kdDebug() << "Got location: " << location << endl;

  QString rrule = prop.namedItem( "rrule" ).toElement().text();
  kdDebug() << "Got rrule: " << rrule << endl;
  if ( !rrule.isEmpty() ) {
    // Timezone should be handled automatically 
    // because we used mFormat->setTimeZone() earlier
    if ( ! mFormat->fromString( event->recurrence(), rrule ) ) {
      kdError() << "ERROR parsing rrule " << rrule << endl;
    }
  }

  QDomElement keywords = prop.namedItem( "Keywords" ).toElement();
  QStringList categories;
  QDomNodeList list = keywords.elementsByTagNameNS( "xml:", "v" );
  for( uint i=0; i < list.count(); i++ ) {
    QDomElement item = list.item(i).toElement();
    categories.append( item.text() );
  }
  event->setCategories( categories );
  // kdDebug() << "Got categories: " << categories.join( ", " ) << endl;


  QDomElement exdate = prop.namedItem( "exdate" ).toElement();
  KCal::DateList exdates;
  list = exdate.elementsByTagNameNS( "xml:", "v" );
  for( uint i=0; i < list.count(); i++ ) {
    QDomElement item = list.item(i).toElement();
    QDate date = utcAsZone( QDateTime::fromString( item.text(), Qt::ISODate ), localTimeZoneId ).date();
    exdates.append( date );
    // kdDebug() << "Got exdate: " << date.toString() << endl;
  }
  event->setExDates( exdates );

  // Exchange sentitivity values:
  // 0 None
  // 1 Personal
  // 2 Private
  // 3 Company Confidential
  QString sensitivity = prop.namedItem( "sensitivity" ).toElement().text();
  if ( ! sensitivity.isNull() ) 
  switch( sensitivity.toInt() ) {
    case 0: event->setSecrecy( KCal::Incidence::SecrecyPublic ); break;
    case 1: event->setSecrecy( KCal::Incidence::SecrecyPrivate ); break;
    case 2: event->setSecrecy( KCal::Incidence::SecrecyPrivate ); break;
    case 3: event->setSecrecy( KCal::Incidence::SecrecyConfidential ); break;
    default: kdWarning() << "Unknown sensitivity: " << sensitivity << endl;
  }
  // kdDebug() << "Got sensitivity: " << sensitivity << endl;


  QString reminder = prop.namedItem( "reminderoffset" ).toElement().text();
  // kdDebug() << "Reminder offset: " << reminder << endl;
  if ( !reminder.isEmpty() ) {
    // Duration before event in seconds
    KCal::Duration offset( - reminder.toInt() );
    KCal::Alarm *alarm = event->newAlarm();
    alarm->setStartOffset( offset );
    alarm->setEnabled( true );
    // TODO: multiple alarms; alarm->setType( KCal::Alarm::xxxx );
  }
  /** Create a new alarm which is associated with this incidence */
    //Alarm* newAlarm();
    /** Add an alarm which is associated with this incidence */
    //void addAlarm(Alarm*);

    /** point at some other event to which the event relates */
    //void setRelatedTo(Incidence *relatedTo);
    /** Add an event which is related to this event */
    //void addRelation(Incidence *);

    /** set the list of attachments/associated files for this event */
    //void setAttachments(const QStringList &attachments);
 
     /** set resources used, such as Office, Car, etc. */
    //void setResources(const QStringList &resources);

    /** set the event's priority, 0 is undefined, 1 highest (decreasing order) */
    //void setPriority(int priority);

    /**
      Add Attendee to this incidence. IncidenceBase takes ownership of the
      Attendee object.
    */

    //void addAttendee(Attendee *a, bool doupdate=true );

  // THE FOLLOWING EVENT PROPERTIES ARE NOT READ

  // Revision ID in webdav is a String, not an int
    /** set the number of revisions this event has seen */
    //void setRevision(int rev);

  // Problem: When you sync Outlook to a Palm, the conduit splits up
  // multi-day events into single-day events WITH ALL THE SAME UID
  // Grrrrrrr.
  if ( mCalendar ) {
    KCal::Event *oldEvent = mCalendar->event( event->uid() );
    if ( oldEvent ) {
      kdWarning() << "Already got his event, replace it..." << endl;
      mCalendar->deleteEvent( oldEvent );
    }
    kdDebug() << "ADD EVENT" << endl;
    mCalendar->addEvent( event );
  } else {
    kdDebug() << "EMIT gotEvent" << endl;
    emit gotEvent( event, static_cast<KIO::DavJob *>( job )->url() );
//    mEvents->append( event );
  }

  decreaseDownloads();
}

void ExchangeDownload::increaseDownloads()
{
  mDownloadsBusy++;
  emit startDownload();
}

void ExchangeDownload::decreaseDownloads()
{
  mDownloadsBusy--;
  // kdDebug() << "Download finished, waiting for " << mDownloadsBusy << " more" << endl;
  emit finishDownload();
  if ( mDownloadsBusy == 0 ) {
    kdDebug() << "All downloads finished" << endl;
    finishUp( ExchangeClient::ResultOK );
  }
}

void ExchangeDownload::finishUp( int result, const QString &moreInfo )
{
  kdDebug() << "ExchangeDownload::finishUp() " << result << " "
            << moreInfo << endl;

  if ( mCalendar ) mCalendar->setModified( true );
  // Disconnect from progress bar
  if ( mProgress ) {
    disconnect( this, 0, mProgress, 0 );
    disconnect( mProgress, 0, this, 0 );
    mProgress->delayedDestruct();
  }

//  if ( mEvents ) {
//    emit finished( this, result, moreInfo, *mEvents );
//  } else {
    emit finished( this, result, moreInfo );
//  }
}

void ExchangeDownload::finishUp( int result, KIO::Job *job )
{
  finishUp( result, QString("WebDAV job error code = ") +
                    QString::number( job->error() ) + ";\n" + "\"" +
                    job->errorString() + "\"" );
}

#include "exchangedownload.moc"
