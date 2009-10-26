/*
    This file is part of libkcal.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Copyright (c) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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
#include <libkcal/calendarresources.h>

#include <libemailfunctions/email.h>

#include <ktnef/ktnefparser.h>
#include <ktnef/ktnefmessage.h>
#include <ktnef/ktnefdefs.h>
#include <kabc/phonenumber.h>
#include <kabc/vcardconverter.h>
#include <kabc/stdaddressbook.h>

#include <kapplication.h>
#include <kemailsettings.h>
// #include <kdebug.h>

#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kcalendarsystem.h>
#include <kmimetype.h>

#include <qbuffer.h>
#include <qstylesheet.h>
#include <qdatetime.h>

#include <time.h>

using namespace KCal;

/*******************
 *  General helpers
 *******************/

static QString htmlAddLink( const QString &ref, const QString &text,
                             bool newline = true )
{
  QString tmpStr( "<a href=\"" + ref + "\">" + text + "</a>" );
  if ( newline ) tmpStr += "\n";
  return tmpStr;
}

static QString htmlAddTag( const QString & tag, const QString & text )
{
  int numLineBreaks = text.contains( "\n" );
  QString str = "<" + tag + ">";
  QString tmpText = text;
  QString tmpStr = str;
  if( numLineBreaks >= 0 ) {
    if ( numLineBreaks > 0) {
      int pos = 0;
      QString tmp;
      for( int i = 0; i <= numLineBreaks; i++ ) {
        pos = tmpText.find( "\n" );
        tmp = tmpText.left( pos );
        tmpText = tmpText.right( tmpText.length() - pos - 1 );
        tmpStr += tmp + "<br>";
      }
    } else {
      tmpStr += tmpText;
    }
  }
  tmpStr += "</" + tag + ">";
  return tmpStr;
}

static bool iamAttendee( Attendee *attendee )
{
  // Check if I'm this attendee

  bool iam = false;
  KEMailSettings settings;
  QStringList profiles = settings.profiles();
  for( QStringList::Iterator it=profiles.begin(); it!=profiles.end(); ++it ) {
    settings.setProfile( *it );
    if ( settings.getSetting( KEMailSettings::EmailAddress ) == attendee->email() ) {
      iam = true;
      break;
    }
  }
  return iam;
}

static bool iamOrganizer( Incidence *incidence )
{
  // Check if I'm the organizer for this incidence

  if ( !incidence ) {
    return false;
  }

  bool iam = false;
  KEMailSettings settings;
  QStringList profiles = settings.profiles();
  for( QStringList::Iterator it=profiles.begin(); it!=profiles.end(); ++it ) {
    settings.setProfile( *it );
    if ( settings.getSetting( KEMailSettings::EmailAddress ) == incidence->organizer().email() ) {
      iam = true;
      break;
    }
  }
  return iam;
}

/*******************************************************************
 *  Helper functions for the extensive display (display viewer)
 *******************************************************************/

static QString displayViewLinkPerson( const QString& email, QString name, QString uid )
{
  // Make the search, if there is an email address to search on,
  // and either name or uid is missing
  if ( !email.isEmpty() && ( name.isEmpty() || uid.isEmpty() ) ) {
    KABC::AddressBook *add_book = KABC::StdAddressBook::self( true );
    KABC::Addressee::List addressList = add_book->findByEmail( email );
    if ( !addressList.isEmpty() ) {
      KABC::Addressee o = addressList.first();
      if ( !o.isEmpty() && addressList.size() < 2 ) {
        if ( name.isEmpty() ) {
          // No name set, so use the one from the addressbook
          name = o.formattedName();
        }
        uid = o.uid();
      } else {
        // Email not found in the addressbook. Don't make a link
        uid = QString::null;
      }
    }
  }
  kdDebug(5850) << "formatAttendees: uid = " << uid << endl;

  // Show the attendee
  QString tmpString;
  if ( !uid.isEmpty() ) {
    // There is a UID, so make a link to the addressbook
    if ( name.isEmpty() ) {
      // Use the email address for text
      tmpString += htmlAddLink( "uid:" + uid, email );
    } else {
      tmpString += htmlAddLink( "uid:" + uid, name );
    }
  } else {
    // No UID, just show some text
    tmpString += ( name.isEmpty() ? email : name );
  }

  // Make the mailto link
  if ( !email.isEmpty() ) {
    KURL mailto;
    mailto.setProtocol( "mailto" );
    mailto.setPath( email );
    const QString iconPath =
      KGlobal::iconLoader()->iconPath( "mail_new", KIcon::Small );
    tmpString += htmlAddLink( mailto.url(),
                              "<img valign=\"top\" src=\"" + iconPath + "\">" );
  }

  return tmpString;
}

static QString displayViewFormatAttendees( Incidence *incidence )
{
  QString tmpStr;
  Attendee::List attendees = incidence->attendees();

  // Add organizer link
  tmpStr += "<tr>";
  tmpStr += "<td><b>" + i18n( "Organizer:" ) + "</b></td>";
  tmpStr += "<td>" +
            displayViewLinkPerson( incidence->organizer().email(),
                                   incidence->organizer().name(),
                                   QString::null ) +
            "</td>";
  tmpStr += "</tr>";

  // Add attendees links
  tmpStr += "<tr>";
  tmpStr += "<td><b>" + i18n( "Attendees:" ) + "</b></td>";
  tmpStr += "<td>";
  Attendee::List::ConstIterator it;
  for( it = attendees.begin(); it != attendees.end(); ++it ) {
    Attendee *a = *it;
    if ( a->email() == incidence->organizer().email() ) {
      // skip attendee that is also the organizer
      continue;
    }
    tmpStr += displayViewLinkPerson( a->email(), a->name(), a->uid() );
    if ( !a->delegator().isEmpty() ) {
      tmpStr += i18n(" (delegated by %1)" ).arg( a->delegator() );
    }
    if ( !a->delegate().isEmpty() ) {
      tmpStr += i18n(" (delegated to %1)" ).arg( a->delegate() );
    }
    tmpStr += "<br>";
  }
  if ( tmpStr.endsWith( "<br>" ) ) {
    tmpStr.truncate( tmpStr.length() - 4 );
  }

  tmpStr += "</td>";
  tmpStr += "</tr>";
  return tmpStr;
}

static QString displayViewFormatAttachments( Incidence *incidence )
{
  QString tmpStr;
  Attachment::List as = incidence->attachments();
  Attachment::List::ConstIterator it;
  uint count = 0;
  for( it = as.begin(); it != as.end(); ++it ) {
    count++;
    if ( (*it)->isUri() ) {
      QString name;
      if ( (*it)->uri().startsWith( "kmail:" ) ) {
        name = i18n( "Show mail" );
      } else {
        name = (*it)->label();
      }
      tmpStr += htmlAddLink( (*it)->uri(), name );
    } else {
      tmpStr += (*it)->label();
    }
    if ( count < as.count() ) {
      tmpStr += "<br>";
    }
  }
  return tmpStr;
}

static QString displayViewFormatCategories( Incidence *incidence )
{
  return incidence->categoriesStr();
}

static QString displayViewFormatCreationDate( Incidence *incidence )
{
  return i18n( "Creation date: %1" ).
    arg( IncidenceFormatter::dateTimeToString( incidence->created(), false, true ) );
}

static QString displayViewFormatBirthday( Event *event )
{
  if ( !event ) {
    return  QString::null;
  }
  if ( event->customProperty("KABC","BIRTHDAY") != "YES" ) {
    return QString::null;
  }

  QString uid = event->customProperty("KABC","UID-1");
  QString name = event->customProperty("KABC","NAME-1");
  QString email= event->customProperty("KABC","EMAIL-1");

  QString tmpStr = displayViewLinkPerson( email, name, uid );

  if ( event->customProperty( "KABC", "ANNIVERSARY") == "YES" ) {
    uid = event->customProperty("KABC","UID-2");
    name = event->customProperty("KABC","NAME-2");
    email= event->customProperty("KABC","EMAIL-2");
    tmpStr += "<br>";
    tmpStr += displayViewLinkPerson( email, name, uid );
  }

  return tmpStr;
}

static QString displayViewFormatHeader( Incidence *incidence )
{
  QString tmpStr = "<table><tr>";

  // show icons
  {
    tmpStr += "<td>";

    if ( incidence->type() == "Event" ) {
      QString iconPath;
      if ( incidence->customProperty( "KABC", "BIRTHDAY" ) == "YES" ) {
        if ( incidence->customProperty( "KABC", "ANNIVERSARY" ) == "YES" ) {
          iconPath =
            KGlobal::iconLoader()->iconPath( "calendaranniversary", KIcon::Small );
        } else {
          iconPath = KGlobal::iconLoader()->iconPath( "calendarbirthday", KIcon::Small );
        }
      } else {
        iconPath = KGlobal::iconLoader()->iconPath( "appointment", KIcon::Small );
      }
      tmpStr += "<img valign=\"top\" src=\"" + iconPath + "\">";
    }
    if ( incidence->type() == "Todo" ) {
      tmpStr += "<img valign=\"top\" src=\"" +
                KGlobal::iconLoader()->iconPath( "todo", KIcon::Small ) +
                "\">";
    }
    if ( incidence->type() == "Journal" ) {
      tmpStr += "<img valign=\"top\" src=\"" +
                KGlobal::iconLoader()->iconPath( "journal", KIcon::Small ) +
                "\">";
    }
    if ( incidence->isAlarmEnabled() ) {
      tmpStr += "<img valign=\"top\" src=\"" +
                KGlobal::iconLoader()->iconPath( "bell", KIcon::Small ) +
                "\">";
    }
    if ( incidence->doesRecur() ) {
      tmpStr += "<img valign=\"top\" src=\"" +
                KGlobal::iconLoader()->iconPath( "recur", KIcon::Small ) +
                "\">";
    }
    if ( incidence->isReadOnly() ) {
      tmpStr += "<img valign=\"top\" src=\"" +
                KGlobal::iconLoader()->iconPath( "readonlyevent", KIcon::Small ) +
                "\">";
    }

    tmpStr += "</td>";
  }

  tmpStr += "<td>";
  tmpStr += "<b><u>" + incidence->summary() + "</u></b>";
  tmpStr += "</td>";

  tmpStr += "</tr></table>";

  return tmpStr;
}

