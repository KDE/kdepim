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

#include <qstring.h>
#include <qregexp.h>

#include <kurl.h>
#include <kdebug.h>
#include <krfcdate.h>
#include <kio/job.h>

#include <kio/slave.h>
#include <kio/scheduler.h>
#include <kio/slavebase.h>
#include <kio/davjob.h>
#include <kio/http.h>

extern "C" {
  #include <ical.h>
}

#include <libkcal/event.h>
#include <libkcal/icalformat.h>
#include <libkcal/icalformatimpl.h>
#include <libkcal/recurrence.h>
#include <libkcal/incidence.h>
#include <libkcal/event.h>

#include "exchangeclient.h"
#include "exchangeprogress.h"
#include "exchangeupload.h"
#include "exchangeaccount.h"
#include "utils.h"

using namespace KPIM;

ExchangeUpload::ExchangeUpload( KCal::Event *event, ExchangeAccount *account,
                                const QString &timeZoneId, QWidget *window )
  : mTimeZoneId( timeZoneId ), mWindow( window )
{
  kdDebug() << "Called ExchangeUpload" << endl;

  mAccount = account;
  m_currentUpload = event;
  m_currentUploadNumber = 0;

//  kdDebug() << "Trying to add appointment " << m_currentUpload->summary() << endl;

  // TODO: For exisiting events the URL for the uid should already be known.
  // Store it after downloading and keep the mapping

  findUid( m_currentUpload->uid() );
}

ExchangeUpload::~ExchangeUpload()
{
  kdDebug() << "Entering ExchangeUpload destructor" << endl;
  kdDebug() << "Finished ExchangeUpload destructor" << endl;
}

void ExchangeUpload::findUid( QString const &uid )
{
  QString query = 
        "SELECT \"DAV:href\", \"urn:schemas:calendar:uid\"\r\n"
        "FROM Scope('shallow traversal of \"\"')\r\n"
        "WHERE \"urn:schemas:calendar:uid\" = '" + uid + "'\r\n";

  kdDebug() << "Find uid query: " << endl << query << endl;
  kdDebug() << "Looking for uid " << uid << endl;
  
  KIO::DavJob* job = KIO::davSearch( mAccount->calendarURL(), "DAV:", "sql",
                                     query, false );
  job->setWindow( mWindow );
  connect( job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotFindUidResult( KIO::Job * ) ) );
}

void ExchangeUpload::slotFindUidResult( KIO::Job * job )
{
  kdDebug() << "slotFindUidResult()" << endl;

  if ( job->error() ) {
    kdDebug() << "Error: " << job->error() << endl;
    job->showErrorDialog( 0 );
    emit finished( this, ExchangeClient::CommunicationError,
                   "IO Error: " + QString::number(job->error()) + ":" +
                   job->errorString() );
    return;
  }
  QDomDocument &response = static_cast<KIO::DavJob *>( job )->response();

  kdDebug() << "Search uid result: " << endl << response.toString() << endl;

  QDomElement item = response.documentElement().firstChild().toElement();
  QDomElement hrefElement = item.namedItem( "href" ).toElement();
  if ( item.isNull() || hrefElement.isNull() ) {
    // No appointment with this UID in exchange database
    // Create a new filename for this appointment and store it there
    tryExist();
    return;
  }
  // The appointment is already in the exchange database
  // Overwrite it with the new data
  QString href = hrefElement.text();
  KURL url( href );
  kdDebug() << "Found URL with identical uid: " << url.prettyURL()
            << ", overwriting that one" << endl;

  startUpload( toDAV( url ) );  
}

void ExchangeUpload::tryExist()
{
  // FIXME: we should first check if current's uid is already in the Exchange database
  // Maybe use locking?
  KURL url = mAccount->calendarURL();
  if ( m_currentUploadNumber == 0 )
    url.addPath( m_currentUpload->summary() + ".EML" );
  else
    url.addPath( m_currentUpload->summary() + "-" + QString::number( m_currentUploadNumber ) + ".EML" );

  kdDebug() << "Trying to see whether " << url.prettyURL() << " exists" << endl;
 
  QDomDocument doc;
  QDomElement root = addElement( doc, doc, "DAV:", "propfind" );
  QDomElement prop = addElement( doc, root, "DAV:", "prop" );
  addElement( doc, prop, "DAV:", "displayname" );
  addElement( doc, prop, "urn:schemas:calendar", "uid" );

  KIO::DavJob *job = KIO::davPropFind( url, doc, "0", false );
  job->setWindow( mWindow );
  job->addMetaData( "errorPage", "false" );
  connect( job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotPropFindResult( KIO::Job * ) ) );
}

