/*
    This file is part of the exchange resource.
    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Parts are derived from the old libkpimexchange library:
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

#include "exchangeconvertercalendar.h"
#include <webdavhandler.h>
#include <libkcal/incidence.h>
#include <libkcal/event.h>
#include <libkcal/journal.h>
#include <libkcal/todo.h>
#include <libkcal/icalformat.h>
#include <libemailfunctions/email.h>

#include <kdebug.h>

using namespace KCal;

#define TaskNamespace1 "http://schemas.microsoft.com/mapi/id/{00062003-0000-0000-C000-000000000046}/"
#define TaskProp_Status            "0x00008101"
#define TaskProp_PercentCompleted  "0x00008102"
#define TaskProp_DtStart           "0x00008104"
#define TaskProp_DtDue             "0x00008105"
#define TaskProp_Duration          "0x00008106"
#define TaskProp_CompletionDate    "0x0000810f"
#define TaskProp_IsCompleted       "0x0000811C"
#define TaskProp_Owner             "0x0000811F"
#define TaskProp_DoesRecur         "0x00008126"

#define TaskNamespace2 "http://schemas.microsoft.com/mapi/id/{00062008-0000-0000-C000-000000000046}/"
#define TaskProp_ReminderTime      "0x00008502"
#define TaskProp_ReminderSet       "0x00008503"
#define TaskProp_ReminderPlaySound "0x0000851E"
#define TaskProp_ReminderSoundFile "0x0000851F"
#define TaskProp_ContactNames      "0x0000853A"


ExchangeConverterCalendar::ExchangeConverterCalendar()
{
}

void ExchangeConverterCalendar::setTimeZone( const QString &id )
{
  mFormat.setTimeZone( id, true );
}

#define propertyDAV( prop ) \
    WebdavHandler::addElement( doc, root, "d:"prop )
#define propertyNS( ns, prop ) \
    WebdavHandler::addElementNS( doc, root, ns, prop )
#define propertyCalendar( prop ) \
    WebdavHandler::addElement( doc, root, "c:"prop )
#define propertyHTTPMail( prop ) \
    WebdavHandler::addElement( doc, root, "m:"prop )
#define propertyMailHeader( prop ) \
    WebdavHandler::addElement( doc, root, "h:"prop )
#define property( prop ) \
    WebdavHandler::addElement( doc, root, prop )

void ExchangeConverterCalendar::createRequestIncidence( QDomDocument &doc, QDomElement &root )
{
  propertyDAV( "contentclass" );
  propertyDAV( "getcontenttype" );
  propertyNS( "http://schemas.microsoft.com/exchange/", "outlookmessageclass" );
  propertyDAV( "getetag" );
  propertyDAV( "href" );
  propertyDAV( "isreadonly" );
  propertyNS( "http://schemas.microsoft.com/repl/", "repl-uid" );
  propertyHTTPMail( "subject" );
  propertyHTTPMail( "textdescription" );
  propertyHTTPMail( "date" );
  propertyDAV( "comment" );
  propertyNS( "urn:schemas-microsoft-com:office:office", "Keywords" );
  propertyNS( "http://schemas.microsoft.com/exchange/", "sensitivity" );
  propertyHTTPMail( "priority" );
  propertyHTTPMail( "from" );
  propertyHTTPMail( "to" );
  propertyHTTPMail( "cc" );
  propertyHTTPMail( "bcc" );
  propertyHTTPMail( "hasattachment" );
}

void ExchangeConverterCalendar::createRequestAppointment( QDomDocument &doc, QDomElement &root )
{
  createRequestIncidence( doc, root );
  QDomAttr att_c = doc.createAttribute( "xmlns:c" );
  att_c.setValue( "urn:schemas:calendar:" );
  doc.documentElement().setAttributeNode( att_c );
  propertyCalendar( "uid" );
  propertyCalendar( "created" );
  propertyCalendar( "lastmodified" );
  propertyCalendar( "dtstamp" );
  propertyCalendar( "sequence" );
  propertyCalendar( "location" );
  propertyCalendar( "busystatus" );
  propertyCalendar( "transparent" );
  propertyCalendar( "timezone" );
  propertyCalendar( "alldayevent" );
  propertyCalendar( "dtstart" );
  propertyCalendar( "dtend" );
  propertyCalendar( "duration" );
  propertyCalendar( "rrule" );
  propertyCalendar( "rdate" );
  propertyCalendar( "exrule" );
  propertyCalendar( "exdate" );
  propertyCalendar( "recurrenceid" );
  propertyCalendar( "instancetype" );
  propertyCalendar( "reminderoffset" );
  propertyCalendar( "resources" );
}

#define propertyTask1( prop ) \
    WebdavHandler::addElement( doc, props, "t1:"prop )
#define propertyTask2( prop ) \
    WebdavHandler::addElement( doc, props, "t2:"prop )

void ExchangeConverterCalendar::createRequestTask( QDomDocument &doc, QDomElement &props )
{
  createRequestIncidence( doc, props );

  QDomElement root = doc.documentElement();

  QDomAttr att_t1 = doc.createAttribute( "xmlns:t1" );
  att_t1.setValue( TaskNamespace1 );
  root.setAttributeNode( att_t1 );

  QDomAttr att_t2 = doc.createAttribute( "xmlns:t2" );
  att_t2.setValue( TaskNamespace2 );
  root.setAttributeNode( att_t2 );

  // TODO: Insert the correct namespaces here:
//  propertyTask1( TaskProp_UID );
  propertyDAV( "creationdate" );
  propertyDAV( "getlastmodified" );
  propertyTask1( TaskProp_Owner );
  propertyTask2( TaskProp_ContactNames );
  propertyTask1( TaskProp_DtStart );
  propertyTask1( TaskProp_DtDue );
  propertyTask1( TaskProp_Duration );
  propertyTask1( TaskProp_IsCompleted );
  propertyTask1( TaskProp_PercentCompleted );
  propertyTask1( TaskProp_CompletionDate );
  propertyTask1( TaskProp_DoesRecur );
  // What to do about recurrence rules?
  propertyTask2( TaskProp_ReminderSet );
  propertyTask2( TaskProp_ReminderTime );
  propertyTask2( TaskProp_ReminderPlaySound );
  propertyTask2( TaskProp_ReminderSoundFile );
  propertyTask1( TaskProp_Status );
}
#undef propertyTask1
#undef propertyTask2

void ExchangeConverterCalendar::createRequestJournal( QDomDocument &doc, QDomElement &root )
{
  createRequestIncidence( doc, root );
  propertyDAV( "uid" );
  propertyDAV( "creationdate" );
  propertyDAV( "getlastmodified" );
}
#undef propertyDAV
#undef propertyNS
#undef propertyCalendar
#undef propertyHTTPMail
#undef propertyMailHeader
#undef property

bool ExchangeConverterCalendar::readTZ( const QDomElement &node, Incidence */*incidence*/ )
{
  QString timezoneid;
  if ( WebdavHandler::extractString( node, "timezoneid", timezoneid ) ) {
    // kdDebug() << "DEBUG: timezoneid = " << timezoneid << endl;
  }

  QString timezone;
  if ( WebdavHandler::extractString( node, "timezone", timezone ) ) {
    // kdDebug() << "DEBUG: timezone = " << timezone << endl;
  }

  // TODO:
/*  // mFormat is used for parsing recurrence rules.
  QString localTimeZoneId;
  if ( mCalendar ) {
    mFormat.setTimeZone( mCalendar->timeZoneId(), !mCalendar->isLocalTime() );
    localTimeZoneId = mCalendar->timeZoneId();
  }  else {
    localTimeZoneId = "UTC";
    // If no mCalendar, stay in UTC
  }
*/
  return true;
}