static QString displayViewFormatEvent( Calendar *calendar, Event *event,
                                       const QDate &date )
{
  if ( !event ) {
    return QString::null;
  }

  QString tmpStr = displayViewFormatHeader( event );

  tmpStr += "<table>";
  tmpStr += "<col width=\"25%\"/>";
  tmpStr += "<col width=\"75%\"/>";

  if ( calendar ) {
    QString calStr = IncidenceFormatter::resourceString( calendar, event );
    if ( !calStr.isEmpty() ) {
      tmpStr += "<tr>";
      tmpStr += "<td><b>" + i18n( "Calendar:" ) + "</b></td>";
      tmpStr += "<td>" + calStr + "</td>";
      tmpStr += "</tr>";
    }
  }

  if ( !event->location().isEmpty() ) {
    tmpStr += "<tr>";
    tmpStr += "<td><b>" + i18n( "Location:" ) + "</b></td>";
    tmpStr += "<td>" + event->location() + "</td>";
    tmpStr += "</tr>";
  }

  tmpStr += "<tr>";
  QDateTime startDt = event->dtStart();
  QDateTime endDt = event->dtEnd();
  if ( event->doesRecur() ) {
    if ( date.isValid() ) {
      QDateTime dt( date, QTime( 0, 0, 0 ) );
      int diffDays = startDt.daysTo( dt );
      dt = dt.addSecs( -1 );
      startDt.setDate( event->recurrence()->getNextDateTime( dt ).date() );
      if ( event->hasEndDate() ) {
        endDt = endDt.addDays( diffDays );
        if ( startDt > endDt ) {
          startDt.setDate( event->recurrence()->getPreviousDateTime( dt ).date() );
          endDt = startDt.addDays( event->dtStart().daysTo( event->dtEnd() ) );
        }
      }
    }
  }
  if ( event->doesFloat() ) {
    if ( event->isMultiDay() ) {
      tmpStr += "<td><b>" + i18n( "Time:" ) + "</b></td>";
      tmpStr += "<td>" +
                i18n("<beginTime> - <endTime>","%1 - %2").
                arg( IncidenceFormatter::dateToString( startDt, false ) ).
                arg( IncidenceFormatter::dateToString( endDt, false ) ) +
                "</td>";
    } else {
      tmpStr += "<td><b>" + i18n( "Date:" ) + "</b></td>";
      tmpStr += "<td>" +
                i18n("date as string","%1").
                arg( IncidenceFormatter::dateToString( startDt, false ) ) +
                "</td>";
    }
  } else {
    if ( event->isMultiDay() ) {
      tmpStr += "<td><b>" + i18n( "Time:" ) + "</b></td>";
      tmpStr += "<td>" +
                i18n("<beginTime> - <endTime>","%1 - %2").
                arg( IncidenceFormatter::dateToString( startDt, false ) ).
                arg( IncidenceFormatter::dateToString( endDt, false ) ) +
                "</td>";
    } else {
      tmpStr += "<td><b>" + i18n( "Time:" ) + "</b></td>";
      if ( event->hasEndDate() && startDt != endDt ) {
        tmpStr += "<td>" +
                  i18n("<beginTime> - <endTime>","%1 - %2").
                  arg( IncidenceFormatter::timeToString( startDt, true ) ).
                  arg( IncidenceFormatter::timeToString( endDt, true ) ) +
                  "</td>";
      } else {
        tmpStr += "<td>" +
                  IncidenceFormatter::timeToString( startDt, true ) +
                  "</td>";
      }
      tmpStr += "</tr><tr>";
      tmpStr += "<td><b>" + i18n( "Date:" ) + "</b></td>";
      tmpStr += "<td>" +
                i18n("date as string","%1").
                arg( IncidenceFormatter::dateToString( startDt, false ) ) +
                "</td>";
    }
  }
  tmpStr += "</tr>";

  if ( event->customProperty("KABC","BIRTHDAY")== "YES" ) {
    tmpStr += "<tr>";
    if ( event->customProperty( "KABC", "ANNIVERSARY" ) == "YES" ) {
      tmpStr += "<td><b>" + i18n( "Anniversary:" ) + "</b></td>";
    } else {
      tmpStr += "<td><b>" + i18n( "Birthday:" ) + "</b></td>";
    }
    tmpStr += "<td>" + displayViewFormatBirthday( event ) + "</td>";
    tmpStr += "</tr>";
    tmpStr += "</table>";
    return tmpStr;
  }

  if ( !event->description().isEmpty() ) {
    tmpStr += "<tr>";
    tmpStr += "<td><b>" + i18n( "Description:" ) + "</b></td>";
    tmpStr += "<td>" + event->description() + "</td>";
    tmpStr += "</tr>";
  }

  int categoryCount = event->categories().count();
  if ( categoryCount > 0 ) {
    tmpStr += "<tr>";
    tmpStr += "<td><b>" +
              i18n( "Category:", "Categories:", categoryCount ) +
              "</b></td>";
    tmpStr += "<td>" + displayViewFormatCategories( event ) + "</td>";
    tmpStr += "</tr>";
  }

  if ( event->doesRecur() ) {
    QDateTime dt =
      event->recurrence()->getNextDateTime( QDateTime::currentDateTime() );
    tmpStr += "<tr>";
    tmpStr += "<td><b>" + i18n( "next recurrence", "Next:" ) + "</b></td>";
    tmpStr += "<td>" +
              ( dt.isValid() ?
                IncidenceFormatter::dateTimeToString( dt, event->doesFloat(), false ) :
                i18n( "no date", "none" ) ) +
              "</td>";
    tmpStr += "</tr>";
  }

  if ( event->attendees().count() > 1 ) {
    tmpStr += displayViewFormatAttendees( event );
  }

  int attachmentCount = event->attachments().count();
  if ( attachmentCount > 0 ) {
    tmpStr += "<tr>";
    tmpStr += "<td><b>" +
              i18n( "Attachment:", "Attachments:", attachmentCount ) +
              "</b></td>";
    tmpStr += "<td>" + displayViewFormatAttachments( event ) + "</td>";
    tmpStr += "</tr>";
  }
  tmpStr += "</table>";

  tmpStr += "<em>" + displayViewFormatCreationDate( event ) + "</em>";

  return tmpStr;
}

static QString displayViewFormatTodo( Calendar *calendar, Todo *todo,
                                      const QDate &date )
{
  if ( !todo ) {
    return QString::null;
  }

  QString tmpStr = displayViewFormatHeader( todo );

  tmpStr += "<table>";
  tmpStr += "<col width=\"25%\"/>";
  tmpStr += "<col width=\"75%\"/>";

  if ( calendar ) {
    QString calStr = IncidenceFormatter::resourceString( calendar, todo );
    if ( !calStr.isEmpty() ) {
      tmpStr += "<tr>";
      tmpStr += "<td><b>" + i18n( "Calendar:" ) + "</b></td>";
      tmpStr += "<td>" + calStr + "</td>";
      tmpStr += "</tr>";
    }
  }

  if ( !todo->location().isEmpty() ) {
    tmpStr += "<tr>";
    tmpStr += "<td><b>" + i18n( "Location:" ) + "</b></td>";
    tmpStr += "<td>" + todo->location() + "</td>";
    tmpStr += "</tr>";
  }

  if ( todo->hasStartDate() && todo->dtStart().isValid() ) {
    QDateTime startDt = todo->dtStart();
    if ( todo->doesRecur() ) {
      if ( date.isValid() ) {
        startDt.setDate( date );
      }
    }
    tmpStr += "<tr>";
    tmpStr += "<td><b>" + i18n( "Start:" ) + "</b></td>";
    tmpStr += "<td>" +
              IncidenceFormatter::dateTimeToString( startDt,
                                                    todo->doesFloat(), false ) +
              "</td>";
    tmpStr += "</tr>";
  }

  if ( todo->hasDueDate() && todo->dtDue().isValid() ) {
    QDateTime dueDt = todo->dtDue();
    if ( todo->doesRecur() ) {
      if ( date.isValid() ) {
        dueDt.addDays( todo->dtDue().date().daysTo( date ) );
      }
    }
    tmpStr += "<tr>";
    tmpStr += "<td><b>" + i18n( "Due:" ) + "</b></td>";
    tmpStr += "<td>" +
              IncidenceFormatter::dateTimeToString( dueDt,
                                                    todo->doesFloat(), false ) +
              "</td>";
    tmpStr += "</tr>";
  }

  if ( !todo->description().isEmpty() ) {
    tmpStr += "<tr>";
    tmpStr += "<td><b>" + i18n( "Description:" ) + "</b></td>";
    tmpStr += "<td>" + todo->description() + "</td>";
    tmpStr += "</tr>";
  }

  int categoryCount = todo->categories().count();
  if ( categoryCount > 0 ) {
    tmpStr += "<tr>";
    tmpStr += "<td><b>" +
              i18n( "Category:", "Categories:", categoryCount ) +
              "</b></td>";
    tmpStr += "<td>" + displayViewFormatCategories( todo ) + "</td>";
    tmpStr += "</tr>";
  }

  tmpStr += "<tr>";
  tmpStr += "<td><b>" + i18n( "Priority:" ) + "</b></td>";
  tmpStr += "<td>";
  if ( todo->priority() > 0 ) {
    tmpStr += QString::number( todo->priority() );
  } else {
    tmpStr += i18n( "Unspecified" );
  }
  tmpStr += "</td>";
  tmpStr += "</tr>";

  tmpStr += "<tr>";
  tmpStr += "<td><b>" + i18n( "Completed:" ) + "</b></td>";
  tmpStr += "<td>" + i18n( "%1%" ).arg( todo->percentComplete() ) + "</td>";
  tmpStr += "</tr>";

  if ( todo->doesRecur() ) {
    QDateTime dt =
      todo->recurrence()->getNextDateTime( QDateTime::currentDateTime() );
    tmpStr += "<tr>";
    tmpStr += "<td><b>" +
              i18n( "next recurrence", "Next:" ) +
              "</b></td>";
    tmpStr += ( dt.isValid() ?
                IncidenceFormatter::dateTimeToString( dt, todo->doesFloat(), false ) :
                i18n( "no date", "none" ) ) +
              "</td>";
    tmpStr += "</tr>";
  }

  if ( todo->attendees().count() > 1 ) {
    tmpStr += displayViewFormatAttendees( todo );
  }

  int attachmentCount = todo->attachments().count();
  if ( attachmentCount > 0 ) {
    tmpStr += "<tr>";
    tmpStr += "<td><b>" +
              i18n( "Attachment:", "Attachments:", attachmentCount ) +
              "</b></td>";
    tmpStr += "<td>" + displayViewFormatAttachments( todo ) + "</td>";
    tmpStr += "</tr>";
  }

  tmpStr += "</table>";

  tmpStr += "<em>" + displayViewFormatCreationDate( todo ) + "</em>";

  return tmpStr;
}

