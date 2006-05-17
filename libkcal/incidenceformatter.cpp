/*
    This file is part of libkcal.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

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

#include "incidenceformatter.h"

#include <libkcal/attachment.h>
#include <libkcal/event.h>
#include <libkcal/todo.h>
#include <libkcal/journal.h>
#include <libkcal/calendar.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/icalformat.h>
#include <libkcal/freebusy.h>

#include <libemailfunctions/email.h>

#include <ktnef/ktnefparser.h>
#include <ktnef/ktnefmessage.h>
#include <ktnef/ktnefdefs.h>
#include <kabc/phonenumber.h>
#include <kabc/vcardconverter.h>
#include <kabc/stdaddressbook.h>

#include <kapplication.h>
// #include <kdebug.h>

#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

#include <QBuffer>
#include <QDateTime>
//Added by qt3to4:
#include <QList>

#include <time.h>
#include <QTextDocument>


using namespace KCal;


/*******************************************************************
 *  Helper functions for the extensive display (event viewer)
 *******************************************************************/

static QString eventViewerAddLink( const QString &ref, const QString &text,
                             bool newline = true )
{
  QString tmpStr( "<a href=\"" + ref + "\">" + text + "</a>" );
  if ( newline ) tmpStr += "\n";
  return tmpStr;
}

static QString eventViewerAddTag( const QString & tag, const QString & text )
{
  int numLineBreaks = text.count( "\n" );
  QString str = "<" + tag + '>';
  QString tmpText = text;
  QString tmpStr = str;
  if( numLineBreaks >= 0 ) {
    if ( numLineBreaks > 0) {
      int pos = 0;
      QString tmp;
      for( int i = 0; i <= numLineBreaks; i++ ) {
        pos = tmpText.indexOf( "\n" );
        tmp = tmpText.left( pos );
        tmpText = tmpText.right( tmpText.length() - pos - 1 );
        tmpStr += tmp + "<br>";
      }
    } else {
      tmpStr += tmpText;
    }
  }
  tmpStr += "</" + tag + '>';
  return tmpStr;
}


static QString eventViewerFormatCategories( Incidence *event )
{
  QString tmpStr;
  if ( !event->categoriesStr().isEmpty() ) {
    if ( event->categories().count() == 1 ) {
      tmpStr = eventViewerAddTag( "h2", i18n("Category") );
    } else {
      tmpStr = eventViewerAddTag( "h2", i18n("Categories") );
    }
    tmpStr += eventViewerAddTag( "p", event->categoriesStr() );
  }
  return tmpStr;
}

static QString linkPerson( const QString& email, QString name,
                           QString uid, const QString& iconPath )
{
  // Make the search, if there is an email address to search on,
  // and either name or uid is missing
  if ( !email.isEmpty() && ( name.isEmpty() || uid.isEmpty() ) ) {
    KABC::AddressBook *add_book = KABC::StdAddressBook::self( true );
    KABC::Addressee::List addressList = add_book->findByEmail( email );
    KABC::Addressee o = addressList.first();
    if ( !o.isEmpty() && addressList.size() < 2 ) {
      if ( name.isEmpty() )
        // No name set, so use the one from the addressbook
        name = o.formattedName();
      uid = o.uid();
    } else
      // Email not found in the addressbook. Don't make a link
      uid.clear();
  }
  kDebug(5850) << "formatAttendees: uid = " << uid << endl;

  // Show the attendee
  QString tmpString = "<li>";
  if ( !uid.isEmpty() ) {
    // There is a UID, so make a link to the addressbook
    if ( name.isEmpty() )
      // Use the email address for text
      tmpString += eventViewerAddLink( "uid:" + uid, email );
    else
      tmpString += eventViewerAddLink( "uid:" + uid, name );
  } else {
    // No UID, just show some text
    tmpString += ( name.isEmpty() ? email : name );
  }
  tmpString += '\n';

  // Make the mailto link
  if ( !email.isEmpty() && !iconPath.isNull() ) {
    KCal::Person person( name, email );
    KUrl mailto;
    mailto.setProtocol( "mailto" );
    mailto.setPath( person.fullName() );
    tmpString += eventViewerAddLink( mailto.url(),
                                     "<img src=\"" + iconPath + "\">" );
  }
  tmpString += "</li>\n";

  return tmpString;
}

static QString eventViewerFormatAttendees( Incidence *event )
{
  QString tmpStr;
  Attendee::List attendees = event->attendees();
  if ( attendees.count() ) {
    KIconLoader iconLoader;
    const QString iconPath = iconLoader.iconPath( "mail_generic",
                                                  K3Icon::Small );

    // Add organizer link
    tmpStr += eventViewerAddTag( "h3", i18n("Organizer") );
    tmpStr += "<ul>";
    tmpStr += linkPerson( event->organizer().email(),
                          event->organizer().name(), QString(), iconPath );
    tmpStr += "</ul>";

    // Add attendees links
    tmpStr += eventViewerAddTag( "h3", i18n("Attendees") );
    tmpStr += "<ul>";
    Attendee::List::ConstIterator it;
    for( it = attendees.begin(); it != attendees.end(); ++it ) {
      Attendee *a = *it;
      tmpStr += linkPerson( a->email(), a->name(), a->uid(), iconPath );
    }
    tmpStr += "</ul>";
  }
  return tmpStr;
}

static QString eventViewerFormatAttachments( Incidence *i )
{
  QString tmpStr;
  Attachment::List as = i->attachments();
  if ( as.count() > 0 ) {
    Attachment::List::ConstIterator it;
    for( it = as.begin(); it != as.end(); ++it ) {
      if ( (*it)->isUri() ) {
        tmpStr += eventViewerAddLink( (*it)->uri(), (*it)->label() );
        tmpStr += "<br>";
      }
    }
  }
  return tmpStr;
}

/*
  FIXME:This function depends of kaddressbook. Is necessary a new
  type of event?
*/
static QString eventViewerFormatBirthday( Event *event )
{
  if ( !event) return  QString();
  if ( event->customProperty("KABC","BIRTHDAY")!= "YES" ) return QString();

  QString uid_1 = event->customProperty("KABC","UID-1");
  QString name_1 = event->customProperty("KABC","NAME-1");
  QString email_1= event->customProperty("KABC","EMAIL-1");

  KIconLoader iconLoader;
  const QString iconPath = iconLoader.iconPath( "mail_generic",
                                                  K3Icon::Small );
  //TODO: add a tart icon
  QString tmpString = "<ul>";
  tmpString += linkPerson( email_1, name_1, uid_1, iconPath );

  if ( event->customProperty( "KABC", "ANNIVERSARY") == "YES" ) {
    QString uid_2 = event->customProperty("KABC","UID-2");
    QString name_2 = event->customProperty("KABC","NAME-2");
    QString email_2= event->customProperty("KABC","EMAIL-2");
    tmpString += linkPerson( email_2, name_2, uid_2, iconPath );
  }

  tmpString += "</ul>";
  return tmpString;
}

static QString eventViewerFormatHeader( Incidence *incidence )
{
  QString tmpStr = "<table><tr>";

  // show icons
  {
    KIconLoader iconLoader;

    tmpStr += "<td>";

    if ( incidence->type() == "Todo" ) {
      tmpStr += "<img src=\"" +
                iconLoader.iconPath( "todo", K3Icon::Small ) +
                "\">";
    }
    if ( incidence->isAlarmEnabled() ) {
      tmpStr += "<img src=\"" +
                iconLoader.iconPath( "bell", K3Icon::Small ) +
                "\">";
    }
    if ( incidence->doesRecur() ) {
      tmpStr += "<img src=\"" +
                iconLoader.iconPath( "recur", K3Icon::Small ) +
                "\">";
    }
    if ( incidence->isReadOnly() ) {
      tmpStr += "<img src=\"" +
                iconLoader.iconPath( "readonlyevent", K3Icon::Small ) +
                "\">";
    }

    tmpStr += "</td>";
  }

  tmpStr += "<td>" + eventViewerAddTag( "h1", incidence->summary() ) + "</td>";
  tmpStr += "</tr></table><br>";

  return tmpStr;
}

