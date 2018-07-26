/*
    This file is part of libkcal.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Copyright (c) 2009-2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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
#include <libkcal/calhelper.h>
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

#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kcalendarsystem.h>
#include <kmdcodec.h>
#include <kmimetype.h>

#include <qbuffer.h>
#include <qstylesheet.h>
#include <qdatetime.h>
#include <qregexp.h>

#include <time.h>

using namespace KCal;

//#define KORG_DEBUG_SCHEDULING_IDS

static QString invitationSummary( Incidence *incidence, bool noHtmlMode );

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

static QString htmlAddMailtoLink( const QString &email, const QString &name )
{
  QString str;

  if ( !email.isEmpty() ) {
    KCal::Person person( name, email );
    KURL mailto;
    mailto.setProtocol( "mailto" );
    mailto.setPath( person.fullName() );
    str = htmlAddLink( mailto.url(), name.isEmpty() ? email : name );
  }
  return str;
}

static QString htmlAddUidLink( const QString &email, const QString &name, const QString &uid )
{
  Q_UNUSED( name );
  QString str;

  if ( !uid.isEmpty() ) {
    // There is a UID, so make a link to the addressbook
    const QString url = "uid:" + uid;
    const QString iconPath = KGlobal::iconLoader()->iconPath( "view_pim_contacts", KIcon::Small );
    str = htmlAddLink( url, "<img valign=\"middle\" src=\"" + iconPath + "\">" );
  }
  return str;
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

static QPair<QString, QString> searchNameAndUid( const QString &email, const QString &name,
                                                 const QString &uid )
{
  QString sName = name;
  QString sUid = uid;
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
          sName = o.formattedName();
        }
        sUid = o.uid();
      } else {
        // Email not found in the addressbook
        sUid = QString::null;
      }
    }
  }
  QPair<QString, QString>s;
  s.first = sName;
  s.second = sUid;
  return s;
}

static QString searchName( const QString &email, const QString &name )
{
  QString printName = name;
  // Search, if there is an email address to search on
  if ( name.isEmpty() && !email.isEmpty() ) {
    KABC::AddressBook *add_book = KABC::StdAddressBook::self( true );
    KABC::Addressee::List addressList = add_book->findByEmail( email );
    if ( !addressList.isEmpty() ) {
      KABC::Addressee o = addressList.first();
      if ( !o.isEmpty() && addressList.size() < 2 ) {
        // No name set, so use the one from the addressbook
        printName = o.formattedName();
      }
    }
  }
  return printName;
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

static bool senderIsOrganizer( Incidence *incidence, const QString &sender )
{
  // Check if the specified sender is the organizer

  if ( !incidence || sender.isEmpty() ) {
    return true;
  }
  bool isorg = true;
  QString senderName, senderEmail;
  if ( KPIM::getNameAndMail( sender, senderName, senderEmail ) ) {
    // for this heuristic, we say the sender is the organizer if either the name or the email match.
    if ( incidence->organizer().email() != senderEmail &&
         incidence->organizer().name() != senderName ) {
      isorg = false;
    }
  }
  return isorg;
}

static bool attendeeIsOrganizer( Incidence *incidence, Attendee *attendee )
{
  if ( incidence && attendee &&
       ( incidence->organizer().email() == attendee->email() ) ) {
    return true;
  } else {
    return false;
  }
}

static QString organizerName( Incidence *incidence, const QString &defName )
{
  QString tName;
  if ( !defName.isEmpty() ) {
    tName = defName;
  } else {
    tName = i18n( "<Organizer Unknown>" );
  }

  QString name;
  if ( incidence ) {
    name = incidence->organizer().name();
    if ( name.isEmpty() ) {
      name = incidence->organizer().email();
    }
  }
  if ( name.isEmpty() ) {
    name = tName;
  }
  return name;
}

static QString firstAttendeeName( Incidence *incidence, const QString &defName )
{
  QString tName;
  if ( !defName.isEmpty() ) {
    tName = defName;
  } else {
    tName = i18n( "Sender" );
  }

  QString name;
  if ( incidence ) {
    Attendee::List attendees = incidence->attendees();
    if ( attendees.count() > 0 ) {
      Attendee *attendee = *attendees.begin();
      name = attendee->name();
      if ( name.isEmpty() ) {
        name = attendee->email();
      }
    }
  }
  if ( name.isEmpty() ) {
    name = tName;
  }
  return name;
}

static QString rsvpStatusIconPath( Attendee::PartStat status )
{
  QString iconPath;
  switch ( status ) {
  case Attendee::Accepted:
    iconPath = KGlobal::iconLoader()->iconPath( "ok", KIcon::Small );
    break;
  case Attendee::Declined:
    iconPath = KGlobal::iconLoader()->iconPath( "no", KIcon::Small );
    break;
  case Attendee::NeedsAction:
  case Attendee::InProcess:
    iconPath = KGlobal::iconLoader()->iconPath( "help", KIcon::Small );
    break;
  case Attendee::Tentative:
    iconPath = KGlobal::iconLoader()->iconPath( "apply", KIcon::Small );
    break;
  case Attendee::Delegated:
    iconPath = KGlobal::iconLoader()->iconPath( "mail_forward", KIcon::Small );
    break;
  default:
    break;
  }
  return iconPath;
}

/*******************************************************************
 *  Helper functions for the extensive display (display viewer)
 *******************************************************************/

static QString displayViewFormatPerson( const QString &email, const QString &name,
                                        const QString &uid, const QString &iconPath )
{
  // Search for new print name or uid, if needed.
  QPair<QString, QString> s = searchNameAndUid( email, name, uid );
  const QString printName = s.first;
  const QString printUid = s.second;

  QString personString;
  if ( !iconPath.isEmpty() ) {
    personString += "<img valign=\"top\" src=\"" + iconPath + "\">" + "&nbsp;";
  }

  // Make the uid link
  if ( !printUid.isEmpty() ) {
    personString += htmlAddUidLink( email, printName, printUid );
  } else {
    // No UID, just show some text
    personString += ( printName.isEmpty() ? email : printName );
  }

  // Make the mailto link
  if ( !email.isEmpty() ) {
    personString += "&nbsp;" + htmlAddMailtoLink( email, printName );
  }

  return personString;
}

static QString displayViewFormatPerson( const QString &email, const QString &name,
                                        const QString &uid, Attendee::PartStat status )
{
  return displayViewFormatPerson( email, name, uid, rsvpStatusIconPath( status ) );
}

static QString displayViewFormatAttendeeRoleList( Incidence *incidence, Attendee::Role role,
                                                  bool showStatus )
{
  QString tmpStr;
  Attendee::List::ConstIterator it;
  Attendee::List attendees = incidence->attendees();

  for ( it = attendees.begin(); it != attendees.end(); ++it ) {
    Attendee *a = *it;
    if ( a->role() != role ) {
      // skip this role
      continue;
    }
    if ( attendeeIsOrganizer( incidence, a ) ) {
      // skip attendee that is also the organizer
      continue;
    }
    tmpStr += displayViewFormatPerson( a->email(), a->name(), a->uid(),
                                       showStatus ? a->status() : Attendee::None );
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
  return tmpStr;
}

static QString displayViewFormatAttendees( Calendar *calendar, Incidence *incidence )
{
  QString tmpStr, str;

  // Add organizer link
  int attendeeCount = incidence->attendees().count();
  if ( attendeeCount > 1 ||
       ( attendeeCount == 1 &&
         !attendeeIsOrganizer( incidence, incidence->attendees().first() ) ) ) {

    QPair<QString, QString> s = searchNameAndUid( incidence->organizer().email(),
                                                  incidence->organizer().name(),
                                                  QString() );
    tmpStr += "<tr>";
    tmpStr += "<td><b>" + i18n( "Organizer:" ) + "</b></td>";
    QString iconPath = KGlobal::iconLoader()->iconPath( "organizer", KIcon::Small );
    tmpStr += "<td>" + displayViewFormatPerson( incidence->organizer().email(),
                                                s.first, s.second, iconPath ) +
              "</td>";
    tmpStr += "</tr>";
  }

  // Show the attendee status if the incidence's organizer owns the resource calendar,
  // which means they are running the show and have all the up-to-date response info.
  bool showStatus = CalHelper::incOrganizerOwnsCalendar( calendar, incidence );

  // Add "chair"
  str = displayViewFormatAttendeeRoleList( incidence, Attendee::Chair, showStatus );
  if ( !str.isEmpty() ) {
    tmpStr += "<tr>";
    tmpStr += "<td><b>" + i18n( "Chair:" ) + "</b></td>";
    tmpStr += "<td>" + str + "</td>";
    tmpStr += "</tr>";
  }

  // Add required participants
  str = displayViewFormatAttendeeRoleList( incidence, Attendee::ReqParticipant, showStatus );
  if ( !str.isEmpty() ) {
    tmpStr += "<tr>";
    tmpStr += "<td><b>" + i18n( "Required Participants:" ) + "</b></td>";
    tmpStr += "<td>" + str + "</td>";
    tmpStr += "</tr>";
  }

  // Add optional participants
  str = displayViewFormatAttendeeRoleList( incidence, Attendee::OptParticipant, showStatus );
  if ( !str.isEmpty() ) {
    tmpStr += "<tr>";
    tmpStr += "<td><b>" + i18n( "Optional Participants:" ) + "</b></td>";
    tmpStr += "<td>" + str + "</td>";
    tmpStr += "</tr>";
  }

  // Add observers
  str = displayViewFormatAttendeeRoleList( incidence, Attendee::NonParticipant, showStatus );
  if ( !str.isEmpty() ) {
    tmpStr += "<tr>";
    tmpStr += "<td><b>" + i18n( "Observers:" ) + "</b></td>";
    tmpStr += "<td>" + str + "</td>";
    tmpStr += "</tr>";
  }

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
        if ( (*it)->label().isEmpty() ) {
          name = (*it)->uri();
        } else {
          name = (*it)->label();
        }
      }
      tmpStr += htmlAddLink( (*it)->uri(), name );
    } else {
      const QCString encodedLabel = KCodecs::base64Encode((*it)->label().utf8());
      tmpStr += htmlAddLink( "ATTACH:" + incidence->uid() + ':' + QString::fromUtf8(encodedLabel.data(), encodedLabel.length()),
                             (*it)->label(), false );
    }
    if ( count < as.count() ) {
      tmpStr += "<br>";
    }
  }
  return tmpStr;
}