void ExchangeUpload::slotPropFindResult( KIO::Job *job )
{
  kdDebug() << "slotPropFindResult()" << endl;
  int error = job->error(); 
  kdDebug() << "PROPFIND error: " << error << ":" << job->errorString() << endl;
  if ( error && error != KIO::ERR_DOES_NOT_EXIST ) {
    job->showErrorDialog( 0 );
    emit finished( this, ExchangeClient::CommunicationError,
                   "IO Error: " + QString::number(error) + ":" +
                   job->errorString() );
    return;
  }

  if ( !error ) {
    // File exist, try another one
    m_currentUploadNumber++;
    tryExist();
    return;
  }

  // We got a 404 error, resource doesn't exist yet, create it
  // FIXME: race condition possible if resource is created under
  // our nose.

  KURL url = mAccount->calendarURL();
  if ( m_currentUploadNumber == 0 )
    url.addPath( m_currentUpload->summary() + ".EML" );
  else
    url.addPath( m_currentUpload->summary() + "-" +
                 QString::number( m_currentUploadNumber ) + ".EML" );

  startUpload( url );
}

QString timezoneid( int offset )
{
  switch ( offset ) {
    case 0: return "0";
    case -60: return "3";
    case -120: return "5";
    case -180: return "51";
    case -210: return "25";
    case -240: return "24"; // Abu Dhabi
    case -270: return "48"; // Kabul
    case -300: return "47"; // Islamabad
    case -330: return "23"; // Bombay
    case -360: return "46"; // Dhaka
    case -420: return "22"; // Bangkok
    case -480: return "45"; // Beijing
    case -540: return "20"; // Tokyo
    case -570: return "44"; // Darwin
    case -600: return "18"; // Brisbane
    case -660: return "41"; // Solomon Islands
    case -720: return "17"; // Auckland
    case 60: return "29"; // Azores
    case 120: return "30"; // Mid Atlantic
    case 180: return "8"; // Brasilia
    case 210: return "28";  // Newfoundland
    case 240: return "9"; // Atlantic time Canada
    case 300: return "10"; // Eastern
    case 360: return "11"; // Central time
    case 420: return "12"; // Mountain time
    case 480: return "13"; // Pacific time
    case 540: return "14"; // Alaska time
    case 600: return "15"; // Hawaii
    case 660: return "16"; // Midway Island
    case 720: return "39"; // Eniwetok
    default: return "52"; // Invalid time zone
  }
}