static QString eventViewerFormatEvent( Event *event )
{
  if ( !event ) return QString();
  QString tmpStr = eventViewerFormatHeader( event );

  tmpStr += "<table>";
  if ( !event->location().isEmpty() ) {
    tmpStr += "<tr>";
    tmpStr += "<td align=\"right\"><b>" + i18n( "Location" ) + "</b></td>";
    tmpStr += "<td>" + event->location() + "</td>";
    tmpStr += "</tr>";
  }

  tmpStr += "<tr>";
  if ( event->doesFloat() ) {
    if ( event->isMultiDay() ) {
      tmpStr += "<td align=\"right\"><b>" + i18n( "Time" ) + "</b></td>";
      tmpStr += "<td>" + i18nc("<beginTime> - <endTime>","%1 - %2",
                      event->dtStartDateStr() ,
                      event->dtEndDateStr() ) + "</td>";
    } else {
      tmpStr += "<td align=\"right\"><b>" + i18n( "Date" ) + "</b></td>";
      tmpStr += "<td>" + i18nc("date as string","%1", event->dtStartDateStr() ) + "</td>";
    }
  } else {
    if ( event->isMultiDay() ) {
      tmpStr += "<td align=\"right\"><b>" + i18n( "Time" ) + "</b></td>";
      tmpStr += "<td>" + i18nc("<beginTime> - <endTime>","%1 - %2",
                      event->dtStartStr() ,
                      event->dtEndStr() ) + "</td>";
    } else {
      tmpStr += "<td align=\"right\"><b>" + i18n( "Time" ) + "</b></td>";
      if ( event->hasEndDate() && event->dtStart() != event->dtEnd()) {
        tmpStr += "<td>" + i18nc("<beginTime> - <endTime>","%1 - %2",
                        event->dtStartTimeStr() ,
                        event->dtEndTimeStr() ) + "</td>";
      } else {
        tmpStr += "<td>" + event->dtStartTimeStr() + "</td>";
      }
      tmpStr += "</tr><tr>";
      tmpStr += "<td align=\"right\"><b>" + i18n( "Date" ) + "</b></td>";
      tmpStr += "<td>" + i18nc("date as string","%1",
                      event->dtStartDateStr() ) + "</td>";
    }
  }
  tmpStr += "</tr>";

  if ( event->customProperty("KABC","BIRTHDAY")== "YES" ) {
    tmpStr += "<tr>";
    tmpStr += "<td align=\"right\"><b>" + i18n( "Birthday" ) + "</b></td>";
    tmpStr += "<td>" + eventViewerFormatBirthday( event ) + "</td>";
    tmpStr += "</tr>";
    tmpStr += "</table>";
    return tmpStr;
  }

  if ( !event->description().isEmpty() ) {
    tmpStr += "<tr>";
    tmpStr += "<td></td>";
    tmpStr += "<td>" + eventViewerAddTag( "p", event->description() ) + "</td>";
    tmpStr += "</tr>";
  }

  if ( event->categories().count() > 0 ) {
    tmpStr += "<tr>";
    tmpStr += "<td align=\"right\"><b>" + i18np( "1 category", "%n categories", event->categories().count() )+ "</b></td>";
    tmpStr += "<td>" + event->categoriesStr() + "</td>";
    tmpStr += "</tr>";
  }

  if ( event->doesRecur() ) {
    QDateTime dt = event->recurrence()->getNextDateTime(
                                          QDateTime::currentDateTime() );
    tmpStr += "<tr>";
    tmpStr += "<td align=\"right\"><b>" + i18n( "Next Occurrence" )+ "</b></td>";
    tmpStr += "<td>" + KGlobal::locale()->formatDateTime( dt, true ) + "</td>";
    tmpStr += "</tr>";
  }

  tmpStr += "<tr><td colspan=\"2\">";
  tmpStr += eventViewerFormatAttendees( event );
  tmpStr += "</td></tr>";

  int attachmentCount = event->attachments().count();
  if ( attachmentCount > 0 ) {
    tmpStr += "<tr>";
    tmpStr += "<td align=\"right\"><b>" + i18np( "1 attachment", "%n attachments", attachmentCount )+ "</b></td>";
    tmpStr += "<td>" + eventViewerFormatAttachments( event ) + "</td>";
    tmpStr += "</tr>";
  }

  tmpStr += "</table>";
  tmpStr += "<p><em>" + i18n( "Creation date: %1.",
    KGlobal::locale()->formatDateTime( event->created() , true ) ) + "</em>";
  return tmpStr;
}

static QString eventViewerFormatTodo( Todo *todo )
{
  if ( !todo ) return QString();
  QString tmpStr = eventViewerFormatHeader( todo );

  if ( !todo->location().isEmpty() ) {
    tmpStr += eventViewerAddTag( "b", i18n("Location: %1", todo->location()) );
    tmpStr += "<br>";
  }
  if ( todo->hasDueDate() ) {
    tmpStr += i18n("<b>Due on:</b> %1", todo->dtDueStr() );
  }

  if ( !todo->description().isEmpty() )
    tmpStr += eventViewerAddTag( "p", todo->description() );

  tmpStr += eventViewerFormatCategories( todo );

  if ( todo->priority() > 0 )
    tmpStr += i18n("<p><b>Priority:</b> %2</p>", todo->priority());
  else
    tmpStr += i18n("<p><b>Priority:</b> %2</p>", i18n("Unspecified"));

  tmpStr += i18n("<p><i>%1 % completed</i></p>",
                  todo->percentComplete() );

  if ( todo->doesRecur() ) {
    QDateTime dt = todo->recurrence()->getNextDateTime(
                                         QDateTime::currentDateTime() );
    tmpStr += eventViewerAddTag( "p", "<em>" +
      i18n("This is a recurring to-do. The next occurrence will be on %1.",
      KGlobal::locale()->formatDateTime( dt, true ) ) + "</em>" );
  }
  tmpStr += eventViewerFormatAttendees( todo );
  tmpStr += eventViewerFormatAttachments( todo );
  tmpStr += "<p><em>" + i18n( "Creation date: %1.",
    KGlobal::locale()->formatDateTime( todo->created() , true ) ) + "</em>";
  return tmpStr;
}

static QString eventViewerFormatJournal( Journal *journal )
{
  if ( !journal ) return QString();
  QString tmpStr;
  if ( !journal->summary().isEmpty() )
    tmpStr+= eventViewerAddTag( "h1", journal->summary() );
  tmpStr += eventViewerAddTag( "h2", i18n("Journal for %1", journal->dtStartDateStr( false ) ) );
  if ( !journal->description().isEmpty() )
    tmpStr += eventViewerAddTag( "p", journal->description() );
  return tmpStr;
}

static QString eventViewerFormatFreeBusy( FreeBusy *fb )
{
  if ( !fb ) return QString();
  QString tmpStr( eventViewerAddTag( "h1", i18n("Free/Busy information for %1", fb->organizer().fullName() ) ) );
  tmpStr += eventViewerAddTag( "h3", i18n("Busy times in date range %1 - %2:",
        KGlobal::locale()->formatDate( fb->dtStart().date(), true ) ,
        KGlobal::locale()->formatDate( fb->dtEnd().date(), true ) ) );

  QList<Period> periods = fb->busyPeriods();

  QString text = eventViewerAddTag( "em", eventViewerAddTag( "b", i18n("Busy:") ) );
  QList<Period>::iterator it;
  for ( it = periods.begin(); it != periods.end(); ++it ) {
    Period per = *it;
    if ( per.hasDuration() ) {
      int dur = per.duration().asSeconds();
      QString cont;
      if ( dur >= 3600 ) {
        cont += i18np("1 hour ", "%n hours ", dur / 3600 );
        dur %= 3600;
      }
      if ( dur >= 60 ) {
        cont += i18np("1 minute ", "%n minutes ", dur / 60);
        dur %= 60;
      }
      if ( dur > 0 ) {
        cont += i18np("1 second", "%n seconds", dur);
      }
      text += i18nc("startDate for duration", "%1 for %2",
            KGlobal::locale()->formatDateTime( per.start(), false ) ,
            cont );
      text += "<br>";
    } else {
      if ( per.start().date() == per.end().date() ) {
        text += i18nc("date, fromTime - toTime ", "%1, %2 - %3",
              KGlobal::locale()->formatDate( per.start().date() ) ,
              KGlobal::locale()->formatTime( per.start().time() ) ,
              KGlobal::locale()->formatTime( per.end().time() ) );
      } else {
        text += i18nc("fromDateTime - toDateTime", "%1 - %2",
            KGlobal::locale()->formatDateTime( per.start(), false ) ,
            KGlobal::locale()->formatDateTime( per.end(), false ) );
      }
      text += "<br>";
    }
  }
  tmpStr += eventViewerAddTag( "p", text );
  return tmpStr;
}

class IncidenceFormatter::EventViewerVisitor : public IncidenceBase::Visitor
{
  public:
    EventViewerVisitor() { mResult = ""; }
    bool act( IncidenceBase *incidence ) { return incidence->accept( *this ); }
    QString result() const { return mResult; }
  protected:
    bool visit( Event *event )
    {
      mResult = eventViewerFormatEvent( event );
      return !mResult.isEmpty();
    }
    bool visit( Todo *todo )
    {
      mResult = eventViewerFormatTodo( todo );
      return !mResult.isEmpty();
    }
    bool visit( Journal *journal )
    {
      mResult = eventViewerFormatJournal( journal );
      return !mResult.isEmpty();
    }
    bool visit( FreeBusy *fb )
    {
      mResult = eventViewerFormatFreeBusy( fb );
      return !mResult.isEmpty();
    }