static QString displayViewFormatJournal( Calendar *calendar, Journal *journal )
{
  if ( !journal ) {
    return QString::null;
  }

  QString tmpStr = displayViewFormatHeader( journal );

  tmpStr += "<table>";
  tmpStr += "<col width=\"25%\"/>";
  tmpStr += "<col width=\"75%\"/>";

  if ( calendar ) {
    QString calStr = IncidenceFormatter::resourceString( calendar, journal );
    if ( !calStr.isEmpty() ) {
      tmpStr += "<tr>";
      tmpStr += "<td><b>" + i18n( "Calendar:" ) + "</b></td>";
      tmpStr += "<td>" + calStr + "</td>";
      tmpStr += "</tr>";
    }
  }

  tmpStr += "<tr>";
  tmpStr += "<td><b>" + i18n( "Date:" ) + "</b></td>";
  tmpStr += "<td>" +
            IncidenceFormatter::dateToString( journal->dtStart(), false ) +
            "</td>";
  tmpStr += "</tr>";

  if ( !journal->description().isEmpty() ) {
    tmpStr += "<tr>";
    tmpStr += "<td><b>" + i18n( "Description:" ) + "</b></td>";
    tmpStr += "<td>" + journal->description() + "</td>";
    tmpStr += "</tr>";
  }

  int categoryCount = journal->categories().count();
  if ( categoryCount > 0 ) {
    tmpStr += "<tr>";
    tmpStr += "<td><b>" +
              i18n( "Category:", "Categories:", categoryCount ) +
              "</b></td>";
    tmpStr += "<td>" + displayViewFormatCategories( journal ) + "</td>";
    tmpStr += "</tr>";
  }
  tmpStr += "</table>";

  tmpStr += "<em>" + displayViewFormatCreationDate( journal ) + "</em>";

  return tmpStr;
}

static QString displayViewFormatFreeBusy( Calendar * /*calendar*/, FreeBusy *fb )
{
  if ( !fb ) {
    return QString::null;
  }

  QString tmpStr = htmlAddTag( "h2",
                               htmlAddTag( "b",
                                           i18n("Free/Busy information for %1").
                                           arg( fb->organizer().fullName() ) ) );

  tmpStr += htmlAddTag( "h4", i18n("Busy times in date range %1 - %2:").
                        arg( IncidenceFormatter::dateToString( fb->dtStart(), true ) ).
                        arg( IncidenceFormatter::dateToString( fb->dtEnd(), true ) ) );

  QValueList<Period> periods = fb->busyPeriods();

  QString text = htmlAddTag( "em", htmlAddTag( "b", i18n("Busy:") ) );
  QValueList<Period>::iterator it;
  for ( it = periods.begin(); it != periods.end(); ++it ) {
    Period per = *it;
    if ( per.hasDuration() ) {
      int dur = per.duration().asSeconds();
      QString cont;
      if ( dur >= 3600 ) {
        cont += i18n("1 hour ", "%n hours ", dur / 3600 );
        dur %= 3600;
      }
      if ( dur >= 60 ) {
        cont += i18n("1 minute ", "%n minutes ", dur / 60);
        dur %= 60;
      }
      if ( dur > 0 ) {
        cont += i18n("1 second", "%n seconds", dur);
      }
      text += i18n("startDate for duration", "%1 for %2").
              arg( IncidenceFormatter::dateTimeToString( per.start(), false, true ) ).
              arg( cont );
      text += "<br>";
    } else {
      if ( per.start().date() == per.end().date() ) {
        text += i18n("date, fromTime - toTime ", "%1, %2 - %3").
                arg( IncidenceFormatter::dateToString( per.start().date(), true ) ).
                arg( IncidenceFormatter::timeToString( per.start(), true ) ).
                arg( IncidenceFormatter::timeToString( per.end(), true ) );
      } else {
        text += i18n("fromDateTime - toDateTime", "%1 - %2").
                arg( IncidenceFormatter::dateTimeToString( per.start(), false, true ) ).
                arg( IncidenceFormatter::dateTimeToString( per.end(), false, true ) );
      }
      text += "<br>";
    }
  }
  tmpStr += htmlAddTag( "p", text );
  return tmpStr;
}

class IncidenceFormatter::EventViewerVisitor : public IncidenceBase::Visitor
{
  public:
    EventViewerVisitor()
      : mCalendar( 0 ), mResult( "" ) {}

    bool act( Calendar *calendar, IncidenceBase *incidence, const QDate &date )
    {
      mCalendar = calendar;
      mDate = date;
      mResult = "";
      return incidence->accept( *this );
    }
    QString result() const { return mResult; }

  protected:
    bool visit( Event *event )
    {
      mResult = displayViewFormatEvent( mCalendar, event, mDate );
      return !mResult.isEmpty();
    }
    bool visit( Todo *todo )
    {
      mResult = displayViewFormatTodo( mCalendar, todo, mDate );
      return !mResult.isEmpty();
    }
    bool visit( Journal *journal )
    {
      mResult = displayViewFormatJournal( mCalendar, journal );
      return !mResult.isEmpty();
    }
    bool visit( FreeBusy *fb )
    {
      mResult = displayViewFormatFreeBusy( mCalendar, fb );
      return !mResult.isEmpty();
    }

  protected:
    Calendar *mCalendar;
    QDate mDate;
    QString mResult;
};

QString IncidenceFormatter::extensiveDisplayString( IncidenceBase *incidence )
{
  return extensiveDisplayStr( 0, incidence, QDate() );
}

QString IncidenceFormatter::extensiveDisplayStr( Calendar *calendar,
                                                 IncidenceBase *incidence,
                                                 const QDate &date )
{
  if ( !incidence ) {
    return QString::null;
  }

  EventViewerVisitor v;
  if ( v.act( calendar, incidence, date ) ) {
    return v.result();
  } else {
    return QString::null;
  }
}

/***********************************************************************
 *  Helper functions for the body part formatter of kmail (Invitations)
 ***********************************************************************/

static QString string2HTML( const QString& str )
{
  return QStyleSheet::convertFromPlainText(str, QStyleSheetItem::WhiteSpaceNormal);
}

static QString eventStartTimeStr( Event *event )
{
  QString tmp;
  if ( !event->doesFloat() ) {
    tmp = i18n( "%1: Start Date, %2: Start Time", "%1 %2" ).
          arg( IncidenceFormatter::dateToString( event->dtStart(), true ),
               IncidenceFormatter::timeToString( event->dtStart(), true ) );
  } else {
    tmp = i18n( "%1: Start Date", "%1 (all day)" ).
          arg( IncidenceFormatter::dateToString( event->dtStart(), true ) );
  }
  return tmp;
}

static QString eventEndTimeStr( Event *event )
{
  QString tmp;
  if ( event->hasEndDate() && event->dtEnd().isValid() ) {
    if ( !event->doesFloat() ) {
      tmp = i18n( "%1: End Date, %2: End Time", "%1 %2" ).
            arg( IncidenceFormatter::dateToString( event->dtEnd(), true ),
                 IncidenceFormatter::timeToString( event->dtEnd(), true ) );
    } else {
      tmp = i18n( "%1: End Date", "%1 (all day)" ).
            arg( IncidenceFormatter::dateToString( event->dtEnd(), true ) );
    }
  }
  return tmp;
}

static QString invitationRow( const QString &cell1, const QString &cell2 )
{
  return "<tr><td>" + cell1 + "</td><td>" + cell2 + "</td></tr>\n";
}

static Attendee *findMyAttendee( Incidence *incidence )
{
  // Return the attendee for the incidence that is probably me

  Attendee *attendee = 0;
  if ( !incidence ) {
    return attendee;
  }

  KEMailSettings settings;
  QStringList profiles = settings.profiles();
  for( QStringList::Iterator it=profiles.begin(); it!=profiles.end(); ++it ) {
    settings.setProfile( *it );

    Attendee::List attendees = incidence->attendees();
    Attendee::List::ConstIterator it2;
    for ( it2 = attendees.begin(); it2 != attendees.end(); ++it2 ) {
      Attendee *a = *it2;
      if ( settings.getSetting( KEMailSettings::EmailAddress ) == a->email() ) {
        attendee = a;
        break;
      }
    }
  }
  return attendee;
}

static Attendee *findAttendee( Incidence *incidence, const QString &email )
{
  // Search for an attendee by email address

  Attendee *attendee = 0;
  if ( !incidence ) {
    return attendee;
  }

  Attendee::List attendees = incidence->attendees();
  Attendee::List::ConstIterator it;
  for ( it = attendees.begin(); it != attendees.end(); ++it ) {
    Attendee *a = *it;
    if ( email == a->email() ) {
      attendee = a;
      break;
    }
  }
  return attendee;
}

static bool rsvpRequested( Incidence *incidence )
{
  if ( !incidence ) {
    return false;
  }

  //use a heuristic to determine if a response is requested.

  bool rsvp = true; // better send superfluously than not at all
  Attendee::List attendees = incidence->attendees();
  Attendee::List::ConstIterator it;
  for ( it = attendees.begin(); it != attendees.end(); ++it ) {
    if ( it == attendees.begin() ) {
      rsvp = (*it)->RSVP(); // use what the first one has
    } else {
      if ( (*it)->RSVP() != rsvp ) {
        rsvp = true; // they differ, default
        break;
      }
    }
  }
  return rsvp;
}

static QString rsvpRequestedStr( bool rsvpRequested )
{
  if ( rsvpRequested ) {
    return i18n( "Your response is requested" );
  } else {
    return i18n( "A response is not necessary" );
  }
}

static QString invitationPerson( const QString& email, QString name, QString uid )
{
  // Make the search, if there is an email address to search on,
  // and either name or uid is missing
  if ( !email.isEmpty() && ( name.isEmpty() || uid.isEmpty() ) ) {
    KABC::AddressBook *add_book = KABC::StdAddressBook::self( true );
    KABC::Addressee::List addressList = add_book->findByEmail( email );
    if ( !addressList.isEmpty() ) {
      KABC::Addressee o = addressList.first();
      if ( !o.isEmpty() && addressList.size() < 2 ) {
        if ( name.isEmpty() ) {
          // No name set, so use the one from the addressbook
          name = o.formattedName();
        }
        uid = o.uid();
      } else {
        // Email not found in the addressbook. Don't make a link
        uid = QString::null;
      }
    }
  }

  // Show the attendee
  QString tmpString;
  if ( !uid.isEmpty() ) {
    // There is a UID, so make a link to the addressbook
    if ( name.isEmpty() ) {
      // Use the email address for text
      tmpString += htmlAddLink( "uid:" + uid, email );
    } else {
      tmpString += htmlAddLink( "uid:" + uid, name );
    }
  } else {
    // No UID, just show some text
    tmpString += ( name.isEmpty() ? email : name );
  }
  tmpString += '\n';

  // Make the mailto link
  if ( !email.isEmpty() ) {
    KCal::Person person( name, email );
    KURL mailto;
    mailto.setProtocol( "mailto" );
    mailto.setPath( person.fullName() );
    const QString iconPath =
      KGlobal::iconLoader()->iconPath( "mail_new", KIcon::Small );
    tmpString += htmlAddLink( mailto.url(), "<img src=\"" + iconPath + "\">" )
;
  }
  tmpString += "\n";

  return tmpString;
}

