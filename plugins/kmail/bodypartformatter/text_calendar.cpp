/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

#include <interfaces/bodypartformatter.h>
#include <interfaces/bodypart.h>
#include <interfaces/bodyparturlhandler.h>
#include <khtmlparthtmlwriter.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/icalformat.h>
#include <libkcal/attendee.h>
#include <libkcal/incidence.h>

#include <kpimprefs.h> // for the timezone

#include <kmail/callback.h>
#include <kmail/kmmessage.h>
#include <kmail/kmcommands.h>
#include <libkdepim/email.h>

#include <kglobal.h>
#include <klocale.h>
#include <kstringhandler.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <ktempfile.h>

#include <qurl.h>
#include <qdir.h>
#include <qtextstream.h>

using namespace KCal;

namespace {

static void vPartMicroParser( const QString& str, QString& s )
{
  QString line;
  uint len = str.length();

  for( uint i=0; i<len; ++i) {
    if( str[i] == '\r' || str[i] == '\n' ) {
      if( str[i] == '\r' )
        ++i;
      if( i+1 < len && str[i+1] == ' ' ) {
        // Found a continuation line, skip it's leading blanc
        ++i;
      } else {
        // Found a logical line end, process the line
        if( line.startsWith( s ) ) {
          s = line.mid( s.length() + 1 );
          return;
        }
        line = "";
      }
    } else {
      line += str[i];
    }
  }
  s.truncate(0);
}

static void string2HTML( QString& str )
{
  str.replace( QChar( '&' ), "&amp;" );
  str.replace( QChar( '<' ), "&lt;" );
  str.replace( QChar( '>' ), "&gt;" );
  str.replace( QChar( '\"' ), "&quot;" );
  str.replace( "\\n", "<br>" );
  str.replace( "\\,", "," );
}

static QString meetingDetails( Incidence* incidence, Event* event )
{
  // Meeting details are formatted into an HTML table

  QString html;

  QString sSummary = i18n( "Summary unspecified" );
  if ( incidence ) {
    if ( ! incidence->summary().isEmpty() ) {
      sSummary = incidence->summary();
      string2HTML( sSummary );
    }
  }

  QString sLocation = i18n( "Location unspecified" );
  if ( incidence ) {
    if ( ! incidence->location().isEmpty() ) {
      sLocation = incidence->location();
      string2HTML( sLocation );
    }
  }

  QString dir = ( QApplication::reverseLayout() ? "rtl" : "ltr" );
  html = QString("<div dir=\"%1\">\n").arg(dir);

  html += "<table border=\"0\" cellpadding=\"1\" cellspacing=\"1\">\n";

  // Meeting Summary Row
  html += "<tr>";
  html += "<td>" + i18n( "What:" ) + "</td>";
  html += "<td>" + sSummary + "</td>";
  html += "</tr>\n";

  // Meeting Location Row
  html += "<tr>";
  html += "<td>" + i18n( "Where:" ) + "</td>";
  html += "<td>" + sLocation + "</td>";
  html += "</tr>\n";

  // Meeting Start Time Row
  html += "<tr>";
  html += "<td>" + i18n( "Start Time:" ) + "</td>";
  html += "<td>";
  if ( ! event->doesFloat() ) {
    html +=  i18n("%1: Start Date, %2: Start Time", "%1 %2")
             .arg( event->dtStartDateStr(), event->dtStartTimeStr() );
  } else {
    html += i18n("%1: Start Date", "%1 (time unspecified)")
            .arg( event->dtStartDateStr() );
  }
  html += "</td>";
  html += "</tr>\n";

  // Meeting End Time Row
  html += "<tr>";
  html += "<td>" + i18n( "End Time:" ) + "</td>";
  html += "<td>";
  if ( event->hasEndDate() ) {
    if ( ! event->doesFloat() ) {
      html +=  i18n("%1: End Date, %2: End Time", "%1 %2")
               .arg( event->dtEndDateStr(), event->dtEndTimeStr() );
    } else {
      html += i18n("%1: End Date", "%1 (time unspecified)")
              .arg( event->dtEndDateStr() );
    }
  } else {
    html += i18n( "Unspecified" );
  }
  html += "</td>";
  html += "</tr>\n";

  // Meeting Duration Row
  if ( !event->doesFloat() && event->hasEndDate() ) {
    html += "<tr>";
    QTime sDuration, t;
    int secs = event->dtStart().secsTo( event->dtEnd() );
    t = sDuration.addSecs( secs );
    html += "<td>" + i18n( "Duration:" ) + "</td>";
    html += "<td>";
    if ( t.hour() > 0 ) {
      html += i18n( "1 hour ", "%n hours ", t.hour() );
    }
    if ( t.minute() > 0 ) {
      html += i18n( "1 minute ", "%n minutes ",  t.minute() );
    }
    html += "</td>";
    html += "</tr>\n";
  }

  html += "</table>\n";
  html += "</div>\n";

  return html;
}

static QString taskDetails( Incidence* incidence )
{
  // Task details are formatted into an HTML table

  QString html;

  QString sSummary = i18n( "Summary unspecified" );
  QString sDescr = i18n( "Description unspecified" );
  if ( incidence ) {
    if ( ! incidence->summary().isEmpty() ) {
      sSummary = incidence->summary();
    }
    if ( ! incidence->description().isEmpty() ) {
      sDescr = incidence->description();
    }
  }
  html = "<table border=\"0\" cellpadding=\"1\" cellspacing=\"1\">\n";

  // Task Summary Row
  html += "<tr>";
  html += "<td>" + i18n( "Summary:" ) + "</td>";
  html += "<td>" + sSummary + "</td>";
  html += "</tr>\n";

  // Task Description Row
  html += "<tr>";
  html += "<td>" + i18n( "Description:" ) + "</td>";
  html += "<td>" + sDescr + "</td>";
  html += "</tr>\n";

  html += "</table>\n";

  return html;
}

class Formatter : public KMail::Interface::BodyPartFormatter
{
  public:
    Result format( KMail::Interface::BodyPart *bodyPart,
                   KMail::HtmlWriter *writer ) const
    {
      if ( !writer )
        // Guard against crashes in createReply()
        return Ok;

      const QString iCalendar = bodyPart->asText();
      if ( iCalendar.isEmpty() ) return AsIcon;

      CalendarLocal cl( KPimPrefs::timezone() );
      ICalFormat format;
      format.setTimeZone( cl.timeZoneId(), !cl.isLocalTime() );
      format.fromString( &cl, iCalendar );

      // Make a shallow copy of the event and task lists
      if( cl.events().count() == 0 && cl.todos().count() == 0 ) {
        kdDebug(5850) << "No iCal in this one\n";
        return AsIcon;
      }

      // Parse the first event out of the vcal
      // TODO: Is it legal to have several events/todos per mail part?
      Incidence* incidence = 0;
      Event* event = 0;
      Todo* todo = 0;
      if( cl.events().count() > 0 )
        incidence = event = cl.events().first();
      else
        incidence = todo = cl.todos().first();

      // TODO: Actually the scheduler needs to do this:
      QString sMethod; // = incidence->method();
      // TODO: This is a temporary workaround to get the method
      sMethod = "METHOD";
      vPartMicroParser( iCalendar, sMethod );
      sMethod = sMethod.lower();

      if ( sMethod.isEmpty() ) return AsIcon;

      // First make the text of the message
      QString html;
/*
      html += "<em>Hint for kdepim developers: This iCalendar attachement was "
              "formatted by the new bodypartformatter plugin.</em><br>";
*/
      QString tableStyle = QString::fromLatin1(
        "style=\""
        "border: solid 1px; "
        "margin: 0em;\"");
      QString tableHead = QString::fromLatin1(
        "<div align=\"center\">"
        "<table width=\"80%\" cellpadding=\"1\" cellspacing=\"0\" %1>"
        "<tr><td>").arg(tableStyle);

      html += tableHead;

      if( sMethod == "request" ) {
        // FIXME: All these "if (event) ... else ..." constructions are ugly.
        if( event ) {
          html += i18n( "<h2>You have been invited to this meeting</h2>" );
          html += meetingDetails( incidence, event );
        } else {
          html += i18n( "<h2>You have been assigned this task</h2>" );
          html += taskDetails( incidence );
        }
      } else if( sMethod == "reply" ) {
        Attendee::List attendees = incidence->attendees();
        if( attendees.count() == 0 ) {
          kdDebug(5850) << "No attendees in the iCal reply!\n";
          return AsIcon;
        }
        if( attendees.count() != 1 )
          kdDebug(5850) << "Warning: attendeecount in the reply should be 1 "
                        << "but is " << attendees.count() << endl;
        Attendee* attendee = *attendees.begin();

        switch( attendee->status() ) {
        case Attendee::Accepted:
          if( event ) {
            html += i18n( "<h2>Sender accepts this meeting invitation</h2>" );
            html += meetingDetails( incidence, event );
          } else {
            html += i18n( "<h2>Sender accepts this task</h2>" );
            html += taskDetails( incidence );
          }
          break;

        case Attendee::Tentative:
          if( event ) {
            html += i18n( "<h2>Sender tentatively accepts this "
                         "meeting invitation</h2>" );
            html += meetingDetails( incidence, event );
          } else {
            html += i18n( "<h2>Sender tentatively accepts this task</h2>" );
            html += taskDetails( incidence );
          }
          break;

        case Attendee::Declined:
          if( event ) {
            html += i18n( "<h2>Sender declines this meeting invitation</h2>" );
            html += meetingDetails( incidence, event );
          } else {
            html += i18n( "<h2>Sender declines this task</h2>" );
            html += taskDetails( incidence );
          }
          break;

        default:
          if( event ) {
            html += i18n( "<h2>Unknown response to this meeting invitation</h2>" );
            html += meetingDetails( incidence, event );
          } else {
            html += i18n( "<h2>Unknown response to this task</h2>" );
            html += taskDetails( incidence );
          }
        }
      } else if( sMethod == "cancel" ) {
        if( event ) {
          html += i18n( "<h2>This meeting has been canceled</h2>" );
          html += meetingDetails( incidence, event );
        } else {
          html += i18n( "<h2>This task was canceled</h2>" );
          html += taskDetails( incidence );
        }
      } else if ( sMethod == "publish" ) {
        if ( event ) {
          html += i18n("<h2>This event has been published</h2>");
          html += meetingDetails( incidence, event );
        } else {
          html += i18n("<h2>This task has been published</h2>");
          html += taskDetails( incidence );
        }
      } else {
        html += i18n("Error: iMIP message with unknown method: '%1'")
                .arg( sMethod );
      }

#if 0
      html += "<br>";
      html += "<a href=\"" + bodyPart->makeLink( "accept" ) + "\"><b>";
      html += i18n("[Enter this into my calendar]");
      html += "</b></a>";
#endif

      // Add the groupware URLs
      html += "<br>&nbsp;<br>&nbsp;<br>";
      html += "<table border=\"0\" cellspacing=\"0\"><tr><td>&nbsp;</td><td>";
      if( sMethod == "request" || sMethod == "update" ) {
        // Accept
        html += "<a href=\"" +
                bodyPart->makeLink( "accept" ) + "\"><b>";
        html += i18n( "[Accept]" );
        html += "</b></a></td><td> &nbsp; </td><td>";

        // Accept conditionally
        html += "<a href=\"" +
                bodyPart->makeLink( "accept_conditionally" ) +
                "\"><b>";
        html += i18n( "Accept conditionally", "[Accept cond.]" );
        html += "</b></a></td><td> &nbsp; </td><td>";

        // Decline
        html += "<a href=\"" +
                bodyPart->makeLink( "decline" ) + "\"><b>";
        html += i18n( "[Decline]" );
#if 0
        // TODO: Do this
        if( event ) {
          // Check my calendar...
          html += "</b></a></td><td> &nbsp; </td><td>";
          html += "<a href=\"" +
                  bodyPart->makeLink( "check_calendar" ) + "\"><b>";
          html += i18n("[Check my calendar...]" );
        }
#endif
      } else if( sMethod == "reply" || sMethod == "publish" ) {
        // Enter this into my calendar
        html += "<a href=\"" +
                bodyPart->makeLink( "reply" ) + "\"><b>";
        if( event )
          html += i18n( "[Enter this into my calendar]" );
        else
          html += i18n( "[Enter this into my task list]" );
        // Throw away
        html += "<a href=\"" +
                bodyPart->makeLink( "ignore" ) +
                "\"><b>";
        html += i18n( "Throw away", "[Throw Away]" );
        html += "</b></a></td><td> &nbsp; </td><td>";
      } else if( sMethod == "cancel" ) {
        // Cancel event from my calendar
        html += "<a href=\"" +
                bodyPart->makeLink( "cancel" ) + "\"><b>";
        html += i18n( "[Remove this from my calendar]" );
      }
      html += "</b></a></td></tr></table>";

      QString sDescr = incidence->description();
      if( ( sMethod == "reply" || sMethod == "request" || sMethod == "cancel" ) &&
          !sDescr.isEmpty() ) {
        string2HTML( sDescr );
        html += "<br>&nbsp;<br>&nbsp;<br><u>" + i18n("Description:")
          + "</u><br><table border=\"0\"><tr><td>&nbsp;</td><td>";
        html += sDescr + "</td></tr></table>";
      }

      html += "</td></tr></table><br></div>";
      writer->queue( html );

      return Ok;
    }
};

class UrlHandler : public KMail::Interface::BodyPartURLHandler
{
  public:
    UrlHandler()
    {
      kdDebug() << "UrlHandler() (iCalendar)" << endl;
    }

