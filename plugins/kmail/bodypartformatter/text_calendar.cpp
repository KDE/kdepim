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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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
#include <kmail/kmcommands.h>

#include <email.h>

#include <kglobal.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kstringhandler.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kdcopservicestarter.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <ktempfile.h>

#include <qurl.h>
#include <qdir.h>
#include <qtextstream.h>

#include <kdepimmacros.h>

#include <dcopclient.h>
#include <dcopref.h>

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
      QString source;
      /* If the bodypart does not have a charset specified, we need to fall back to
         utf8, not the KMail fallback encoding, so get the contents as binary and decode
         explicitely. */
      if ( bodyPart->contentTypeParameter( "charset").isEmpty() ) {
        const QByteArray &ba = bodyPart->asBinary();
        source = QString::fromUtf8(ba);
      } else {
        source = bodyPart->asText();
      }
      QString html = IncidenceFormatter::formatICalInvitation( source, &cl, &helper );

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

    Incidence* icalToString( const QString& iCal ) const
    {
      CalendarLocal calendar( KPimPrefs::timezone() ) ;
      ICalFormat format;
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
        // Only one attendee, that must be me
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
      }
      return myself;
    }

    static bool heuristicalRSVP( Incidence *incidence )
    {
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

    static Attendee::Role heuristicalRole( Incidence *incidence )
    {
      Attendee::Role role = Attendee::OptParticipant;
      Attendee::List attendees = incidence->attendees();
      Attendee::List::ConstIterator it;
      for ( it = attendees.begin(); it != attendees.end(); ++it ) {
        if ( it == attendees.begin() ) {
          role = (*it)->role(); // use what the first one has
        } else {
          if ( (*it)->role() != role ) {
            role = Attendee::OptParticipant; // they differ, default
            break;
          }
        }
      }
      return role;

    }

    Attendee* setStatusOnMyself( Incidence* incidence, Attendee* myself,
                            Attendee::PartStat status, const QString &receiver ) const
    {
      Attendee* newMyself = 0;
      QString name;
      QString email;
      KPIM::getNameAndMail( receiver, name, email );
      if ( name.isEmpty() && myself ) name = myself->name();
      if ( email.isEmpty()&& myself ) email = myself->email();
      Q_ASSERT( !email.isEmpty() ); // delivery must be possible
      newMyself = new Attendee( name,
                                email,
                                true, // RSVP, otherwise we would not be here
                                status,
                                myself ? myself->role() : heuristicalRole( incidence ),
                                myself ? myself->uid() : QString::null );
      if ( myself ) {
        newMyself->setDelegate( myself->delegate() );
        newMyself->setDelegator( myself->delegator() );
      }

      // find our delegator, we need to keep him in
      Attendee *delegator = 0;
      if ( !newMyself->delegator().isEmpty() ) {
        Attendee *oldDelegator = 0;
        Attendee::List attendees = incidence->attendees();
        for ( Attendee::List::ConstIterator it = attendees.constBegin(); it != attendees.constEnd(); ++it ) {
          if( KPIM::compareEmail( (*it)->fullName(), newMyself->delegator(), false ) && (*it)->status() == Attendee::Delegated ) {
            oldDelegator = *it;
            break;
          }
        }
        if ( oldDelegator ) {
          delegator = new Attendee( oldDelegator->name(),
                                    oldDelegator->email(),
                                    true,
                                    oldDelegator->status(),
                                    oldDelegator->role(),
                                    oldDelegator->uid() );
          delegator->setDelegate( oldDelegator->delegate() );
          delegator->setDelegator( oldDelegator->delegator() );
        }
      }

      // Make sure only ourselves is in the event
      incidence->clearAttendees();
      if( newMyself )
        incidence->addAttendee( newMyself );
      if ( delegator )
        incidence->addAttendee( delegator );
      return newMyself;
    }

    enum MailType {
      Answer,
      Delegation,
      Forward
    };

    bool mail( Incidence* incidence, KMail::Callback& callback, Scheduler::Method method = Scheduler::Reply,
               const QString &to = QString::null, MailType type = Answer ) const
    {
      ICalFormat format;
      format.setTimeZone( KPimPrefs::timezone(), false );
      QString msg = format.createScheduleMessage( incidence, method );
      QString summary = incidence->summary();
      if ( summary.isEmpty() )
        summary = i18n( "Incidence with no summary" );
      QString subject;
      switch ( type ) {
        case Answer:
          subject = i18n( "Answer: %1" ).arg( summary );
          break;
        case Delegation:
          subject = i18n( "Delegated: %1" ).arg( summary );
          break;
        case Forward:
          subject = i18n( "Forwarded: %1" ).arg( summary );
          break;
      }

      QString recv = to;
      if ( recv.isEmpty() )
        recv = incidence->organizer().fullName();
      return callback.mailICal( recv, msg, subject );
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
      file.close();

      // Now ensure that korganizer is running; otherwise start it, to prevent surprises
      // (https://intevation.de/roundup/kolab/issue758)
      QString error;
      QCString dcopService;
      int result = KDCOPServiceStarter::self()->findServiceFor( "DCOP/Organizer", QString::null, QString::null, &error, &dcopService );
      if ( result == 0 ) {
        // OK, so korganizer (or kontact) is running. Now ensure the object we want is available
        // [that's not the case when kontact was already running, but korganizer not loaded into it...]
        static const char* const dcopObjectId = "KOrganizerIface";
        QCString dummy;
        if ( !kapp->dcopClient()->findObject( dcopService, dcopObjectId, "", QByteArray(), dummy, dummy ) ) {
          DCOPRef ref( dcopService, dcopService ); // talk to the KUniqueApplication or its kontact wrapper
          DCOPReply reply = ref.call( "load()" );
          if ( reply.isValid() && (bool)reply ) {
            kdDebug() << "Loaded " << dcopService << " successfully" << endl;
            Q_ASSERT( kapp->dcopClient()->findObject( dcopService, dcopObjectId, "", QByteArray(), dummy, dummy ) );
          } else
            kdWarning() << "Error loading " << dcopService << endl;
        }

        // We don't do anything with it, we just need it to be running so that it handles
        // the incoming directory.
      }
      else
        kdWarning() << "Couldn't start DCOP/Organizer: " << dcopService << " " << error << endl;

      return true;
    }

    bool handleInvitation( const QString& iCal, Attendee::PartStat status,
                           KMail::Callback &callback ) const
    {
      bool ok = true;
      const QString receiver = callback.receiver();
      if ( receiver.isEmpty() )
        // Must be some error. Still return true though, since we did handle it
        return true;

      // First, save it for KOrganizer to handle
      QString dir;
      if ( status == Attendee::Accepted ) dir = "accepted";
      else if ( status == Attendee::Tentative  ) dir = "tentative";
      else if ( status == Attendee::Declined ) dir = "cancel";
      else if ( status == Attendee::Delegated ) dir = "delegated";
      else return true; // unknown status

      if ( status != Attendee::Delegated ) // we do that below for delegated incidences
        saveFile( receiver, iCal, dir );

      QString delegateString;
      if ( status == Attendee::Delegated ) {
        // TODO: add real attendee selector
        bool ok = false;
        delegateString = KInputDialog::getText( "Select Delegate", "Delegate:", QString::null, &ok );
        if ( delegateString.isEmpty() || !ok )
          return true;
      }

      // Now produce the return message
      Incidence* incidence = icalToString( iCal );

      if( !incidence ) return false;
      Attendee *myself = findMyself( incidence, receiver );
      if ( ( myself && myself->RSVP() ) || heuristicalRSVP( incidence ) ) {
        Attendee* newMyself = setStatusOnMyself( incidence, myself, status, receiver );
        if ( newMyself && status == Attendee::Delegated )
          newMyself->setDelegate( delegateString );
        ok =  mail( incidence, callback );

        // check if we need to inform our delegator about this as well
        if ( newMyself && (status == Attendee::Accepted || status == Attendee::Declined) ) {
          Attendee::List attendees = incidence->attendees();
          for ( Attendee::List::ConstIterator it = attendees.constBegin(); it != attendees.constEnd(); ++it ) {
            if ( (*it) == newMyself )
              continue;
            if ( KPIM::compareEmail( (*it)->delegate(), newMyself->fullName(), false ) && (*it)->RSVP() ) {
              ok = mail( incidence, callback, Scheduler::Reply, (*it)->fullName() );
              break;
            }
          }
        }

      } else {
        ( new KMDeleteMsgCommand( callback.getMsg()->getMsgSerNum() ) )->start();
      }
      delete incidence;

      // create invitation for the delegate (same as the original invitation
      // with the delegate as additional attendee), we also use that for updating
      // our calendar
      if ( status == Attendee::Delegated ) {
        incidence = icalToString( iCal );
        myself = findMyself( incidence, receiver );
        myself->setStatus( status );
        myself->setDelegate( delegateString );
        QString name, email;
        KPIM::getNameAndMail( delegateString, name, email );
        Attendee *delegate = new Attendee( name, email, true );
        delegate->setDelegator( receiver );
        incidence->addAttendee( delegate );

        ICalFormat format;
        format.setTimeZone( KPimPrefs::timezone(), false );
        QString iCal = format.createScheduleMessage( incidence, Scheduler::Request );
        saveFile( receiver, iCal, dir );

        ok = mail( incidence, callback, Scheduler::Request, delegateString, Delegation );
      }
      return ok;
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
      bool result = false;
      if ( path == "accept" )
        result = handleInvitation( iCal, Attendee::Accepted, c );
      if ( path == "accept_conditionally" )
        result = handleInvitation( iCal, Attendee::Tentative, c );
      if ( path == "ignore" )
        result = handleIgnore( iCal, c );
      if ( path == "decline" )
        result = handleInvitation( iCal, Attendee::Declined, c );
      if ( path == "delegate" )
        result = handleInvitation( iCal, Attendee::Delegated, c );
      if ( path == "forward" )
        kdDebug() << k_funcinfo << "forwarding not yet implemented" << endl;
      if ( path == "reply" || path == "cancel" ) {
        // These should just be saved with their type as the dir
        if ( saveFile( "Receiver Not Searched", iCal, path ) ) {
          ( new KMDeleteMsgCommand( c.getMsg()->getMsgSerNum() ) )->start();
          result = true;
        }
      }
      if ( result )
        c.closeIfSecondaryWindow();
      return result;
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
        if ( path == "delegate" )
          return i18n( "Delegate incidence" );
        if ( path == "forward" )
          return i18n( "Forward incidence" );
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
KDE_EXPORT KMail::Interface::BodyPartFormatterPlugin *
libkmail_bodypartformatter_text_calendar_create_bodypart_formatter_plugin()
{
  KGlobal::locale()->insertCatalogue( "kmail_text_calendar_plugin" );
  return new Plugin();
}