static QString invitationsDetailsIncidence( Incidence *incidence )
{
  QString html;
  QString descr;
  QStringList comments;
  if ( incidence->comments().isEmpty() && !incidence->description().isEmpty() ) {
    comments << incidence->description();
  } else {
    descr = incidence->description();
    comments = incidence->comments();
  }

  if( !descr.isEmpty() ) {
    html += "<br/><u>" + i18n("Description:")
      + "</u><table border=\"0\"><tr><td>&nbsp;</td><td>";
    html += string2HTML(descr) + "</td></tr></table>";
  }
  if ( !comments.isEmpty() ) {
    html += "<br><u>" + i18n("Comments:")
          + "</u><table border=\"0\"><tr><td>&nbsp;</td><td>";
    if ( comments.count() > 1 ) {
      html += "<ul>";
      for ( uint i = 0; i < comments.count(); ++i )
        html += "<li>" + string2HTML( comments[i] ) + "</li>";
      html += "</ul>";
    } else {
      html += string2HTML( comments[0] );
    }
    html += "</td></tr></table>";
  }
  return html;
}

static QString invitationDetailsEvent( Event* event )
{
  // Invitation details are formatted into an HTML table
  if ( !event )
    return QString::null;

  QString html;

  QString sSummary = i18n( "Summary unspecified" );
  if ( ! event->summary().isEmpty() ) {
    sSummary = QStyleSheet::escape( event->summary() );
  }

  QString sLocation = i18n( "Location unspecified" );
  if ( ! event->location().isEmpty() ) {
    sLocation = QStyleSheet::escape( event->location() );
  }

  QString dir = ( QApplication::reverseLayout() ? "rtl" : "ltr" );
  html = QString("<div dir=\"%1\">\n").arg(dir);

  html += "<table border=\"0\" cellpadding=\"1\" cellspacing=\"1\">\n";

  // Invitation summary & location rows
  html += invitationRow( i18n( "What:" ), sSummary );
  html += invitationRow( i18n( "Where:" ), sLocation );

  // If a 1 day event
  if ( event->dtStart().date() == event->dtEnd().date() ) {
    html += invitationRow( i18n( "Date:" ),
                           IncidenceFormatter::dateToString( event->dtStart(), false ) );
    if ( !event->doesFloat() ) {
      html += invitationRow( i18n( "Time:" ),
                             IncidenceFormatter::timeToString( event->dtStart(), true ) +
                             " - " +
                             IncidenceFormatter::timeToString( event->dtEnd(), true ) );
    }
  } else {
    html += invitationRow( i18n( "Starting date of an event", "From:" ),
                           IncidenceFormatter::dateToString( event->dtStart(), false ) );
    if ( !event->doesFloat() ) {
      html += invitationRow( i18n( "Starting time of an event", "At:" ),
                             IncidenceFormatter::timeToString( event->dtStart(), true ) );
    }
    if ( event->hasEndDate() ) {
      html += invitationRow( i18n( "Ending date of an event", "To:" ),
                             IncidenceFormatter::dateToString( event->dtEnd(), false ) );
      if ( !event->doesFloat() ) {
        html += invitationRow( i18n( "Starting time of an event", "At:" ),
                               IncidenceFormatter::timeToString( event->dtEnd(), true ) );
      }
    } else {
      html += invitationRow( i18n( "Ending date of an event", "To:" ),
                             i18n( "no end date specified" ) );
    }
  }

  // Invitation Duration Row
  if ( !event->doesFloat() && event->hasEndDate() ) {
    QString tmp;
    int secs = event->dtStart().secsTo( event->dtEnd() );
    int days = secs / 86400;
    if ( days > 0 ) {
      tmp += i18n( "1 day", "%n days", days );
      tmp += ' ';
      secs -= ( days * 86400 );
    }
    int hours = secs / 3600;
    if ( hours > 0 ) {
      tmp += i18n( "1 hour", "%n hours", hours );
      tmp += ' ';
      secs -= ( hours * 3600 );
    }
    int mins = secs / 60;
    if ( mins > 0 ) {
      tmp += i18n( "1 minute", "%n minutes",  mins );
      tmp += ' ';
    }
    html += invitationRow( i18n( "Duration:" ), tmp );
  }

  if ( event->doesRecur() )
    html += invitationRow( i18n( "Recurrence:" ), IncidenceFormatter::recurrenceString( event ) );

  html += "</table>\n";
  html += invitationsDetailsIncidence( event );
  html += "</div>\n";

  return html;
}

static QString invitationDetailsTodo( Todo *todo )
{
  // Task details are formatted into an HTML table
  if ( !todo )
    return QString::null;

  QString sSummary = i18n( "Summary unspecified" );
  if ( ! todo->summary().isEmpty() ) {
    sSummary = QStyleSheet::escape( todo->summary() );
  }

  QString sLocation = i18n( "Location unspecified" );
  if ( ! todo->location().isEmpty() ) {
    sLocation = QStyleSheet::escape( todo->location() );
  }

  QString dir = ( QApplication::reverseLayout() ? "rtl" : "ltr" );
  QString html = QString("<div dir=\"%1\">\n").arg(dir);

  html += "<table border=\"0\" cellpadding=\"1\" cellspacing=\"1\">\n";

  // Invitation summary & location rows
  html += invitationRow( i18n( "What:" ), sSummary );
  html += invitationRow( i18n( "Where:" ), sLocation );

  if ( todo->hasStartDate() && todo->dtStart().isValid() ) {
    html += invitationRow( i18n( "Start Date:" ),
                           IncidenceFormatter::dateToString( todo->dtStart(), false ) );
  }
  if ( todo->hasDueDate() && todo->dtDue().isValid() ) {
    html += invitationRow( i18n( "Due Date:" ),
                           IncidenceFormatter::dateToString( todo->dtDue(), false ) );
    if ( !todo->doesFloat() ) {
      html += invitationRow( i18n( "Due Time:" ),
                             KGlobal::locale()->formatTime( todo->dtDue().time() ) );
    }

  } else {
    html += invitationRow( i18n( "Due Date:" ),
                           i18n( "Due Date: None", "None" ) );
  }

  html += "</table></div>\n";
  html += invitationsDetailsIncidence( todo );

  return html;
}

static QString invitationDetailsJournal( Journal *journal )
{
  if ( !journal )
    return QString::null;

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
  html += invitationRow( i18n( "Date:" ),
                         IncidenceFormatter::dateToString( journal->dtStart(), false ) );
  html += invitationRow( i18n( "Description:" ), sDescr );
  html += "</table>\n";
  html += invitationsDetailsIncidence( journal );

  return html;
}

static QString invitationDetailsFreeBusy( FreeBusy *fb )
{
  if ( !fb )
    return QString::null;
  QString html( "<table border=\"0\" cellpadding=\"1\" cellspacing=\"1\">\n" );

  html += invitationRow( i18n("Person:"), fb->organizer().fullName() );
  html += invitationRow( i18n("Start date:"),
                         IncidenceFormatter::dateToString( fb->dtStart(), true ) );
  html += invitationRow( i18n("End date:"),
                         KGlobal::locale()->formatDate( fb->dtEnd().date(), true ) );
  html += "<tr><td colspan=2><hr></td></tr>\n";
  html += "<tr><td colspan=2>Busy periods given in this free/busy object:</td></tr>\n";

  QValueList<Period> periods = fb->busyPeriods();

  QValueList<Period>::iterator it;
  for ( it = periods.begin(); it != periods.end(); ++it ) {
    Period per = *it;
    if ( per.hasDuration() ) {
      int dur = per.duration().asSeconds();
      QString cont;
      if ( dur >= 3600 ) {
        cont += i18n("1 hour ", "%n hours ", dur / 3600);
        dur %= 3600;
      }
      if ( dur >= 60 ) {
        cont += i18n("1 minute", "%n minutes ", dur / 60);
        dur %= 60;
      }
      if ( dur > 0 ) {
        cont += i18n("1 second", "%n seconds", dur);
      }
      html += invitationRow( QString::null, i18n("startDate for duration", "%1 for %2")
          .arg( KGlobal::locale()->formatDateTime( per.start(), false ) )
          .arg(cont) );
    } else {
      QString cont;
      if ( per.start().date() == per.end().date() ) {
        cont = i18n("date, fromTime - toTime ", "%1, %2 - %3")
            .arg( KGlobal::locale()->formatDate( per.start().date() ) )
            .arg( KGlobal::locale()->formatTime( per.start().time() ) )
            .arg( KGlobal::locale()->formatTime( per.end().time() ) );
      } else {
        cont = i18n("fromDateTime - toDateTime", "%1 - %2")
          .arg( KGlobal::locale()->formatDateTime( per.start(), false ) )
          .arg( KGlobal::locale()->formatDateTime( per.end(), false ) );
      }

      html += invitationRow( QString::null, cont );
    }
  }

  html += "</table>\n";
  return html;
}