  protected:
    QString mResult;
};

QString IncidenceFormatter::extensiveDisplayString( IncidenceBase *incidence )
{
  if ( !incidence ) return QString();
  EventViewerVisitor v;
  if ( v.act( incidence ) ) {
    return v.result();
  } else
    return QString();
}




/*******************************************************************
 *  Helper functions for the body part formatter of kmail
 *******************************************************************/

static QString string2HTML( const QString& str )
{
  return Qt::convertFromPlainText(str);
}

static QString invitationRow( const QString &cell1, const QString &cell2 )
{
  return "<tr><td>" + cell1 + "</td><td>" + cell2 + "</td></tr>\n";
}

static QString invitationDetailsEvent( Event* event )
{
  // Meeting details are formatted into an HTML table
  if ( !event )
    return QString();

  QString html;
  QString tmp;

  QString sSummary = i18n( "Summary unspecified" );
  if ( ! event->summary().isEmpty() ) {
    sSummary = string2HTML( event->summary() );
  }

  QString sLocation = i18n( "Location unspecified" );
  if ( ! event->location().isEmpty() ) {
    sLocation = string2HTML( event->location() );
  }

  QString dir = ( QApplication::isRightToLeft() ? "rtl" : "ltr" );
  html = QString("<div dir=\"%1\">\n").arg(dir);

  html += "<table border=\"0\" cellpadding=\"1\" cellspacing=\"1\">\n";

  // Meeting summary & location rows
  html += invitationRow( i18n( "What:" ), sSummary );
  html += invitationRow( i18n( "Where:" ), sLocation );

  // Meeting Start Time Row
  if ( ! event->doesFloat() ) {
    tmp =  i18nc("%1: Start Date, %2: Start Time", "%1 %2",
                 event->dtStartDateStr(), event->dtStartTimeStr() );
  } else {
    tmp = i18nc("%1: Start Date", "%1 (time unspecified)",
              event->dtStartDateStr() );
  }
  html += invitationRow( i18n( "Start Time:" ), tmp );

  // Meeting End Time Row
  if ( event->hasEndDate() ) {
    if ( ! event->doesFloat() ) {
      tmp =  i18nc("%1: End Date, %2: End Time", "%1 %2",
                   event->dtEndDateStr(), event->dtEndTimeStr() );
    } else {
      tmp = i18nc("%1: End Date", "%1 (time unspecified)",
                event->dtEndDateStr() );
    }
  } else {
    tmp = i18n( "Unspecified" );
  }
  html += invitationRow( i18n( "End Time:" ), tmp );

  // Meeting Duration Row
  if ( !event->doesFloat() && event->hasEndDate() ) {
    tmp.clear();
    QTime sDuration(0,0,0), t;
    int secs = event->dtStart().secsTo( event->dtEnd() );
    t = sDuration.addSecs( secs );
    if ( t.hour() > 0 ) {
      tmp += i18np( "1 hour ", "%n hours ", t.hour() );
    }
    if ( t.minute() > 0 ) {
      tmp += i18np( "1 minute ", "%n minutes ", t.minute() );
    }

    html += invitationRow( i18n( "Duration:" ), tmp );
  }

  html += "</table>\n";
  html += "</div>\n";

  return html;
}

static QString invitationDetailsTodo( Todo *todo )
{
  // Task details are formatted into an HTML table
  if ( !todo )
    return QString();

  QString sSummary = i18n( "Summary unspecified" );
  QString sDescr = i18n( "Description unspecified" );
  if ( ! todo->summary().isEmpty() ) {
    sSummary = todo->summary();
  }
  if ( ! todo->description().isEmpty() ) {
    sDescr = todo->description();
  }
  QString html( "<table border=\"0\" cellpadding=\"1\" cellspacing=\"1\">\n" );
  html += invitationRow( i18n( "Summary:" ), sSummary );
  html += invitationRow( i18n( "Description:" ), sDescr );
  html += "</table>\n";

  return html;
}

static QString invitationDetailsJournal( Journal *journal )
{
  if ( !journal )
    return QString();

  QString sSummary = i18n( "Summary unspecified" );
  QString sDescr = i18n( "Description unspecified" );
  if ( ! journal->summary().isEmpty() ) {
    sSummary = journal->summary();
  }
  if ( ! journal->description().isEmpty() ) {
    sDescr = journal->description();
  }
  QString html( "<table border=\"0\" cellpadding=\"1\" cellspacing=\"1\">\n" );
  html += invitationRow( i18n( "Summary:" ), sSummary );
  html += invitationRow( i18n( "Date:" ), journal->dtStartDateStr( false ) );
  html += invitationRow( i18n( "Description:" ), sDescr );
  html += "</table>\n";

  return html;
}

static QString invitationDetailsFreeBusy( FreeBusy *fb )
{
  if ( !fb )
    return QString();
  QString html( "<table border=\"0\" cellpadding=\"1\" cellspacing=\"1\">\n" );

  html += invitationRow( i18n("Person:"), fb->organizer().fullName() );
  html += invitationRow( i18n("Start date:"), fb->dtStartDateStr() );
  html += invitationRow( i18n("End date:"),
      KGlobal::locale()->formatDate( fb->dtEnd().date(), true ) );
  html += "<tr><td colspan=2><hr></td></tr>\n";
  html += "<tr><td colspan=2>Busy periods given in this free/busy object:</td></tr>\n";

  QList<Period> periods = fb->busyPeriods();

  QList<Period>::iterator it;
  for ( it = periods.begin(); it != periods.end(); ++it ) {
    Period per = *it;
    if ( per.hasDuration() ) {
      int dur = per.duration().asSeconds();
      QString cont;
      if ( dur >= 3600 ) {
        cont += i18np("1 hour ", "%n hours ", dur / 3600);
        dur %= 3600;
      }
      if ( dur >= 60 ) {
        cont += i18np("1 minute", "%n minutes ", dur / 60);
        dur %= 60;
      }
      if ( dur > 0 ) {
        cont += i18np("1 second", "%n seconds", dur);
      }
      html += invitationRow( QString(), i18nc("startDate for duration", "%1 for %2",
            KGlobal::locale()->formatDateTime( per.start(), false ) ,
           cont) );
    } else {
      QString cont;
      if ( per.start().date() == per.end().date() ) {
        cont = i18nc("date, fromTime - toTime ", "%1, %2 - %3",
              KGlobal::locale()->formatDate( per.start().date() ) ,
              KGlobal::locale()->formatTime( per.start().time() ) ,
              KGlobal::locale()->formatTime( per.end().time() ) );
      } else {
        cont = i18nc("fromDateTime - toDateTime", "%1 - %2",
            KGlobal::locale()->formatDateTime( per.start(), false ) ,
            KGlobal::locale()->formatDateTime( per.end(), false ) );
      }

      html += invitationRow( QString(), cont );
    }
  }

  html += "</table>\n";
  return html;
}

static QString invitationHeaderEvent( Event *event, ScheduleMessage *msg )
{
  if ( !msg || !event )
    return QString();
  switch ( msg->method() ) {
    case Scheduler::Publish:
        return i18n("This event has been published");
    case Scheduler::Request:
        if ( event->revision() > 0 )
          return i18n( "<h2>This meeting has been updated</h2>" );
        else
          return i18n( "You have been invited to this meeting" );
    case Scheduler::Refresh:
        return i18n( "This invitation was refreshed" );
    case Scheduler::Cancel:
        return i18n( "This meeting has been canceled" );
    case Scheduler::Add:
        return i18n( "Addition to the meeting invitation" );
    case Scheduler::Reply: {
        Attendee::List attendees = event->attendees();
        if( attendees.count() == 0 ) {
          kDebug(5850) << "No attendees in the iCal reply!\n";
          return QString();
        }
        if( attendees.count() != 1 )
          kDebug(5850) << "Warning: attendeecount in the reply should be 1 "
                        << "but is " << attendees.count() << endl;
        Attendee* attendee = *attendees.begin();

        switch( attendee->status() ) {
          case Attendee::NeedsAction:
              return i18n( "Sender indicates this invitation still needs some action" );
          case Attendee::Accepted:
              return i18n( "Sender accepts this meeting invitation" );
          case Attendee::Tentative:
              return i18n( "Sender tentatively accepts this meeting invitation" );
          case Attendee::Declined:
              return i18n( "Sender declines this meeting invitation" );
          case Attendee::Delegated:
              return i18n( "Sender has delegated this meeting invitation" );
          case Attendee::Completed:
              return i18n( "This meeting invitation is now completed" );
          case Attendee::InProcess:
              return i18n( "Sender is still processing the invitation" );
          default:
              return i18n( "Unknown response to this meeting invitation" );
        }
        break; }
    case Scheduler::Counter:
        return i18n( "Sender makes this counter proposal" );
    case Scheduler::Declinecounter:
        return i18n( "Sender declines the counter proposal" );
    case Scheduler::NoMethod:
        return i18n("Error: iMIP message with unknown method: '%1'",
              msg->method() );
  }
  return QString();
}