    Incidence* icalToString( const QString& iCal, ICalFormat& format ) const
    {
      CalendarLocal calendar( KPimPrefs::timezone() );
      ScheduleMessage *message =
        format.parseScheduleMessage( &calendar, iCal );
      if ( !message )
        //TODO: Error message?
        return 0;
      return dynamic_cast<Incidence*>( message->event() );
    }


    Attendee *findMyself( Incidence* incidence, const QString& receiver ) const
    {
      Attendee::List attendees = incidence->attendees();
      Attendee::List::ConstIterator it;
      Attendee* myself = 0;
      // Find myself. There will always be all attendees listed, even if
      // only I need to answer it.
      if ( attendees.count() == 1 )
        // Only one attendee, that must be me, or a distribution list
        myself = *attendees.begin();
      else {
        for ( it = attendees.begin(); it != attendees.end(); ++it ) {
          // match only the email part, not the name
          if( KPIM::compareEmail( (*it)->email(), receiver, false ) ) {
            // We are the current one, and even the receiver, note
            // this and quit searching.
            myself = (*it);
            break;
          }
        }
        // TODO: If still not found, ask about it
      }
      return myself;
    }

    void setStatusOnMyself( Incidence* incidence, Attendee* myself, 
                            Attendee::PartStat status, const QString &receiver ) const
    {
      Q_ASSERT( myself );
      Attendee* newMyself = 0;
      if( myself ) {
        myself->setStatus( status );
        QString name;
        QString email;
        KPIM::getNameAndMail( receiver, name, email );
        if ( name.isEmpty() ) name = myself->name();
        if ( email.isEmpty() ) email = myself->email();
        newMyself = new Attendee( name,
                                  email,
                                  myself->RSVP(),
                                  myself->status(),
                                  myself->role(),
                                  myself->uid() );
      }

      // Make sure only ourselves is in the event
      incidence->clearAttendees();
      if( newMyself )
        incidence->addAttendee( newMyself );
    }

