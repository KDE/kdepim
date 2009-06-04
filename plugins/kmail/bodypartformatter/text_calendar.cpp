/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#include "attendeeselector.h"
#include "delegateselector.h"

#include <interfaces/bodypartformatter.h>
#include <interfaces/bodypart.h>
#include <interfaces/bodyparturlhandler.h>
#include <khtmlparthtmlwriter.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/calendarresources.h>
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

#include "kcalendariface_stub.h"

using namespace KCal;

namespace {

class CalendarManager
{
  public:
    CalendarManager();
    ~CalendarManager();
    static KCal::Calendar* calendar();

  private:
    KCal::CalendarResources* mCalendar;
    static CalendarManager* mSelf;
};

static KStaticDeleter<CalendarManager> sCalendarDeleter;
CalendarManager* CalendarManager::mSelf = 0;

CalendarManager::CalendarManager()
{
  mCalendar = new CalendarResources( KPimPrefs::timezone() );
  mCalendar->readConfig();
  mCalendar->load();
  bool multipleKolabResources = false;
  CalendarResourceManager *mgr = mCalendar->resourceManager();
  for ( CalendarResourceManager::ActiveIterator it = mgr->activeBegin(); it != mgr->activeEnd(); ++it ) {
    if ( (*it)->type() == "imap" || (*it)->type() == "kolab" ) {
      const QStringList subResources = (*it)->subresources();
      QMap<QString, int> prefixSet; // KDE4: QSet
      for ( QStringList::ConstIterator subIt = subResources.begin(); subIt != subResources.end(); ++subIt ) {
        if ( !(*subIt).contains( "/.INBOX.directory/" ) )
          // we don't care about shared folders
          continue;
        prefixSet.insert( (*subIt).left( (*subIt).find( "/.INBOX.directory/" ) ), 0 );
      }
      if ( prefixSet.count() > 1 )
        multipleKolabResources = true;
    }
  }
  if ( multipleKolabResources ) {
    kdDebug() << k_funcinfo << "disabling calendar lookup because multiple active Kolab resources" << endl;
    delete mCalendar;
    mCalendar = 0;
  }
}

CalendarManager::~CalendarManager()
{
  delete mCalendar;
  mSelf = 0;
}

KCal::Calendar* CalendarManager::calendar()
{
  if ( !mSelf ) {
    sCalendarDeleter.setObject( mSelf, new CalendarManager() );
  }
  return mSelf->mCalendar;
}


class KMInvitationFormatterHelper : public KCal::InvitationFormatterHelper
{
  public:
    KMInvitationFormatterHelper( KMail::Interface::BodyPart *bodyPart ) : mBodyPart( bodyPart ) {}
    virtual QString generateLinkURL( const QString &id ) { return mBodyPart->makeLink( id ); }
    KCal::Calendar* calendar() const { return CalendarManager::calendar(); }
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

static QString directoryForStatus( Attendee::PartStat status )
{
  QString dir;
  switch ( status ) {
  case Attendee::Accepted:
    dir = "accepted";
    break;
  case Attendee::Tentative:
    dir = "tentative";
    break;
  case Attendee::Declined:
    dir = "cancel";
    break;
  case Attendee::Delegated:
    dir = "delegated";
    break;
  default:
    break;
  }
  return dir;
}

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
      for ( it = attendees.begin(); it != attendees.end(); ++it ) {
        // match only the email part, not the name
        if( KPIM::compareEmail( (*it)->email(), receiver, false ) ) {
          // We are the current one, and even the receiver, note
          // this and quit searching.
          myself = (*it);
          break;
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

      // Make sure only ourselves is in the event
      incidence->clearAttendees();
      if( newMyself )
        incidence->addAttendee( newMyself );
      return newMyself;
    }

    enum MailType {
      Answer,
      Delegation,
      Forward,
      DeclineCounter
    };

    bool mail( Incidence* incidence, KMail::Callback& callback,
               Attendee::PartStat status,
               Scheduler::Method method = Scheduler::Reply,
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
        case DeclineCounter:
          subject = i18n( "Declined Counter Proposal: %1" ).arg( summary );
          break;
      }

      QString recv = to;
      if ( recv.isEmpty() )
        recv = incidence->organizer().fullName();
      QString statusString = directoryForStatus( status );  //it happens to return the right strings
      return callback.mailICal( recv, msg, subject, statusString, type != Forward );
    }