bool ExchangeConverterCalendar::readIncidence( const QDomElement &node, Incidence *incidence )
{
kdDebug()<<"ExchangeConverterCalendar::readIncidencd"<<endl;
  QDateTime tmpdt;
  bool tmpbool;
  QString tmpstr;
  long tmplng;
  QStringList tmplst;

  readTZ( node, incidence );

  if ( WebdavHandler::extractString( node, "getetag", tmpstr ) )
    incidence->setCustomProperty( "KDEPIM-Exchange-Resource", "fingerprint", tmpstr );
  if ( WebdavHandler::extractString( node, "href", tmpstr ) )
    incidence->setCustomProperty( "KDEPIM-Exchange-Resource", "href", tmpstr );
  
  // FIXME: use repl-uid as scheduling id?
  if ( WebdavHandler::extractString( node, "textdescription", tmpstr ) )
    incidence->setDescription( tmpstr );
  if ( WebdavHandler::extractString( node, "subject", tmpstr ) )
    incidence->setSummary( tmpstr );
  if ( WebdavHandler::extractStringList( node, "Keywords", tmplst ) )
    incidence->setCategories( tmplst );

  // Use "created" or "creationdate"?
  if ( WebdavHandler::extractBool( node, "isreadonly" , tmpbool ) )
    incidence->setReadOnly( tmpbool );

  // FIXME: Ignore the comment for now

  // Exchange sentitivity values:
  // 0 None, 1 Personal, 2 Private, 3 Company Confidential
  if ( WebdavHandler::extractLong( node, "sensitivity", tmplng ) ) {
    switch( tmplng ) {
      case 0: incidence->setSecrecy( KCal::Incidence::SecrecyPublic ); break;
      case 1:
      case 2: incidence->setSecrecy( KCal::Incidence::SecrecyPrivate ); break;
      case 3: incidence->setSecrecy( KCal::Incidence::SecrecyConfidential ); break;
      default: kdWarning() << "Unknown sensitivity: " << tmplng << endl;
    }
  }

  if ( WebdavHandler::extractBool( node, "hasattachment", tmpbool ) && tmpbool ) {
    // FIXME: Extract attachments...
  }

  if ( WebdavHandler::extractLong( node, "priority", tmplng ) )
    incidence->setPriority( tmplng );

  // FIXME: Use the urn:schemes:httpmail:date property for what?

  // Organizer, required and optional Attendees:
  if ( WebdavHandler::extractString( node, "from", tmpstr ) )
    incidence->setOrganizer( tmpstr );
  if ( WebdavHandler::extractString( node, "to", tmpstr ) ) {
    QStringList atts( KPIM::splitEmailAddrList( tmpstr ) );
    for ( QStringList::Iterator it = atts.begin(); it != atts.end(); ++it ) {
      QString name, email;
      KPIM::getNameAndMail( *it, name, email );
      Attendee *att = new Attendee( name, email );
      att->setRole( KCal::Attendee::ReqParticipant );
      // FIXME: Retrieve the other attendee properties somehow...
      // urn:schemas:calendar:method
      // urn:schemas:calendar:responserequested
      // urn:schemas:calendar:meetingstatus
      // urn:schemas:calendar:replytime
      incidence->addAttendee( att );
    }
  }
  if ( WebdavHandler::extractString( node, "cc", tmpstr ) ) {
    QStringList atts( KPIM::splitEmailAddrList( tmpstr ) );
    for ( QStringList::Iterator it = atts.begin(); it != atts.end(); ++it ) {
      QString name, email;
      KPIM::getNameAndMail( *it, name, email );
      Attendee *att = new Attendee( name, email );
      att->setRole( KCal::Attendee::OptParticipant );
      // FIXME: Retrieve the other attendee properties somehow...
      // urn:schemas:calendar:method
      // urn:schemas:calendar:responserequested
      // urn:schemas:calendar:meetingstatus
      // urn:schemas:calendar:replytime
      incidence->addAttendee( att );
    }
  }

  return true;
}