static QString invitationHeaderEvent( Event *event, ScheduleMessage *msg )
{
  if ( !msg || !event )
    return QString::null;

  switch ( msg->method() ) {
  case Scheduler::Publish:
    return i18n( "This event has been published" );
  case Scheduler::Request:
    if ( event->revision() > 0 ) {
      return i18n( "This invitation has been updated" );
    }
    if ( iamOrganizer( event ) ) {
      return i18n( "I sent this invitation" );
    } else {
      if ( !event->organizer().fullName().isEmpty() ) {
        return i18n( "You received an invitation from %1" ).
          arg( event->organizer().fullName() );
      } else {
        return i18n( "You received an invitation" );
      }
    }
  case Scheduler::Refresh:
    return i18n( "This invitation was refreshed" );
  case Scheduler::Cancel:
    return i18n( "This invitation has been canceled" );
  case Scheduler::Add:
    return i18n( "Addition to the invitation" );
  case Scheduler::Reply: {
    Attendee::List attendees = event->attendees();
    if( attendees.count() == 0 ) {
      kdDebug(5850) << "No attendees in the iCal reply!" << endl;
      return QString::null;
    }
    if( attendees.count() != 1 ) {
      kdDebug(5850) << "Warning: attendeecount in the reply should be 1 "
                    << "but is " << attendees.count() << endl;
    }
    Attendee* attendee = *attendees.begin();
    QString attendeeName = attendee->name();
    if ( attendeeName.isEmpty() ) {
      attendeeName = attendee->email();
    }
    if ( attendeeName.isEmpty() ) {
      attendeeName = i18n( "Sender" );
    }

    QString delegatorName, dummy;
    KPIM::getNameAndMail( attendee->delegator(), delegatorName, dummy );
    if ( delegatorName.isEmpty() ) {
      delegatorName = attendee->delegator();
    }

    switch( attendee->status() ) {
    case Attendee::NeedsAction:
      return i18n( "%1 indicates this invitation still needs some action" ).arg( attendeeName );
    case Attendee::Accepted:
      if ( delegatorName.isEmpty() ) {
        return i18n( "%1 accepts this invitation" ).arg( attendeeName );
      } else {
        return i18n( "%1 accepts this invitation on behalf of %2" ).
          arg( attendeeName ).arg( delegatorName );
      }
    case Attendee::Tentative:
      if ( delegatorName.isEmpty() ) {
        return i18n( "%1 tentatively accepts this invitation" ).
          arg( attendeeName );
      } else {
        return i18n( "%1 tentatively accepts this invitation on behalf of %2" ).
          arg( attendeeName ).arg( delegatorName );
      }
    case Attendee::Declined:
      if ( delegatorName.isEmpty() ) {
        return i18n( "%1 declines this invitation" ).arg( attendeeName );
      } else {
        return i18n( "%1 declines this invitation on behalf of %2" ).
          arg( attendeeName ).arg( delegatorName );
      }
    case Attendee::Delegated: {
      QString delegate, dummy;
      KPIM::getNameAndMail( attendee->delegate(), delegate, dummy );
      if ( delegate.isEmpty() ) {
        delegate = attendee->delegate();
      }
      if ( !delegate.isEmpty() ) {
        return i18n( "%1 has delegated this invitation to %2" ).
          arg( attendeeName ) .arg( delegate );
      } else {
        return i18n( "%1 has delegated this invitation" ).arg( attendeeName );
      }
    }
    case Attendee::Completed:
      return i18n( "This invitation is now completed" );
    case Attendee::InProcess:
      return i18n( "%1 is still processing the invitation" ).
        arg( attendeeName );
    default:
      return i18n( "Unknown response to this invitation" );
    }
    break; }
  case Scheduler::Counter:
    return i18n( "Sender makes this counter proposal" );
  case Scheduler::Declinecounter:
    return i18n( "Sender declines the counter proposal" );
  case Scheduler::NoMethod:
    return i18n("Error: iMIP message with unknown method: '%1'").
      arg( msg->method() );
  }
  return QString::null;
}

static QString invitationHeaderTodo( Todo *todo, ScheduleMessage *msg )
{
  if ( !msg || !todo ) {
    return QString::null;
  }

  switch ( msg->method() ) {
  case Scheduler::Publish:
    return i18n("This task has been published");
  case Scheduler::Request:
    if ( todo->revision() > 0 ) {
      return i18n( "This task has been updated" );
    } else {
      return i18n( "You have been assigned this task" );
    }
  case Scheduler::Refresh:
    return i18n( "This task was refreshed" );
  case Scheduler::Cancel:
    return i18n( "This task was canceled" );
  case Scheduler::Add:
    return i18n( "Addition to the task" );
  case Scheduler::Reply: {
    Attendee::List attendees = todo->attendees();
    if( attendees.count() == 0 ) {
      kdDebug(5850) << "No attendees in the iCal reply!" << endl;
      return QString::null;
    }
    if( attendees.count() != 1 ) {
      kdDebug(5850) << "Warning: attendeecount in the reply should be 1 "
                    << "but is " << attendees.count() << endl;
    }
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
    case Attendee::Delegated: {
      QString delegate, dummy;
      KPIM::getNameAndMail( attendee->delegate(), delegate, dummy );
      if ( delegate.isEmpty() ) {
        delegate = attendee->delegate();
      }
      if ( !delegate.isEmpty() ) {
        return i18n( "Sender has delegated this request for the task to %1" ).arg( delegate );
      } else {
        return i18n( "Sender has delegated this request for the task " );
      }
    }
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
    return i18n("Error: iMIP message with unknown method: '%1'").
      arg( msg->method() );
  }
  return QString::null;
}

static QString invitationHeaderJournal( Journal *journal, ScheduleMessage *msg )
{
  if ( !msg || !journal ) {
    return QString::null;
  }

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
      kdDebug(5850) << "No attendees in the iCal reply!" << endl;
      return QString::null;
    }
    if( attendees.count() != 1 ) {
      kdDebug(5850) << "Warning: attendeecount in the reply should be 1 "
                    << "but is " << attendees.count() << endl;
    }
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
    break;
  }
  case Scheduler::Counter:
    return i18n( "Sender makes this counter proposal" );
  case Scheduler::Declinecounter:
    return i18n( "Sender declines the counter proposal" );
  case Scheduler::NoMethod:
    return i18n("Error: iMIP message with unknown method: '%1'").
      arg( msg->method() );
  }
  return QString::null;
}

static QString invitationHeaderFreeBusy( FreeBusy *fb, ScheduleMessage *msg )
{
  if ( !msg || !fb ) {
    return QString::null;
  }

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
    return i18n("Error: Free/Busy iMIP message with unknown method: '%1'").
      arg( msg->method() );
  }
}

static QString invitationAttendees( Incidence *incidence )
{
  QString tmpStr;
  if ( !incidence ) {
    return tmpStr;
  }

  tmpStr += htmlAddTag( "u", i18n( "Attendee List" ) );
  tmpStr += "<br/>";

  int count=0;
  Attendee::List attendees = incidence->attendees();
  if ( !attendees.isEmpty() ) {

    Attendee::List::ConstIterator it;
    for( it = attendees.begin(); it != attendees.end(); ++it ) {
      Attendee *a = *it;
      if ( !iamAttendee( a ) ) {
        count++;
        if ( count == 1 ) {
          tmpStr += "<table border=\"1\" cellpadding=\"1\" cellspacing=\"0\" columns=\"2\">";
        }
        tmpStr += "<tr>";
        tmpStr += "<td>";
        tmpStr += invitationPerson( a->email(), a->name(), QString::null );
        if ( !a->delegator().isEmpty() ) {
          tmpStr += i18n(" (delegated by %1)" ).arg( a->delegator() );
        }
        if ( !a->delegate().isEmpty() ) {
          tmpStr += i18n(" (delegated to %1)" ).arg( a->delegate() );
        }
        tmpStr += "</td>";
        tmpStr += "<td>" + a->statusStr() + "</td>";
        tmpStr += "</tr>";
      }
    }
  }
  if ( count ) {
    tmpStr += "</table>";
  } else {
    tmpStr += "<i>" + i18n( "No attendee", "None" ) + "</i>";
  }

  return tmpStr;
}

