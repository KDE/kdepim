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
#include <qinputdialog.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <qcstring.h>
#include <qregexp.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kapp.h>
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

#include "exchangeaccount.h"
#include "exchangedownload.h"
#include "exchangeprogress.h"
#include "utils.h"

using namespace KPIM;

ExchangeDownload::ExchangeDownload( ExchangeAccount* account, QWidget* window ) :
  mWindow( window )
{
  mAccount = account;
  mDownloadsBusy = 0;
  mProgress = 0L;
  mCalendar = 0L;
  mFormat = new KCal::ICalFormat();
}

ExchangeDownload::~ExchangeDownload()
{
  kdDebug() << "ExchangeDownload destructor" << endl;
  delete mFormat;
}

void ExchangeDownload::download(KCal::Calendar* calendar, const QDate& start, const QDate& end, bool showProgress)
{
  mCalendar = calendar;

  if( showProgress ) {
    //kdDebug() << "Creating progress dialog" << endl;
    mProgress = new ExchangeProgress();
    mProgress->show();
  
    connect( this, SIGNAL(startDownload()), mProgress, SLOT(slotTransferStarted()) );
    connect( this, SIGNAL(finishDownload()), mProgress, SLOT(slotTransferFinished()) );
  }

  QString sql = dateSelectQuery( start, end.addDays( 1 ) );
 
  // kdDebug() << "Exchange download query: " << endl << sql << endl;

  increaseDownloads();

  KIO::DavJob *job = KIO::davSearch( mAccount->calendarURL(), "DAV:", "sql", sql, false );
  KIO::Scheduler::scheduleJob(job);
  job->setWindow( mWindow );
  connect(job, SIGNAL(result( KIO::Job * )), this, SLOT(slotSearchResult(KIO::Job *)));
}

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


void ExchangeDownload::slotSearchResult( KIO::Job *job )
{
  if ( job->error() ) {
    kdDebug() << "Error result for search: " << job->error() << endl;
    job->showErrorDialog( 0L );
    decreaseDownloads();
    return;
  }
  QDomDocument& response = static_cast<KIO::DavJob *>( job )->response();

  // kdDebug() << "Search result: " << endl << response.toString() << endl;

  handleAppointments( response, true );
  
  decreaseDownloads();
}

void ExchangeDownload::slotMasterResult( KIO::Job *job )
{
  if ( job->error() ) {
    job->showErrorDialog( 0L );
    decreaseDownloads();
    return;
  }
  QDomDocument& response = static_cast<KIO::DavJob *>( job )->response();

  // kdDebug() << "Search (master) result: " << endl << response.toString() << endl;

  handleAppointments( response, false );
  
  decreaseDownloads();
}

void ExchangeDownload::handleAppointments( const QDomDocument& response, bool recurrence ) {
  //kdDebug() << "Entering handleAppointments" << endl;
  for( QDomElement item = response.documentElement().firstChild().toElement();
       !item.isNull();
       item = item.nextSibling().toElement() )
  {
    //kdDebug() << "Current item:" << item.tagName() << endl;
    QDomNodeList propstats = item.elementsByTagNameNS( "DAV:", "propstat" );
    // kdDebug() << "Item has " << propstats.count() << " propstat children" << endl; 
    for( uint i=0; i < propstats.count(); i++ )
    {
      QDomElement propstat = propstats.item(i).toElement();
      QDomElement prop = propstat.namedItem( "prop" ).toElement();
      if ( prop.isNull() )
      {
        kdDebug() << "Error: no <prop> in response" << endl;
	continue;
      }

      QDomElement instancetypeElement = prop.namedItem( "instancetype" ).toElement();
      if ( instancetypeElement.isNull() ) {
        kdDebug() << "Error: no instance type in Exchange server reply" << endl;
        continue;
      }
      int instanceType = instancetypeElement.text().toInt();
      //kdDebug() << "Instance type: " << instanceType << endl;
    
      if ( recurrence && instanceType > 0 ) {
        QDomElement uidElement = prop.namedItem( "uid" ).toElement();
        if ( uidElement.isNull() ) {
          kdDebug() << "Error: no uid in Exchange server reply" << endl;
          continue;
        }
        QString uid = uidElement.text();
        if ( ! m_uids.contains( uid ) ) {
          m_uids[uid] = 1;
          handleRecurrence(uid);
        }
        continue;
      }

      QDomElement hrefElement = prop.namedItem( "href" ).toElement();
      if ( hrefElement.isNull() ) {
        kdDebug() << "Error: no href in Exchange server reply" << endl;
        continue;
      }
      QString href = hrefElement.text();
      KURL url(href);
      url.setProtocol("webdav");
      
      kdDebug() << "Getting appointment from url: " << url.prettyURL() << endl;
      
      readAppointment( url );
    }
  }
}  