    bool mail( Incidence* incidence, KMail::Callback& callback ) const
    {
      ICalFormat format;
      QString msg = format.createScheduleMessage( incidence,
                                                  Scheduler::Reply );
      QString subject;
      if ( !incidence->summary().isEmpty() )
        subject = i18n( "Answer: %1" ).arg( incidence->summary() );
      else
        subject = i18n( "Answer: Incidence with no summary" );
      return callback.mailICal( incidence->organizer().fullName(), msg, subject );
    }

    bool saveFile( const QString& receiver, const QString& iCal,
                   const QString& type ) const
    {
      KTempFile file( locateLocal( "data", "korganizer/income." + type + '/',
                                   true ) );
      QTextStream* ts = file.textStream();
      if ( !ts ) {
        KMessageBox::error( 0, i18n("Could not save file to KOrganizer") );
        return false;
      }
      ts->setEncoding( QTextStream::UnicodeUTF8 );
      (*ts) << receiver << '\n' << iCal;
      return true;
    }

    bool handleAccept( const QString& iCal, KMail::Callback& callback ) const
    {
      const QString receiver = callback.receiver();
      if ( receiver.isEmpty() )
        // Must be some error. Still return true though, since we did handle it
        return true;

      // First, save it for KOrganizer to handle
      saveFile( receiver, iCal, "accepted" );

      // Now produce the return message
      ICalFormat format;
      Incidence* incidence = icalToString( iCal, format );

      if( !incidence ) return false;
      Attendee *myself = findMyself( incidence, receiver );
      if ( myself && myself->RSVP() ) {
        setStatusOnMyself( incidence, myself, Attendee::Accepted, receiver );
        return mail( incidence, callback );
      } else {
        ( new KMDeleteMsgCommand( callback.getMsg()->getMsgSerNum() ) )->start();
        return true;
      }
    }