    void ensureKorganizerRunning() const
    {
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
          ref.call( "newInstance()" ); // activate korganizer window
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
      ensureKorganizerRunning();

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

      // get comment for tentative acceptance
      Incidence* incidence = icalToString( iCal );

      if ( callback.askForComment( status ) ) {
        bool ok = false;
        QString comment = KInputDialog::getMultiLineText( i18n("Reaction to Invitation"),
            i18n("Comment:"), QString(), &ok );
        if ( !ok )
          return true;
        if ( !comment.isEmpty() ) {
          if ( callback.outlookCompatibleInvitationReplyComments() ) {
            incidence->setDescription( comment );
          } else {
            incidence->addComment( comment );
          }
        }
      }

      // First, save it for KOrganizer to handle
      QString dir = directoryForStatus( status );
      if ( dir.isEmpty() )
        return true; // unknown status
      if ( status != Attendee::Delegated ) // we do that below for delegated incidences
        saveFile( receiver, iCal, dir );

      QString delegateString;
      bool delegatorRSVP = false;
      if ( status == Attendee::Delegated ) {
        DelegateSelector dlg;
        if ( dlg.exec() == QDialog::Rejected )
          return true;
        delegateString = dlg.delegate();
        delegatorRSVP = dlg.rsvp();
        if ( delegateString.isEmpty() )
          return true;
        if ( KPIM::compareEmail( delegateString, incidence->organizer().email(), false ) ) {
          KMessageBox::sorry( 0, i18n("Delegation to organizer is not possible.") );
          return true;
        }
      }

      if( !incidence ) return false;
      Attendee *myself = findMyself( incidence, receiver );

      // find our delegator, we need to inform him as well
      QString delegator;
      if ( myself && !myself->delegator().isEmpty() ) {
        Attendee::List attendees = incidence->attendees();
        for ( Attendee::List::ConstIterator it = attendees.constBegin(); it != attendees.constEnd(); ++it ) {
          if( KPIM::compareEmail( (*it)->fullName(), myself->delegator(), false ) && (*it)->status() == Attendee::Delegated ) {
            delegator = (*it)->fullName();
            delegatorRSVP = (*it)->RSVP();
            break;
          }
        }
      }

      if ( ( myself && myself->RSVP() ) || heuristicalRSVP( incidence ) ) {
        Attendee* newMyself = setStatusOnMyself( incidence, myself, status, receiver );
        if ( newMyself && status == Attendee::Delegated ) {
          newMyself->setDelegate( delegateString );
          newMyself->setRSVP( delegatorRSVP );
        }
        ok = mail( incidence, callback, status );

        // check if we need to inform our delegator about this as well
        if ( newMyself && (status == Attendee::Accepted || status == Attendee::Declined)
             && !delegator.isEmpty() ) {
          if ( delegatorRSVP || status == Attendee::Declined )
            ok = mail( incidence, callback, status, Scheduler::Reply, delegator );
        }

      } else if ( !myself && (status != Attendee::Declined) ) {
        // forwarded invitation
        Attendee* newMyself = 0;
        QString name;
        QString email;
        KPIM::getNameAndMail( receiver, name, email );
        if ( !email.isEmpty() ) {
          newMyself = new Attendee( name,
                                    email,
                                    true, // RSVP, otherwise we would not be here
                                    status,
                                    heuristicalRole( incidence ),
                                    QString::null );
          incidence->clearAttendees();
          incidence->addAttendee( newMyself );
          ok = mail( incidence, callback, status, Scheduler::Reply );
        }
      } else {
        if ( callback.deleteInvitationAfterReply() )
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

        ok = mail( incidence, callback, status, Scheduler::Request, delegateString, Delegation );
      }
      return ok;
    }

    void showCalendar( const QDate &date ) const
    {
      ensureKorganizerRunning();
      // raise korganizer part in kontact or the korganizer app
      kapp->dcopClient()->send( "korganizer", "korganizer", "newInstance()", QByteArray() );
      QByteArray arg;
      QDataStream s( arg, IO_WriteOnly );
      s << QString( "kontact_korganizerplugin" );
      kapp->dcopClient()->send( "kontact", "KontactIface", "selectPlugin(QString)", arg );

      KCalendarIface_stub *iface = new KCalendarIface_stub( kapp->dcopClient(), "korganizer", "CalendarIface" );
      iface->showEventView();
      iface->showDate( date );
      delete iface;
    }

    bool handleIgnore( const QString&, KMail::Callback& c ) const
    {
      // simply move the message to trash
      ( new KMDeleteMsgCommand( c.getMsg()->getMsgSerNum() ) )->start();
      return true;
    }