/* FIXME: Handle recurrences
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

  KIO::DavJob* job = KIO::davSearch( mAccount->calendarURL(), "DAV:", "sql",
                                     query, false );
  KIO::Scheduler::scheduleJob( job );
  job->setWindow( mWindow );
  connect( job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotMasterResult( KIO::Job * ) ) );
}
*/

bool ExchangeConverterCalendar::readEvent( const QDomElement &node, Event *event )
{
  if ( !readIncidence( node, event ) ) return false;
kdDebug()<<"ExchangeConverterCalendar::readEvent"<<endl;

  QDateTime tmpdt;
  QString tmpstr;
  long tmplng;
  bool tmpbool;

  // The UID is absolutely required!
  if ( WebdavHandler::extractString( node, "uid", tmpstr ) ) {
    event->setUid( tmpstr );
  } else {
kdDebug()<<"ExchangeConverterCalendar::readIncidence: ERROR: No UID given"<<endl;
    return false;
  }
  if ( WebdavHandler::extractDateTime( node, "created", tmpdt ) )
    event->setCreated( tmpdt );
  if ( WebdavHandler::extractDateTime( node, "lastmodified", tmpdt ) )
    event->setLastModified( tmpdt );
  // FIXME: Retrieve time zone: "timezone"
  // FIXME: Use the "recurrenceid" prop for the recurrenceId of the event (which is protected!)

  // FIXME: Retrieve MICROSOFT-CDO-* tags first

  if ( WebdavHandler::extractLong( node, "sequence", tmplng ) )
    event->setRevision( tmplng );

  if ( WebdavHandler::extractString( node, "location", tmpstr ) )
    event->setLocation( tmpstr );

  // FIXME: Use "organizer" here instead of the From: person?
/*  if ( WebdavHandler::extractString( node, "organizer", tmpstr ) )
    incidence->setOrganizer( tmpstr );*/

  if ( WebdavHandler::extractDateTime( node, "dtstart", tmpdt ) )
    event->setDtStart( tmpdt );
  if ( WebdavHandler::extractDateTime( node, "dtend", tmpdt ) ) {
    event->setDtEnd( tmpdt );
  } else if ( WebdavHandler::extractLong( node, "duration", tmplng ) ) {
    event->setDuration( tmplng );
  }

  if ( WebdavHandler::extractBool( node, "alldayevent", tmpbool ) )
    event->setFloats( tmpbool );

  // FIXME: Here we have two different props for the same thing?!?!?
  if ( WebdavHandler::extractLong( node, "transparent", tmplng ) )
    event->setTransparency( tmplng>0 ? Event::Transparent : Event::Opaque );
  if ( WebdavHandler::extractString( node, "busystatus", tmpstr ) ) {
    if ( tmpstr == "FREE" )
      event->setTransparency( KCal::Event::Transparent );
    if ( tmpstr == "BUSY" )
      event->setTransparency( KCal::Event::Opaque );
  }

  if ( WebdavHandler::extractLong( node, "reminderoffset", tmplng ) ) {
    // Duration before event in seconds
    KCal::Duration offset( -tmplng );
    KCal::Alarm *alarm = event->newAlarm();
    alarm->setStartOffset( offset );
    alarm->setEnabled( true );
    // TODO: multiple alarms; alarm->setType( KCal::Alarm::xxxx );
  }


  if ( WebdavHandler::extractString( node, "rrule", tmpstr ) && !tmpstr.isEmpty() ) {
    kdDebug() << "Got rrule: " << tmpstr << endl;
    // Timezone should be handled automatically
    // because we used mFormat.setTimeZone() earlier
    // FIXME: Implement this using the format!
    if ( ! mFormat.fromString( event->recurrence(), tmpstr ) ) {
      kdError() << "ERROR parsing rrule " << tmpstr << endl;
    }
  }

  QStringList tmplst;
  if ( WebdavHandler::extractStringList( node, "exdate", tmplst ) ) {
    QStringList::Iterator it = tmplst.begin();
    KCal::DateList exdates;
    for ( ; it != tmplst.end(); ++it ) {
      exdates.append( /*utcAsZone(*/ QDateTime::fromString( *it, Qt::ISODate )/*,
          localTimeZoneId )*/.date() );
    }
    event->setExDates( exdates );
  }
  // FIXME: use rdate and exrule!
/* FIXME: Recurring events, they are split up
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
*/


  // FIXME: read the resources from the "resources" tag

  // FIXME: Custom fields not yet implemented
  return true;
}