    bool handleAcceptConditionally( const QString& iCal, KMail::Callback& callback ) const
    {
      const QString receiver = callback.receiver();
      if ( receiver.isEmpty() )
        // Must be some error. Still return true though, since we did handle it
        return true;

      // First, save it for KOrganizer to handle
      saveFile( receiver, iCal, "tentative" );

      // Now produce the return message
      ICalFormat format;
      Incidence* incidence = icalToString( iCal, format );
      if( !incidence ) return false;
      Attendee *myself = findMyself( incidence, receiver );
      if ( myself && myself->RSVP() ) {
        setStatusOnMyself( incidence, myself, Attendee::Tentative, receiver );
        return mail( incidence, callback );
      } else {
        ( new KMDeleteMsgCommand( callback.getMsg()->getMsgSerNum() ) )->start();
        return true;
      }

    }

    bool handleDecline( const QString& iCal, KMail::Callback& callback ) const
    {
      const QString receiver = callback.receiver();
      if ( receiver.isEmpty() )
        // Must be some error. Still return true though, since we did handle it
        return true;

      // Produce a decline message
      ICalFormat format;
      Incidence* incidence = icalToString( iCal, format );
      if( !incidence ) return false;
      Attendee *myself = findMyself( incidence, receiver );
      if ( myself && myself->RSVP() ) {
        setStatusOnMyself( incidence, myself, Attendee::Declined, receiver );
        return mail( incidence, callback );
      } else {
        ( new KMDeleteMsgCommand( callback.getMsg()->getMsgSerNum() ) )->start();
        return true;
      }
    }

