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
#include <libkcal/incidenceformatter.h>

#include <kpimprefs.h> // for the timezone

#include <kmail/callback.h>
#include <kmail/kmmessage.h>

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

class KMInvitationFormatterHelper : public KCal::InvitationFormatterHelper
{
  public: 
    KMInvitationFormatterHelper( KMail::Interface::BodyPart *bodyPart ) : mBodyPart( bodyPart ) {}
    virtual QString generateLinkURL( const QString &id ) { return mBodyPart->makeLink( id ); }
  private:
    KMail::Interface::BodyPart *mBodyPart;
};
    
class Formatter : public KMail::Interface::BodyPartFormatter
{
  public:
    Result format( KMail::Interface::BodyPart *bodyPart,
                   KMail::HtmlWriter *writer ) const
    {
      if ( !writer )
        // Guard against crashes in createReply()
        return Ok;
      CalendarLocal cl( KPimPrefs::timezone() );
      KMInvitationFormatterHelper helper( bodyPart );
      
      QString html = IncidenceFormatter::formatICalInvitation( bodyPart->asText(), &cl, &helper );

      if ( html.isEmpty() ) return AsIcon;
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
      CalendarLocal calendar;
      ScheduleMessage *message =
        format.parseScheduleMessage( &calendar, iCal );
      if ( !message )
        //TODO: Error message?
        return 0;
      return dynamic_cast<Incidence*>( message->event() );
    }

    void setStatusOnMyself( Incidence* incidence, Attendee::PartStat status,
                            const QString& receiver ) const
    {
      Attendee::List attendees = incidence->attendees();
      Attendee::List::ConstIterator it;
      Attendee* myself = 0;
      // Find myself. There will always be all attendees listed, even if
      // only I need to answer it.
      if ( attendees.count() == 1 )
        // Only one attendee, that must be me
        myself = *attendees.begin();
      else {
        for ( it = attendees.begin(); it != attendees.end(); ++it ) {
          if( (*it)->email() == receiver ) {
            // We are the current one, and even the receiver, note
            // this and quit searching.
            myself = (*it);
            break;
          }

#if 0
          // This is postponed until we have shared identities.
          // And then it might be unnecessary
          if ( (*it)->email() == KOPrefs::instance()->email() ) {
            // If we are the current one, note that. Still continue to
            // search in case we find the receiver himself.
            myself = (*it);
          }
#endif
        }
        // TODO: If still not found, ask about it
      }

      Q_ASSERT( myself );
      Attendee* newMyself = 0;
      if( myself ) {
        myself->setStatus( status );

        // No more request response
        myself->setRSVP(false);

        newMyself = new Attendee( myself->name(),
                                  receiver.isEmpty() ? myself->email() :
                                  receiver,
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
      setStatusOnMyself( incidence, Attendee::Accepted, receiver );
      return mail( incidence, callback );
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
      setStatusOnMyself( incidence, Attendee::Tentative, receiver );
      return mail( incidence, callback );
    }

    bool handleDecline( const QString& iCal, KMail::Callback& callback ) const
    {
      // Produce a decline message
      ICalFormat format;
      Incidence* incidence = icalToString( iCal, format );
      if( !incidence ) return false;
      setStatusOnMyself( incidence, Attendee::Declined, callback.receiver() );
      return mail( incidence, callback );
    }

    bool handleClick( KMail::Interface::BodyPart *part,
                      const QString &path, KMail::Callback& c ) const
    {
      QString iCal = part->asText();

      if ( path == "accept" )
        return handleAccept( iCal, c );
      if ( path == "accept_conditionally" )
        return handleAcceptConditionally( iCal, c );
      if ( path == "decline" )
        return handleDecline( iCal, c );
      if ( path == "reply" || path == "cancel" )
        // These should just be saved with their type as the dir
        return saveFile( "Reciever Not Searched", iCal, path );

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