void ExchangeDownload::handleRecurrence(QString uid) {
  // kdDebug() << "Handling recurrence info for uid=" << uid << endl;
  QString query = 
        "SELECT \"DAV:href\", \"urn:schemas:calendar:instancetype\"\r\n"
        "FROM Scope('shallow traversal of \"\"')\r\n"
        "WHERE \"urn:schemas:calendar:uid\" = '" + uid + "'\r\n"
	" AND (\"urn:schemas:calendar:instancetype\" = 1)\r\n";
//	"      OR \"urn:schemas:calendar:instancetype\" = 3)\r\n" // FIXME: exception are not handled

  // kdDebug() << "Exchange master query: " << endl << query << endl;

  increaseDownloads();
 
  KIO::DavJob* job = KIO::davSearch( mAccount->calendarURL(), "DAV:", "sql", query, false );
  KIO::Scheduler::scheduleJob(job);
  job->setWindow( mWindow );
  connect(job, SIGNAL(result( KIO::Job * )), this, SLOT(slotMasterResult(KIO::Job *)));
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
  addElement( doc, prop, "DAV:", "isreadonly" );
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
  
  addElement( doc, prop, "urn:schemas-microsoft-com:office:office", "Keywords" );

//  addElement( doc, prop, "", "" );
//  addElement( doc, prop, "DAV:", "" );
//  addElement( doc, prop, "urn:schemas:calendar:", "" );
//  addElement( doc, prop, "urn:content-classes:appointment", "" );
//  addElement( doc, prop, "urn:schemas:httpmail:", "" );

  increaseDownloads();

  KIO::DavJob* job = KIO::davPropFind( url, doc, "0", false );
  KIO::Scheduler::scheduleJob(job);
  job->setWindow( mWindow );
  job->addMetaData( "errorPage", "false" );
  connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotPropFindResult( KIO::Job * ) ) );
}