static QString invitationHeaderTodo( Todo *todo, ScheduleMessage *msg )
{
  if ( !msg || !todo )
    return QString();
  switch ( msg->method() ) {
    case Scheduler::Publish:
        return i18n("This task has been published");
    case Scheduler::Request:
        if ( todo->revision() > 0 )
          return i18n( "This task has been updated" );
        else
          return i18n( "You have been assigned this task" );
    case Scheduler::Refresh:
        return i18n( "This task was refreshed" );
    case Scheduler::Cancel:
        return i18n( "This task was canceled" );
    case Scheduler::Add:
        return i18n( "Addition to the task" );
    case Scheduler::Reply: {
        Attendee::List attendees = todo->attendees();
        if( attendees.count() == 0 ) {
          kDebug(5850) << "No attendees in the iCal reply!\n";
          return QString();
        }
        if( attendees.count() != 1 )
          kDebug(5850) << "Warning: attendeecount in the reply should be 1 "
                        << "but is " << attendees.count() << endl;
        Attendee* attendee = *attendees.begin();

        switch( attendee->status() ) {
          case Attendee::NeedsAction:
              return i18n( "Sender indicates this task assignment still needs some action" );
          case Attendee::Accepted:
              return i18n( "Sender accepts this task" );
          case Attendee::Tentative:
              return i18n( "Sender tentatively accepts this task" );
          case Attendee::Declined:
              return i18n( "Sender declines this task" );
          case Attendee::Delegated:
              return i18n( "Sender has delegated this request for the task " );
          case Attendee::Completed:
              return i18n( "The request for this task is now completed" );
          case Attendee::InProcess:
              return i18n( "Sender is still processing the invitation" );
          default:
              return i18n( "Unknown response to this task" );
          }
        break; }
    case Scheduler::Counter:
        return i18n( "Sender makes this counter proposal" );
    case Scheduler::Declinecounter:
        return i18n( "Sender declines the counter proposal" );
    case Scheduler::NoMethod:
        return i18n("Error: iMIP message with unknown method: '%1'",
              msg->method() );
  }
  return QString();
}

static QString invitationHeaderJournal( Journal *journal, ScheduleMessage *msg )
{
  // TODO: Several of the methods are not allowed for journals, so remove them.
  if ( !msg || !journal )
    return QString();
  switch ( msg->method() ) {
    case Scheduler::Publish:
        return i18n("This journal has been published");
    case Scheduler::Request:
        return i18n( "You have been assigned this journal" );
    case Scheduler::Refresh:
        return i18n( "This journal was refreshed" );
    case Scheduler::Cancel:
        return i18n( "This journal was canceled" );
    case Scheduler::Add:
        return i18n( "Addition to the journal" );
    case Scheduler::Reply: {
        Attendee::List attendees = journal->attendees();
        if( attendees.count() == 0 ) {
          kDebug(5850) << "No attendees in the iCal reply!\n";
          return QString();
        }
        if( attendees.count() != 1 )
          kDebug(5850) << "Warning: attendeecount in the reply should be 1 "
                        << "but is " << attendees.count() << endl;
        Attendee* attendee = *attendees.begin();

        switch( attendee->status() ) {
          case Attendee::NeedsAction:
              return i18n( "Sender indicates this journal assignment still needs some action" );
          case Attendee::Accepted:
              return i18n( "Sender accepts this journal" );
          case Attendee::Tentative:
              return i18n( "Sender tentatively accepts this journal" );
          case Attendee::Declined:
              return i18n( "Sender declines this journal" );
          case Attendee::Delegated:
              return i18n( "Sender has delegated this request for the journal" );
          case Attendee::Completed:
              return i18n( "The request for this journal is now completed" );
          case Attendee::InProcess:
              return i18n( "Sender is still processing the invitation" );
          default:
              return i18n( "Unknown response to this journal" );
          }
        break; }
    case Scheduler::Counter:
        return i18n( "Sender makes this counter proposal" );
    case Scheduler::Declinecounter:
        return i18n( "Sender declines the counter proposal" );
    case Scheduler::NoMethod:
        return i18n("Error: iMIP message with unknown method: '%1'",
              msg->method() );
  }
  return QString();
}

static QString invitationHeaderFreeBusy( FreeBusy *fb, ScheduleMessage *msg )
{
  if ( !msg || !fb )
    return QString();
  switch ( msg->method() ) {
    case Scheduler::Publish:
        return i18n("This free/busy list has been published");
    case Scheduler::Request:
        return i18n( "The free/busy list has been requested" );
    case Scheduler::Refresh:
        return i18n( "This free/busy list was refreshed" );
    case Scheduler::Cancel:
        return i18n( "This free/busy list was canceled" );
    case Scheduler::Add:
        return i18n( "Addition to the free/busy list" );
    case Scheduler::NoMethod:
    default:
        return i18n("Error: Free/Busy iMIP message with unknown method: '%1'",
              msg->method() );
  }
}

class IncidenceFormatter::ScheduleMessageVisitor : public IncidenceBase::Visitor
{
  public:
    ScheduleMessageVisitor() : mMessage(0) { mResult = ""; }
    bool act( IncidenceBase *incidence, ScheduleMessage *msg ) { mMessage = msg; return incidence->accept( *this ); }
    QString result() const { return mResult; }

  protected:
    QString mResult;
    ScheduleMessage *mMessage;
};

class IncidenceFormatter::InvitationHeaderVisitor :
      public IncidenceFormatter::ScheduleMessageVisitor
{
  protected:
    bool visit( Event *event )
    {
      mResult = invitationHeaderEvent( event, mMessage );
      return !mResult.isEmpty();
    }
    bool visit( Todo *todo )
    {
      mResult = invitationHeaderTodo( todo, mMessage );
      return !mResult.isEmpty();
    }
    bool visit( Journal *journal )
    {
      mResult = invitationHeaderJournal( journal, mMessage );
      return !mResult.isEmpty();
    }
    bool visit( FreeBusy *fb )
    {
      mResult = invitationHeaderFreeBusy( fb, mMessage );
      return !mResult.isEmpty();
    }
};

class IncidenceFormatter::InvitationBodyVisitor :
      public IncidenceFormatter::ScheduleMessageVisitor
{
  protected:
    bool visit( Event *event )
    {
      mResult = invitationDetailsEvent( event );
      return !mResult.isEmpty();
    }
    bool visit( Todo *todo )
    {
      mResult = invitationDetailsTodo( todo );
      return !mResult.isEmpty();
    }
    bool visit( Journal *journal )
    {
      mResult = invitationDetailsJournal( journal );
      return !mResult.isEmpty();
    }
    bool visit( FreeBusy *fb )
    {
      mResult = invitationDetailsFreeBusy( fb );
      return !mResult.isEmpty();
    }
};


QString InvitationFormatterHelper::makeLink( const QString &id, const QString &text )
{
  QString res( "<a href=\"%1\"><b>%2</b></a>" );
  return res.arg( generateLinkURL( id ) ).arg( text );
  return res;
}