bool ExchangeConverterCalendar::readTodo( const QDomElement &node, Todo *todo )
{
  if ( !readIncidence( node, todo ) ) return false;
kdDebug()<<"ExchangeConverterCalendar::readTodo"<<endl;

  // FIXME: Retrieve time zone: "timezone"
  // FIXME: What to with TaskProp_Owner and TaskProp_ContactNames?

  QDateTime tmpdt;
  float tmpfloat;
  long tmplong;
  bool tmpbool;
  QString tmpstr;

  // The UID is absolutely required!
  // FIXME: Which field shall be used as uid???
/*  if ( WebdavHandler::extractString( node, "uid", tmpstr ) ) {
    todo->setUid( tmpstr );
  } else {
kdDebug()<<"ExchangeConverterCalendar::readIncidence: ERROR: No UID given"<<endl;
    return false;
  }*/
//  if ( WebdavHandler::extractDateTime( node, "created", tmpdt ) )
/* FIXME: creation and last modification dates:
  if ( WebdavHandler::extractDateTime( node, "creationdate", tmpdt ) )
    incidence->setCreated( tmpdt );
  if ( WebdavHandler::extractDateTime( node, "getlastmodified", tmpdt ) )
    incidence->setLastModified( tmpdt );*/

  if ( WebdavHandler::extractDateTime( node, TaskProp_DtStart, tmpdt ) )
    todo->setDtStart( tmpdt );
  if ( WebdavHandler::extractDateTime( node, TaskProp_DtDue, tmpdt ) )
    todo->setDtDue( tmpdt );
  if ( WebdavHandler::extractLong( node, TaskProp_Duration, tmplong ) )
    todo->setDuration( tmplong );

  if ( WebdavHandler::extractBool( node, TaskProp_IsCompleted, tmpbool ) && tmpbool ) {
    todo->setCompleted( tmpbool );
    if ( tmpbool && WebdavHandler::extractDateTime( node, TaskProp_CompletionDate, tmpdt ) ) {
      todo->setCompleted( tmpdt );
    }
  }
  if ( WebdavHandler::extractFloat( node, TaskProp_PercentCompleted, tmpfloat ) )
    todo->setPercentComplete( (int)(tmpfloat*100) );

  // FIXME: Recurrence, using TaskProp_DoesRecur
  // What to do about recurrence rules?

  // FIXME: Reminders, use TaskProp_ReminderSet, TaskProp_ReminderTime,
  //        TaskProp_ReminderPlaySound, TaskProp_ReminderSoundFile, TaskProp_Status
  //        But how do I get the offset?

  return true;
}