    bool handleDeclineCounter( const QString &iCal, KMail::Callback &callback ) const
    {
      const QString receiver = callback.receiver();
      if ( receiver.isEmpty() )
        return true;
      Incidence* incidence = icalToString( iCal );
      if ( callback.askForComment( Attendee::Declined ) ) {
        bool ok = false;
        QString comment = KInputDialog::getMultiLineText( i18n("Decline Counter Proposal"),
            i18n("Comment:"), QString(), &ok );
        if ( !ok )
          return true;
        if ( !comment.isEmpty() ) {
          if ( callback.outlookCompatibleInvitationReplyComments() ) {
            incidence->setDescription( comment );
          } else {
            incidence->addComment( comment );
          }
        }
      }
      return mail( incidence, callback, Attendee::NeedsAction, Scheduler::Declinecounter,
                   callback.sender(), DeclineCounter );
    }

    bool counterProposal( const QString &iCal, KMail::Callback &callback ) const
    {
      const QString receiver = callback.receiver();
      if ( receiver.isEmpty() )
        return true;
      saveFile( receiver, iCal, "counter" );
      // Don't delete the invitation here in any case, if the counter proposal
      // is declined you might need it again.
      return true;
    }

    bool hasWritableCalendars() const
    {
      CalendarResourceManager *manager = new CalendarResourceManager( "calendar" );
      manager->readConfig();
      for ( CalendarResourceManager::ActiveIterator it = manager->activeBegin(); it != manager->activeEnd(); ++it ) {
        if ( (*it)->readOnly() )
          continue;
        const QStringList subResources = (*it)->subresources();
        if ( subResources.isEmpty() )
          return true;
        for ( QStringList::ConstIterator subIt = subResources.begin(); subIt != subResources.end(); ++subIt ) {
          if ( !(*it)->subresourceActive( (*subIt) ) )
            continue;
          return true;
        }
      }
      return false;
    }

    bool handleClick( KMail::Interface::BodyPart *part,
                      const QString &path, KMail::Callback& c ) const
    {
      if ( !hasWritableCalendars() ) {
        KMessageBox::error( 0, i18n("No writable calendar found.") );
        return false;
      }

      QString iCal;
      /* If the bodypart does not have a charset specified, we need to fall back to
         utf8, not the KMail fallback encoding, so get the contents as binary and decode
         explicitely. */
      if ( part->contentTypeParameter( "charset").isEmpty() ) {
        const QByteArray &ba = part->asBinary();
        iCal = QString::fromUtf8(ba);
      } else {
        iCal = part->asText();
      }
      bool result = false;
      if ( path == "accept" )
        result = handleInvitation( iCal, Attendee::Accepted, c );
      if ( path == "accept_conditionally" )
        result = handleInvitation( iCal, Attendee::Tentative, c );
      if ( path == "counter" )
        result = counterProposal( iCal, c );
      if ( path == "ignore" )
        result = handleIgnore( iCal, c );
      if ( path == "decline" )
        result = handleInvitation( iCal, Attendee::Declined, c );
      if ( path == "decline_counter" ) {
        result = handleDeclineCounter( iCal, c );
      }
      if ( path == "delegate" )
        result = handleInvitation( iCal, Attendee::Delegated, c );
      if ( path == "forward" ) {
        Incidence* incidence = icalToString( iCal );
        AttendeeSelector dlg;
        if ( dlg.exec() == QDialog::Rejected )
          return true;
        QString fwdTo = dlg.attendees().join( ", " );
        if ( fwdTo.isEmpty() )
          return true;
        result = mail( incidence, c, Attendee::Delegated /*### right ?*/,
                       Scheduler::Request, fwdTo, Forward );
      }
      if ( path == "check_calendar" ) {
        Incidence* incidence = icalToString( iCal );
        showCalendar( incidence->dtStart().date() );
      }
      if ( path == "reply" || path == "cancel" || path == "accept_counter" ) {
        // These should just be saved with their type as the dir
        const QString p = (path == "accept_counter" ? QString("reply") : path);
        if ( saveFile( "Receiver Not Searched", iCal, p ) ) {
          if ( c.deleteInvitationAfterReply() )
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
          return i18n("Accept invitation");
        if ( path == "accept_conditionally" )
          return i18n( "Accept invitation conditionally" );
        if ( path == "accept_counter" )
          return i18n( "Accept counter proposal" );
        if ( path == "counter" )
          return i18n( "Create a counter proposal..." );
        if ( path == "ignore" )
          return i18n( "Throw mail away" );
        if ( path == "decline" )
          return i18n( "Decline invitation" );
        if ( path == "decline_counter" )
          return i18n( "Decline counter proposal" );
        if ( path == "check_calendar" )
          return i18n("Check my calendar..." );
        if ( path == "reply" )
          return i18n( "Record response into my calendar" );
        if ( path == "delegate" )
          return i18n( "Delegate invitation" );
        if ( path == "forward" )
          return i18n( "Forward invitation" );
        if ( path == "cancel" )
          return i18n( "Remove invitation from my calendar" );
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