static QString displayViewFormatCategories( Incidence *incidence )
{
  // We do not use Incidence::categoriesStr() since it does not have whitespace
  return incidence->categories().join( ", " );
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
  QString tmpStr = displayViewFormatPerson( email, name, uid, QString() );

  if ( event->customProperty( "KABC", "ANNIVERSARY") == "YES" ) {
    uid = event->customProperty("KABC","UID-2");
    name = event->customProperty("KABC","NAME-2");
    email= event->customProperty("KABC","EMAIL-2");
    tmpStr += "<br>" + displayViewFormatPerson( email, name, uid, QString() );
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

/* Get a pretty one line summary of an event so that it can be used in a list */
static QString displayViewFormatEventForList( Calendar *calendar, Event *event, bool noHtmlMode )
{
  if ( !event || !calendar ) {
    return QString::null;
  }

  QString tmpStr;

  tmpStr += invitationSummary( event, noHtmlMode );
  tmpStr += ": " + IncidenceFormatter::formatStartEnd( event->dtStart(), event->dtEnd(), event->doesFloat() );

  QString calStr = IncidenceFormatter::resourceString( calendar, event );

  if ( !calStr.isEmpty() ) {
    tmpStr += "<small> (" + calStr + ")</small>";
  }

  return tmpStr;
}

/* Format the events on the same day list for an inviation. */
static QString displayViewFormatEventsOnSameDays( InvitationFormatterHelper *helper, Event *event, bool noHtmlMode )
{
  if ( !event || !helper ) {
    return QString::null;
  }

  // Check calendar
  const QString checkCalBtn = helper->makeBtnLink( "check_calendar", i18n("Check calendar..." ), "go_jump_today" );

  QDateTime startDay = event->dtStart();
  QDateTime endDay = event->hasEndDate() ? event->dtEnd() : event->dtStart();
  startDay.setTime( QTime( 0, 0, 0 ) );
  endDay.setTime( QTime( 23, 59, 59 ) );

  Event::List matchingEvents = helper->calendar()->events(startDay.date(), endDay.date(), false);
  if ( matchingEvents.isEmpty() ) {
    return checkCalBtn;
  }

  QString tmpStr;
  tmpStr += "<span class=\"leftColumn\">";
  if ( event->hasEndDate() && event->dateEnd().daysTo( event->dtStart().date() ) > 0 ) {
    tmpStr += i18n( "Events on these days:" );
  } else {
    tmpStr += i18n( "Events on this day:" );
  }
  tmpStr += "&nbsp;&nbsp;" + checkCalBtn + "</span>\n";
  tmpStr += "<ul>\n";
  int count = 0;
  for ( Event::List::ConstIterator it = matchingEvents.begin(), end = matchingEvents.end();
      it != end && count < 50;
      ++it) {
    if ( (*it)->schedulingID() == event->uid() ) {
      // Exclude the same event from the list.
      continue;
    }
    if ( !(*it)->slicesInterval( startDay, endDay ) ) {
      /* Calendar::events includes events that have a recurrence that is
       * "active" in the specified interval. Wether or not the event is actually
       * happening ( has a recurrence that falls into the interval ).
       * This appears to be done deliberately and not to be a bug so we additionally
       * check if the event is actually happening here. */
      continue;
    }
    ++count;
    tmpStr += QString( "<li>" ) + displayViewFormatEventForList( helper->calendar(), *it, noHtmlMode ) +
      QString( "</li>\n" );
  }
  if ( count == 0 ) {
      /* Only the same event on this day. */
      return checkCalBtn;
  }
  if ( count == 50 ) {
    /* Abort after 50 entries to limit resource usage */
    tmpStr += "<li>...</li>\n";
  }
  tmpStr += "</ul>";
  return tmpStr;
}

static QString displayViewFormatEvent( Calendar *calendar, Event *event, const QDate &date )
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

  tmpStr += "<tr>";
  if ( event->doesFloat() ) {
    if ( event->isMultiDay() ) {
      tmpStr += "<td><b>" + i18n( "Date:" ) + "</b></td>";
      tmpStr += "<td>" +
                i18n("<beginDate> - <endDate>","%1 - %2").
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
      tmpStr += "<td><b>" + i18n( "Date:" ) + "</b></td>";
      tmpStr += "<td>" +
                i18n("<beginDate> - <endDate>","%1 - %2").
                arg( IncidenceFormatter::dateToString( startDt, false ) ).
                arg( IncidenceFormatter::dateToString( endDt, false ) ) +
                "</td>";
    } else {
      tmpStr += "<td><b>" + i18n( "Date:" ) + "</b></td>";
      tmpStr += "<td>" +
                i18n("date as string","%1").
                arg( IncidenceFormatter::dateToString( startDt, false ) ) +
                "</td>";

      tmpStr += "</tr><tr>";
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
    }
  }
  tmpStr += "</tr>";

  const QString durStr = IncidenceFormatter::durationString( event );
  if ( !durStr.isEmpty() ) {
    tmpStr += "<tr>";
    tmpStr += "<td><b>" + i18n( "Duration:" ) + "</b></td>";
    tmpStr += "<td>" + durStr + "</td>";
    tmpStr += "</tr>";
  }

  if ( event->doesRecur() ) {
    tmpStr += "<tr>";
    tmpStr += "<td><b>" + i18n( "Recurrence:" ) + "</b></td>";
    tmpStr += "<td>" +
              IncidenceFormatter::recurrenceString( event ) +
              "</td>";
    tmpStr += "</tr>";
  }

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

  // TODO: print comments?

  int reminderCount = event->alarms().count();
  if ( reminderCount > 0 && event->isAlarmEnabled() ) {
    tmpStr += "<tr>";
    tmpStr += "<td><b>" +
              i18n( "Reminder:", "%n Reminders:", reminderCount ) +
              "</b></td>";
    tmpStr += "<td>" + IncidenceFormatter::reminderStringList( event ).join( "<br>" ) + "</td>";
    tmpStr += "</tr>";
  }

  tmpStr += displayViewFormatAttendees( calendar, event );

  int categoryCount = event->categories().count();
  if ( categoryCount > 0 ) {
    tmpStr += "<tr>";
    tmpStr += "<td><b>" +
              i18n( "Category:", "%n Categories:", categoryCount ) +
              "</b></td>";
    tmpStr += "<td>" + displayViewFormatCategories( event ) + "</td>";
    tmpStr += "</tr>";
  }

  int attachmentCount = event->attachments().count();
  if ( attachmentCount > 0 ) {
    const QString formattedAttachments = displayViewFormatAttachments( event );
    if ( !formattedAttachments.isEmpty() ) {
      tmpStr += "<tr>";
      tmpStr += "<td><b>" +
        i18n( "Attachment:", "%n Attachments:", attachmentCount ) +
        "</b></td>";
      tmpStr += "<td>" + formattedAttachments + "</td>";
      tmpStr += "</tr>";
    }
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
    const QString calStr = IncidenceFormatter::resourceString( calendar, todo );
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
        QDateTime dt( date, QTime( 0, 0, 0 ) );
        dt = dt.addSecs( -1 );
        dueDt.setDate( todo->recurrence()->getNextDateTime( dt ).date() );
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

  const QString durStr = IncidenceFormatter::durationString( todo );
  if ( !durStr.isEmpty() ) {
    tmpStr += "<tr>";
    tmpStr += "<td><b>" + i18n( "Duration:" ) + "</b></td>";
    tmpStr += "<td>" + durStr + "</td>";
    tmpStr += "</tr>";
  }

  if ( todo->doesRecur() ) {
    tmpStr += "<tr>";
    tmpStr += "<td><b>" + i18n( "Recurrence:" ) + "</b></td>";
    tmpStr += "<td>" +
              IncidenceFormatter::recurrenceString( todo ) +
              "</td>";
    tmpStr += "</tr>";
  }

  if ( !todo->description().isEmpty() ) {
    tmpStr += "<tr>";
    tmpStr += "<td><b>" + i18n( "Description:" ) + "</b></td>";
    tmpStr += "<td>" + todo->description() + "</td>";
    tmpStr += "</tr>";
  }

  // TODO: print comments?

  int reminderCount = todo->alarms().count();
  if ( reminderCount > 0 && todo->isAlarmEnabled() ) {
    tmpStr += "<tr>";
    tmpStr += "<td><b>" +
              i18n( "Reminder:", "%n Reminders:", reminderCount ) +
              "</b></td>";
    tmpStr += "<td>" + IncidenceFormatter::reminderStringList( todo ).join( "<br>" ) + "</td>";
    tmpStr += "</tr>";
  }

  tmpStr += displayViewFormatAttendees( calendar, todo );

  int categoryCount = todo->categories().count();
  if ( categoryCount > 0 ) {
    tmpStr += "<tr>";
    tmpStr += "<td><b>" +
              i18n( "Category:", "%n Categories:", categoryCount ) +
              "</b></td>";
    tmpStr += "<td>" + displayViewFormatCategories( todo ) + "</td>";
    tmpStr += "</tr>";
  }

  if ( todo->priority() > 0 ) {
    tmpStr += "<tr>";
    tmpStr += "<td><b>" + i18n( "Priority:" ) + "</b></td>";
    tmpStr += "<td>";
    tmpStr += QString::number( todo->priority() );
    tmpStr += "</td>";
    tmpStr += "</tr>";
  }

  tmpStr += "<tr>";
  if ( todo->isCompleted() ) {
    tmpStr += "<td><b>" + i18n( "Completed:" ) + "</b></td>";
    tmpStr += "<td>";
    tmpStr += todo->completedStr();
  } else {
    tmpStr += "<td><b>" + i18n( "Percent Done:" ) + "</b></td>";
    tmpStr += "<td>";
    tmpStr += i18n( "%1%" ).arg( todo->percentComplete() );
  }
  tmpStr += "</td>";
  tmpStr += "</tr>";

  int attachmentCount = todo->attachments().count();
  if ( attachmentCount > 0 ) {
    const QString formattedAttachments = displayViewFormatAttachments( todo );
    if ( !formattedAttachments.isEmpty() ) {
      tmpStr += "<tr>";
      tmpStr += "<td><b>" +
                i18n( "Attachment:", "Attachments:", attachmentCount ) +
                "</b></td>";
      tmpStr += "<td>" + formattedAttachments + "</td>";
      tmpStr += "</tr>";
    }
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
    const QString calStr = IncidenceFormatter::resourceString( calendar, journal );
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
              i18n( "Category:", "%n Categories:", categoryCount ) +
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
  QString html = QStyleSheet::convertFromPlainText(str, QStyleSheetItem::WhiteSpaceNormal);
  // convertFromPlainText always butts the plain text in a paragraph.
  // this is not helpful when you want to layout the plain text in html.
  QRegExp para ( "<p>(.*)</p>" );
  para.search( html );
  return para.cap( 1 ).isEmpty() ? html : para.cap( 1 );
}

static QString cleanHtml( const QString &html )
{
  QRegExp rx( "<body[^>]*>(.*)</body>" );
  rx.setCaseSensitive( false );
  rx.search( html );
  QString body = rx.cap( 1 ).isEmpty() ? html : rx.cap( 1 );

  return QStyleSheet::escape( body.remove( QRegExp( "<[^>]*>" ) ).stripWhiteSpace() );
}

static QString invitationSummary( Incidence *incidence, bool noHtmlMode )
{
  QString summaryStr = i18n( "Summary unspecified" );
  if ( !incidence->summary().isEmpty() ) {
    if ( !QStyleSheet::mightBeRichText( incidence->summary() ) ) {
      summaryStr = QStyleSheet::escape( incidence->summary() );
    } else {
      summaryStr = incidence->summary();
      if ( noHtmlMode ) {
        summaryStr = cleanHtml( summaryStr );
      }
    }
  }
  return summaryStr;
}

static QString invitationLocation( Incidence *incidence, bool noHtmlMode )
{
  QString locationStr;
  if ( !incidence->location().isEmpty() ) {
    if ( !QStyleSheet::mightBeRichText( incidence->location() ) ) {
      locationStr = QStyleSheet::escape( incidence->location() );
    } else {
      locationStr = incidence->location();
      if ( noHtmlMode ) {
        locationStr = cleanHtml( locationStr );
      }
    }
  }
  return locationStr;
}

static QString htmlInvitationDetailsBegin(const QString &iconName, const QString &caption)
{
  QString dir = ( QApplication::reverseLayout() ? "rtl" : "ltr" );

  QString html = QString( "<div dir=\"%1\">\n" ).arg( dir );

  // Start with a caption and an identifing icon on the side
  html += QString( "<h2 class=\"summary\"><img src=\"%1\"/>%2</h2>\n").arg(
      KGlobal::iconLoader()->iconPath( iconName, KIcon::Desktop )).arg(
      caption );

  return html;
}

static QString htmlInvitationDetailsEnd()
{
  return "</div>\n";
}

static QString diffColor()
{
  // Color for printing comparison differences inside invitations.

//  return  "#DE8519"; // hard-coded color from Outlook2007
  return QColor( Qt::red ).name();
}

static QString noteColor()
{
  // Color for printing notes inside invitations.

//  return qApp->palette().color( QPalette::Active, QPalette::Highlight ).name();
  return QColor( Qt::red ).name();
}

static QString htmlCompare( const QString &value, const QString &oldvalue )
{
  // if 'value' is empty, then print nothing
  if ( value.isEmpty() ) {
    return QString::null;
  }

  // if 'value' is new or unchanged, then print normally
  if ( oldvalue.isEmpty() || value == oldvalue ) {
    return value;
  }

  // if 'value' has changed, then make a special print
  QString color = diffColor();
  QString newvalue = "<font color=\"" + color + "\">" + value + "</font>" +
                     "&nbsp;" +
                     "(<strike>" + oldvalue + "</strike>)";
  return newvalue;
}

static QString htmlRow( const QString &title, const QString &value )
{
  if ( !value.isEmpty() ) {
    return "<tr><td class=\"leftColumn\">" + title + "</td>\n<td>" + value + "</td></tr>\n";
  } else {
    return QString::null;
  }
}

static QString htmlRow( const QString &title, const QStringList &values )
{
  if ( !values.isEmpty() ) {
    return "<tr><td class=\"leftColumn\">" + title + "</td>\n<td>" + values.join( "<br/>" ) + "</td></tr>\n";
  } else {
    return QString::null;
  }
}

static QString htmlRow( const QString &title, const QStringList &values, const QStringList &oldValues )
{
  QStringList::ConstIterator it;
  QStringList result;
  QString color = diffColor();

  for ( it = values.constBegin(); it != values.constEnd(); ++it ) {
    if ( oldValues.contains( *it ) ) {
      result << *it; // print normally
    } else {
      // It's new so print it red
      result << "<font color=\"" + color + "\">" + *it + "</font>";
    }
  }
  for ( it = oldValues.constBegin(); it != oldValues.constEnd(); ++it ) {
    if ( !values.contains( *it ) ) {
      // value has been removed so strike it.
      result << "<strike>" + *it + "</strike>";
    }
  }
  return htmlRow( title, result );
}

static QString htmlRow( const QString &title, const QString &value, const QString &oldvalue )
{
  // if 'value' is empty, then print nothing
  if ( value.isEmpty() ) {
    return QString::null;
  }

  return htmlRow( title, htmlCompare ( value, oldvalue ) );
}

static Attendee *findDelegatedFromMyAttendee( Incidence *incidence )
{
  // Return the first attendee that was delegated-from me

  Attendee *attendee = 0;
  if ( !incidence ) {
    return attendee;
  }

  KEMailSettings settings;
  QStringList profiles = settings.profiles();
  for( QStringList::Iterator it=profiles.begin(); it!=profiles.end(); ++it ) {
    settings.setProfile( *it );

    QString delegatorName, delegatorEmail;
    Attendee::List attendees = incidence->attendees();
    Attendee::List::ConstIterator it2;
    for ( it2 = attendees.begin(); it2 != attendees.end(); ++it2 ) {
      Attendee *a = *it2;
      KPIM::getNameAndMail( a->delegator(), delegatorName, delegatorEmail );
      if ( settings.getSetting( KEMailSettings::EmailAddress ) == delegatorEmail ) {
        attendee = a;
        break;
      }
    }
  }
  return attendee;
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

static QString rsvpRequestedStr( bool rsvpRequested, const QString &role )
{
  if ( rsvpRequested ) {
    if ( role.isEmpty() ) {
      return i18n( "Your response is requested" );
    } else {
      return i18n( "Your response as <b>%1</b> is requested" ).arg( role );
    }
  } else {
    if ( role.isEmpty() ) {
      return i18n( "No response is necessary" );
    } else {
      return i18n( "No response as <b>%1</b> is necessary" ).arg( role );
    }
  }
}

static QString myStatusStr( Incidence *incidence )
{
  QString ret;
  Attendee *a = findMyAttendee( incidence );
  if ( a &&
       a->status() != Attendee::NeedsAction && a->status() != Attendee::Delegated ) {
    ret = i18n( "(Note: the Organizer preset your response to <b>%1</b>.)" ).
          arg( Attendee::statusName( a->status() ) );
  }
  return ret;
}

static QString invitationNote( const QString &title, const QString &note,
                               const QString &tag, const QString &color )
{
  QString noteStr;
  if ( !note.isEmpty() ) {
    noteStr += "<table border=\"0\" style=\"margin-top:4px;\">";
    noteStr += "<tr><center><td>";
    if ( !color.isEmpty() ) {
      noteStr += "<font color=\"" + color + "\">";
    }
    if ( !title.isEmpty() ) {
      if ( !tag.isEmpty() ) {
        noteStr += htmlAddTag( tag, title );
      } else {
        noteStr += title;
      }
    }
    noteStr += "&nbsp;" + note;
    if ( !color.isEmpty() ) {
      noteStr += "</font>";
    }
    noteStr += "</td></center></tr>";
    noteStr += "</table>";
  }
  return noteStr;
}

static QString invitationPerson( const QString &email, const QString &name, const QString &uid,
                                 const QString &comment )
{
  QPair<QString, QString> s = searchNameAndUid( email, name, uid );
  const QString printName = s.first;
  const QString printUid = s.second;

  QString personString;
  // Make the uid link
  if ( !printUid.isEmpty() ) {
    personString = htmlAddUidLink( email, printName, printUid );
  }
  // Make the mailto link
  if ( !email.isEmpty() ) {
    personString += "&nbsp;" + htmlAddMailtoLink( email, printName );
  } else {
    personString = ( printName.isEmpty() ? email : printName );
  }

  if ( !comment.isEmpty() ) {
    // beware personString might already contain percent escaped values.
    // so you can not chain qt arg replacement here.
    // personString += i18n( "name (comment)", "%1 (%2)" ).arg( personString ).arg( comment );
    personString += QString( " (%2)" ).arg( comment );
  }

  personString += "\n";

  return personString;
}

static QString invitationCommentsIncidence( Incidence *incidence, bool noHtmlMode )
{
  QString html;
  QStringList comments;

  if ( !incidence || incidence->comments().isEmpty() ) {
    return QString::null;
  }

  // non-empty comments
  QStringList cl = incidence->comments();
  for( QStringList::Iterator it=cl.begin(); it!=cl.end(); ++it ) {
    if ( !QStyleSheet::mightBeRichText( *it ) ) {
      comments.append( string2HTML( *it ) );
    } else {
      if ( noHtmlMode ) {
        comments.append( cleanHtml( *it ) );
      } else {
        comments.append( *it );
      }
    }
  }

  if ( !comments.isEmpty() ) {
    html += "<table><tr>\n<td class=\"leftColumn\">" ;
    if ( comments.count() > 1 ) {
      html += i18n( "Comments:" ) + "</td>\n<td>\n<ul>\n";
      for ( uint i=0; i < comments.count(); ++i ) {
        html += "<li>" + comments[i] + "</li>\n";
      }
      html += "</ul>\n";
    } else {
      html += i18n( "Comment:" ) + "</td>\n<td>\n";
      html += comments[0];
    }
    html += "\n</td>\n</tr></table>";
  }
  return html;
}

static QString invitationDescriptionIncidence( Incidence *incidence, bool noHtmlMode )
{
  QString html;
  QString descr;

  if ( !incidence->description().isEmpty() ) {
    // use description as comments
    if ( !QStyleSheet::mightBeRichText( incidence->description() ) ) {
      descr = string2HTML( incidence->description() );
    } else {
      descr = incidence->description();
      if ( noHtmlMode ) {
        descr = cleanHtml( descr );
      }
    }
  }

  if( !descr.isEmpty() ) {
    html += "<tr>\n<td class=\"leftColumn\">" + i18n( "Description:" ) + "</td>\n";
    html += "<td>" + descr + "</td>\n</tr>\n";
  }
  return html;
}

QString IncidenceFormatter::formatStartEnd( const QDateTime &start, const QDateTime &end, bool floats )
{
  QString tmpStr;

  // <startDate[time]> [- <[endDate][Time]>]
  // The startDate is always printed.
  // If the event does float the time is omited.
  //
  // If it has an end dateTime:
  // on the same day -> Only add end time.
  // if it floats also emit the time.
  tmpStr += IncidenceFormatter::dateTimeToString( start, floats );

  if ( end.isValid() ) {
    if ( start.date() == end.date() ) {
      // same day
      if ( !floats ) {
        tmpStr += " - " + IncidenceFormatter::timeToString( end, true );
      }
    } else {
      tmpStr += " - " + IncidenceFormatter::dateTimeToString( end, floats );
    }
  }
  return tmpStr;
}

static QString invitationDetailsEvent( Event* event, bool noHtmlMode )
{
  // Invitation details are formatted into an HTML table
  if ( !event ) {
    return QString::null;
  }

  QString html = htmlInvitationDetailsBegin( "view_pim_calendar",
                                             invitationSummary( event, noHtmlMode ) );

  html += htmlRow( event->doesRecur() ? i18n( "First event:" ) : i18n( "When:" ),
      IncidenceFormatter::formatStartEnd ( event->dtStart(), event->dtEnd(), event->doesFloat() ) );

  const QString location = invitationLocation( event, noHtmlMode );

  if ( !location.isEmpty() ) {
    html += htmlRow( i18n( "Where:" ), location );
  }

  if ( event->doesRecur() ) {
    html += htmlRow( i18n( "Recurrence:" ), IncidenceFormatter::recurrenceString( event, false ) );

    const QStringList exceptions = IncidenceFormatter::exceptionStrings( event->recurrence() );
    html += htmlRow ( i18n( "Excluding:" ), exceptions );
  }

  html += invitationDescriptionIncidence( event, noHtmlMode );
  html += htmlInvitationDetailsEnd();

  return html;
}

static QString invitationDetailsEvent( Event *event, Event *oldevent, ScheduleMessage *message,
                                       bool noHtmlMode )
{
  if ( !oldevent ) {
    return invitationDetailsEvent( event, noHtmlMode );
  }

  QString html;

  // Print extra info typically dependent on the iTIP
  if ( message->method() == Scheduler::Declinecounter ) {
    html += "<br>";
    html += invitationNote( QString::null,
                            i18n( "Please respond again to the original proposal." ),
                            QString::null, noteColor() );
  }

  html += htmlInvitationDetailsBegin( "view_pim_calendar",
            htmlCompare( invitationSummary( event, noHtmlMode ),
                         invitationSummary( oldevent, noHtmlMode ) ) );

  const QString location = htmlCompare( invitationLocation( event, noHtmlMode ),
                   invitationLocation( oldevent, noHtmlMode ) );

  html += htmlRow( event->doesRecur() ? i18n( "First event:" ) : i18n( "When:" ),
      IncidenceFormatter::formatStartEnd ( event->dtStart(), event->dtEnd(), event->doesFloat() ),
      IncidenceFormatter::formatStartEnd ( oldevent->dtStart(), oldevent->dtEnd(), oldevent->doesFloat() ) );

  if ( !location.isEmpty() ) {
    html += htmlRow( i18n( "Where:" ), location );
  }

  if ( event->doesRecur() || oldevent->doesRecur() ) {
    QString recurStr, oldrecurStr;
    recurStr = IncidenceFormatter::recurrenceString( event, false );
    oldrecurStr = IncidenceFormatter::recurrenceString( oldevent, false );
    html += htmlRow( i18n( "Recurrence:" ), recurStr, oldrecurStr );

    const QStringList oldExceptions = IncidenceFormatter::exceptionStrings( oldevent->recurrence() );
    const QStringList exceptions = IncidenceFormatter::exceptionStrings( event->recurrence() );

    html += htmlRow ( i18n( "Excluding:" ), exceptions, oldExceptions );
  }

  html += invitationDescriptionIncidence( event, noHtmlMode );
  html += htmlInvitationDetailsEnd();

  return html;
}

static QString invitationDetailsTodo( Todo *todo, bool noHtmlMode )
{
  // Task details are formatted into an HTML table
  if ( !todo ) {
    return QString::null;
  }

  QString html = htmlInvitationDetailsBegin( "view_pim_tasks",
                                             invitationSummary( todo, noHtmlMode ) );

  const QString location = invitationLocation( todo, noHtmlMode );

  if ( !location.isEmpty() ) {
    html += htmlRow( i18n( "Where:" ), location );
  }

  if ( todo->hasStartDate() && todo->dtStart().isValid() ) {
    // Start and end combine into a single when.
    html += htmlRow( i18n( "When:" ), IncidenceFormatter::formatStartEnd ( todo->dtStart(),
          todo->dtDue(), todo->doesFloat() ) );
  } else if ( todo->hasDueDate() && todo->dtDue().isValid() ) {
    // Only a due date.
    html += htmlRow( i18n( "Due:" ),
        IncidenceFormatter::dateTimeToString( todo->dtDue(), todo->doesFloat() ) );
  }

  // Completeness
  if ( todo->percentComplete() > 0 ) {
    html += htmlRow( i18n( "Percent Done:" ), i18n( "%1%" ).arg( todo->percentComplete() ) );
  }

  // Invitation Recurrence Row
  if ( todo->doesRecur() ) {
    html += htmlRow( i18n( "Recurrence:" ), IncidenceFormatter::recurrenceString( todo, false ) );

    const QStringList exceptions = IncidenceFormatter::exceptionStrings( todo->recurrence() );
    html += htmlRow ( i18n( "Excluding:" ), exceptions );
  }

  html += invitationDescriptionIncidence( todo, noHtmlMode );
  html += htmlInvitationDetailsEnd();

  return html;
}

static QString invitationDetailsTodo( Todo *todo, Todo *oldtodo, ScheduleMessage *message,
                                      bool noHtmlMode )
{
  if ( !oldtodo ) {
    return invitationDetailsTodo( todo, noHtmlMode );
  }

  QString html;

  // Print extra info typically dependent on the iTIP
  if ( message->method() == Scheduler::Declinecounter ) {
    html += "<br>";
    html += invitationNote( QString::null,
                            i18n( "Please respond again to the original proposal." ),
                            QString::null, noteColor() );
  }

  html += htmlInvitationDetailsBegin( "view_pim_tasks",
            htmlCompare( invitationSummary( todo, noHtmlMode ),
                         invitationSummary( todo, noHtmlMode ) ) );

  html += htmlRow( i18n( "Where:" ),
                   invitationLocation( todo, noHtmlMode ),
                   invitationLocation( oldtodo, noHtmlMode ) );

  if ( ( todo->hasStartDate() && todo->dtStart().isValid() ) || oldtodo->hasStartDate() ) {
    html += htmlRow( i18n( "When:" ),
              IncidenceFormatter::formatStartEnd( todo->dtStart(), todo->dtDue(), todo->doesFloat() ),
              IncidenceFormatter::formatStartEnd( oldtodo->dtStart(), oldtodo->dtDue(), oldtodo->doesFloat() ) );
  } else if ( ( todo->hasDueDate() && todo->dtDue().isValid() ) || oldtodo->hasDueDate() ) {
    html += htmlRow( i18n( "Due:" ),
                     IncidenceFormatter::dateTimeToString( todo->dtDue(), todo->doesFloat() ),
                     IncidenceFormatter::dateTimeToString( oldtodo->dtDue(), todo->doesFloat() ) );
  }

  QString completionStr, oldcompletionStr;
  if ( todo->percentComplete() > 0 || oldtodo->percentComplete() > 0 ) {
    completionStr = i18n( "%1%" ).arg( todo->percentComplete() );
    oldcompletionStr = i18n( "%1%" ).arg( oldtodo->percentComplete() );
    html += htmlRow( i18n( "Percent Done:" ), completionStr, oldcompletionStr );
  }

  QString recurStr, oldrecurStr;
  if ( todo->doesRecur() || oldtodo->doesRecur() ) {
    recurStr = IncidenceFormatter::recurrenceString( todo );
    oldrecurStr = IncidenceFormatter::recurrenceString( oldtodo );
    html += htmlRow( i18n( "Recurrence:" ), recurStr, oldrecurStr );

    const QStringList oldExceptions = IncidenceFormatter::exceptionStrings( oldtodo->recurrence() );
    const QStringList exceptions = IncidenceFormatter::exceptionStrings( todo->recurrence() );

    html += htmlRow ( i18n( "Excluding:" ), exceptions, oldExceptions );
  }

  html += invitationDescriptionIncidence( todo, noHtmlMode );
  html += htmlInvitationDetailsEnd();

  return html;
}

static QString invitationDetailsJournal( Journal *journal, bool noHtmlMode )
{
  if ( !journal ) {
    return QString::null;
  }

  QString html = htmlInvitationDetailsBegin( "view_pim_journal",
                                             invitationSummary( journal, noHtmlMode ) );

  html += htmlRow( i18n( "Date:" ), IncidenceFormatter::dateToString( journal->dtStart(), false ) );

  html += invitationDescriptionIncidence( journal, noHtmlMode );
  html += htmlInvitationDetailsEnd();

  return html;
}

static QString invitationDetailsJournal( Journal *journal, Journal *oldjournal, bool noHtmlMode )
{
  if ( !oldjournal ) {
    return invitationDetailsJournal( journal, noHtmlMode );
  }

  QString html = htmlInvitationDetailsBegin( "view_pim_journal",
                   htmlCompare( invitationSummary( journal, noHtmlMode ),
                                invitationSummary( oldjournal, noHtmlMode ) ) );

  html += htmlRow( i18n( "Date:" ),
                   IncidenceFormatter::dateToString( journal->dtStart(), false ),
                   IncidenceFormatter::dateToString( oldjournal->dtStart(), false ) );

  html += invitationDescriptionIncidence( journal, noHtmlMode );
  html += htmlInvitationDetailsEnd();

  return html;
}

static QString invitationDetailsFreeBusy( FreeBusy *fb, bool /*noHtmlMode*/ )
{
  if ( !fb ) {
    return QString::null;
  }

  QString html = "<table>";

  html += htmlRow( i18n("Person:"), fb->organizer().fullName() );
  html += htmlRow( i18n("Start date:"),
                   IncidenceFormatter::dateToString( fb->dtStart(), true ) );
  html += htmlRow( i18n("End date:"),
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
      html += htmlRow( QString::null, i18n("startDate for duration", "%1 for %2").
                       arg( KGlobal::locale()->formatDateTime( per.start(), false ) ).
                       arg(cont) );
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

      html += htmlRow( QString::null, cont );
    }
  }

  html += "</table>";
  return html;
}

static QString invitationDetailsFreeBusy( FreeBusy *freebusy, FreeBusy *oldfreebusy, bool noHtmlMode )
{
  Q_UNUSED( oldfreebusy );
  return invitationDetailsFreeBusy( freebusy, noHtmlMode );
}

static bool replyMeansCounter( Incidence */*incidence*/ )
{
  return false;
/**
  see kolab/issue3665 for an example of when we might use this for something

  bool status = false;
  if ( incidence ) {
    // put code here that looks at the incidence and determines that
    // the reply is meant to be a counter proposal.  We think this happens
    // with Outlook counter proposals, but we aren't sure how yet.
    if ( condition ) {
      status = true;
    }
  }
  return status;
*/
}

static QString invitationHeaderEvent( Event *event, Incidence *existingIncidence,
                                      ScheduleMessage *msg, const QString &sender )
{
  if ( !msg || !event )
    return QString::null;

  switch ( msg->method() ) {
  case Scheduler::Publish:
    return QString::null; //i18n( "This invitation has been published" );
  case Scheduler::Request:
    if ( existingIncidence && event->revision() > 0 ) {
      QString orgStr = organizerName( event, sender );
      if ( senderIsOrganizer( event, sender ) ) {
        return i18n( "This invitation has been updated by the organizer <b>%1</b>." ).arg( orgStr );
      } else {
        return i18n( "This invitation has been updated by <b>%1</b> as a representative of <b>%2</b>." ).
          arg( sender, orgStr );
      }
    }
    if ( iamOrganizer( event ) ) {
      return QString::null; // i18n( "I created this invitation" );
    } else {
      QString orgStr = organizerName( event, sender );
      if ( senderIsOrganizer( event, sender ) ) {
        return QString::null; // i18n( "You received an invitation from <b>%1</b>" ).arg( orgStr );
      } else {
        return i18n( "You received an invitation from <b>%1</b> as a representative of <b>%2</b>." ).
          arg( sender, orgStr );
      }
    }
  case Scheduler::Refresh:
    return i18n( "This invitation was refreshed." );
  case Scheduler::Cancel:
    if ( iamOrganizer( event ) ) {
      return i18n( "This invitation has been canceled." );
    } else {
      const Event *existingEvent = dynamic_cast<Event*>( existingIncidence );
      if ( existingEvent ) {
        // We check if the CANCEL message contains all attendees from the original
        // message, in this case we assume that the event has been canceled, otherwise
        // only a subset of the attendees (including us) has been disinvited.
        const Attendee::List oldAttendees = existingEvent->attendees();
        const Attendee::List newAttendees = event->attendees();

        bool hasAllAttendees = true;
        for ( unsigned int i = 0; i < oldAttendees.count(); ++i ) {
          bool containsAttendee = false;
          for ( unsigned int j = 0; j < newAttendees.count(); j++ ) {
            if ( oldAttendees[i]->email() == newAttendees[j]->email() ) {
              containsAttendee = true;
              break;
            }
          }

          if ( !containsAttendee ) {
            hasAllAttendees = false;
            break;
          }
        }

        if ( oldAttendees.count() != newAttendees.count() )
          hasAllAttendees = false;

        if ( hasAllAttendees )
          return i18n( "The organizer has canceled the event." );
      }

      return i18n( "The organizer has removed you from the invitation." );
    }
  case Scheduler::Add:
    return i18n( "Addition to the invitation." );
  case Scheduler::Reply:
  {
    if ( replyMeansCounter( event ) ) {
      return i18n( "<b>%1</b> sent a counter proposal." ).arg( sender );
    }

    Attendee::List attendees = event->attendees();
    if( attendees.count() == 0 ) {
      kdDebug(5850) << "No attendees in the iCal reply!" << endl;
      return QString::null;
    }
    if( attendees.count() != 1 ) {
      kdDebug(5850) << "Warning: attendeecount in the reply should be 1 "
                    << "but is " << attendees.count() << endl;
    }
    QString attendeeName = firstAttendeeName( event, sender );

    QString delegatorName, dummy;
    Attendee* attendee = *attendees.begin();
    KPIM::getNameAndMail( attendee->delegator(), delegatorName, dummy );
    if ( delegatorName.isEmpty() ) {
      delegatorName = attendee->delegator();
    }

    switch( attendee->status() ) {
    case Attendee::NeedsAction:
      return i18n( "<b>%1</b> indicates this invitation still needs some action." ).arg( attendeeName );
    case Attendee::Accepted:
      if ( event->revision() > 0 ) {
        if ( !sender.isEmpty() ) {
          return i18n( "This invitation has been updated by attendee <b>%1</b>." ).arg( sender );
        } else {
          return i18n( "This invitation has been updated by an attendee." );
        }
      } else {
        if ( delegatorName.isEmpty() ) {
          return i18n( "<b>%1</b> accepts this invitation." ).arg( attendeeName );
        } else {
          return i18n( "<b>%1</b> accepts this invitation on behalf of <b>%2</b>." ).
            arg( attendeeName ).arg( delegatorName );
        }
      }
    case Attendee::Tentative:
      if ( delegatorName.isEmpty() ) {
        return i18n( "<b>%1</b> tentatively accepts this invitation." ).
          arg( attendeeName );
      } else {
        return i18n( "<b>%1</b> tentatively accepts this invitation on behalf of <b>%2</b>." ).
          arg( attendeeName ).arg( delegatorName );
      }
    case Attendee::Declined:
      if ( delegatorName.isEmpty() ) {
        return i18n( "<b>%1</b> declines this invitation." ).arg( attendeeName );
      } else {
        return i18n( "<b>%1</b> declines this invitation on behalf of <b>%2</b>." ).
          arg( attendeeName ).arg( delegatorName );
      }
    case Attendee::Delegated: {
      QString delegate, dummy;
      KPIM::getNameAndMail( attendee->delegate(), delegate, dummy );
      if ( delegate.isEmpty() ) {
        delegate = attendee->delegate();
      }
      if ( !delegate.isEmpty() ) {
        return i18n( "<b>%1</b> has delegated this invitation to <b>%2</b>." ).
          arg( attendeeName ) .arg( delegate );
      } else {
        return i18n( "<b>%1</b> has delegated this invitation." ).arg( attendeeName );
      }
    }
    case Attendee::Completed:
      return i18n( "This invitation is now completed." );
    case Attendee::InProcess:
      return i18n( "<b>%1</b> is still processing the invitation." ).
        arg( attendeeName );
    default:
      return i18n( "Unknown response to this invitation." );
    }
    break;
  }

  case Scheduler::Counter:
    return i18n( "<b>%1</b> sent a counter proposal." ).arg( sender );

  case Scheduler::Declinecounter:
  {
    QString orgStr = organizerName( event, sender );
    if ( senderIsOrganizer( event, sender ) ) {
      return i18n( "<b>%1</b> declines your counter proposal." ).arg( orgStr );
    } else {
      return i18n( "<b>%1</b> declines your counter proposal on behalf of <b>%2</b>." ).arg( sender, orgStr );
    }
  }

  case Scheduler::NoMethod:
    return i18n("Error: iTIP message with unknown method: '%1'").
      arg( msg->method() );
  }
  return QString::null;
}

static QString invitationHeaderTodo( Todo *todo, Incidence *existingIncidence,
                                     ScheduleMessage *msg, const QString &sender )
{
  if ( !msg || !todo ) {
    return QString::null;
  }

  switch ( msg->method() ) {
  case Scheduler::Publish:
    return QString::null; // i18n("This task has been published");
  case Scheduler::Request:
    if ( existingIncidence && todo->revision() > 0 ) {
      QString orgStr = organizerName( todo, sender );
      if ( senderIsOrganizer( todo, sender ) ) {
        return i18n( "This task has been updated by the organizer <b>%1</b>." ).arg( orgStr );
      } else {
        return i18n( "This task has been updated by <b>%1</b> as a representative of <b>%2</b>." ).
          arg( sender, orgStr );
      }
    } else {
      if ( iamOrganizer( todo ) ) {
        return QString::null; //i18n( "I created this task" );
      } else {
        QString orgStr = organizerName( todo, sender );
        if ( senderIsOrganizer( todo, sender ) ) {
          return i18n( "You have been assigned this task by <b>%1</b>." ).arg( orgStr );
        } else {
          return i18n( "You have been assigned this task by <b>%1</b> as a representative of <b>%2</b>." ).
            arg( sender, orgStr );
        }
      }
    }
  case Scheduler::Refresh:
    return i18n( "This task was refreshed." );
  case Scheduler::Cancel:
    if ( iamOrganizer( todo ) ) {
      return i18n( "This task was canceled." );
    } else {
      return i18n( "The organizer has removed you from this task." );
    }
  case Scheduler::Add:
    return i18n( "Addition to the task." );
  case Scheduler::Reply:
  {
    if ( replyMeansCounter( todo ) ) {
      return i18n( "<b>%1</b> sent a counter proposal." ).arg( sender );
    }

    Attendee::List attendees = todo->attendees();
    if( attendees.count() == 0 ) {
      kdDebug(5850) << "No attendees in the iCal reply!" << endl;
      return QString::null;
    }
    if( attendees.count() != 1 ) {
      kdDebug(5850) << "Warning: attendeecount in the reply should be 1 "
                    << "but is " << attendees.count() << endl;
    }
    QString attendeeName = firstAttendeeName( todo, sender );

    QString delegatorName, dummy;
    Attendee* attendee = *attendees.begin();
    KPIM::getNameAndMail( attendee->delegator(), delegatorName, dummy );
    if ( delegatorName.isEmpty() ) {
      delegatorName = attendee->delegator();
    }

    switch( attendee->status() ) {
    case Attendee::NeedsAction:
      return i18n( "<b>%1</b> indicates this task assignment still needs some action." ).arg( attendeeName );
    case Attendee::Accepted:
      if ( todo->revision() > 0 ) {
        if ( !sender.isEmpty() ) {
          if ( todo->isCompleted() ) {
            return i18n( "This task has been completed by assignee <b>%1</b>." ).arg( sender );
          } else {
            return i18n( "This task has been updated by assignee <b>%1</b>." ).arg( sender );
          }
        } else {
          if ( todo->isCompleted() ) {
            return i18n( "This task has been completed by an assignee." );
          } else {
            return i18n( "This task has been updated by an assignee." );
          }
        }
      } else {
        if ( delegatorName.isEmpty() ) {
          return i18n( "<b>%1</b> accepts this task." ).arg( attendeeName );
        } else {
          return i18n( "<b>%1</b> accepts this task on behalf of <b>%2</b>." ).
            arg( attendeeName ).arg( delegatorName );
        }
      }
    case Attendee::Tentative:
      if ( delegatorName.isEmpty() ) {
        return i18n( "<b>%1</b> tentatively accepts this task." ).
          arg( attendeeName );
      } else {
        return i18n( "<b>%1</b> tentatively accepts this task on behalf of <b>%2</b>." ).
          arg( attendeeName ).arg( delegatorName );
      }
    case Attendee::Declined:
      if ( delegatorName.isEmpty() ) {
        return i18n( "<b>%1</b> declines this task." ).arg( attendeeName );
      } else {
        return i18n( "<b>%1</b> declines this task on behalf of <b>%2</b>." ).
          arg( attendeeName ).arg( delegatorName );
      }
    case Attendee::Delegated: {
      QString delegate, dummy;
      KPIM::getNameAndMail( attendee->delegate(), delegate, dummy );
      if ( delegate.isEmpty() ) {
        delegate = attendee->delegate();
      }
      if ( !delegate.isEmpty() ) {
        return i18n( "<b>%1</b> has delegated this request for the task to <b>%2</b>." ).
          arg( attendeeName ).arg( delegate );
      } else {
        return i18n( "<b>%1</b> has delegated this request for the task." ).
          arg( attendeeName );
      }
    }
    case Attendee::Completed:
      return i18n( "The request for this task is now completed." );
    case Attendee::InProcess:
      return i18n( "<b>%1</b> is still processing the task." ).
        arg( attendeeName );
    default:
      return i18n( "Unknown response to this task." );
    }
    break;
  }

  case Scheduler::Counter:
    return i18n( "<b>%1</b> sent a counter proposal." ).arg( sender );

  case Scheduler::Declinecounter:
  {
    QString orgStr = organizerName( todo, sender );
    if ( senderIsOrganizer( todo, sender ) ) {
      return i18n( "<b>%1</b> declines the counter proposal." ).arg( orgStr );
    } else {
      return i18n( "<b>%1</b> declines the counter proposal on behalf of <b>%2</b>." ).arg( sender, orgStr );
    }
  }

  case Scheduler::NoMethod:
    return i18n( "Error: iTIP message with unknown method: '%1'" ).
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
    return i18n("This journal has been published.");
  case Scheduler::Request:
    return i18n( "You have been assigned this journal." );
  case Scheduler::Refresh:
    return i18n( "This journal was refreshed." );
  case Scheduler::Cancel:
    return i18n( "This journal was canceled." );
  case Scheduler::Add:
    return i18n( "Addition to the journal." );
  case Scheduler::Reply:
  {
    if ( replyMeansCounter( journal ) ) {
      return i18n( "Counter proposal." );
    }

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
      return i18n( "Sender indicates this journal assignment still needs some action." );
    case Attendee::Accepted:
      return i18n( "Sender accepts this journal." );
    case Attendee::Tentative:
      return i18n( "Sender tentatively accepts this journal." );
    case Attendee::Declined:
      return i18n( "Sender declines this journal." );
    case Attendee::Delegated:
      return i18n( "Sender has delegated this request for the journal." );
    case Attendee::Completed:
      return i18n( "The request for this journal is now completed." );
    case Attendee::InProcess:
      return i18n( "Sender is still processing the invitation." );
    default:
      return i18n( "Unknown response to this journal." );
    }
    break;
  }
  case Scheduler::Counter:
    return i18n( "Counter proposal" );
  case Scheduler::Declinecounter:
    return i18n( "Sender declines the counter proposal." );
  case Scheduler::NoMethod:
    return i18n("Error: iTIP message with unknown method: '%1'").
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
    return i18n("This free/busy list has been published.");
  case Scheduler::Request:
    return i18n( "The free/busy list has been requested." );
  case Scheduler::Refresh:
    return i18n( "This free/busy list was refreshed." );
  case Scheduler::Cancel:
    return i18n( "This free/busy list was canceled." );
  case Scheduler::Add:
    return i18n( "Addition to the free/busy list." );
  case Scheduler::NoMethod:
  default:
    return i18n("Error: Free/Busy iTIP message with unknown method: '%1'").
      arg( msg->method() );
  }
}

static QString invitationAttendeeList( Incidence *incidence )
{
  QString tmpStr;
  if ( !incidence ) {
    return tmpStr;
  }

  tmpStr += "<tr>\n<td class=\"leftColumn\">";
  if ( incidence->type() == "Todo" ) {
    tmpStr += i18n( "Assignees" ) + ":";
  } else {
    tmpStr += i18n( "Participants" ) + ":";
  }
  tmpStr += "</td>\n<td>";

  int count=0;
  Attendee::List attendees = incidence->attendees();
  if ( !attendees.isEmpty() ) {
    QStringList comments;
    Attendee::List::ConstIterator it;
    for( it = attendees.begin(); it != attendees.end(); ++it ) {
      Attendee *a = *it;
      if ( !iamAttendee( a ) ) {
        count++;
        comments.clear();
        if ( attendeeIsOrganizer( incidence, a ) ) {
          comments << i18n( "organizer" );
        }
        if ( !a->delegator().isEmpty() ) {
          comments << i18n( "delegated by %1" ).arg( a->delegator() );
        }
        if ( !a->delegate().isEmpty() ) {
          comments << i18n( "delegated to %1" ).arg( a->delegate() );
        }
        tmpStr += invitationPerson( a->email(), a->name(), QString::null, comments.join( "," ) );
        tmpStr += "<br>\n";
      }
    }
  }
  if ( !count ) {
    return QString::null;
  }
  tmpStr += "</td>\n</tr>\n";

  return tmpStr;
}

static QString invitationRsvpList( Incidence *incidence, Attendee *sender )
{
  QString tmpStr;
  if ( !incidence ) {
    return tmpStr;
  }

  tmpStr += "<tr>\n<td class=\"leftColumn\">";

  if ( incidence->type() == "Todo" ) {
    tmpStr += i18n( "Assignees" ) + ":" ;
  } else {
    tmpStr += i18n( "Participants" ) + ":";
  }
  tmpStr += "</td>\n<td>";

  int count=0;
  Attendee::List attendees = incidence->attendees();
  if ( !attendees.isEmpty() ) {
    QStringList comments;
    Attendee::List::ConstIterator it;
    for( it = attendees.begin(); it != attendees.end(); ++it ) {
      Attendee *a = *it;
      if ( !attendeeIsOrganizer( incidence, a ) ) {
        QString statusStr;
        const QString iconPath = rsvpStatusIconPath( a->status() );
        tmpStr += QString( "<img src=\"%1\"/>" ).arg( iconPath );
        if ( sender && ( a->email() == sender->email() ) ) {
          // use the attendee taken from the response incidence,
          // rather than the attendee from the calendar incidence.
          if ( a->status() != sender->status() ) {
            statusStr += "<small>" + i18n( "(Status not yet recorded)" ) + "</small>";
          }
          a = sender;
        }
        count++;
        comments.clear();
        if ( iamAttendee( a ) ) {
          comments << i18n( "myself" );
        }
        if ( !a->delegator().isEmpty() ) {
          comments << i18n( "delegated by %1" ).arg( a->delegator() );
        }
        if ( !a->delegate().isEmpty() ) {
          comments << i18n( "delegated to %1" ).arg( a->delegate() );
        }
        tmpStr += invitationPerson( a->email(), a->name(), QString::null, comments.join( "," ) );
        tmpStr += " " + statusStr + "<br>\n";
      }
    }
  }
  if ( !count ) {
    tmpStr += "<i>" + i18n( "No attendee", "None" ) + "</i>";
  }

  tmpStr += "</td>\n</tr>\n";

  return tmpStr;
}

static QString invitationAttachments( InvitationFormatterHelper *helper, Incidence *incidence )
{
  QString tmpStr;
  if ( !incidence ) {
    return tmpStr;
  }

  Attachment::List attachments = incidence->attachments();
  QString rightCol;
  if ( !attachments.isEmpty() ) {
    tmpStr += "<tr>\n<td class=\"leftColumn\">" + i18n( "Attachments:" ) + "</td><td>";

    Attachment::List::ConstIterator it;
    for( it = attachments.begin(); it != attachments.end(); ++it ) {
      Attachment *a = *it;
      if ( a->label().isEmpty() ) {
          continue;
      }
      // Attachment icon
      KMimeType::Ptr mimeType = KMimeType::mimeType( a->mimeType() );
      const QString iconStr = mimeType ? mimeType->icon( a->uri(), false ) : QString( "application-octet-stream" );
      const QString iconPath = KGlobal::iconLoader()->iconPath( iconStr, KIcon::Small );
      if ( !iconPath.isEmpty() ) {
        rightCol += "<img valign=\"top\" src=\"" + iconPath + "\">";
      }
      const QCString encodedLabel = KCodecs::base64Encode(a->label().utf8());
      rightCol += helper->makeLink( "ATTACH:" + QString::fromUtf8(encodedLabel.data(), encodedLabel.length()), a->label() );
      rightCol += "<br>";
    }
  }

  return rightCol.isEmpty() ? QString::null : tmpStr + rightCol;
}

class IncidenceFormatter::ScheduleMessageVisitor
  : public IncidenceBase::Visitor
{
  public:
    ScheduleMessageVisitor() : mExistingIncidence( 0 ), mMessage( 0 ) { mResult = ""; }
    bool act( IncidenceBase *incidence, Incidence *existingIncidence, ScheduleMessage *msg,
              const QString &sender )
    {
      mExistingIncidence = existingIncidence;
      mMessage = msg;
      mSender = sender;
      return incidence->accept( *this );
    }
    QString result() const { return mResult; }

  protected:
    QString mResult;
    Incidence *mExistingIncidence;
    ScheduleMessage *mMessage;
    QString mSender;
};

class IncidenceFormatter::InvitationHeaderVisitor
  : public IncidenceFormatter::ScheduleMessageVisitor
{
  protected:
    bool visit( Event *event )
    {
      mResult = invitationHeaderEvent( event, mExistingIncidence, mMessage, mSender );
      return !mResult.isEmpty();
    }
    bool visit( Todo *todo )
    {
      mResult = invitationHeaderTodo( todo, mExistingIncidence, mMessage, mSender );
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

class IncidenceFormatter::InvitationBodyVisitor
  : public IncidenceFormatter::ScheduleMessageVisitor
{
  public:
    InvitationBodyVisitor( bool noHtmlMode )
      : ScheduleMessageVisitor(), mNoHtmlMode( noHtmlMode ) {}

  protected:
    bool visit( Event *event )
    {
      Event *oldevent = dynamic_cast<Event *>( mExistingIncidence );
      mResult = invitationDetailsEvent( event, oldevent, mMessage, mNoHtmlMode );
      return !mResult.isEmpty();
    }
    bool visit( Todo *todo )
    {
      Todo *oldtodo = dynamic_cast<Todo *>( mExistingIncidence );
      mResult = invitationDetailsTodo( todo, oldtodo, mMessage, mNoHtmlMode );
      return !mResult.isEmpty();
    }
    bool visit( Journal *journal )
    {
      Journal *oldjournal = dynamic_cast<Journal *>( mExistingIncidence );
      mResult = invitationDetailsJournal( journal, oldjournal, mNoHtmlMode );
      return !mResult.isEmpty();
    }
    bool visit( FreeBusy *fb )
    {
      mResult = invitationDetailsFreeBusy( fb, 0, mNoHtmlMode );
      return !mResult.isEmpty();
    }

  private:
    bool mNoHtmlMode;
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

QString InvitationFormatterHelper::makeBtnLink( const QString &id, const QString &text, const QString &img )
{
    const QString iconPath = KGlobal::iconLoader()->iconPath( img, KIcon::Toolbar );
    QString res = QString( "<a class=\"button\" href=\"%1\"><img src=\"%2\"/>%3</a>  " ).
                  arg( generateLinkURL( id ), iconPath, text );
    return res;
}

static QString responseButtons( Incidence *inc, bool rsvpReq, bool rsvpRec,
                                InvitationFormatterHelper *helper,
                                Incidence *existingInc = NULL)
{
  QString html;
  if ( !helper ) {
    return html;
  }

  bool hideAccept = false,
       hideTentative = false,
       hideDecline = false;

  if ( existingInc ) {
    Attendee *ea = findMyAttendee( existingInc );
    if ( ea ) {
      // If this is an update of an already accepted incidence
      // do not show the buttons that confirm the status.
      hideAccept = ea->status() == Attendee::Accepted;
      hideDecline = ea->status() == Attendee::Declined;
      hideTentative = ea->status() == Attendee::Tentative;
    }
  }

  // Used suffix to indicate that the button will not trigger an immediate
  // response but open a dialog first.
  const QString complexActionSuffix = QString( "..." );

  if ( !rsvpReq && ( inc && inc->revision() == 0 ) ) {
    // Record only
    html += helper->makeBtnLink( "record", i18n( "Record" ), "dialog_ok" );

    // Move to trash
    html += helper->makeBtnLink( "delete", i18n( "Move to Trash" ), "edittrash" );

  } else {
    // Accept
    if ( !hideAccept ) {
      html += helper->makeBtnLink( "accept", i18n( "Accept" ), "dialog_ok_apply" );
    }

    // Tentative
    if ( !hideTentative ) {
      html += helper->makeBtnLink( "accept_conditionally",
                                  i18n( "Accept an invitation conditionally", "Provisorily" ),
                                  "dialog_ok" );
    }

    // Decline
    if ( !hideDecline ) {
      html += helper->makeBtnLink( "decline", i18n( "Decline" ), "process_stop" );
    }

    // Counter proposal
    html += helper->makeBtnLink( "counter", i18n( "Counter proposal" ) + complexActionSuffix,
                                 "edit_undo" );
  }

  if ( !rsvpRec || ( inc && inc->revision() > 0 ) ) {
    // Delegate
    html += helper->makeBtnLink( "delegate", i18n( "Delegate" ) + complexActionSuffix,
                                 "mail_forward" );
  }
  return html;
}

static QString counterButtons( Incidence *incidence,
                               InvitationFormatterHelper *helper )
{
  QString html;
  if ( !helper ) {
    return html;
  }

  // Accept proposal
  html += helper->makeBtnLink( "accept_counter", i18n("Accept"), "dialog_ok_apply" );

  // Decline proposal
  html += helper->makeBtnLink( "decline_counter", i18n("Decline"), "process_stop" );

  return html;
}

QString IncidenceFormatter::formatICalInvitationHelper( QString invitation,
                                                        Calendar *mCalendar,
                                                        InvitationFormatterHelper *helper,
                                                        bool noHtmlMode,
                                                        const QString &sender )
{
  if ( invitation.isEmpty() ) {
    return QString::null;
  }

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
    if ( !existingIncidence ) {
      kdDebug() << "formatICalInvitationHelper: Incidence is not in our calendar. uid: "
                << incBase->uid() << "; incidence count: "
                << helper->calendar()->incidences().count()
                << "; Calendar = " << helper->calendar()
                << endl;
    }

    if ( !CalHelper::isMyCalendarIncidence( helper->calendar(), existingIncidence ) ) {
      //sergio, 03/2011: This never happens I think. If the incidence is in a shared calendar
      //then it's uid will be a new one, and won't match incBase->uid().
      existingIncidence = 0;
    }
    if ( !existingIncidence ) {
      const Incidence::List list = helper->calendar()->incidences();
      for ( Incidence::List::ConstIterator it = list.begin(), end = list.end(); it != end; ++it ) {
        if ( (*it)->schedulingID() == incBase->uid() &&
             CalHelper::isMyCalendarIncidence( helper->calendar(), *it ) ) {
          existingIncidence = *it;
          break;
        }
      }
    }
  }

  Incidence *inc = dynamic_cast<Incidence*>( incBase ); // the incidence in the invitation email

  // Get some more information about this invitation

  // determine if I am the organizer for this invitation
  bool myInc = iamOrganizer( inc );

  // determine if the invitation response has already been recorded
  bool rsvpRec = false;
  Attendee *ea = 0;
  if ( !myInc ) {
    Incidence *rsvpIncidence = existingIncidence;
    if ( !rsvpIncidence && inc && inc->revision() > 0 ) {
      rsvpIncidence = inc;
    }
    if ( rsvpIncidence ) {
      ea = findMyAttendee( rsvpIncidence );
    }
    if ( ea &&
         ( ea->status() == Attendee::Accepted ||
           ea->status() == Attendee::Declined ||
           ea->status() == Attendee::Tentative ) ) {
      rsvpRec = true;
    }
  }

  // determine invitation role
  QString role;
  bool isDelegated = false;
  Attendee *a = findMyAttendee( inc );
  if ( !a && inc ) {
    if ( !inc->attendees().isEmpty() ) {
      a = inc->attendees().first();
    }
  }
  if ( a ) {
    isDelegated = ( a->status() == Attendee::Delegated );
    role = Attendee::roleName( a->role() );
  }

  // determine if RSVP needed, not-needed, or response already recorded
  bool rsvpReq = rsvpRequested( inc );

  // Now make the body
  QString html;

  html += "<div id=\"invitation\">\n";

  InvitationHeaderVisitor headerVisitor;
  headerVisitor.act( inc, existingIncidence, msg, sender );

  QString headerResult = headerVisitor.result();

  html += "<p>"; // Header and event info paragraph
  if ( !headerResult.isEmpty() ) {
    html += headerResult;
  }

  // Some more conditional status information
  QString eventInfo;
  if ( !myInc && a ) {
    if ( rsvpRec && inc ) {
      if ( inc->revision() == 0 ) {
        eventInfo += i18n( "The response <b>\"%1\"</b> has already been recorded." ).
                arg( ea->statusStr() );
      } else {
        eventInfo += i18n( "Your status for this invitation is <b>%1</b>." ).
                arg( ea->statusStr() );
      }
      rsvpReq = false;
    } else if ( msg->method() == Scheduler::Cancel ) {
      eventInfo += i18n( "This invitation was canceled." );
    } else if ( msg->method() == Scheduler::Add ) {
      eventInfo += i18n( "This invitation was accepted." );
    } else if ( msg->method() == Scheduler::Declinecounter ) {
      rsvpReq = true;
      // eventInfo += rsvpRequestedStr( rsvpReq, role );
    } else {
      if ( isDelegated ) {
        eventInfo += i18n( "Awaiting delegation response." );
      } else {
      //  eventInfo += rsvpRequestedStr( rsvpReq, role );
      }
    }
  }
  if ( !eventInfo.isEmpty() ) {
    html += QString( "<br/><i>%1</i>" ).arg( eventInfo );
  }

  // Print if the organizer gave you a preset status
  if ( !myInc ) {
    if ( inc && inc->revision() == 0 ) {
      QString statStr = myStatusStr( inc );
      if ( !statStr.isEmpty() ) {
        html += "<br/>";
        html += "<i>";
        html += statStr;
        html += "</i>";
      }
    }
  }
  html += "</p>";

  // Add the groupware links if necessary
  html += formatGroupwareLinks(helper,
                               existingIncidence,
                               inc,
                               msg,
                               rsvpRec,
                               a);

  html += invitationCommentsIncidence( inc, noHtmlMode );

  html += "\n<hr>\n<table>";

  InvitationBodyVisitor bodyVisitor( noHtmlMode );
  bool bodyOk;
  if ( msg->method() == Scheduler::Request || msg->method() == Scheduler::Reply ||
       msg->method() == Scheduler::Declinecounter ) {
    if ( inc && existingIncidence &&
         inc->lastModified() < existingIncidence->lastModified() ) {
      bodyOk = bodyVisitor.act( existingIncidence, inc, msg, sender );
    } else {
      bodyOk = bodyVisitor.act( inc, existingIncidence, msg, sender );
    }
  } else {
    bodyOk = bodyVisitor.act( inc, 0, msg, sender );
  }
  if ( bodyOk ) {
    html += bodyVisitor.result();
  } else {
    return QString::null;
  }

  // Add the attendee list
  if ( myInc ) {
    html += invitationRsvpList( existingIncidence, a );
  } else {
    html += invitationAttendeeList( inc );
  }

  // Add the attachment list
  html += invitationAttachments( helper, inc );

  html += "\n</table>";

  html += "\n<hr/>\n";

  // Add events on the same day
  html += displayViewFormatEventsOnSameDays ( helper, dynamic_cast<Event*>( inc ),
                                              noHtmlMode );

  html += "</div>";

  return html;
}

QString IncidenceFormatter::formatGroupwareLinks(InvitationFormatterHelper *helper,
                                                        Incidence *existingIncidence,
                                                        Incidence *inc,
                                                        ScheduleMessage *msg,
                                                        bool rsvpRec,
                                                        Attendee *a)
{
  QString html;
  // Add groupware links
  bool myInc = iamOrganizer( inc );
  bool rsvpReq = rsvpRequested( inc );

  switch ( msg->method() ) {
    case Scheduler::Publish:
    case Scheduler::Request:
    case Scheduler::Refresh:
    case Scheduler::Add:
    {
      if ( inc && inc->revision() > 0 && ( existingIncidence || !helper->calendar() ) ) {
        if ( inc->type() == "Todo" ) {
          html += helper->makeBtnLink( "reply", i18n( "Record in my task list" ), "dialog_ok" );
        } else {
          html += helper->makeBtnLink( "reply", i18n( "Record in my calendar" ), "dialog_ok" );
        }
      }

      if ( !myInc && a ) {
        if ( !html.isEmpty() ) {
            // Break after the record in my.. button
            html += "<br/>";
        }
        html += responseButtons( inc, rsvpReq, rsvpRec, helper, existingIncidence );
      }
      break;
    }

    case Scheduler::Cancel:
      // Remove invitation
      if ( inc ) {
        if ( inc->type() == "Todo" ) {
          html += helper->makeBtnLink( "cancel", i18n( "Remove invitation from my task list" ), "dialog_ok" );
        } else {
          html += helper->makeBtnLink( "cancel", i18n( "Remove invitation from my calendar" ), "dialog_ok" );
        }
      }
      break;

    case Scheduler::Reply:
    {
      // Record invitation response
      Attendee *a = 0;
      Attendee *ea = 0;
      if ( inc ) {
        // First, determine if this reply is really a counter in disguise.
        if ( replyMeansCounter( inc ) ) {
          html += counterButtons( inc, helper );
          break;
        }

        // Next, maybe this is a declined reply that was delegated from me?
        // find first attendee who is delegated-from me
        // look a their PARTSTAT response, if the response is declined,
        // then we need to start over which means putting all the action
        // buttons and NOT putting on the [Record response..] button
        a = findDelegatedFromMyAttendee( inc );
        if ( a ) {
          if ( a->status() != Attendee::Accepted ||
               a->status() != Attendee::Tentative ) {
            html += responseButtons( inc, rsvpReq, rsvpRec, helper );
            break;
          }
        }

        // Finally, simply allow a Record of the reply
        if ( !inc->attendees().isEmpty() ) {
          a = inc->attendees().first();
        }
        if ( a ) {
          ea = findAttendee( existingIncidence, a->email() );
        }
      }
      if ( ea && ( ea->status() != Attendee::NeedsAction ) && ( ea->status() == a->status() ) ) {
        // we have seen this invitation and recorded a response
        if ( inc->revision() > 0 &&
             ( inc->lastModified() > existingIncidence->lastModified() ) ) {
          // newer than we have recorded, so an update
          if ( inc->type() == "Todo" ) {
            html += helper->makeBtnLink( "reply", i18n( "Record update in my task list" ), "dialog_ok" );
          } else {
            html += helper->makeBtnLink( "reply", i18n( "Record update in my calendar" ), "dialog_ok" );
          }
        } else {
          // not newer than we have recorded
          html += "<br><i>";
          if ( inc->revision() > 0 ) {
            // an update we already have recorded
            html += i18n( "This update has been recorded." );
          } else {
            // not an update
            html += i18n( "This <b>%1</b> response has been recorded." ).arg( ea->statusStr() );
          }
          html += "</i>";
        }
      } else {
        // Not seen or recorded with a response yet
        if ( inc ) {
          if ( inc->type() == "Todo" ) {
            html += helper->makeBtnLink( "reply", i18n( "Record response in my task list" ), "dialog_ok" );
          } else {
            html += helper->makeBtnLink( "reply", i18n( "Record response in my calendar" ), "dialog_ok" );
          }
        }
      }
      break;
    }

    case Scheduler::Counter:
      // Counter proposal
      html += counterButtons( inc, helper );
      break;

    case Scheduler::Declinecounter:
      html += responseButtons( inc, rsvpReq, rsvpRec, helper );
      break;

    case Scheduler::NoMethod:
      break;
  }
  return html;
}
QString IncidenceFormatter::formatICalInvitation( QString invitation,
                                                  Calendar *mCalendar,
                                                  InvitationFormatterHelper *helper )
{
  return formatICalInvitationHelper( invitation, mCalendar, helper, false, QString() );
}

QString IncidenceFormatter::formatICalInvitationNoHtml( QString invitation,
                                                        Calendar *mCalendar,
                                                        InvitationFormatterHelper *helper )
{
  return formatICalInvitationHelper( invitation, mCalendar, helper, true, QString() );
}

QString IncidenceFormatter::formatICalInvitationNoHtml( QString invitation,
                                                        Calendar *mCalendar,
                                                        InvitationFormatterHelper *helper,
                                                        const QString &sender )
{
  return formatICalInvitationHelper( invitation, mCalendar, helper, true, sender );
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
        QDateTime dt( date, QTime( 0, 0, 0 ) );
        dt = dt.addSecs( -1 );
        dueDt.setDate( todo->recurrence()->getNextDateTime( dt ).date() );
      }
    }
    ret += "<br>" +
           i18n("<i>Due:</i>&nbsp;%1").
           arg( IncidenceFormatter::dateTimeToString( dueDt, floats, false ).
                replace( " ", "&nbsp;" ) );
  }

  // Print priority and completed info here, for lack of a better place

  if ( todo->priority() > 0 ) {
    ret += "<br>";
    ret += "<i>" + i18n( "Priority:" ) + "</i>" + "&nbsp;";
    ret += QString::number( todo->priority() );
  }

  ret += "<br>";
  if ( todo->isCompleted() ) {
    ret += "<i>" + i18n( "Completed:" ) + "</i>" + "&nbsp;";
    ret += todo->completedStr().replace( " ", "&nbsp;" );
  } else {
    ret += "<i>" + i18n( "Percent Done:" ) + "</i>" + "&nbsp;";
    ret += i18n( "%1%" ).arg( todo->percentComplete() );
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

static QString tooltipPerson( const QString &email, const QString &name, Attendee::PartStat status )
{
  // Search for a new print name, if needed.
  const QString printName = searchName( email, name );

  // Get the icon corresponding to the attendee participation status.
  const QString iconPath = rsvpStatusIconPath( status );

  // Make the return string.
  QString personString;
  if ( !iconPath.isEmpty() ) {
    personString += "<img valign=\"top\" src=\"" + iconPath + "\">" + "&nbsp;";
  }
  if ( status != Attendee::None ) {
    personString += i18n( "attendee name (attendee status)", "%1 (%2)" ).
                    arg( printName.isEmpty() ? email : printName ).
                    arg( Attendee::statusName( status ) );
  } else {
    personString += i18n( "%1" ).arg( printName.isEmpty() ? email : printName );
  }
  return personString;
}

static QString tooltipFormatOrganizer( const QString &email, const QString &name )
{
  // Search for a new print name, if needed
  const QString printName = searchName( email, name );

  // Get the icon for organizer
  const QString iconPath = KGlobal::iconLoader()->iconPath( "organizer", KIcon::Small );

  // Make the return string.
  QString personString;
  personString += "<img valign=\"top\" src=\"" + iconPath + "\">" + "&nbsp;";
  personString += ( printName.isEmpty() ? email : printName );
  return personString;
}

static QString tooltipFormatAttendeeRoleList( Incidence *incidence, Attendee::Role role,
                                              bool showStatus )
{
  const int maxNumAtts = 8; // maximum number of people to print per attendee role
  const QString etc = i18n( "elipsis", "..." );

  int i = 0;
  QString tmpStr;
  Attendee::List::ConstIterator it;
  Attendee::List attendees = incidence->attendees();

  for ( it = attendees.begin(); it != attendees.end(); ++it ) {
    Attendee *a = *it;
    if ( a->role() != role ) {
      // skip not this role
      continue;
    }
    if ( attendeeIsOrganizer( incidence, a ) ) {
      // skip attendee that is also the organizer
      continue;
    }
    if ( i == maxNumAtts ) {
      tmpStr += "&nbsp;&nbsp;" + etc;
      break;
    }
    tmpStr += "&nbsp;&nbsp;" + tooltipPerson( a->email(), a->name(),
                                              showStatus ? a->status() : Attendee::None );
    if ( !a->delegator().isEmpty() ) {
      tmpStr += i18n(" (delegated by %1)" ).arg( a->delegator() );
    }
    if ( !a->delegate().isEmpty() ) {
      tmpStr += i18n(" (delegated to %1)" ).arg( a->delegate() );
    }
    tmpStr += "<br>";
    i++;
  }

  if ( tmpStr.endsWith( "<br>" ) ) {
    tmpStr.truncate( tmpStr.length() - 4 );
  }
  return tmpStr;
}

static QString tooltipFormatAttendees( Calendar *calendar, Incidence *incidence )
{
  QString tmpStr, str;

  // Add organizer link
  int attendeeCount = incidence->attendees().count();
  if ( attendeeCount > 1 ||
       ( attendeeCount == 1 &&
         !attendeeIsOrganizer( incidence, incidence->attendees().first() ) ) ) {
    tmpStr += "<i>" + i18n( "Organizer:" ) + "</i>" + "<br>";
    tmpStr += "&nbsp;&nbsp;" + tooltipFormatOrganizer( incidence->organizer().email(),
                                                       incidence->organizer().name() );
  }

  // Show the attendee status if the incidence's organizer owns the resource calendar,
  // which means they are running the show and have all the up-to-date response info.
  bool showStatus = CalHelper::incOrganizerOwnsCalendar( calendar, incidence );

  // Add "chair"
  str = tooltipFormatAttendeeRoleList( incidence, Attendee::Chair, showStatus );
  if ( !str.isEmpty() ) {
    tmpStr += "<br><i>" + i18n( "Chair:" ) + "</i>" + "<br>";
    tmpStr += str;
  }

  // Add required participants
  str = tooltipFormatAttendeeRoleList( incidence, Attendee::ReqParticipant, showStatus );
  if ( !str.isEmpty() ) {
    tmpStr += "<br><i>" + i18n( "Required Participants:" ) + "</i>" + "<br>";
    tmpStr += str;
  }

  // Add optional participants
  str = tooltipFormatAttendeeRoleList( incidence, Attendee::OptParticipant, showStatus );
  if ( !str.isEmpty() ) {
    tmpStr += "<br><i>" + i18n( "Optional Participants:" ) + "</i>" + "<br>";
    tmpStr += str;
  }

  // Add observers
  str = tooltipFormatAttendeeRoleList( incidence, Attendee::NonParticipant, showStatus );
  if ( !str.isEmpty() ) {
    tmpStr += "<br><i>" + i18n( "Observers:" ) + "</i>" + "<br>";
    tmpStr += str;
  }

  return tmpStr;
}

QString IncidenceFormatter::ToolTipVisitor::generateToolTip( Incidence *incidence, QString dtRangeText )
{
  const QString etc = i18n( "elipsis", "..." );
  const uint maxDescLen = 120; // maximum description chars to print (before elipsis)

  if ( !incidence ) {
    return QString::null;
  }

  QString tmp = "<qt>";

  // header

#ifdef KORG_DEBUG_SCHEDULING_IDS
  tmp += "<b>Uid: " + incidence->uid() + "<br>schedulingID: " + incidence->schedulingID() + "</b><br>";
#endif

  tmp += "<b>" + incidence->summary().replace( "\n", "<br>" ) + "</b>";
  //NOTE: using <hr> seems to confuse Qt3 tooltips in some cases so use "-----"
  tmp += "<br>----------<br>";

  if ( mCalendar ) {
    QString calStr = IncidenceFormatter::resourceString( mCalendar, incidence );
    if ( !calStr.isEmpty() ) {
      tmp += "<i>" + i18n( "Calendar:" ) + "</i>" + "&nbsp;";
      tmp += calStr;
    }
  }

  tmp += dtRangeText;

  if ( !incidence->location().isEmpty() ) {
    tmp += "<br>";
    tmp += "<i>" + i18n( "Location:" ) + "</i>" + "&nbsp;";
    tmp += incidence->location().replace( "\n", "<br>" );
  }

  QString durStr = IncidenceFormatter::durationString( incidence );
  if ( !durStr.isEmpty() ) {
    tmp += "<br>";
    tmp += "<i>" + i18n( "Duration:" ) + "</i>" + "&nbsp;";
    tmp += durStr;
  }

  if ( incidence->doesRecur() ) {
    tmp += "<br>";
    tmp += "<i>" + i18n( "Recurrence:" ) + "</i>" + "&nbsp;";
    tmp += IncidenceFormatter::recurrenceString( incidence );
  }

  if ( !incidence->description().isEmpty() ) {
    QString desc( incidence->description() );
    if ( desc.length() > maxDescLen ) {
      desc = desc.left( maxDescLen ) + etc;
    }
    tmp += "<br>----------<br>";
    tmp += "<i>" + i18n( "Description:" ) + "</i>" + "<br>";
    tmp += desc.replace( "\n", "<br>" );
    tmp += "<br>----------";
  }

  int reminderCount = incidence->alarms().count();
  if ( reminderCount > 0 && incidence->isAlarmEnabled() ) {
    tmp += "<br>";
    tmp += "<i>" + i18n( "Reminder:", "%n Reminders:", reminderCount ) + "</i>" + "&nbsp;";
    tmp += IncidenceFormatter::reminderStringList( incidence ).join( ", " );
  }

  tmp += "<br>";
  tmp += tooltipFormatAttendees( mCalendar, incidence );

  int categoryCount = incidence->categories().count();
  if ( categoryCount > 0 ) {
    tmp += "<br>";
    tmp += "<i>" + i18n( "Category:", "%n Categories:", categoryCount ) + "</i>" + "&nbsp;";
    tmp += incidence->categories().join( ", " );
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

static QString recurEnd( Incidence *incidence )
{
  QString endstr;
  if ( incidence->doesFloat() ) {
    endstr = KGlobal::locale()->formatDate( incidence->recurrence()->endDate() );
  } else {
    endstr = KGlobal::locale()->formatDateTime( incidence->recurrence()->endDateTime() );
  }
  return endstr;
}

/************************************
 *  More static formatting functions
 ************************************/
QStringList IncidenceFormatter::exceptionStrings( Recurrence * recur )
{
  if ( !recur ) {
    return QStringList();
  }
  const KCalendarSystem *calSys = KGlobal::locale()->calendar();
  DateTimeList exceptions = recur->exDateTimes();
  DateTimeList::ConstIterator il;
  QStringList exStr;
  for ( il = exceptions.constBegin(); il != exceptions.constEnd(); ++il ) {
    switch ( recur->recurrenceType() ) {
    case Recurrence::rMinutely:
      exStr << i18n( "minute %1" ).arg( (*il).time().minute() );
      break;
    case Recurrence::rHourly:
      exStr << KGlobal::locale()->formatTime( (*il).time() );
      break;
    case Recurrence::rWeekly:
      exStr << calSys->weekDayName( (*il).date(), false );
      break;
    case Recurrence::rDaily:
    case Recurrence::rYearlyDay:
    case Recurrence::rYearlyPos:
    case Recurrence::rMonthlyPos:
    case Recurrence::rMonthlyDay:
      exStr << KGlobal::locale()->formatDate( (*il).date(), false );
      break;
    case Recurrence::rYearlyMonth:
      exStr << calSys->monthName( (*il).date(), false );
      break;
    }
  }

  DateList d = recur->exDates();
  DateList::ConstIterator dl;
  for ( dl = d.constBegin(); dl != d.constEnd(); ++dl ) {
    if ( recur->recurrenceType() == Recurrence::rYearlyMonth ) {
      exStr << calSys->monthName( (*dl), false );
    } else {
      exStr << KGlobal::locale()->formatDate( (*dl), true );
    }
  }

  return exStr;
}

QString IncidenceFormatter::recurrenceString( Incidence *incidence, bool includeExceptions )
{
  if ( !incidence->doesRecur() ) {
    return i18n( "No recurrence" );
  }
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
  dayList.append( i18n( "6th" ) );
  dayList.append( i18n( "7th" ) );
  dayList.append( i18n( "8th" ) );
  dayList.append( i18n( "9th" ) );
  dayList.append( i18n( "10th" ) );
  dayList.append( i18n( "11th" ) );
  dayList.append( i18n( "12th" ) );
  dayList.append( i18n( "13th" ) );
  dayList.append( i18n( "14th" ) );
  dayList.append( i18n( "15th" ) );
  dayList.append( i18n( "16th" ) );
  dayList.append( i18n( "17th" ) );
  dayList.append( i18n( "18th" ) );
  dayList.append( i18n( "19th" ) );
  dayList.append( i18n( "20th" ) );
  dayList.append( i18n( "21st" ) );
  dayList.append( i18n( "22nd" ) );
  dayList.append( i18n( "23rd" ) );
  dayList.append( i18n( "24th" ) );
  dayList.append( i18n( "25th" ) );
  dayList.append( i18n( "26th" ) );
  dayList.append( i18n( "27th" ) );
  dayList.append( i18n( "28th" ) );
  dayList.append( i18n( "29th" ) );
  dayList.append( i18n( "30th" ) );
  dayList.append( i18n( "31st" ) );

  int weekStart = KGlobal::locale()->weekStartDay();
  QString dayNames;

  const KCalendarSystem *calSys = KGlobal::locale()->calendar();

  Recurrence *recur = incidence->recurrence();

  QString recurStr;
  switch ( recur->recurrenceType() ) {
  case Recurrence::rNone:
    return i18n( "No recurrence" );

  case Recurrence::rMinutely:
    recurStr = i18n( "Recurs every minute", "Recurs every %n minutes", recur->frequency() );
    if ( recur->duration() != -1 ) {
      recurStr = i18n( "%1 until %2" ).arg( recurStr ).arg( recurEnd( incidence ) );
      if ( recur->duration() >  0 ) {
        recurStr += i18n( " (%1 occurrences)" ).arg( recur->duration() );
      }
    }
    break;

  case Recurrence::rHourly:
    recurStr = i18n( "Recurs hourly", "Recurs every %n hours", recur->frequency() );
    if ( recur->duration() != -1 ) {
      recurStr = i18n( "%1 until %2" ).arg( recurStr ).arg( recurEnd( incidence ) );
      if ( recur->duration() >  0 ) {
        recurStr += i18n( " (%1 occurrences)" ).arg( recur->duration() );
      }
    }
    break;

  case Recurrence::rDaily:
    recurStr = i18n( "Recurs daily", "Recurs every %n days", recur->frequency() );
    if ( recur->duration() != -1 ) {
      recurStr = i18n( "%1 until %2" ).arg( recurStr ).arg( recurEnd( incidence ) );
      if ( recur->duration() >  0 ) {
        recurStr += i18n( " (%1 occurrences)" ).arg( recur->duration() );
      }
    }
    break;

  case Recurrence::rWeekly:
  {
    recurStr = i18n( "Recurs weekly", "Recurs every %n weeks", recur->frequency() );

    bool addSpace = false;
    for ( int i = 0; i < 7; ++i ) {
      if ( recur->days().testBit( ( i + weekStart + 6 ) % 7 ) ) {
        if ( addSpace ) {
          dayNames.append( i18n( "separator for list of days", ", " ) );
        }
        dayNames.append( calSys->weekDayName( ( ( i + weekStart + 6 ) % 7 ) + 1, false ) );
        addSpace = true;
      }
    }
    if ( dayNames.isEmpty() ) {
      dayNames = i18n( "Recurs weekly on no days", "no days" );
    }
    if ( recur->duration() != -1 ) {
      recurStr = i18n( "%1 on %2 until %3" ).
                 arg( recurStr ).arg( dayNames ).arg( recurEnd( incidence ) );
      if ( recur->duration() >  0 ) {
        recurStr += i18n( " (%1 occurrences)" ).arg( recur->duration() );
      }
    } else {
      recurStr = i18n( "%1 on %2" ).arg( recurStr ).arg( dayNames );
    }
    break;
  }

  case Recurrence::rMonthlyPos:
  {
    recurStr = i18n( "Recurs monthly", "Recurs every %n months", recur->frequency() );

    if ( !recur->monthPositions().isEmpty() ) {
      KCal::RecurrenceRule::WDayPos rule = recur->monthPositions()[0];
      if ( recur->duration() != -1 ) {
        recurStr = i18n( "%1 on the %2 %3 until %4" ).
                   arg( recurStr ).
                   arg( dayList[rule.pos() + 31] ).
                   arg( calSys->weekDayName( rule.day(), false ) ).
                   arg( recurEnd( incidence ) );
        if ( recur->duration() >  0 ) {
          recurStr += i18n( " (%1 occurrences)" ).arg( recur->duration() );
        }
      } else {
        recurStr = i18n( "%1 on the %2 %3" ).
                   arg( recurStr ).
                   arg( dayList[rule.pos() + 31] ).
                   arg( calSys->weekDayName( rule.day(), false ) );
      }
    }
    break;
  }

  case Recurrence::rMonthlyDay:
  {
    recurStr = i18n( "Recurs monthly", "Recurs every %n months", recur->frequency() );

    if ( !recur->monthDays().isEmpty() ) {
      int days = recur->monthDays()[0];
      if ( recur->duration() != -1 ) {
        recurStr = i18n( "%1 on the %2 day until %3" ).
                   arg( recurStr ).
                   arg( dayList[days + 31] ).
                   arg( recurEnd( incidence ) );
        if ( recur->duration() >  0 ) {
          recurStr += i18n( " (%1 occurrences)" ).arg( recur->duration() );
        }
      } else {
        recurStr = i18n( "%1 on the %2 day" ).arg( recurStr ).arg( dayList[days + 31] );
      }
    }
    break;
  }
  case Recurrence::rYearlyMonth:
  {
    recurStr = i18n( "Recurs yearly", "Recurs every %n years", recur->frequency() );

    if ( recur->duration() != -1 ) {
      if ( !recur->yearDates().isEmpty() ) {
        recurStr = i18n( "%1 on %2 %3 until %4" ).
                   arg( recurStr ).
                   arg( calSys->monthName( recur->yearMonths()[0], recur->startDate().year() ) ).
                   arg( dayList[ recur->yearDates()[0] + 31 ] ).
                   arg( recurEnd( incidence ) );
        if ( recur->duration() >  0 ) {
          recurStr += i18n( " (%1 occurrences)" ).arg( recur->duration() );
        }
      }
    } else {
      if ( !recur->yearDates().isEmpty() ) {
        recurStr = i18n( "%1 on %2 %3" ).
                   arg( recurStr ).
                   arg( calSys->monthName( recur->yearMonths()[0], recur->startDate().year() ) ).
                   arg( dayList[ recur->yearDates()[0] + 31 ] );
      } else {
        if ( !recur->yearMonths().isEmpty() ) {
          recurStr = i18n( "Recurs yearly on %1 %2" ).
                     arg( calSys->monthName( recur->yearMonths()[0],
                                             recur->startDate().year() ) ).
                     arg( dayList[ recur->startDate().day() + 31 ] );
        } else {
          recurStr = i18n( "Recurs yearly on %1 %2" ).
                     arg( calSys->monthName( recur->startDate().month(),
                                             recur->startDate().year() ) ).
                     arg( dayList[ recur->startDate().day() + 31 ] );
        }
      }
    }
    break;
  }
  case Recurrence::rYearlyDay:
  {
    recurStr = i18n( "Recurs yearly", "Recurs every %n years", recur->frequency() );
    if ( !recur->yearDays().isEmpty() ) {
      if ( recur->duration() != -1 ) {
        recurStr = i18n( "%1 on day %2 until %3" ).
                   arg( recurStr ).
                   arg( recur->yearDays()[0] ).
                   arg( recurEnd( incidence ) );
        if ( recur->duration() >  0 ) {
          recurStr += i18n( " (%1 occurrences)" ).arg( recur->duration() );
        }
      } else {
        recurStr = i18n( "%1 on day %2" ).arg( recurStr ).arg( recur->yearDays()[0] );
      }
    }
    break;
  }
  case Recurrence::rYearlyPos:
  {
    recurStr = i18n( "Every year", "Every %n years", recur->frequency() );
    if ( !recur->yearPositions().isEmpty() && !recur->yearMonths().isEmpty() ) {
      KCal::RecurrenceRule::WDayPos rule = recur->yearPositions()[0];
      if ( recur->duration() != -1 ) {
        recurStr = i18n( "%1 on the %2 %3 of %4 until %5" ).
                   arg( recurStr ).
                   arg( dayList[rule.pos() + 31] ).
                   arg( calSys->weekDayName( rule.day(), false ) ).
                   arg( calSys->monthName( recur->yearMonths()[0], recur->startDate().year() ) ).
                   arg( recurEnd( incidence ) );
        if ( recur->duration() >  0 ) {
          recurStr += i18n( " (%1 occurrences)" ).arg( recur->duration() );
        }
      } else {
        recurStr = i18n( "%1 on the %2 %3 of %4" ).
                   arg( recurStr ).
                   arg( dayList[rule.pos() + 31] ).
                   arg( calSys->weekDayName( rule.day(), false ) ).
                   arg( calSys->monthName( recur->yearMonths()[0], recur->startDate().year() ) );
      }
    }
   break;
  }
  }

  if ( recurStr.isEmpty() ) {
    recurStr = i18n( "Incidence recurs" );
  }
  if ( includeExceptions ) {
    const QStringList exStrs = IncidenceFormatter::exceptionStrings( recur );
    if ( !exStrs.isEmpty() ) {
      recurStr = i18n( "%1 (excluding %2)" ).arg( recurStr, exStrs.join( "," ) );
    }
  }

  return recurStr;
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
  if ( !date.isValid() ) {
    return QString::null;
  }
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

static QString secs2Duration( int secs )
{
  QString tmp;
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
  }
  return tmp;
}

QString IncidenceFormatter::durationString( Incidence *incidence )
{
  QString tmp;
  if ( incidence->type() == "Event" ) {
    Event *event = static_cast<Event *>( incidence );
    if ( event->hasEndDate() ) {
      if ( !event->doesFloat() ) {
        tmp = secs2Duration( event->dtStart().secsTo( event->dtEnd() ) );
      } else {
        tmp = i18n( "1 day", "%n days",
                    event->dtStart().date().daysTo( event->dtEnd().date() ) + 1 );
      }
    } else {
      tmp = i18n( "forever" );
    }
  } else if ( incidence->type() == "Todo" ) {
    Todo *todo = static_cast<Todo *>( incidence );
    if ( todo->hasDueDate() ) {
      if ( todo->hasStartDate() ) {
        if ( !todo->doesFloat() ) {
          tmp = secs2Duration( todo->dtStart().secsTo( todo->dtDue() ) );
        } else {
          tmp = i18n( "1 day", "%n days",
                      todo->dtStart().date().daysTo( todo->dtDue().date() ) + 1 );
        }
      }
    }
  }
  return tmp;
}

QStringList IncidenceFormatter::reminderStringList( Incidence *incidence, bool shortfmt )
{
  //TODO: implement shortfmt=false
  Q_UNUSED( shortfmt );

  QStringList reminderStringList;

  if ( incidence ) {
    Alarm::List alarms = incidence->alarms();
    Alarm::List::ConstIterator it;
    for ( it = alarms.begin(); it != alarms.end(); ++it ) {
      Alarm *alarm = *it;
      int offset = 0;
      QString remStr, atStr, offsetStr;
      if ( alarm->hasTime() ) {
        offset = 0;
        if ( alarm->time().isValid() ) {
          atStr = KGlobal::locale()->formatDateTime( alarm->time() );
        }
      } else if ( alarm->hasStartOffset() ) {
        offset = alarm->startOffset().asSeconds();
        if ( offset < 0 ) {
          offset = -offset;
          offsetStr = i18n( "N days/hours/minutes before the start datetime",
                            "%1 before the start" );
        } else if ( offset > 0 ) {
          offsetStr = i18n( "N days/hours/minutes after the start datetime",
                            "%1 after the start" );
        } else { //offset is 0
          if ( incidence->dtStart().isValid() ) {
            atStr = KGlobal::locale()->formatDateTime( incidence->dtStart() );
          }
        }
      } else if ( alarm->hasEndOffset() ) {
        offset = alarm->endOffset().asSeconds();
        if ( offset < 0 ) {
          offset = -offset;
          if ( incidence->type() == "Todo" ) {
            offsetStr = i18n( "N days/hours/minutes before the due datetime",
                              "%1 before the to-do is due" );
          } else {
            offsetStr = i18n( "N days/hours/minutes before the end datetime",
                              "%1 before the end" );
          }
        } else if ( offset > 0 ) {
          if ( incidence->type() == "Todo" ) {
            offsetStr = i18n( "N days/hours/minutes after the due datetime",
                              "%1 after the to-do is due" );
          } else {
            offsetStr = i18n( "N days/hours/minutes after the end datetime",
                              "%1 after the end" );
          }
        } else { //offset is 0
          if ( incidence->type() == "Todo" ) {
            Todo *t = static_cast<Todo *>( incidence );
            if ( t->dtDue().isValid() ) {
              atStr = KGlobal::locale()->formatDateTime( t->dtDue() );
            }
          } else {
            Event *e = static_cast<Event *>( incidence );
            if ( e->dtEnd().isValid() ) {
              atStr = KGlobal::locale()->formatDateTime( e->dtEnd() );
            }
          }
        }
      }
      if ( offset == 0 ) {
        if ( !atStr.isEmpty() ) {
          remStr = i18n( "reminder occurs at datetime", "at %1" ).arg( atStr );
        }
      } else {
        remStr = offsetStr.arg( secs2Duration( offset ) );
      }

      if ( alarm->repeatCount() > 0 ) {
        QString countStr = i18n( "repeats once", "repeats %n times", alarm->repeatCount() );
        QString intervalStr = i18n( "interval is N days/hours/minutes", "interval is %1" ).
                              arg( secs2Duration( alarm->snoozeTime().asSeconds() ) );
        QString repeatStr = i18n( "(repeat string, interval string)", "(%1, %2)" ).
                            arg( countStr, intervalStr );
        remStr = remStr + ' ' + repeatStr;

      }
      reminderStringList << remStr;
    }
  }

  return reminderStringList;
}