static QString invitationAttachments( InvitationFormatterHelper *helper, Incidence *incidence )
{
  QString tmpStr;
  if ( !incidence ) {
    return tmpStr;
  }

  Attachment::List attachments = incidence->attachments();
  if ( !attachments.isEmpty() ) {
    tmpStr += i18n( "Attached Documents:" ) + "<ol>";

    Attachment::List::ConstIterator it;
    for( it = attachments.begin(); it != attachments.end(); ++it ) {
      Attachment *a = *it;
      tmpStr += "<li>";
      // Attachment icon
      KMimeType::Ptr mimeType = KMimeType::mimeType( a->mimeType() );
      QString iconStr = mimeType->icon( a->uri(), false );
      QString iconPath = KGlobal::iconLoader()->iconPath( iconStr, KIcon::Small );
      if ( !iconPath.isEmpty() ) {
        tmpStr += "<img valign=\"top\" src=\"" + iconPath + "\">";
      }
      tmpStr += helper->makeLink( "ATTACH:" + a->label(), a->label() );
      tmpStr += "</li>";
    }
    tmpStr += "</ol>";
  }

  return tmpStr;
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

class IncidenceFormatter::IncidenceCompareVisitor :
  public IncidenceBase::Visitor
{
  public:
    IncidenceCompareVisitor() : mExistingIncidence(0) {}
    bool act( IncidenceBase *incidence, Incidence* existingIncidence )
    {
      Incidence *inc = dynamic_cast<Incidence*>( incidence );
      if ( !inc || !existingIncidence || inc->revision() <= existingIncidence->revision() )
        return false;
      mExistingIncidence = existingIncidence;
      return incidence->accept( *this );
    }

    QString result() const
    {
      if ( mChanges.isEmpty() ) {
        return QString::null;
      }
      QString html = "<div align=\"left\"><ul><li>";
      html += mChanges.join( "</li><li>" );
      html += "</li><ul></div>";
      return html;
    }

  protected:
    bool visit( Event *event )
    {
      compareEvents( event, dynamic_cast<Event*>( mExistingIncidence ) );
      compareIncidences( event, mExistingIncidence );
      return !mChanges.isEmpty();
    }
    bool visit( Todo *todo )
    {
      compareIncidences( todo, mExistingIncidence );
      return !mChanges.isEmpty();
    }
    bool visit( Journal *journal )
    {
      compareIncidences( journal, mExistingIncidence );
      return !mChanges.isEmpty();
    }
    bool visit( FreeBusy *fb )
    {
      Q_UNUSED( fb );
      return !mChanges.isEmpty();
    }

  private:
    void compareEvents( Event *newEvent, Event *oldEvent )
    {
      if ( !oldEvent || !newEvent )
        return;
      if ( oldEvent->dtStart() != newEvent->dtStart() || oldEvent->doesFloat() != newEvent->doesFloat() )
        mChanges += i18n( "The invitation starting time has been changed from %1 to %2" )
            .arg( eventStartTimeStr( oldEvent ) ).arg( eventStartTimeStr( newEvent ) );
      if ( oldEvent->dtEnd() != newEvent->dtEnd() || oldEvent->doesFloat() != newEvent->doesFloat() )
        mChanges += i18n( "The invitation ending time has been changed from %1 to %2" )
            .arg( eventEndTimeStr( oldEvent ) ).arg( eventEndTimeStr( newEvent ) );
    }

    void compareIncidences( Incidence *newInc, Incidence *oldInc )
    {
      if ( !oldInc || !newInc )
        return;
      if ( oldInc->summary() != newInc->summary() )
        mChanges += i18n( "The summary has been changed to: \"%1\"" ).arg( newInc->summary() );
      if ( oldInc->location() != newInc->location() )
        mChanges += i18n( "The location has been changed to: \"%1\"" ).arg( newInc->location() );
      if ( oldInc->description() != newInc->description() )
        mChanges += i18n( "The description has been changed to: \"%1\"" ).arg( newInc->description() );
      Attendee::List oldAttendees = oldInc->attendees();
      Attendee::List newAttendees = newInc->attendees();
      for ( Attendee::List::ConstIterator it = newAttendees.constBegin(); it != newAttendees.constEnd(); ++it ) {
        Attendee *oldAtt = oldInc->attendeeByMail( (*it)->email() );
        if ( !oldAtt ) {
          mChanges += i18n( "Attendee %1 has been added" ).arg( (*it)->fullName() );
        } else {
          if ( oldAtt->status() != (*it)->status() )
            mChanges += i18n( "The status of attendee %1 has been changed to: %2" ).arg( (*it)->fullName() )
                .arg( (*it)->statusStr() );
        }
      }
      for ( Attendee::List::ConstIterator it = oldAttendees.constBegin(); it != oldAttendees.constEnd(); ++it ) {
        Attendee *newAtt = newInc->attendeeByMail( (*it)->email() );
        if ( !newAtt )
          mChanges += i18n( "Attendee %1 has been removed" ).arg( (*it)->fullName() );
      }
    }

  private:
    Incidence* mExistingIncidence;
    QStringList mChanges;
};


QString InvitationFormatterHelper::makeLink( const QString &id, const QString &text )
{
  if ( !id.startsWith( "ATTACH:" ) ) {
    QString res = QString( "<a href=\"%1\"><b>%2</b></a>" ).
                  arg( generateLinkURL( id ), text );
    return res;
  } else {
    // draw the attachment links in non-bold face
    QString res = QString( "<a href=\"%1\">%2</a>" ).
                  arg( generateLinkURL( id ), text );
    return res;
  }
}

// Check if the given incidence is likely one that we own instead one from
// a shared calendar (Kolab-specific)
static bool incidenceOwnedByMe( Calendar *calendar, Incidence *incidence )
{
  CalendarResources *cal = dynamic_cast<CalendarResources*>( calendar );
  if ( !cal || !incidence ) {
    return true;
  }
  ResourceCalendar *res = cal->resource( incidence );
  if ( !res ) {
    return true;
  }
  const QString subRes = res->subresourceIdentifier( incidence );
  if ( !subRes.contains( "/.INBOX.directory/" ) ) {
    return false;
  }
  return true;
}

QString IncidenceFormatter::formatICalInvitation( QString invitation, Calendar *mCalendar,
    InvitationFormatterHelper *helper )
{
  if ( invitation.isEmpty() ) return QString::null;

  ICalFormat format;
  // parseScheduleMessage takes the tz from the calendar, no need to set it manually here for the format!
  ScheduleMessage *msg = format.parseScheduleMessage( mCalendar, invitation );

  if( !msg ) {
    kdDebug( 5850 ) << "Failed to parse the scheduling message" << endl;
    Q_ASSERT( format.exception() );
    kdDebug( 5850 ) << format.exception()->message() << endl;
    return QString::null;
  }

  IncidenceBase *incBase = msg->event();

  // Determine if this incidence is in my calendar (and owned by me)
  Incidence *existingIncidence = 0;
  if ( incBase && helper->calendar() ) {
    existingIncidence = helper->calendar()->incidence( incBase->uid() );
    if ( !incidenceOwnedByMe( helper->calendar(), existingIncidence ) ) {
      existingIncidence = 0;
    }
    if ( !existingIncidence ) {
      const Incidence::List list = helper->calendar()->incidences();
      for ( Incidence::List::ConstIterator it = list.begin(), end = list.end(); it != end; ++it ) {
        if ( (*it)->schedulingID() == incBase->uid() &&
             incidenceOwnedByMe( helper->calendar(), *it ) ) {
          existingIncidence = *it;
          break;
        }
      }
    }
  }

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
    return QString::null;
  html += "<b>" + headerVisitor.result() + "</b>";

  InvitationBodyVisitor bodyVisitor;
  if ( !bodyVisitor.act( incBase, msg ) )
    return QString::null;
  html += bodyVisitor.result();

  if ( msg->method() == Scheduler::Request ) { // ### Scheduler::Publish/Refresh/Add as well?
    IncidenceCompareVisitor compareVisitor;
    if ( compareVisitor.act( incBase, existingIncidence ) ) {
      html += i18n("<p align=\"left\">The following changes have been made by the organizer:</p>");
      html += compareVisitor.result();
    }
  }

  Incidence *inc = dynamic_cast<Incidence*>( incBase );

  // determine if I am the organizer for this invitation
  bool myInc = iamOrganizer( inc );

  // determine if the invitation response has already been recorded
  bool rsvpRec = false;
  Attendee *ea = 0;
  if ( !myInc ) {
    if ( existingIncidence ) {
      ea = findMyAttendee( existingIncidence );
    }
    if ( ea && ( ea->status() == Attendee::Accepted || ea->status() == Attendee::Declined ) ) {
      rsvpRec = true;
    }
  }

  // Print if RSVP needed, not-needed, or response already recorded
  bool rsvpReq = rsvpRequested( inc );
  if ( !myInc ) {
    html += "<br/>";
    html += "<i><u>";
    if ( rsvpRec && ( inc && inc->revision() == 0 ) ) {
      html += i18n( "Your response has already been recorded [%1]" ).
              arg( ea->statusStr() );
      rsvpReq = false;
    } else if ( msg->method() == Scheduler::Cancel ) {
      html += i18n( "This invitation was declined" );
    } else if ( msg->method() == Scheduler::Add ) {
      html += i18n( "This invitation was accepted" );
    } else {
      html += rsvpRequestedStr( rsvpReq );
    }
    html += "</u></i><br>";
  }

  // Add groupware links

  html += "<table border=\"0\" cellspacing=\"0\"><tr><td>&nbsp;</td></tr><tr>";

  switch ( msg->method() ) {
    case Scheduler::Publish:
    case Scheduler::Request:
    case Scheduler::Refresh:
    case Scheduler::Add:
    {
      if ( inc && inc->revision() > 0 && ( existingIncidence || !helper->calendar() ) ) {
        if ( inc->type() == "Todo" ) {
          html += "<td colspan=\"9\">";
          html += helper->makeLink( "reply", i18n( "[Record invitation to my task list]" ) );
        } else {
          html += "<td colspan=\"13\">";
          html += helper->makeLink( "reply", i18n( "[Record invitation to my calendar]" ) );
        }
        html += "</td></tr><tr>";
      }
      html += "<td>";

      if ( !myInc ) {
        if ( rsvpReq ) {
          // Accept
          html += helper->makeLink( "accept", i18n( "[Accept]" ) );
          html += "</td><td> &nbsp; </td><td>";
          html += helper->makeLink( "accept_conditionally",
                                    i18n( "Accept conditionally", "[Accept cond.]" ) );
          html += "</td><td> &nbsp; </td><td>";
        }

        if ( rsvpReq ) {
          // Counter proposal
          html += helper->makeLink( "counter", i18n( "[Counter proposal]" ) );
          html += "</td><td> &nbsp; </td><td>";
        }

        if ( rsvpReq ) {
          // Decline
          html += helper->makeLink( "decline", i18n( "[Decline]" ) );
          html += "</td><td> &nbsp; </td><td>";
        }

        if ( !rsvpRec || ( inc && inc->revision() > 0 ) ) {
          // Delegate
          html += helper->makeLink( "delegate", i18n( "[Delegate]" ) );
          html += "</td><td> &nbsp; </td><td>";

          // Forward
          html += helper->makeLink( "forward", i18n( "[Forward]" ) );

          // Check calendar
          if ( inc && inc->type() == "Event" ) {
            html += "</td><td> &nbsp; </td><td>";
            html += helper->makeLink( "check_calendar", i18n("[Check my calendar]" ) );
          }
        }
      }
      break;
    }

    case Scheduler::Cancel:
      // Remove invitation
      if ( inc->type() == "Todo" ) {
        html += helper->makeLink( "cancel", i18n( "[Remove invitation from my task list]" ) );
      } else {
        html += helper->makeLink( "cancel", i18n( "[Remove invitation from my calendar]" ) );
      }
      break;

    case Scheduler::Reply:
    {
      // Record invitation response
      Attendee *a = 0;
      Attendee *ea = 0;
      if ( inc ) {
        a = inc->attendees().first();
        if ( a ) {
          ea = findAttendee( existingIncidence, a->email() );
        }
      }
      if ( ea && ( ea->status() != Attendee::NeedsAction ) && ( ea->status() == a->status() ) ) {
        if ( inc && inc->revision() > 0 ) {
          html += "<br><u><i>";
          html += i18n( "The response has been recorded [%1]" ).arg( ea->statusStr() );
          html += "</i></u>";
        }
      } else {
        if ( inc ) {
          if ( inc->type() == "Todo" ) {
            html += helper->makeLink( "reply", i18n( "[Record response into my task list]" ) );
          } else {
            html += helper->makeLink( "reply", i18n( "[Record response into my calendar]" ) );
          }
        }
      }
      break;
    }

    case Scheduler::Counter:
      // Counter proposal
      html += helper->makeLink( "accept_counter", i18n("[Accept]") );
      html += "&nbsp;";
      html += helper->makeLink( "decline_counter", i18n("[Decline]") );
      html += "&nbsp;";
      html += helper->makeLink( "check_calendar", i18n("[Check my calendar]" ) );
      break;

    case Scheduler::Declinecounter:
    case Scheduler::NoMethod:
      break;
  }

  // close the groupware table
  html += "</td></tr></table>";

  // Add the attendee list if I am the organizer
  if ( myInc && helper->calendar() ) {
    html += invitationAttendees( helper->calendar()->incidence( inc->uid() ) );
  }

  // close the top-level table
  html += "</td></tr></table><br></div>";

  // Add the attachment list
  html += invitationAttachments( helper, inc );

  return html;
}




/*******************************************************************
 *  Helper functions for the msTNEF -> VPart converter
 *******************************************************************/


//-----------------------------------------------------------------------------

static QString stringProp( KTNEFMessage* tnefMsg, const Q_UINT32& key,
                           const QString& fallback = QString::null)
{
  return tnefMsg->findProp( key < 0x10000 ? key & 0xFFFF : key >> 16,
                            fallback );
}

static QString sNamedProp( KTNEFMessage* tnefMsg, const QString& name,
                           const QString& fallback = QString::null )
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

  //kdDebug(5006) << "set_tz(), timezone before = " << timezone << endl;

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
  //kdDebug(5006) << "set_tz(), timezone after = " << timezone << endl;

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
  QBuffer buf( tnef );
  CalendarLocal cal ( QString::fromLatin1( "UTC" ) );
  KABC::Addressee addressee;
  KABC::VCardConverter cardConv;
  ICalFormat calFormat;
  Event* event = new Event();

  if( parser.openDevice( &buf ) ) {
    KTNEFMessage* tnefMsg = parser.message();
    //QMap<int,KTNEFProperty*> props = parser.message()->properties();

    // Everything depends from property PR_MESSAGE_CLASS
    // (this is added by KTNEFParser):
    QString msgClass = tnefMsg->findProp( 0x001A, QString::null, true )
      .upper();
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
          int colon = sSenderSearchKeyEmail.find( ':' );
          // May be e.g. "SMTP:KHZ@KDE.ORG"
          if( sSenderSearchKeyEmail.find( ':' ) == -1 )
            sSenderSearchKeyEmail.remove( 0, colon+1 );
        }

        QString s( tnefMsg->findProp( 0x0e04 ) );
        QStringList attendees = QStringList::split( ';', s );
        if( attendees.count() ) {
          for( QStringList::Iterator it = attendees.begin();
               it != attendees.end(); ++it ) {
            // Skip all entries that have no '@' since these are
            // no mail addresses
            if( (*it).find('@') == -1 ) {
              s = (*it).stripWhiteSpace();

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
            Attendee *attendee = new Attendee( QString::null, QString::null,
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

        s = tnefMsg->findProp( 0x8516 ).replace( QChar( '-' ), QString::null )
          .replace( QChar( ':' ), QString::null );
        event->setDtStart( QDateTime::fromString( s ) ); // ## Format??

        s = tnefMsg->findProp( 0x8517 ).replace( QChar( '-' ), QString::null )
          .replace( QChar( ':' ), QString::null );
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
        s = tnefMsg->findProp( 0x8202 ).replace( QChar( '-' ), QString::null )
          .replace( QChar( ':' ), QString::null );
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
          .replace( QChar( '-' ), QString::null )
          .replace( QChar( ':' ), QString::null );
        if( !s.isEmpty() )
          addressee.insertCustom( "KADDRESSBOOK", "X-Anniversary", s );

        addressee.setUrl( KURL( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_WEBPAGE )  ) );

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
          .replace( QChar( '-' ), QString::null )
          .replace( QChar( ':' ), QString::null );
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
  return converter.createVCard( addressee );
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
    ToolTipVisitor()
      : mCalendar( 0 ), mRichText( true ), mResult( "" ) {}

    bool act( Calendar *calendar, IncidenceBase *incidence,
              const QDate &date=QDate(), bool richText=true )
    {
      mCalendar = calendar;
      mDate = date;
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

    QString dateRangeText( Event *event, const QDate &date );
    QString dateRangeText( Todo *todo, const QDate &date );
    QString dateRangeText( Journal *journal );
    QString dateRangeText( FreeBusy *fb );

    QString generateToolTip( Incidence* incidence, QString dtRangeText );

  protected:
    Calendar *mCalendar;
    QDate mDate;
    bool mRichText;
    QString mResult;
};

QString IncidenceFormatter::ToolTipVisitor::dateRangeText( Event *event, const QDate &date )
{
  QString ret;
  QString tmp;

  QDateTime startDt = event->dtStart();
  QDateTime endDt = event->dtEnd();
  if ( event->doesRecur() ) {
    if ( date.isValid() ) {
      QDateTime dt( date, QTime( 0, 0, 0 ) );
      int diffDays = startDt.daysTo( dt );
      dt = dt.addSecs( -1 );
      startDt.setDate( event->recurrence()->getNextDateTime( dt ).date() );
      if ( event->hasEndDate() ) {
        endDt = endDt.addDays( diffDays );
        if ( startDt > endDt ) {
          startDt.setDate( event->recurrence()->getPreviousDateTime( dt ).date() );
          endDt = startDt.addDays( event->dtStart().daysTo( event->dtEnd() ) );
        }
      }
    }
  }
  if ( event->isMultiDay() ) {

    tmp = "<br>" + i18n("Event start", "<i>From:</i>&nbsp;%1");
    if (event->doesFloat())
      ret += tmp.arg( IncidenceFormatter::dateToString( startDt, false ).replace(" ", "&nbsp;") );
    else
      ret += tmp.arg( IncidenceFormatter::dateToString( startDt ).replace(" ", "&nbsp;") );

    tmp = "<br>" + i18n("Event end","<i>To:</i>&nbsp;%1");
    if (event->doesFloat())
      ret += tmp.arg( IncidenceFormatter::dateToString( endDt, false ).replace(" ", "&nbsp;") );
    else
      ret += tmp.arg( IncidenceFormatter::dateToString( endDt ).replace(" ", "&nbsp;") );

  } else {

    ret += "<br>"+i18n("<i>Date:</i>&nbsp;%1").
           arg( IncidenceFormatter::dateToString( startDt, false ).replace(" ", "&nbsp;") );
    if ( !event->doesFloat() ) {
      const QString dtStartTime =
        IncidenceFormatter::timeToString( startDt, true ).replace( " ", "&nbsp;" );
      const QString dtEndTime =
        IncidenceFormatter::timeToString( endDt, true ).replace( " ", "&nbsp;" );
      if ( dtStartTime == dtEndTime ) { // to prevent 'Time: 17:00 - 17:00'
        tmp = "<br>" + i18n("time for event, &nbsp; to prevent ugly line breaks",
        "<i>Time:</i>&nbsp;%1").
        arg( dtStartTime );
      } else {
        tmp = "<br>" + i18n("time range for event, &nbsp; to prevent ugly line breaks",
        "<i>Time:</i>&nbsp;%1&nbsp;-&nbsp;%2").
        arg( dtStartTime, dtEndTime );
      }
      ret += tmp;
    }

  }
  return ret;
}

QString IncidenceFormatter::ToolTipVisitor::dateRangeText( Todo *todo, const QDate &date )
{
  QString ret;
  bool floats( todo->doesFloat() );

  if ( todo->hasStartDate() && todo->dtStart().isValid() ) {
    QDateTime startDt = todo->dtStart();
    if ( todo->doesRecur() ) {
      if ( date.isValid() ) {
        startDt.setDate( date );
      }
    }
    ret += "<br>" +
           i18n("<i>Start:</i>&nbsp;%1").
           arg( IncidenceFormatter::dateTimeToString( startDt, floats, false ).
                replace( " ", "&nbsp;" ) );
  }

  if ( todo->hasDueDate() && todo->dtDue().isValid() ) {
    QDateTime dueDt = todo->dtDue();
    if ( todo->doesRecur() ) {
      if ( date.isValid() ) {
        dueDt.addDays( todo->dtDue().date().daysTo( date ) );
      }
    }
    ret += "<br>" +
           i18n("<i>Due:</i>&nbsp;%1").
           arg( IncidenceFormatter::dateTimeToString( todo->dtDue(), floats, false ).
                replace( " ", "&nbsp;" ) );
  }

  if ( todo->isCompleted() ) {
    ret += "<br>" +
           i18n("<i>Completed:</i>&nbsp;%1").
           arg( todo->completedStr().replace( " ", "&nbsp;" ) );
  } else {
    ret += "<br>" +
           i18n( "%1% completed" ).arg(todo->percentComplete() );
  }

  return ret;
}

QString IncidenceFormatter::ToolTipVisitor::dateRangeText( Journal*journal )
{
  QString ret;
  if (journal->dtStart().isValid() ) {
    ret += "<br>" +
           i18n("<i>Date:</i>&nbsp;%1").
           arg( IncidenceFormatter::dateToString( journal->dtStart(), false ) );
  }
  return ret;
}

QString IncidenceFormatter::ToolTipVisitor::dateRangeText( FreeBusy *fb )
{
  QString tmp( "<br>" + i18n("<i>Period start:</i>&nbsp;%1") );
  QString ret = tmp.arg( KGlobal::locale()->formatDateTime( fb->dtStart() ) );
  tmp = "<br>" + i18n("<i>Period start:</i>&nbsp;%1");
  ret += tmp.arg( KGlobal::locale()->formatDateTime( fb->dtEnd() ) );
  return ret;
}



bool IncidenceFormatter::ToolTipVisitor::visit( Event *event )
{
  mResult = generateToolTip( event, dateRangeText( event, mDate ) );
  return !mResult.isEmpty();
}

bool IncidenceFormatter::ToolTipVisitor::visit( Todo *todo )
{
  mResult = generateToolTip( todo, dateRangeText( todo, mDate ) );
  return !mResult.isEmpty();
}

bool IncidenceFormatter::ToolTipVisitor::visit( Journal *journal )
{
  mResult = generateToolTip( journal, dateRangeText( journal ) );
  return !mResult.isEmpty();
}

bool IncidenceFormatter::ToolTipVisitor::visit( FreeBusy *fb )
{
  mResult = "<qt><b>" + i18n("Free/Busy information for %1")
        .arg(fb->organizer().fullName()) + "</b>";
  mResult += dateRangeText( fb );
  mResult += "</qt>";
  return !mResult.isEmpty();
}

QString IncidenceFormatter::ToolTipVisitor::generateToolTip( Incidence* incidence, QString dtRangeText )
{
  if ( !incidence )
    return QString::null;

  QString tmp = "<qt><b>"+ incidence->summary().replace("\n", "<br>")+"</b>";

  if ( mCalendar ) {
    QString calStr = IncidenceFormatter::resourceString( mCalendar, incidence );
    if ( !calStr.isEmpty() ) {
      tmp += "<br>" + i18n( "<i>Calendar:</i> %1" ).arg( calStr );
    }
  }

  tmp += dtRangeText;

  if (!incidence->location().isEmpty()) {
    tmp += "<br>"+i18n("<i>Location:</i>&nbsp;%1").
      arg( incidence->location().replace("\n", "<br>") );
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
  return toolTipStr( 0, incidence, QDate(), richText );
}

QString IncidenceFormatter::toolTipStr( Calendar *calendar,
                                        IncidenceBase *incidence,
                                        const QDate &date,
                                        bool richText )
{
  ToolTipVisitor v;
  if ( v.act( calendar, incidence, date, richText ) ) {
    return v.result();
  } else {
    return QString::null;
  }
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
    body += i18n("Summary: %1\n").arg( incidence->summary() );
  }
  if ( !incidence->organizer().isEmpty() ) {
    body += i18n("Organizer: %1\n").arg( incidence->organizer().fullName() );
  }
  if ( !incidence->location().isEmpty() ) {
    body += i18n("Location: %1\n").arg( incidence->location() );
  }
  return body;
}

bool IncidenceFormatter::MailBodyVisitor::visit( Event *event )
{
  QString recurrence[]= {i18n("no recurrence", "None"),
    i18n("Minutely"), i18n("Hourly"), i18n("Daily"),
    i18n("Weekly"), i18n("Monthly Same Day"), i18n("Monthly Same Position"),
    i18n("Yearly"), i18n("Yearly"), i18n("Yearly")};

  mResult = mailBodyIncidence( event );
  mResult += i18n("Start Date: %1\n").
             arg( IncidenceFormatter::dateToString( event->dtStart(), true ) );
  if ( !event->doesFloat() ) {
    mResult += i18n("Start Time: %1\n").
               arg( IncidenceFormatter::timeToString( event->dtStart(), true ) );
  }
  if ( event->dtStart() != event->dtEnd() ) {
    mResult += i18n("End Date: %1\n").
               arg( IncidenceFormatter::dateToString( event->dtEnd(), true ) );
  }
  if ( !event->doesFloat() ) {
    mResult += i18n("End Time: %1\n").
               arg( IncidenceFormatter::timeToString( event->dtEnd(), true ) );
  }
  if ( event->doesRecur() ) {
    Recurrence *recur = event->recurrence();
    // TODO: Merge these two to one of the form "Recurs every 3 days"
    mResult += i18n("Recurs: %1\n")
             .arg( recurrence[ recur->recurrenceType() ] );
    mResult += i18n("Frequency: %1\n")
             .arg( event->recurrence()->frequency() );

    if ( recur->duration() > 0 ) {
      mResult += i18n ("Repeats once", "Repeats %n times", recur->duration());
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
        mResult += i18n("Repeat until: %1\n").arg( endstr );
      } else {
        mResult += i18n("Repeats forever\n");
      }
    }
  }
  QString details = event->description();
  if ( !details.isEmpty() ) {
    mResult += i18n("Details:\n%1\n").arg( details );
  }
  return !mResult.isEmpty();
}

bool IncidenceFormatter::MailBodyVisitor::visit( Todo *todo )
{
  mResult = mailBodyIncidence( todo );

  if ( todo->hasStartDate() ) {
    mResult += i18n("Start Date: %1\n").
               arg( IncidenceFormatter::dateToString( todo->dtStart( false ), true ) );
    if ( !todo->doesFloat() ) {
      mResult += i18n("Start Time: %1\n").
                 arg( IncidenceFormatter::timeToString( todo->dtStart( false ),true ) );
    }
  }
  if ( todo->hasDueDate() ) {
    mResult += i18n("Due Date: %1\n").
               arg( IncidenceFormatter::dateToString( todo->dtDue(), true ) );
    if ( !todo->doesFloat() ) {
      mResult += i18n("Due Time: %1\n").
                 arg( IncidenceFormatter::timeToString( todo->dtDue(), true ) );
    }
  }
  QString details = todo->description();
  if ( !details.isEmpty() ) {
    mResult += i18n("Details:\n%1\n").arg( details );
  }
  return !mResult.isEmpty();
}

bool IncidenceFormatter::MailBodyVisitor::visit( Journal *journal )
{
  mResult = mailBodyIncidence( journal );
  mResult += i18n("Date: %1\n").
             arg( IncidenceFormatter::dateToString( journal->dtStart(), true ) );
  if ( !journal->doesFloat() ) {
    mResult += i18n("Time: %1\n").
               arg( IncidenceFormatter::timeToString( journal->dtStart(), true ) );
  }
  if ( !journal->description().isEmpty() )
    mResult += i18n("Text of the journal:\n%1\n").arg( journal->description() );
  return !mResult.isEmpty();
}



QString IncidenceFormatter::mailBodyString( IncidenceBase *incidence )
{
  if ( !incidence )
    return QString::null;

  MailBodyVisitor v;
  if ( v.act( incidence ) ) {
    return v.result();
  }
  return QString::null;
}

/************************************
 *  More static formatting functions
 ************************************/

QString IncidenceFormatter::recurrenceString(Incidence * incidence)
{
  if ( !incidence->doesRecur() )
    return i18n( "No recurrence" );

     // recurrence
  QStringList dayList;
  dayList.append( i18n( "31st Last" ) );
  dayList.append( i18n( "30th Last" ) );
  dayList.append( i18n( "29th Last" ) );
  dayList.append( i18n( "28th Last" ) );
  dayList.append( i18n( "27th Last" ) );
  dayList.append( i18n( "26th Last" ) );
  dayList.append( i18n( "25th Last" ) );
  dayList.append( i18n( "24th Last" ) );
  dayList.append( i18n( "23rd Last" ) );
  dayList.append( i18n( "22nd Last" ) );
  dayList.append( i18n( "21st Last" ) );
  dayList.append( i18n( "20th Last" ) );
  dayList.append( i18n( "19th Last" ) );
  dayList.append( i18n( "18th Last" ) );
  dayList.append( i18n( "17th Last" ) );
  dayList.append( i18n( "16th Last" ) );
  dayList.append( i18n( "15th Last" ) );
  dayList.append( i18n( "14th Last" ) );
  dayList.append( i18n( "13th Last" ) );
  dayList.append( i18n( "12th Last" ) );
  dayList.append( i18n( "11th Last" ) );
  dayList.append( i18n( "10th Last" ) );
  dayList.append( i18n( "9th Last" ) );
  dayList.append( i18n( "8th Last" ) );
  dayList.append( i18n( "7th Last" ) );
  dayList.append( i18n( "6th Last" ) );
  dayList.append( i18n( "5th Last" ) );
  dayList.append( i18n( "4th Last" ) );
  dayList.append( i18n( "3rd Last" ) );
  dayList.append( i18n( "2nd Last" ) );
  dayList.append( i18n( "last day of the month", "Last" ) );
  dayList.append( i18n( "unknown day of the month", "unknown" ) ); //#31 - zero offset from UI
  dayList.append( i18n( "1st" ) );
  dayList.append( i18n( "2nd" ) );
  dayList.append( i18n( "3rd" ) );
  dayList.append( i18n( "4th" ) );
  dayList.append( i18n( "5th" ) );

  QString recurString;
  const KCalendarSystem *calSys = KGlobal::locale()->calendar();;

  Recurrence *recurs = incidence->recurrence();
  switch ( recurs->recurrenceType() ) {

      case Recurrence::rNone:
          recurString = i18n( "no recurrence", "None" );
          break;
      case Recurrence::rDaily:
          recurString = i18n( "Every day", "Every %1 days", recurs->frequency() );
          break;
      case Recurrence::rWeekly:
      {
          QString dayNames;
          // Respect start of week setting
          int weekStart = KGlobal::locale()->weekStartDay();
          bool addSpace = false;
          for ( int i = 0; i < 7; ++i ) {
              if ( recurs->days().testBit( (i+weekStart+6)%7 )) {
                  if (addSpace) dayNames.append(" ");
                  dayNames.append( calSys->weekDayName( ((i+weekStart+6)%7)+1, true ) );
                  addSpace=true;
              }
          }
          recurString = i18n( "Every week on %1",
                              "Every %n weeks on %1",
                              recurs->frequency()).arg( dayNames );
          break;
      }
      case Recurrence::rMonthlyPos:
      {
          KCal::RecurrenceRule::WDayPos rule = recurs->monthPositions()[0];
          recurString = i18n( "Every month on the %1 %2",
                              "Every %n months on the %1 %2",
                              recurs->frequency() ).arg(dayList[rule.pos() + 31]).arg(
                                      calSys->weekDayName( rule.day(),false ) );
          break;
      }
      case Recurrence::rMonthlyDay:
      {
          int days = recurs->monthDays()[0];
          if (days < 0) {
              recurString = i18n( "Every month on the %1 day",
                                  "Every %n months on the %1 day",
                                  recurs->frequency() ).arg( dayList[days + 31] );
          } else {
              recurString = i18n( "Every month on day %1",
                                  "Every %n months on day %1",
                                  recurs->frequency() ).arg( recurs->monthDays()[0] );
          }
          break;
      }

      case Recurrence::rYearlyMonth:
      {
          recurString = i18n( "Every year on day %1 of %2",
                              "Every %n years on day %1 of %2",
                              recurs->frequency() )
                  .arg(recurs->yearDates()[0])
                  .arg(calSys->monthName( recurs->yearMonths()[0], recurs->startDate().year() ) );
          break;
      }
      case Recurrence::rYearlyPos:
      {
          KCal::RecurrenceRule::WDayPos rule = recurs->yearPositions()[0];
          recurString = i18n( "Every year on the %1 %2 of %3",
                              "Every %n years on the %1 %2of %3",
                              recurs->frequency()).arg( dayList[rule.pos() + 31] )
                  .arg( calSys->weekDayName( rule.day(), false ))
                  .arg( calSys->monthName( recurs->yearMonths()[0], recurs->startDate().year() ) );
          break;
      }
      case Recurrence::rYearlyDay:
      {
          recurString = i18n( "Every year on day %1",
                              "Every %n years on day %1",
                              recurs->frequency()).arg( recurs->yearDays()[0] );
          break;
      }

      default:
          return i18n( "Incidence recurs" );
  }

  return recurString;
}

QString IncidenceFormatter::timeToString( const QDateTime &date, bool shortfmt )
{
  return KGlobal::locale()->formatTime( date.time(), !shortfmt );
}

QString IncidenceFormatter::dateToString( const QDateTime &date, bool shortfmt )
{
  return
    KGlobal::locale()->formatDate( date.date(), shortfmt );
}

QString IncidenceFormatter::dateTimeToString( const QDateTime &date,
                                              bool allDay, bool shortfmt )
{
  if ( allDay ) {
    return dateToString( date, shortfmt );
  }

  return  KGlobal::locale()->formatDateTime( date, shortfmt );
}

QString IncidenceFormatter::resourceString( Calendar *calendar, Incidence *incidence )
{
  if ( !calendar || !incidence ) {
    return QString::null;
  }

  CalendarResources *calendarResource = dynamic_cast<CalendarResources*>( calendar );
  if ( !calendarResource ) {
    return QString::null;
  }

  ResourceCalendar *resourceCalendar = calendarResource->resource( incidence );
  if ( resourceCalendar ) {
    if ( !resourceCalendar->subresources().isEmpty() ) {
      QString subRes = resourceCalendar->subresourceIdentifier( incidence );
      if ( subRes.isEmpty() ) {
        return resourceCalendar->resourceName();
      } else {
        return resourceCalendar->labelForSubresource( subRes );
      }
    }
    return resourceCalendar->resourceName();
  }

  return QString::null;
}