void ExchangeDownload::slotPropFindResult( KIO::Job * job )
{
  kdDebug() << "slotPropFindResult" << endl;

  QDomDocument response = static_cast<KIO::DavJob *>( job )->response();
//  kdDebug() << "Response: " << endl;
//  kdDebug() << response.toString() << endl;

  int error = job->error(); 
  if ( error )
  {
    job->showErrorDialog( 0L );
    decreaseDownloads();
    return;
  }

  QDomElement prop = response.documentElement().namedItem( "response" ).namedItem( "propstat" ).namedItem( "prop" ).toElement();
 
  KCal::Event* event = new KCal::Event();

  QDomElement uidElement = prop.namedItem( "uid" ).toElement();
  if ( uidElement.isNull() ) {
    kdDebug() << "Error: no uid in Exchange server reply" << endl;
    decreaseDownloads();
    return;
  }
  event->setUid( QString::fromUtf8( uidElement.text() ) );
  kdDebug() << "Got UID: " << uidElement.text() << endl;

  QString timezoneid = prop.namedItem( "timezoneid" ).toElement().text();
  kdDebug() << "DEBUG: timezoneid = " << timezoneid << endl;

  QString timezone = prop.namedItem( "timezone" ).toElement().text();
  kdDebug() << "DEBUG: timezone = " << timezone << endl;

  // mFormat is used for parsing recurrence rules.
  mFormat->setTimeZone( mCalendar->timeZoneId(), !mCalendar->isLocalTime() );

  QString lastModified = prop.namedItem( "lastmodified" ).toElement().text();
  QDateTime dt = utcAsZone( QDateTime::fromString( lastModified, Qt::ISODate ), mCalendar->timeZoneId() );
  event->setLastModified( dt );
  kdDebug() << "Got lastModified:" << lastModified << ", " << dt.toString() << endl;

  QString organizer = QString::fromUtf8( prop.namedItem( "organizer" ).toElement().text() );
  event->setOrganizer( organizer );
  kdDebug() << "Got organizer: " << organizer << endl;

  // Trying to find attendees, not working yet
  QString contact = QString::fromUtf8( prop.namedItem( "contact" ).toElement().text() );
//  event->setOrganizer( organizer );
  kdDebug() << "DEBUG: Got contact: " << contact << endl;

  // This looks promising for finding attendees
  QString to = QString::fromUtf8( prop.namedItem( "to" ).toElement().text() );
//  event->setOrganizer( organizer );
  kdDebug() << "DEBUG: Got to: " << to << endl;

  QString readonly = prop.namedItem( "isreadonly" ).toElement().text();
  event->setReadOnly( readonly != "0" );
  kdDebug() << "Got readonly: " << readonly << ":" << (readonly != "0") << endl;

  QString created = prop.namedItem( "created" ).toElement().text();
  dt = utcAsZone( QDateTime::fromString( created, Qt::ISODate ), mCalendar->timeZoneId() );
  event->setCreated( dt );
  kdDebug() << "got created: " << dt.toString() << endl;

  QString dtstart = prop.namedItem( "dtstart" ).toElement().text();
  dt = utcAsZone( QDateTime::fromString( dtstart, Qt::ISODate ), mCalendar->timeZoneId() );
  event->setDtStart( dt );
  kdDebug() << "got dtstart: " << dtstart << " becomes in timezone " << dt.toString() << endl;

  QString alldayevent = prop.namedItem( "alldayevent" ).toElement().text();
  bool floats = alldayevent.toInt() != 0;
  event->setFloats( floats );
  kdDebug() << "Got alldayevent: \"" << alldayevent << "\":" << floats << endl;

  QString dtend = prop.namedItem( "dtend" ).toElement().text();
  dt = utcAsZone( QDateTime::fromString( dtend, Qt::ISODate ), mCalendar->timeZoneId() );
  // Outlook thinks differently about floating event timing than libkcal
  if ( floats ) dt = dt.addDays( -1 );
  event->setDtEnd( dt );
  kdDebug() << "got dtstart: " << dtend << " becomes in timezone " << dt.toString() << endl;

  QString transparent = prop.namedItem( "transparent" ).toElement().text();
  event->setTransparency( transparent.toInt() );
  kdDebug() << "Got transparent: " << transparent << endl;

  QString description = QString::fromUtf8( prop.namedItem( "textdescription" ).toElement().text() );
  event->setDescription( description );
  kdDebug() << "Got description: " << description << endl;

  QString subject = QString::fromUtf8( prop.namedItem( "subject" ).toElement().text() );
  event->setSummary( subject );
  kdDebug() << "Got summary: " << subject << endl;

  QString location = QString::fromUtf8( prop.namedItem( "location" ).toElement().text() );
  event->setLocation( location );
  kdDebug() << "Got location: " << location << endl;

  QString rrule = prop.namedItem( "rrule" ).toElement().text();
  kdDebug() << "Got rrule: " << rrule << endl;
  if ( ! rrule.isNull() ) {
    // Timezone should be handled automatically 
    // because we used mFormat->setTimeZone() earlier
    if ( ! mFormat->fromString( event->recurrence(), rrule ) ) {
      kdDebug() << "ERROR parsing rrule " << rrule << endl;
    }
  }

  QDomElement keywords = prop.namedItem( "Keywords" ).toElement();
  QStringList categories;
  QDomNodeList list = keywords.elementsByTagNameNS( "xml:", "v" );
  for( uint i=0; i < list.count(); i++ ) {
    QDomElement item = list.item(i).toElement();
    categories.append( QString::fromUtf8( item.text() ) );
  }
  event->setCategories( categories );
  kdDebug() << "Got categories: " << categories.join( ", " ) << endl;


  QDomElement exdate = prop.namedItem( "exdate" ).toElement();
  KCal::DateList exdates;
  list = exdate.elementsByTagNameNS( "xml:", "v" );
  for( uint i=0; i < list.count(); i++ ) {
    QDomElement item = list.item(i).toElement();
    QDate date = utcAsZone( QDateTime::fromString( item.text(), Qt::ISODate ), mCalendar->timeZoneId() ).date();
    exdates.append( date );
    kdDebug() << "Got exdate: " << date.toString() << endl;
  }
  event->setExDates( exdates );

  // Exchange sentitivity values:
  // 0 None
  // 1 Personal
  // 2 Private
  // 3 Company Confidential
  QString sensitivity = prop.namedItem( "sensitivity" ).toElement().text();
  if ( sensitivity.isNull() ) 
  switch( sensitivity.toInt() ) {
    case 0: event->setSecrecy( KCal::Incidence::SecrecyPublic ); break;
    case 1: event->setSecrecy( KCal::Incidence::SecrecyPrivate ); break;
    case 2: event->setSecrecy( KCal::Incidence::SecrecyPrivate ); break;
    case 3: event->setSecrecy( KCal::Incidence::SecrecyConfidential ); break;
    default: kdDebug() << "Unknown sensitivity: " << sensitivity << endl;
  }
  kdDebug() << "Got sensitivity: " << sensitivity << endl;

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

    /** Create a new alarm which is associated with this incidence */
    //Alarm* newAlarm();
    /** Add an alarm which is associated with this incidence */
    //void addAlarm(Alarm*);


  // THE FOLLOWING EVENT PROPERTIES ARE NOT READ

  // Revision ID in webdav is a String, not an int
    /** set the number of revisions this event has seen */
    //void setRevision(int rev);

  KCal::Event* oldEvent = mCalendar->event( event->uid() );
  if ( oldEvent ) {
    *oldEvent = *event;
  } else {
    mCalendar->addEvent( event );
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
    mCalendar->setModified( true );
    if ( mProgress ) {
      disconnect( this, 0, mProgress, 0 );
      disconnect( mProgress, 0, this, 0 );
      mProgress->delayedDestruct();
    }
    emit finished( this );
  }
}

#include "exchangedownload.moc"