bool ExchangeConverterCalendar::readJournal( const QDomElement &node, Journal *journal )
{
  if ( !readIncidence( node, journal ) ) return false;
kdDebug()<<"ExchangeConverterCalendar::readJournal"<<endl;
  QDateTime tmpdt;
  QString tmpstr;
  // The UID is absolutely required!
  // FIXME: Which field shall be used as UID?
  if ( WebdavHandler::extractString( node, "uid", tmpstr ) ) {
    journal->setUid( tmpstr );
  } else {
kdDebug()<<"ExchangeConverterCalendar::readJournal: ERROR: No UID given"<<endl;
    return false;
  }
/* FIXME: creation and last modification times:
  if ( WebdavHandler::extractDateTime( node, "created", tmpdt ) )
    incidence->setCreated( tmpdt );
  if ( WebdavHandler::extractDateTime( node, "lastmodified", tmpdt ) )
    incidence->setLastModified( tmpdt );*/

  if ( WebdavHandler::extractDateTime( node, "date", tmpdt ) )
    journal->setDtStart( tmpdt );
  return true;
}

Incidence::List ExchangeConverterCalendar::parseWebDAV( const QDomDocument& davdata )
{
  QDomElement prop = davdata.documentElement().namedItem( "response" )
                     .namedItem( "propstat" ).namedItem( "prop" ).toElement();
  if ( prop.isNull() ) return Incidence::List();

  QString contentclass;
  bool success = WebdavHandler::extractString( prop, "contentclass", contentclass );
  if ( !success ) return Incidence::List();

  Incidence *incidence = 0;
  success = false;
  if ( contentclass == "urn:content-classes:appointment" ) {
    Event *event = new Event();
    success = readEvent( prop, event );
    incidence = event;
  } else if ( contentclass == "urn:content-classes:task" ) {
    Todo *todo = new Todo();
    success = readTodo( prop, todo );
    incidence = todo;
  } else if ( contentclass == "urn:content-classes:journal" ||
              contentclass == "urn:content-classes:message" ) {
    Journal *journal = new Journal();
    success = readJournal( prop, journal );
    incidence = journal;
  }

  Incidence::List list;
  if ( success ) {
    list.append( incidence );
  }
  return list;
}