QString IncidenceFormatter::formatICalInvitation( QString invitation, Calendar *mCalendar,
    InvitationFormatterHelper *helper )
{
  if ( invitation.isEmpty() ) return QString();

  ICalFormat format;
  // parseScheduleMessage takes the tz from the calendar, no need to set it manually here for the format!
  ScheduleMessage *msg = format.parseScheduleMessage( mCalendar, invitation );

  if( !msg ) {
    kDebug( 5850 ) << "Failed to parse the scheduling message" << endl;
    Q_ASSERT( format.exception() );
    kDebug( 5850 ) << format.exception()->message() << endl;
    return QString();
  }

  IncidenceBase *incBase = msg->event();

  // First make the text of the message
  QString html;

  QString tableStyle = QString::fromLatin1(
    "style=\"border: solid 1px; margin: 0em;\"" );
  QString tableHead = QString::fromLatin1(
    "<div align=\"center\">"
    "<table width=\"80%\" cellpadding=\"1\" cellspacing=\"0\" %1>"
    "<tr><td>").arg(tableStyle);

  html += tableHead;
  InvitationHeaderVisitor headerVisitor;
  // The InvitationHeaderVisitor returns false if the incidence is somehow invalid, or not handled
  if ( !headerVisitor.act( incBase, msg ) )
    return QString();
  html += "<h2>" + headerVisitor.result() + "</h2>";

  InvitationBodyVisitor bodyVisitor;
  if ( !bodyVisitor.act( incBase, msg ) )
    return QString();
  html += bodyVisitor.result();

  html += "<br>&nbsp;<br>&nbsp;<br>";
  html += "<table border=\"0\" cellspacing=\"0\"><tr><td>&nbsp;</td><td>";

#if 0
  html += helper->makeLinkURL( "accept", i18n("[Enter this into my calendar]") );
  html += "</td><td> &nbsp; </td><td>";
#endif

  // Add groupware links

  Incidence* incidence = dynamic_cast<Incidence*>( incBase );
  switch ( msg->method() ) {
    case Scheduler::Publish:
    case Scheduler::Request:
    case Scheduler::Refresh:
    case Scheduler::Add:
      if ( incidence && incidence->revision() == 0 ) {
        // Accept
        html += helper->makeLink( "accept", i18n( "[Accept]" ) );
        html += "</td><td> &nbsp; </td><td>";
        html += helper->makeLink( "accept_conditionally",
            i18nc( "Accept conditionally", "[Accept cond.]" ) );
        html += "</td><td> &nbsp; </td><td>";
        // Decline
        html += helper->makeLink( "decline", i18n( "[Decline]" ) );
#if 0
        // TODO: implement this
        html += "</b></a></td><td> &nbsp; </td><td>";
        html += helper->makeLink( "check_calendar", i18n("[Check my calendar...]" ) );
#endif
      } else {
        if ( incBase->type() == "Todo" ) {
            html += helper->makeLink( "reply", i18n( "[Enter this into my task list]" ) );
          } else {
            html += helper->makeLink( "reply", i18n( "[Enter this into my calendar]" ) );
          }
        }
        break;
    case Scheduler::Cancel:
        // Cancel event from my calendar
        html += helper->makeLink( "cancel", i18n( "[Remove this from my calendar]" ) );
        break;

    case Scheduler::Reply:
        // Enter this into my calendar
        if ( incBase->type() == "Todo" ) {
          html += helper->makeLink( "reply", i18n( "[Enter this into my task list]" ) );
        } else {
          html += helper->makeLink( "reply", i18n( "[Enter this into my calendar]" ) );
        }
        break;

    case Scheduler::Counter:
    case Scheduler::Declinecounter:
    case Scheduler::NoMethod:
        break;
  }

  html += "</td></tr></table>";

  if ( incidence ) {
    QString sDescr = incidence->description();
    if( ( msg->method() == Scheduler::Request || msg->method() == Scheduler::Cancel ) &&
        !sDescr.isEmpty() ) {
      html += "<br>&nbsp;<br>&nbsp;<br><u>" + i18n("Description:")
        + "</u><br><table border=\"0\"><tr><td>&nbsp;</td><td>";
      html += string2HTML(sDescr) + "</td></tr></table>";
    }
  }

  html += "</td></tr></table><br></div>";

  return html;
}




/*******************************************************************
 *  Helper functions for the msTNEF -> VPart converter
 *******************************************************************/


//-----------------------------------------------------------------------------

static QString stringProp( KTNEFMessage* tnefMsg, const quint32& key,
                           const QString& fallback = QString())
{
  return tnefMsg->findProp( key < 0x10000 ? key & 0xFFFF : key >> 16,
                            fallback );
}

static QString sNamedProp( KTNEFMessage* tnefMsg, const QString& name,
                           const QString& fallback = QString() )
{
  return tnefMsg->findNamedProp( name, fallback );
}

struct save_tz { char* old_tz; char* tz_env_str; };

/* temporarily go to a different timezone */
static struct save_tz set_tz( const char* _tc )
{
  const char *tc = _tc?_tc:"UTC";

  struct save_tz rv;

  rv.old_tz = 0;
  rv.tz_env_str = 0;

  //kDebug(5006) << "set_tz(), timezone before = " << timezone << endl;

  char* tz_env = 0;
  if( getenv( "TZ" ) ) {
    tz_env = strdup( getenv( "TZ" ) );
    rv.old_tz = tz_env;
  }
  char* tmp_env = (char*)malloc( strlen( tc ) + 4 );
  strcpy( tmp_env, "TZ=" );
  strcpy( tmp_env+3, tc );
  putenv( tmp_env );

  rv.tz_env_str = tmp_env;

  /* tmp_env is not free'ed -- it is part of the environment */

  tzset();
  //kDebug(5006) << "set_tz(), timezone after = " << timezone << endl;

  return rv;
}

/* restore previous timezone */
static void unset_tz( struct save_tz old_tz )
{
  if( old_tz.old_tz ) {
    char* tmp_env = (char*)malloc( strlen( old_tz.old_tz ) + 4 );
    strcpy( tmp_env, "TZ=" );
    strcpy( tmp_env+3, old_tz.old_tz );
    putenv( tmp_env );
    /* tmp_env is not free'ed -- it is part of the environment */
    free( old_tz.old_tz );
  } else {
    /* clear TZ from env */
    putenv( strdup("TZ") );
  }
  tzset();

  /* is this OK? */
  if( old_tz.tz_env_str ) free( old_tz.tz_env_str );
}

static QDateTime utc2Local( const QDateTime& utcdt )
{
  struct tm tmL;

  save_tz tmp_tz = set_tz("UTC");
  time_t utc = utcdt.toTime_t();
  unset_tz( tmp_tz );

  localtime_r( &utc, &tmL );
  return QDateTime( QDate( tmL.tm_year+1900, tmL.tm_mon+1, tmL.tm_mday ),
                    QTime( tmL.tm_hour, tmL.tm_min, tmL.tm_sec ) );
}


static QDateTime pureISOToLocalQDateTime( const QString& dtStr,
                                          bool bDateOnly = false )
{
  QDate tmpDate;
  QTime tmpTime;
  int year, month, day, hour, minute, second;

  if( bDateOnly ) {
    year = dtStr.left( 4 ).toInt();
    month = dtStr.mid( 4, 2 ).toInt();
    day = dtStr.mid( 6, 2 ).toInt();
    hour = 0;
    minute = 0;
    second = 0;
  } else {
    year = dtStr.left( 4 ).toInt();
    month = dtStr.mid( 4, 2 ).toInt();
    day = dtStr.mid( 6, 2 ).toInt();
    hour = dtStr.mid( 9, 2 ).toInt();
    minute = dtStr.mid( 11, 2 ).toInt();
    second = dtStr.mid( 13, 2 ).toInt();
  }
  tmpDate.setYMD( year, month, day );
  tmpTime.setHMS( hour, minute, second );

  if( tmpDate.isValid() && tmpTime.isValid() ) {
    QDateTime dT = QDateTime( tmpDate, tmpTime );

    if( !bDateOnly ) {
      // correct for GMT ( == Zulu time == UTC )
      if (dtStr.at(dtStr.length()-1) == 'Z') {
        //dT = dT.addSecs( 60 * KRFCDate::localUTCOffset() );
        //localUTCOffset( dT ) );
        dT = utc2Local( dT );
      }
    }
    return dT;
  } else
    return QDateTime();
}