void ExchangeUpload::startUpload( const KURL &url )
{
  KCal::Event *event = static_cast<KCal::Event *>( m_currentUpload );
  if ( ! event ) {
    kdDebug() << "ERROR: trying to upload a non-Event Incidence" << endl;
    emit finished( this, ExchangeClient::NonEventError, "The incidence that is to be uploaded to the exchange server is not of type KCal::Event" );
    return;
  }

  QDomDocument doc;
  QDomElement root = addElement( doc, doc, "DAV:", "propertyupdate" );
  QDomElement set = addElement( doc, root, "DAV:", "set" );
  QDomElement prop = addElement( doc, set, "DAV:", "prop" );
  addElement( doc, prop, "DAV:", "contentclass", "urn:content-classes:appointment" );
//  addElement( doc, prop, "http://schemas.microsoft.com/exchange/", "outlookmessageclass", "IPM.appointment" );
  addElement( doc, prop, "http://schemas.microsoft.com/exchange/",
              "outlookmessageclass", "IPM.Appointment" );
 // addElement( doc, prop, "urn:schemas:calendar:", "method", "Add" );
  addElement( doc, prop, "urn:schemas:calendar:", "alldayevent", 
      event->doesFloat() ? "1" : "0" );
  addElement( doc, prop, "urn:schemas:calendar:", "busystatus", 
      event->transparency() ? "Free" : "Busy" );
  // KLUDGE: somehow we need to take the opposite of the
  // value that localUTCOffset() supplies...
  // FIXME: What do we need that offset for anyway???
  int tzOffset = - KRFCDate::localUTCOffset(); 
  QString offsetString;
  if ( tzOffset == 0 ) 
    offsetString = "Z";
  else if ( tzOffset > 0 ) 
    offsetString = QString( "+%1:%2" ).arg(tzOffset/60, 2).arg( tzOffset%60, 2 );
  else
    offsetString = QString( "-%1:%2" ).arg((-tzOffset)/60, 2).arg( (-tzOffset)%60, 2 );
  offsetString = offsetString.replace( QRegExp(" "), "0" );

  kdDebug() << "Timezone offset: " << tzOffset << " : " << offsetString << endl;
  kdDebug() << "ExchangeUpload::mTimeZoneId=" << mTimeZoneId << endl;

  addElement( doc, prop, "urn:schemas:calendar:", "dtstart", 
      zoneAsUtc( event->dtStart(), mTimeZoneId ).toString( Qt::ISODate ) + "Z" );
  //    event->dtStart().toString( "yyyy-MM-ddThh:mm:ss.zzzZ" ) );
  //    2002-06-04T08:00:00.000Z" );
  addElement( doc, prop, "urn:schemas:calendar:", "dtend", 
      zoneAsUtc( event->dtEnd(), mTimeZoneId ).toString( Qt::ISODate ) + "Z" );
#if 0
  addElement( doc, prop, "urn:schemas:calendar:", "dtstart", 
      event->dtStart().toString( "yyyy-MM-ddThh:mm:ss.zzz" )+ offsetString );
  //    event->dtStart().toString( "yyyy-MM-ddThh:mm:ss.zzzZ" ) );
  //    2002-06-04T08:00:00.000Z" );
  addElement( doc, prop, "urn:schemas:calendar:", "dtend", 
      event->dtEnd().toString( "yyyy-MM-ddThh:mm:ss.zzz" ) + offsetString );
#endif
  addElement( doc, prop, "urn:schemas:calendar:", "lastmodified", zoneAsUtc( event->lastModified(), mTimeZoneId ).toString( Qt::ISODate )+"Z" );

//  addElement( doc, prop, "urn:schemas:calendar:", "meetingstatus", "confirmed" );
  addElement( doc, prop, "urn:schemas:httpmail:", "textdescription", event->description() );
  addElement( doc, prop, "urn:schemas:httpmail:", "subject", event->summary() );
  addElement( doc, prop, "urn:schemas:calendar:", "location", event->location() );
  // addElement( doc, prop, "urn:schemas:mailheader:", "subject", event->summary() );
  addElement( doc, prop, "urn:schemas:calendar:", "uid", event->uid() );
//  addElement( doc, prop, "urn:schemas:calendar:", "organizer", event->organizer() );

  KCal::Recurrence *recurrence = event->recurrence();
  kdDebug() << "Recurrence->doesRecur(): " << recurrence->doesRecur() << endl;
  if ( recurrence->doesRecur() != KCal::Recurrence::rNone ) {
    addElement( doc, prop, "urn:schemas:calendar:", "instancetype", "1" );
    KCal::ICalFormat *format = new KCal::ICalFormat();
    QString recurstr = format->toString( recurrence );
    // Strip leading "RRULE\n :" and whitespace
    recurstr = recurstr.replace( QRegExp("^[A-Z]*[\\s]*:"), "").stripWhiteSpace();
    kdDebug() << "Recurrence rule after replace: \"" << recurstr << "\"" << endl;
    delete format;
    QDomElement rrule = addElement( doc, prop, "urn:schemas:calendar:", "rrule" );
    addElement( doc, rrule, "xml:", "v", recurstr );
    addElement( doc, prop, "urn:schemas:calendar:", "timezoneid", timezoneid( tzOffset ) );
  } else {
    addElement( doc, prop, "urn:schemas:calendar:", "instancetype", "0" );
  }

  KCal::DateList exdates = event->exDates();
  if ( !exdates.isEmpty() ) {
    QDomElement exdate = addElement( doc, prop, "urn:schemas:calendar:", "exdate" );
    KCal::DateList::iterator it;
    for ( it = exdates.begin(); it != exdates.end(); ++it ) {
      QString date = (*it).toString( "yyyy-MM-ddT00:00:00.000" )+ offsetString;
//      QString date = zoneAsUtc( (*it), mTimeZoneId ).toString( Qt::ISODate );
      addElement( doc, exdate, "xml:", "v", date );
    }
  }

  KCal::Alarm::List alarms = event->alarms();
  if ( alarms.count() > 0 ) {
    KCal::Alarm* alarm = alarms.first();
    // TODO: handle multiple alarms
    // TODO: handle end offsets and general alarm times
    // TODO: handle alarm types
    if ( alarm->hasStartOffset() ) {
      int offset = - alarm->startOffset().asSeconds();
      addElement( doc, prop, "urn:schemas:calendar:", "reminderoffset", QString::number( offset ) );
    }
  }

  kdDebug() << "Uploading event: " << endl;
  kdDebug() << doc.toString() << endl;

  kdDebug() << "Upload url: " << url << endl;

  KIO::DavJob *job = KIO::davPropPatch( url, doc, false );
  job->setWindow( mWindow );
  connect( job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotPatchResult( KIO::Job * ) ) );
}

void ExchangeUpload::slotPatchResult( KIO::Job *job )
{
  kdDebug() << "slotPropPatchResult()" << endl;
  if ( job->error() ) {
    job->showErrorDialog( 0 );
    kdDebug() << "Error: " << job->error() << endl;
    emit finished( this, ExchangeClient::CommunicationError,
                   "IO Error: " + QString::number(job->error()) + ":" +
                   job->errorString() );
    return;
  }
  QDomDocument response = static_cast<KIO::DavJob *>( job )->response();
  kdDebug() << "Patch result: " << response.toString() << endl;

  // Either we have a "201 Created" (if a new event has been created) or 
  // we have a "200 OK" (if an existing event has been altered),
  // or else an error has occurred ;)
  QDomElement status = response.documentElement().namedItem( "response" )
                       .namedItem( "status" ).toElement();
  QDomElement propstat = response.documentElement().namedItem( "response" )
                         .namedItem( "propstat" ).namedItem( "status" )
                         .toElement();
  kdDebug() << "status: " << status.text() << endl;
  kdDebug() << "propstat: " << propstat.text() << endl;
  if ( ! ( status.text().contains( "201" ) || 
           propstat.text().contains( "200" ) ) )
    emit finished( this, ExchangeClient::EventWriteError,
                   "Upload error response: \n" + response.toString() ); 
  else 
    emit finished( this, ExchangeClient::ResultOK, QString::null );
}

#include "exchangeupload.moc"