#define domDavProperty( name, value ) \
  WebdavHandler::addElement( mDoc, mElement, "d:"name, value )
#define domProperty( NS, name, value ) \
  WebdavHandler::addElementNS( mDoc, mElement, NS, name, value )
#define domCalendarProperty( name, value ) \
  WebdavHandler::addElement( mDoc, mElement, "c:"name, value )
#define domHTTPMailProperty( name, value ) \
  WebdavHandler::addElement( mDoc, mElement, "m:"name, value )
#define domMailHeaderProperty( name, value ) \
  WebdavHandler::addElement( mDoc, mElement, "h:"name, value )




class ExchangeConverterCalendar::createWebDAVVisitor : public IncidenceBase::Visitor
{
  public:
    createWebDAVVisitor() : Visitor() {}
    bool act( QDomDocument doc, QDomElement el, IncidenceBase *incidence )
    {
      mDoc = doc;
      mElement = el;
      return incidence->accept( *this );
    }
  protected:
    void addBoolProp( QDomElement &el ) { el.setAttribute( "b:dt", "boolean" ); }
    void addDateProp( QDomElement &el ) { el.setAttribute( "b:dt", "dateTime.tz" ); }
    void addFloatProp( QDomElement &el ) { el.setAttribute( "b:dt", "float" ); }
    void addIntProp( QDomElement &el ) { el.setAttribute( "b:dt", "int" ); }
    QString timePropString( const QDateTime &dt ) { return dt.toString( Qt::ISODate )+"Z"; }