QString IncidenceFormatter::msTNEFToVPart( const QByteArray& tnef )
{
  bool bOk = false;

  KTNEFParser parser;
  QByteArray b( tnef );
  QBuffer buf( &b );
  CalendarLocal cal ( QLatin1String( "UTC" ) );
  KABC::Addressee addressee;
  ICalFormat calFormat;
  Event* event = new Event();

  if( parser.openDevice( &buf ) ) {
    KTNEFMessage* tnefMsg = parser.message();
    //QMap<int,KTNEFProperty*> props = parser.message()->properties();

    // Everything depends from property PR_MESSAGE_CLASS
    // (this is added by KTNEFParser):
    QString msgClass = tnefMsg->findProp( 0x001A, QString(), true )
      .toUpper();
    if( !msgClass.isEmpty() ) {
      // Match the old class names that might be used by Outlook for
      // compatibility with Microsoft Mail for Windows for Workgroups 3.1.
      bool bCompatClassAppointment = false;
      bool bCompatMethodRequest = false;
      bool bCompatMethodCancled = false;
      bool bCompatMethodAccepted = false;
      bool bCompatMethodAcceptedCond = false;
      bool bCompatMethodDeclined = false;
      if( msgClass.startsWith( "IPM.MICROSOFT SCHEDULE." ) ) {
        bCompatClassAppointment = true;
        if( msgClass.endsWith( ".MTGREQ" ) )
          bCompatMethodRequest = true;
        if( msgClass.endsWith( ".MTGCNCL" ) )
          bCompatMethodCancled = true;
        if( msgClass.endsWith( ".MTGRESPP" ) )
          bCompatMethodAccepted = true;
        if( msgClass.endsWith( ".MTGRESPA" ) )
          bCompatMethodAcceptedCond = true;
        if( msgClass.endsWith( ".MTGRESPN" ) )
          bCompatMethodDeclined = true;
      }
      bool bCompatClassNote = ( msgClass == "IPM.MICROSOFT MAIL.NOTE" );

      if( bCompatClassAppointment || "IPM.APPOINTMENT" == msgClass ) {
        // Compose a vCal
        bool bIsReply = false;
        QString prodID = "-//Microsoft Corporation//Outlook ";
        prodID += tnefMsg->findNamedProp( "0x8554", "9.0" );
        prodID += "MIMEDIR/EN\n";
        prodID += "VERSION:2.0\n";
        calFormat.setApplication( "Outlook", prodID );

        Scheduler::Method method;
        if( bCompatMethodRequest )
          method = Scheduler::Request;
        else if( bCompatMethodCancled )
          method = Scheduler::Cancel;
        else if( bCompatMethodAccepted || bCompatMethodAcceptedCond ||
                 bCompatMethodDeclined ) {
          method = Scheduler::Reply;
          bIsReply = true;
        } else {
          // pending(khz): verify whether "0x0c17" is the right tag ???
          //
          // at the moment we think there are REQUESTS and UPDATES
          //
          // but WHAT ABOUT REPLIES ???
          //
          //

          if( tnefMsg->findProp(0x0c17) == "1" )
            bIsReply = true;
          method = Scheduler::Request;
        }

        /// ###  FIXME Need to get this attribute written
        ScheduleMessage schedMsg(event, method, ScheduleMessage::Unknown );

        QString sSenderSearchKeyEmail( tnefMsg->findProp( 0x0C1D ) );

        if( !sSenderSearchKeyEmail.isEmpty() ) {
          int colon = sSenderSearchKeyEmail.indexOf( ':' );
          // May be e.g. "SMTP:KHZ@KDE.ORG"
          if( sSenderSearchKeyEmail.indexOf( ':' ) == -1 )
            sSenderSearchKeyEmail.remove( 0, colon+1 );
        }

        QString s( tnefMsg->findProp( 0x0e04 ) );
        QStringList attendees = s.split( ';' );
        if( attendees.count() ) {
          for( QStringList::Iterator it = attendees.begin();
               it != attendees.end(); ++it ) {
            // Skip all entries that have no '@' since these are
            // no mail addresses
            if( (*it).indexOf('@') == -1 ) {
              s = (*it).trimmed();

              Attendee *attendee = new Attendee( s, s, true );
              if( bIsReply ) {
                if( bCompatMethodAccepted )
                  attendee->setStatus( Attendee::Accepted );
                if( bCompatMethodDeclined )
                  attendee->setStatus( Attendee::Declined );
                if( bCompatMethodAcceptedCond )
                  attendee->setStatus(Attendee::Tentative);
              } else {
                attendee->setStatus( Attendee::NeedsAction );
                attendee->setRole( Attendee::ReqParticipant );
              }
              event->addAttendee(attendee);
            }
          }
        } else {
          // Oops, no attendees?
          // This must be old style, let us use the PR_SENDER_SEARCH_KEY.
          s = sSenderSearchKeyEmail;
          if( !s.isEmpty() ) {
            Attendee *attendee = new Attendee( QString(), QString(),
                                               true );
            if( bIsReply ) {
              if( bCompatMethodAccepted )
                attendee->setStatus( Attendee::Accepted );
              if( bCompatMethodAcceptedCond )
                attendee->setStatus( Attendee::Declined );
              if( bCompatMethodDeclined )
                attendee->setStatus( Attendee::Tentative );
            } else {
              attendee->setStatus(Attendee::NeedsAction);
              attendee->setRole(Attendee::ReqParticipant);
            }
            event->addAttendee(attendee);
          }
        }
        s = tnefMsg->findProp( 0x0c1f ); // look for organizer property
        if( s.isEmpty() && !bIsReply )
          s = sSenderSearchKeyEmail;
        // TODO: Use the common name?
        if( !s.isEmpty() )
          event->setOrganizer( s );

        s = tnefMsg->findProp( 0x8516 ).replace( QChar( '-' ), QString() )
          .replace( QChar( ':' ), QString() );
        event->setDtStart( QDateTime::fromString( s ) ); // ## Format??

        s = tnefMsg->findProp( 0x8517 ).replace( QChar( '-' ), QString() )
          .replace( QChar( ':' ), QString() );
        event->setDtEnd( QDateTime::fromString( s ) );

        s = tnefMsg->findProp( 0x8208 );
        event->setLocation( s );

        // is it OK to set this to OPAQUE always ??
        //vPart += "TRANSP:OPAQUE\n"; ###FIXME, portme!
        //vPart += "SEQUENCE:0\n";

        // is "0x0023" OK  -  or should we look for "0x0003" ??
        s = tnefMsg->findProp( 0x0023 );
        event->setUid( s );

        // PENDING(khz): is this value in local timezone? Must it be
        // adjusted? Most likely this is a bug in the server or in
        // Outlook - we ignore it for now.
        s = tnefMsg->findProp( 0x8202 ).replace( QChar( '-' ), QString() )
          .replace( QChar( ':' ), QString() );
        // ### libkcal always uses currentDateTime()
        // event->setDtStamp(QDateTime::fromString(s));

        s = tnefMsg->findNamedProp( "Keywords" );
        event->setCategories( s );

        s = tnefMsg->findProp( 0x1000 );
        event->setDescription( s );

        s = tnefMsg->findProp( 0x0070 );
        event->setSummary( s );

        s = tnefMsg->findProp( 0x0026 );
        event->setPriority( s.toInt() );

        // is reminder flag set ?
        if(!tnefMsg->findProp(0x8503).isEmpty()) {
          Alarm *alarm = new Alarm(event);
          QDateTime highNoonTime =
            pureISOToLocalQDateTime( tnefMsg->findProp( 0x8502 )
                                     .replace( QChar( '-' ), "" )
                                     .replace( QChar( ':' ), "" ) );
          QDateTime wakeMeUpTime =
            pureISOToLocalQDateTime( tnefMsg->findProp( 0x8560, "" )
                                     .replace( QChar( '-' ), "" )
                                     .replace( QChar( ':' ), "" ) );
          alarm->setTime(wakeMeUpTime);

          if( highNoonTime.isValid() && wakeMeUpTime.isValid() )
            alarm->setStartOffset( Duration( highNoonTime, wakeMeUpTime ) );
          else
            // default: wake them up 15 minutes before the appointment
            alarm->setStartOffset( Duration( 15*60 ) );
          alarm->setDisplayAlarm( i18n( "Reminder" ) );

          // Sorry: the different action types are not known (yet)
          //        so we always set 'DISPLAY' (no sounds, no images...)
          event->addAlarm( alarm );
        }
        cal.addEvent( event );
        bOk = true;
        // we finished composing a vCal
      } else if( bCompatClassNote || "IPM.CONTACT" == msgClass ) {
        addressee.setUid( stringProp( tnefMsg, attMSGID ) );
        addressee.setFormattedName( stringProp( tnefMsg, MAPI_TAG_PR_DISPLAY_NAME ) );
        addressee.insertEmail( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_EMAIL1EMAILADDRESS ), true );
        addressee.insertEmail( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_EMAIL2EMAILADDRESS ), false );
        addressee.insertEmail( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_EMAIL3EMAILADDRESS ), false );
        addressee.insertCustom( "KADDRESSBOOK", "X-IMAddress", sNamedProp( tnefMsg, MAPI_TAG_CONTACT_IMADDRESS ) );
        addressee.insertCustom( "KADDRESSBOOK", "X-SpousesName", stringProp( tnefMsg, MAPI_TAG_PR_SPOUSE_NAME ) );
        addressee.insertCustom( "KADDRESSBOOK", "X-ManagersName", stringProp( tnefMsg, MAPI_TAG_PR_MANAGER_NAME ) );
        addressee.insertCustom( "KADDRESSBOOK", "X-AssistantsName", stringProp( tnefMsg, MAPI_TAG_PR_ASSISTANT ) );
        addressee.insertCustom( "KADDRESSBOOK", "X-Department", stringProp( tnefMsg, MAPI_TAG_PR_DEPARTMENT_NAME ) );
        addressee.insertCustom( "KADDRESSBOOK", "X-Office", stringProp( tnefMsg, MAPI_TAG_PR_OFFICE_LOCATION ) );
        addressee.insertCustom( "KADDRESSBOOK", "X-Profession", stringProp( tnefMsg, MAPI_TAG_PR_PROFESSION ) );

        QString s = tnefMsg->findProp( MAPI_TAG_PR_WEDDING_ANNIVERSARY )
          .replace( QChar( '-' ), QString() )
          .replace( QChar( ':' ), QString() );
        if( !s.isEmpty() )
          addressee.insertCustom( "KADDRESSBOOK", "X-Anniversary", s );

        addressee.setUrl( KUrl( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_WEBPAGE )  ) );

        // collect parts of Name entry
        addressee.setFamilyName( stringProp( tnefMsg, MAPI_TAG_PR_SURNAME ) );
        addressee.setGivenName( stringProp( tnefMsg, MAPI_TAG_PR_GIVEN_NAME ) );
        addressee.setAdditionalName( stringProp( tnefMsg, MAPI_TAG_PR_MIDDLE_NAME ) );
        addressee.setPrefix( stringProp( tnefMsg, MAPI_TAG_PR_DISPLAY_NAME_PREFIX ) );
        addressee.setSuffix( stringProp( tnefMsg, MAPI_TAG_PR_GENERATION ) );

        addressee.setNickName( stringProp( tnefMsg, MAPI_TAG_PR_NICKNAME ) );
        addressee.setRole( stringProp( tnefMsg, MAPI_TAG_PR_TITLE ) );
        addressee.setOrganization( stringProp( tnefMsg, MAPI_TAG_PR_COMPANY_NAME ) );
        /*
        the MAPI property ID of this (multiline) )field is unknown:
        vPart += stringProp(tnefMsg, "\n","NOTE", ... , "" );
        */

        KABC::Address adr;
        adr.setPostOfficeBox( stringProp( tnefMsg, MAPI_TAG_PR_HOME_ADDRESS_PO_BOX ) );
        adr.setStreet( stringProp( tnefMsg, MAPI_TAG_PR_HOME_ADDRESS_STREET ) );
        adr.setLocality( stringProp( tnefMsg, MAPI_TAG_PR_HOME_ADDRESS_CITY ) );
        adr.setRegion( stringProp( tnefMsg, MAPI_TAG_PR_HOME_ADDRESS_STATE_OR_PROVINCE ) );
        adr.setPostalCode( stringProp( tnefMsg, MAPI_TAG_PR_HOME_ADDRESS_POSTAL_CODE ) );
        adr.setCountry( stringProp( tnefMsg, MAPI_TAG_PR_HOME_ADDRESS_COUNTRY ) );
        adr.setType(KABC::Address::Home);
        addressee.insertAddress(adr);

        adr.setPostOfficeBox( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_BUSINESSADDRESSPOBOX ) );
        adr.setStreet( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_BUSINESSADDRESSSTREET ) );
        adr.setLocality( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_BUSINESSADDRESSCITY ) );
        adr.setRegion( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_BUSINESSADDRESSSTATE ) );
        adr.setPostalCode( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_BUSINESSADDRESSPOSTALCODE ) );
        adr.setCountry( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_BUSINESSADDRESSCOUNTRY ) );
        adr.setType( KABC::Address::Work );
        addressee.insertAddress( adr );

        adr.setPostOfficeBox( stringProp( tnefMsg, MAPI_TAG_PR_OTHER_ADDRESS_PO_BOX ) );
        adr.setStreet( stringProp(tnefMsg, MAPI_TAG_PR_OTHER_ADDRESS_STREET ) );
        adr.setLocality( stringProp(tnefMsg, MAPI_TAG_PR_OTHER_ADDRESS_CITY ) );
        adr.setRegion( stringProp(tnefMsg, MAPI_TAG_PR_OTHER_ADDRESS_STATE_OR_PROVINCE ) );
        adr.setPostalCode( stringProp(tnefMsg, MAPI_TAG_PR_OTHER_ADDRESS_POSTAL_CODE ) );
        adr.setCountry( stringProp(tnefMsg, MAPI_TAG_PR_OTHER_ADDRESS_COUNTRY ) );
        adr.setType( KABC::Address::Dom );
        addressee.insertAddress(adr);

        // problem: the 'other' address was stored by KOrganizer in
        //          a line looking like the following one:
        // vPart += "\nADR;TYPE=dom;TYPE=intl;TYPE=parcel;TYPE=postal;TYPE=work;TYPE=home:other_pobox;;other_str1\nother_str2;other_loc;other_region;other_pocode;other_country

        QString nr;
        nr = stringProp( tnefMsg, MAPI_TAG_PR_HOME_TELEPHONE_NUMBER );
        addressee.insertPhoneNumber( KABC::PhoneNumber( nr, KABC::PhoneNumber::Home ) );
        nr = stringProp( tnefMsg, MAPI_TAG_PR_BUSINESS_TELEPHONE_NUMBER );
        addressee.insertPhoneNumber( KABC::PhoneNumber( nr, KABC::PhoneNumber::Work ) );
        nr = stringProp( tnefMsg, MAPI_TAG_PR_MOBILE_TELEPHONE_NUMBER );
        addressee.insertPhoneNumber( KABC::PhoneNumber( nr, KABC::PhoneNumber::Cell ) );
        nr = stringProp( tnefMsg, MAPI_TAG_PR_HOME_FAX_NUMBER );
        addressee.insertPhoneNumber( KABC::PhoneNumber( nr, KABC::PhoneNumber::Fax | KABC::PhoneNumber::Home ) );
        nr = stringProp( tnefMsg, MAPI_TAG_PR_BUSINESS_FAX_NUMBER );
        addressee.insertPhoneNumber( KABC::PhoneNumber( nr, KABC::PhoneNumber::Fax | KABC::PhoneNumber::Work ) );

        s = tnefMsg->findProp( MAPI_TAG_PR_BIRTHDAY )
          .replace( QChar( '-' ), QString() )
          .replace( QChar( ':' ), QString() );
        if( !s.isEmpty() )
          addressee.setBirthday( QDateTime::fromString( s ) );

        bOk = ( !addressee.isEmpty() );
      } else if( "IPM.NOTE" == msgClass ) {

      } // else if ... and so on ...
    }
  }

  // Compose return string
  QString iCal = calFormat.toString( &cal );
  if( !iCal.isEmpty() )
    // This was an iCal
    return iCal;

  // Not an iCal - try a vCard
  KABC::VCardConverter converter;
  return QString::fromUtf8( converter.createVCard( addressee ) );
}