    bool handleIgnore( const QString&, KMail::Callback& c ) const
    {
      // simply move the message to trash
      ( new KMDeleteMsgCommand( c.getMsg()->getMsgSerNum() ) )->start();
      return true;
    }

    bool handleClick( KMail::Interface::BodyPart *part,
                      const QString &path, KMail::Callback& c ) const
    {
      QString iCal = part->asText();

      if ( path == "accept" )
        return handleAccept( iCal, c );
      if ( path == "accept_conditionally" )
        return handleAcceptConditionally( iCal, c );
      if ( path == "ignore" )
        return handleIgnore( iCal, c );
      if ( path == "decline" )
        return handleDecline( iCal, c );
      if ( path == "reply" || path == "cancel" )
        // These should just be saved with their type as the dir
        if ( saveFile( "Receiver Not Searched", iCal, path ) )
          ( new KMDeleteMsgCommand( c.getMsg()->getMsgSerNum() ) )->start();

      return false;
    }

    bool handleContextMenuRequest( KMail::Interface::BodyPart *,
                                   const QString &,
                                   const QPoint & ) const
    {
      return false;
    }

    QString statusBarMessage( KMail::Interface::BodyPart *,
                              const QString &path ) const
    {
      if ( !path.isEmpty() ) {
        if ( path == "accept" )
          return i18n("Accept incidence");
        if ( path == "accept_conditionally" )
          return i18n( "Accept incidence conditionally" );
        if ( path == "ignore" )
          return i18n( "Throw mail away" );
        if ( path == "decline" )
          return i18n( "Decline incidence" );
        if ( path == "check_calendar" )
          return i18n("Check my calendar..." );
        if ( path == "reply" )
          return i18n( "Enter incidence into my calendar" );
        if ( path == "cancel" )
          return i18n( "Remove incidence from my calendar" );
      }

      return QString::null;
    }
};

class Plugin : public KMail::Interface::BodyPartFormatterPlugin
{
  public:
    const KMail::Interface::BodyPartFormatter *bodyPartFormatter( int idx ) const
    {
      if ( idx == 0 ) return new Formatter();
      else return 0;
    }

    const char *type( int idx ) const
    {
      if ( idx == 0 ) return "text";
      else return 0;
    }

    const char *subtype( int idx ) const
    {
      if ( idx == 0 ) return "calendar";
      else return 0;
    }

    const KMail::Interface::BodyPartURLHandler * urlHandler( int idx ) const
    {
      if ( idx == 0 ) return new UrlHandler();
      else return 0;
    }
};

}

extern "C"
KMail::Interface::BodyPartFormatterPlugin *
libkmail_bodypartformatter_text_calendar_create_bodypart_formatter_plugin()
{
  KGlobal::locale()->insertCatalogue( "kmail_text_calendar_plugin" );
  return new Plugin();
}