    bool visitIncidence( Incidence *incidence )
    {
      QString tmpstr;
      domDavProperty( "isreadonly", (incidence->isReadOnly())?"1":"0" );
      // FIXME: scheduling ID
//       domProperty( "http://schemas.microsoft.com/repl/", "repl-uid", ??? );
      domHTTPMailProperty( "subject", incidence->summary() );
//       domHTTPMailProperty( "textdescription", incidence->description() );
      // FIXME: timestampt, comments and categories
//       domHTTPMailProperty( "date", ??? ); // timestamp not available in libkcal
//       domDavProperty( "comment", incidence->comments() ); // libkcal has a QStringlist, not one string
//       domProperty( "urn:schemas-microsoft-com:office:office", "Keywords", ??? ); // It's a <v>entyr1</v><v>entry2</v> String list!
      tmpstr = QString::null;
      switch ( incidence->secrecy() ) {
        case KCal::Incidence::SecrecyPublic: tmpstr = "0"; break;
        case KCal::Incidence::SecrecyPrivate: tmpstr = "2"; break;
        case KCal::Incidence::SecrecyConfidential: tmpstr = "3"; break;
        default: break;
      }
      if ( !tmpstr.isEmpty() )
        domProperty( "http://schemas.microsoft.com/exchange/", "sensitivity", tmpstr );

      domHTTPMailProperty( "priority", QString::number(incidence->priority()) );
/* FIXME: from, to, and cc always lead to a 403 Forbidden error...
      domHTTPMailProperty( "from", incidence->organizer().fullName() );

      // FIXME: Attendees:
      tmpstr = QString::null;
      QStringList reqattnames;
      QStringList optattnames;
      Attendee::List atts = incidence->attendees();
      for ( Attendee::List::Iterator it = atts.begin(); it != atts.end(); ++it ) {
        switch ( (*it)->role() ) {
          case KCal::Attendee::Chair:
          case KCal::Attendee::ReqParticipant:
            reqattnames << (*it)->fullName();
            break;
          case KCal::Attendee::OptParticipant:
          case KCal::Attendee::NonParticipant:
            optattnames << (*it)->fullName();
            break;
          default: break;
        }
      }
      domHTTPMailProperty( "to", reqattnames.join(", ") );
      domHTTPMailProperty( "cc", optattnames.join(", ") );
*/
      // FIXME: Attachments: propertyHTTPMail( "hasattachment" );

      return true;
    }
    bool visit( Event *event )
    {
      if ( !visitIncidence(event) ) return false;

      QDomAttr att_c = mDoc.createAttribute( "xmlns:c" );
      att_c.setValue( "urn:schemas:calendar:" );
      mDoc.documentElement().setAttributeNode( att_c );

      domDavProperty( "contentclass", "urn:content-classes:appointment" );
      domProperty( "http://schemas.microsoft.com/exchange/",
                   "outlookmessageclass", "IPM.Appointment" );
      domCalendarProperty( "uid", event->uid() );
      QDomElement el = domCalendarProperty( "created", timePropString( event->created() ) );
      addDateProp( el );
      el = domCalendarProperty( "lastmodified", timePropString( event->lastModified() ) );
      addDateProp( el );
      // FIXME: domCalendarProperty( "dtstamp", ??);
//       FIXME: domCalendarProperty( "sequence", event->sequence() );
      domCalendarProperty( "location", event->location() );

      QString tmpstr( QString::null );
      switch ( event->transparency() ) {
        case KCal::Event::Transparent: tmpstr = "FREE"; break;
        case KCal::Event::Opaque: tmpstr = "BUSY"; break;
      }
      if ( !tmpstr.isEmpty() )
        domCalendarProperty( "busystatus", tmpstr );
//       FIXME: What do do with the "transparent" property?
//       FIXME: Use the "timezone" property...
      domCalendarProperty( "alldayevent", event->doesFloat()?"1":"0" );
      el = domCalendarProperty( "dtstart", timePropString( event->dtStart() ) );
      addDateProp( el );
      if ( event->hasEndDate() ) {
        el = domCalendarProperty( "dtend", timePropString( event->dtEnd() ) );
        addDateProp( el );
      } else {
        domCalendarProperty( "duration", QString::number( event->duration() ) );
      }
      // FIXME: Convert the recurrence rule to a string:
      if ( event->doesRecur() ) {
//       tmpstr = event->recurrence().....
//       domCalendarProperty( "rrule", tmpstr );
        // FIXME: Use "rdate" and "exrule"
        // FIXME: Use "exdate", what's the syntax?
        // FIXME: use the "instancetype" property
      }
      // FIXME: RecurrenceID is protected!
//       domCalendarProperty( "recurrenceid", event->recurrenceId() );
      // FIXME: "reminderoffset"
      // FIXME: "resources"
      return true;
    }
    bool visit( Todo *todo )
    {
      if ( !visitIncidence(todo) ) return false;

      QDomAttr att_t1 = mDoc.createAttribute( "xmlns:t1" );
      att_t1.setValue( TaskNamespace1 );
      mDoc.documentElement().setAttributeNode( att_t1 );

      QDomAttr att_t2 = mDoc.createAttribute( "xmlns:t2" );
      att_t2.setValue( TaskNamespace2 );
      mDoc.documentElement().setAttributeNode( att_t2 );


      domDavProperty( "contentclass", "urn:content-classes:task" );
      domProperty( "http://schemas.microsoft.com/exchange/",
                   "outlookmessageclass", "IPM.Task" );

/* FIXME:
      domCalendarProperty( "uid", todo->uid() );
      domCalendarProperty( "created", todo->created().toString( Qt::ISODate ) );
      domCalendarProperty( "lastmodified", todo->lastModified().toString( Qt::ISODate ) );*/
      // TODO
/*propertyTask1( TaskProp_Owner );
  propertyTask2( TaskProp_ContactNames );
  propertyTask1( TaskProp_DtStart );
  propertyTask1( TaskProp_DtDue );
  propertyTask1( TaskProp_Duration );
  propertyTask1( TaskProp_IsCompleted );
  propertyTask1( TaskProp_PercentCompleted );
  propertyTask1( TaskProp_CompetionDate );
  propertyTask1( TaskProp_DoesRecur );
  // What to do about recurrence rules?
  propertyTask2( TaskProp_ReminderSet );
  propertyTask2( TaskProp_ReminderTime );
  propertyTask2( TaskProp_ReminderPlaySound );
  propertyTask2( TaskProp_ReminderSoundFile );
  propertyTask1( TaskProp_Status );*/
      return true;
    }
    bool visit( Journal *journal )
    {
      if ( !visitIncidence(journal) ) return false;
      domDavProperty( "contentclass", "urn:content-classes:journal" );
      domProperty( "http://schemas.microsoft.com/exchange/",
                   "outlookmessageclass", "IPM.Journal" );
/* FIXME:
      domCalendarProperty( "uid", todo->uid() );
      domCalendarProperty( "created", todo->created().toString( Qt::ISODate ) );
      domCalendarProperty( "lastmodified", todo->lastModified().toString( Qt::ISODate ) );*/
      // TODO
      return true;
    }