QString IncidenceFormatter::formatTNEFInvitation( const QByteArray& tnef,
        Calendar *mCalendar, InvitationFormatterHelper *helper )
{
  QString vPart = IncidenceFormatter::msTNEFToVPart( tnef );
  QString iCal = IncidenceFormatter::formatICalInvitation( vPart, mCalendar, helper );
  if( !iCal.isEmpty() )
    return iCal;
  return vPart;
}




/*******************************************************************
 *  Helper functions for the Incidence tooltips
 *******************************************************************/

class IncidenceFormatter::ToolTipVisitor : public IncidenceBase::Visitor
{
  public:
    ToolTipVisitor() : mRichText( true ), mResult( "" ) {}

    bool act( IncidenceBase *incidence, bool richText=true)
    {
      mRichText = richText;
      mResult = "";
      return incidence ? incidence->accept( *this ) : false;
    }
    QString result() const { return mResult; }

  protected:
    bool visit( Event *event );
    bool visit( Todo *todo );
    bool visit( Journal *journal );
    bool visit( FreeBusy *fb );

    QString dateRangeText( Event*event );
    QString dateRangeText( Todo *todo );
    QString dateRangeText( Journal *journal );
    QString dateRangeText( FreeBusy *fb );

    QString generateToolTip( Incidence* incidence, QString dtRangeText );

  protected:
    bool mRichText;
    QString mResult;
};

QString IncidenceFormatter::ToolTipVisitor::dateRangeText( Event*event )
{
  QString ret;
  QString tmp;
  if ( event->isMultiDay() ) {

    if (event->doesFloat())
      tmp = event->dtStartDateStr().replace(" ", "&nbsp;");
    else
      tmp = event->dtStartStr().replace(" ", "&nbsp;");
    ret += "<br>" + i18nc("Event start", "<i>From:</i>&nbsp;%1", tmp);

    if (event->doesFloat())
      tmp = event->dtEndDateStr().replace(" ", "&nbsp;");
    else
      tmp = event->dtEndStr().replace(" ", "&nbsp;");
    ret += "<br>" + i18nc("Event end","<i>To:</i>&nbsp;%1", tmp);

  } else {

    ret += "<br>"+i18n("<i>Date:</i>&nbsp;%1",
         event->dtStartDateStr().replace(" ", "&nbsp;") );
    if ( !event->doesFloat() ) {
      if ( event->dtStartTimeStr() == event->dtEndTimeStr() ) { // to prevent 'Time: 17:00 - 17:00'
        tmp = "<br>" + i18nc("time for event, &nbsp; to prevent ugly line breaks",
        "<i>Time:</i>&nbsp;%1",
         event->dtStartTimeStr().replace(" ", "&nbsp;") );
      } else {
        tmp = "<br>" + i18nc("time range for event, &nbsp; to prevent ugly line breaks",
        "<i>Time:</i>&nbsp;%1&nbsp;-&nbsp;%2",
         event->dtStartTimeStr().replace(" ", "&nbsp;") ,
         event->dtEndTimeStr().replace(" ", "&nbsp;") );
      }
      ret += tmp;
    }

  }
  return ret;
}

QString IncidenceFormatter::ToolTipVisitor::dateRangeText( Todo*todo )
{
  QString ret;
  bool floats( todo->doesFloat() );
  if (todo->hasStartDate())
    // No need to add <i> here. This is separated issue and each line
    // is very visible on its own. On the other hand... Yes, I like it
    // italics here :)
    ret += "<br>" + i18n("<i>Start:</i>&nbsp;%1",
      (floats)
        ?(todo->dtStartDateStr().replace(" ", "&nbsp;"))
        :(todo->dtStartStr().replace(" ", "&nbsp;")) ) ;
  if (todo->hasDueDate())
    ret += "<br>" + i18n("<i>Due:</i>&nbsp;%1",
      (floats)
        ?(todo->dtDueDateStr().replace(" ", "&nbsp;"))
        :(todo->dtDueStr().replace(" ", "&nbsp;")) );
  if (todo->isCompleted())
    ret += "<br>" + i18n("<i>Completed:</i>&nbsp;%1", todo->completedStr().replace(" ", "&nbsp;") );
  else
    ret += "<br>" + i18n("%1 % completed", todo->percentComplete());

  return ret;
}

QString IncidenceFormatter::ToolTipVisitor::dateRangeText( Journal*journal )
{
  QString ret;
  if (journal->dtStart().isValid() ) {
    ret += "<br>" + i18n("<i>Date:</i>&nbsp;%1", journal->dtStartDateStr( false ) );
  }
  return ret;
}

QString IncidenceFormatter::ToolTipVisitor::dateRangeText( FreeBusy *fb )
{
  QString ret = "<br>" + i18n("<i>Period start:</i>&nbsp;%1", KGlobal::locale()->formatDateTime( fb->dtStart() ) );
  ret += "<br>" + i18n("<i>Period start:</i>&nbsp;%1", KGlobal::locale()->formatDateTime( fb->dtEnd() ) );
  return ret;
}



bool IncidenceFormatter::ToolTipVisitor::visit( Event *event )
{
  mResult = generateToolTip( event, dateRangeText( event ) );
  return !mResult.isEmpty();
}

bool IncidenceFormatter::ToolTipVisitor::visit( Todo *todo )
{
  mResult = generateToolTip( todo, dateRangeText( todo ) );
  return !mResult.isEmpty();
}

bool IncidenceFormatter::ToolTipVisitor::visit( Journal *journal )
{
  mResult = generateToolTip( journal, dateRangeText( journal ) );
  return !mResult.isEmpty();
}

bool IncidenceFormatter::ToolTipVisitor::visit( FreeBusy *fb )
{
  mResult = "<qt><b>" + i18n("Free/Busy information for %1",
         fb->organizer().fullName()) + "</b>";
  mResult += dateRangeText( fb );
  mResult += "</qt>";
  return !mResult.isEmpty();
}

QString IncidenceFormatter::ToolTipVisitor::generateToolTip( Incidence* incidence, QString dtRangeText )
{
  QString tmp = "<qt><b>"+ incidence->summary().replace("\n", "<br>")+"</b>";

  tmp += dtRangeText;

  if (!incidence->location().isEmpty()) {
    // Put Location: in italics
    tmp += "<br>"+i18n("<i>Location:</i>&nbsp;%1",
       incidence->location().replace("\n", "<br>") );
  }
  if (!incidence->description().isEmpty()) {
    QString desc(incidence->description());
    if (desc.length()>120) {
      desc = desc.left(120) + "...";
    }
    tmp += "<br>----------<br>" + i18n("<i>Description:</i><br>") + desc.replace("\n", "<br>");
  }
  tmp += "</qt>";
  return tmp;
}

QString IncidenceFormatter::toolTipString( IncidenceBase *incidence, bool richText )
{
  ToolTipVisitor v;
  if ( v.act( incidence, richText ) ) {
    return v.result();
  } else
    return QString();
}




/*******************************************************************
 *  Helper functions for the Incidence tooltips
 *******************************************************************/

class IncidenceFormatter::MailBodyVisitor : public IncidenceBase::Visitor
{
  public:
    MailBodyVisitor() : mResult( "" ) {}

    bool act( IncidenceBase *incidence )
    {
      mResult = "";
      return incidence ? incidence->accept( *this ) : false;
    }
    QString result() const { return mResult; }

  protected:
    bool visit( Event *event );
    bool visit( Todo *todo );
    bool visit( Journal *journal );
    bool visit( FreeBusy * ) { mResult = i18n("This is a Free Busy Object"); return !mResult.isEmpty(); }
  protected:
    QString mResult;
};


static QString mailBodyIncidence( Incidence *incidence )
{
  QString body;
  if ( !incidence->summary().isEmpty() ) {
    body += i18n("Summary: %1\n", incidence->summary() );
  }
  if ( !incidence->organizer().isEmpty() ) {
    body += i18n("Organizer: %1\n", incidence->organizer().fullName() );
  }
  if ( !incidence->location().isEmpty() ) {
    body += i18n("Location: %1\n", incidence->location() );
  }
  return body;
}

bool IncidenceFormatter::MailBodyVisitor::visit( Event *event )
{
  QString recurrence[]= {i18nc("no recurrence", "None"),
    i18n("Minutely"), i18n("Hourly"), i18n("Daily"),
    i18n("Weekly"), i18n("Monthly Same Day"), i18n("Monthly Same Position"),
    i18n("Yearly"), i18n("Yearly"), i18n("Yearly")};

  mResult = mailBodyIncidence( event );
  mResult += i18n("Start Date: %1\n", event->dtStartDateStr() );
  if ( !event->doesFloat() ) {
    mResult += i18n("Start Time: %1\n", event->dtStartTimeStr() );
  }
  if ( event->dtStart() != event->dtEnd() ) {
    mResult += i18n("End Date: %1\n", event->dtEndDateStr() );
  }
  if ( !event->doesFloat() ) {
    mResult += i18n("End Time: %1\n", event->dtEndTimeStr() );
  }
  if ( event->doesRecur() ) {
    Recurrence *recur = event->recurrence();
    // TODO: Merge these two to one of the form "Recurs every 3 days"
    mResult += i18n("Recurs: %1\n",
               recurrence[ recur->recurrenceType() ] );
    mResult += i18n("Frequency: %1\n",
               event->recurrence()->frequency() );

    if ( recur->duration() > 0 ) {
      mResult += i18np ("Repeats once", "Repeats %n times", recur->duration());
      mResult += '\n';
    } else {
      if ( recur->duration() != -1 ) {
// TODO_Recurrence: What to do with floating
        QString endstr;
        if ( event->doesFloat() ) {
          endstr = KGlobal::locale()->formatDate( recur->endDate() );
        } else {
          endstr = KGlobal::locale()->formatDateTime( recur->endDateTime() );
        }
        mResult += i18n("Repeat until: %1\n", endstr );
      } else {
        mResult += i18n("Repeats forever\n");
      }
    }
  }
  QString details = event->description();
  if ( !details.isEmpty() ) {
    mResult += i18n("Details:\n%1\n", details );
  }
  return !mResult.isEmpty();
}

bool IncidenceFormatter::MailBodyVisitor::visit( Todo *todo )
{
  mResult = mailBodyIncidence( todo );

  if ( todo->hasStartDate() ) {
    mResult += i18n("Start Date: %1\n", todo->dtStartDateStr() );
    if ( !todo->doesFloat() ) {
      mResult += i18n("Start Time: %1\n", todo->dtStartTimeStr() );
    }
  }
  if ( todo->hasDueDate() ) {
    mResult += i18n("Due Date: %1\n", todo->dtDueDateStr() );
    if ( !todo->doesFloat() ) {
      mResult += i18n("Due Time: %1\n", todo->dtDueTimeStr() );
    }
  }
  QString details = todo->description();
  if ( !details.isEmpty() ) {
    mResult += i18n("Details:\n%1\n", details );
  }
  return !mResult.isEmpty();
}

bool IncidenceFormatter::MailBodyVisitor::visit( Journal *journal )
{
  mResult = mailBodyIncidence( journal );
  mResult += i18n("Date: %1\n", journal->dtStartDateStr() );
  if ( !journal->doesFloat() ) {
    mResult += i18n("Time: %1\n", journal->dtStartTimeStr() );
  }
  if ( !journal->description().isEmpty() )
    mResult += i18n("Text of the journal:\n%1\n", journal->description() );
  return !mResult.isEmpty();
}



QString IncidenceFormatter::mailBodyString( IncidenceBase *incidence )
{
  if ( !incidence )
    return QString();

  MailBodyVisitor v;
  if ( v.act( incidence ) ) {
    return v.result();
  }
  return QString();
}