  protected:
    QDomDocument mDoc;
    QDomElement mElement;
};

// Prefixes for the namespaces:
// d... DAV:
// b... urn:schemas-microsoft-com:datatypes
// c... calendar
// m... httpmail
// h... httpheader
// p... mapi
// o... office
//

QDomDocument ExchangeConverterCalendar::createWebDAV( Incidence *incidence )
{
  // TODO
  QDomDocument doc;
  QDomElement root = WebdavHandler::addDavElement( doc, doc, "d:propertyupdate" );
  QDomElement set = WebdavHandler::addElement( doc, root, "d:set" );
  QDomElement prop = WebdavHandler::addElement( doc, set, "d:prop" );

  QDomAttr att_b = doc.createAttribute( "xmlns:b" );
  att_b.setValue( "urn:schemas-microsoft-com:datatypes" );
  root.setAttributeNode( att_b );

  QDomAttr att_h = doc.createAttribute( "xmlns:h" );
  att_h.setValue( "urn:schemas:mailheader:" );
  root.setAttributeNode( att_h );

  QDomAttr att_m = doc.createAttribute( "xmlns:m" );
  att_m.setValue( "urn:schemas:httpmail:" );
  root.setAttributeNode( att_m );

//   QDomAttr att1 = doc.createAttributeNS( "do:whatever:you:like", "x:attname");
//   att1.setValue( "value" );
//   prop.setAttributeNodeNS( att1 );
//   root.setAttributeNodeNS( att1 );
//   set.setAttributeNode( att1 );
// //   prop.setAttributeNS ( "xmlns:b", "xmlns:b", "urn:schemas-microsoft-com:datatypes" );

  ExchangeConverterCalendar::createWebDAVVisitor v;
  v.act( doc, prop, incidence );

  return doc;
}
#undef domDavProperty
#undef domProperty
#undef domCalendarProperty
#undef domHTTPMailProperty
#undef domMailHeaderProperty
