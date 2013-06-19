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

#include <libkdepim/kfileio.h>
#include <libkdepim/stdcalendar.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/calendarresources.h>
#include <libkcal/calhelper.h>
#include <libkcal/icalformat.h>
#include <libkcal/attendee.h>
#include <libkcal/attachmenthandler.h>
#include <libkcal/incidence.h>
#include <libkcal/incidenceformatter.h>

#include <kpimprefs.h> // for the timezone

#include <kmail/callback.h>
#include <kmail/kmmessage.h>
#include <kmail/kmcommands.h>

#include <email.h>

#include <kglobal.h>
#include <kfiledialog.h>
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
#include <kmdcodec.h>
#include <kmimetype.h>
#include <kpopupmenu.h>
#include <krun.h>
#include <kio/netaccess.h>

#include <qurl.h>
#include <qdir.h>
#include <qtextstream.h>

#include <kdepimmacros.h>

#include <dcopclient.h>
#include <dcopref.h>

#include "kcalendariface_stub.h"

using namespace KCal;

namespace {

class KMInvitationFormatterHelper : public InvitationFormatterHelper
{
  public:
    KMInvitationFormatterHelper( KMail::Interface::BodyPart *bodyPart ) : mBodyPart( bodyPart ) {}
    virtual QString generateLinkURL( const QString &id ) { return mBodyPart->makeLink( id ); }
    Calendar* calendar() const { return KCal::StdCalendar::self(); }
  private:
    KMail::Interface::BodyPart *mBodyPart;
};

class Formatter : public KMail::Interface::BodyPartFormatter
{
  public:
    Result format( KMail::Interface::BodyPart *bodyPart,
                   KMail::HtmlWriter *writer, KMail::Callback &callback ) const
    {
      if ( !writer )
        // Guard against crashes in createReply()
        return Ok;
      CalendarLocal cl( KPimPrefs::timezone() );
      KMInvitationFormatterHelper helper( bodyPart );
      QString source;
      /* If the bodypart does not have a charset specified, we need to fall back to
         utf8, not the KMail fallback encoding, so get the contents as binary and decode
         explicitly. */
      if ( bodyPart->contentTypeParameter( "charset").isEmpty() ) {
        const QByteArray &ba = bodyPart->asBinary();
        source = QString::fromUtf8(ba);
      } else {
        source = bodyPart->asText();
      }
      QString html =
        IncidenceFormatter::formatICalInvitationNoHtml(
          source, &cl, &helper, callback.sender(),
          callback.outlookCompatibleInvitationComparisons() );

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

static Incidence *icalToString( const QString &iCal )
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

static ScheduleMessage *icalToMessage( const QString &iCal )
{
  CalendarLocal calendar( KPimPrefs::timezone() ) ;
  ICalFormat format;
  return format.parseScheduleMessage( &calendar, iCal );
}

class UrlHandler : public KMail::Interface::BodyPartURLHandler
{
  public:
    UrlHandler()
    {
      KCal::StdCalendar::self()->load();
      kdDebug() << "UrlHandler() (iCalendar)" << endl;
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

      // Set the organizer to the sender, if the ORGANIZER hasn't been set.
      if ( incidence->organizer().isEmpty() ) {
        QString tname, temail;
        KPIM::getNameAndMail( callback.sender(), tname, temail );
        incidence->setOrganizer( Person( tname, temail ) );
      }

      QString recv = to;
      if ( recv.isEmpty() )
        recv = incidence->organizer().fullName();
      QString statusString = directoryForStatus( status );  //it happens to return the right strings
      return callback.mailICal( recv, msg, subject, statusString, type != Forward );
    }

    void ensureKorganizerRunning( bool switchTo ) const
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
          if ( switchTo ) {
            ref.call( "newInstance()" ); // activate korganizer window
          }

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
      ensureKorganizerRunning( false );

      return true;
    }

    bool cancelPastInvites( Incidence *incidence, const QString &path ) const
    {
      QString warnStr;
      QDateTime now = QDateTime::currentDateTime();
      QDate today = now.date();
      Event * const event = dynamic_cast<Event *>( incidence );
      Todo * const todo = dynamic_cast<Todo *>( incidence );
      if ( incidence->type() == "Event" ) {
        Q_ASSERT( event );
        if ( !event->doesFloat() ) {
          if ( event->dtEnd() < now ) {
            warnStr = i18n( "\"%1\" occurred already." ).arg( event->summary() );
          } else if ( event->dtStart() <= now && now <= event->dtEnd() ) {
            warnStr = i18n( "\"%1\" is currently in-progress." ).arg( event->summary() );
          }
        } else {
          if ( event->dtEnd().date() < today ) {
            warnStr = i18n( "\"%1\" occurred already." ).arg( event->summary() );
          } else if ( event->dtStart().date() <= today && today <= event->dtEnd().date() ) {
            warnStr = i18n( "\"%1\", happening all day today, is currently in-progress." ).
                      arg( event->summary() );
          }
        }
      } else if ( incidence->type() == "Todo" ) {
        Q_ASSERT( todo );
        if ( !todo->doesFloat() ) {
          if ( todo->hasDueDate() ) {
            if ( todo->dtDue() < now ) {
              warnStr = i18n( "\"%1\" is past due." ).arg( todo->summary() );
            } else if ( todo->hasStartDate() && todo->dtStart() <= now && now <= todo->dtDue() ) {
              warnStr = i18n( "\"%1\" is currently in-progress." ).arg( todo->summary() );
            }
          } else if ( todo->hasStartDate() ) {
            if ( todo->dtStart() < now ) {
              warnStr = i18n( "\"%1\" has already started." ).arg( todo->summary() );
            }
          }
        } else {
          if ( todo->hasDueDate() ) {
            if ( todo->dtDue().date() < today) {
              warnStr = i18n( "\"%1\" is past due." ).arg( todo->summary() );
            } else if ( todo->hasStartDate() &&
                        todo->dtStart().date() <= today && today <= todo->dtDue().date() ) {
              warnStr = i18n( "\"%1\", happening all-day today, is currently in-progress." ).
                        arg( todo->summary() );
            }
          } else if ( todo->hasStartDate() ) {
            if ( todo->dtStart().date() < today ) {
              warnStr = i18n( "\"%1\", happening all day, has already started." ).
                        arg( todo->summary() );
            }
          }
        }
      }

      if ( !warnStr.isEmpty() ) {
        QString queryStr;
        Q_ASSERT( event || todo );
        if ( path == "accept" ) {
          if ( event ) {
            queryStr = i18n( "Do you still want to accept the invitation?" );
          } else if ( todo ) {
            queryStr = i18n( "Do you still want to accept the task?" );
          }
        } else if ( path == "accept_conditionally" ) {
          if ( event ) {
            queryStr = i18n( "Do you still want to send conditional acceptance of the invitation?" );
          } else if ( todo ) {
            queryStr = i18n( "Do you still want to send conditional acceptance of the task?" );
          }
        } else if ( path == "accept_counter" ) {
          queryStr = i18n( "Do you still want to accept the counter proposal?" );
        } else if ( path == "counter" ) {
          queryStr = i18n( "Do you still want to send a counter proposal?" );
        } else if ( path == "decline" ) {
          queryStr = i18n( "Do you still want to send a decline response?" );
        } else if ( path == "decline_counter" ) {
          queryStr = i18n( "Do you still want to decline the counter proposal?" );
        } else if ( path == "reply" ) {
          queryStr = i18n( "Do you still want to record this reponse in your calendar?" );
        } else if ( path == "delegate" ) {
          if ( event ) {
            queryStr = i18n( "Do you still want to delegate this invitation?" );
          } else if ( todo ) {
            queryStr = i18n( "Do you still want to delegate this task?" );
          }
        } else if ( path == "forward" ) {
          if ( event ) {
            queryStr = i18n( "Do you still want to forward this invitation?" );
          } else if ( todo ) {
            queryStr = i18n( "Do you still want to forward this task?" );
          }
        } else if ( path == "cancel" ) {
          if ( event ) {
            queryStr = i18n( "Do you still want to cancel this invitation?" );
          } else if ( todo ) {
            queryStr = i18n( "Do you still want to cancel this task?" );
          }
        } else if ( path == "check_calendar" ) {
          queryStr = i18n( "Do you still want to check your calendar?" );
        } else if ( path == "record" ) {
          if ( event ) {
            queryStr = i18n( "Do you still want to record this invitation in your calendar?" );
          } else if ( todo ) {
            queryStr = i18n( "Do you still want to record this task in your calendar?" );
          }
        } else if ( path.startsWith( "ATTACH:" ) ) {
          return false;
        } else {
          queryStr = i18n( "%1?" ).arg( path );
        }

        if ( KMessageBox::warningYesNo(
               0,
               i18n( "%1\n%2" ).arg( warnStr ).arg( queryStr ) ) == KMessageBox::No ) {
          return true;
        }
      }
      return false;
    }

    bool handleInvitation( const QString& iCal, Attendee::PartStat status,
                           KMail::Callback &callback ) const
    {
      bool ok = true;
      const QString receiver = callback.receiver();

      if ( receiver.isEmpty() )
        // Must be some error. Still return true though, since we did handle it
        return true;

      Incidence *incidence = icalToString( iCal );

      // get comment for tentative acceptance
      if ( callback.askForComment( status ) ) {
        bool ok = false;
        QString comment =
          KInputDialog::getMultiLineText( i18n( "Reaction to Invitation" ),
                                          i18n( "Comment:" ), QString(), &ok );
        if ( !ok ) {
          return true;
        }

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
        if ( callback.deleteInvitationAfterReply() ) {
          ( new KMDeleteMsgCommand( callback.getMsg()->getMsgSerNum() ) )->start();
        } else {
          callback.updateReaderWindow();
        }
      }
      delete incidence;

      // create invitation for the delegate (same as the original invitation
      // with the delegate as additional attendee), we also use that for updating
      // our calendar
      if ( status == Attendee::Delegated ) {
        incidence = icalToString( iCal );
        myself = findMyself( incidence, receiver );
        if ( myself ) {
          myself->setStatus( status );
          myself->setDelegate( delegateString );
        }
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
      ensureKorganizerRunning( true );
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
      if ( receiver.isEmpty() ) {
        return true;
      }

      Incidence *incidence = icalToString( iCal );
      Incidence *existing = 0;
      existing = KCal::StdCalendar::self()->incidence( incidence->uid() );

      if ( !existing ) {
        KMessageBox::sorry(
          0,
          i18n( "Cannot locate the original invitation in your calendar.\n"
                "You may have removed the associated incidence or calendar in the meantime.\n\n"
                "Unable to decline the counter proprosal." ) );
        return true;
      }

      if ( callback.askForComment( Attendee::Declined ) ) {
        bool ok = false;
        QString comment =
          KInputDialog::getMultiLineText( i18n( "Decline Counter Proposal" ),
                                          i18n( "Comment:" ), QString(), &ok );
        if ( !ok ) {
          return true;
        }

        if ( !comment.isEmpty() ) {
          if ( callback.outlookCompatibleInvitationReplyComments() ) {
            existing->setDescription( comment );
          } else {
            existing->addComment( comment );
          }
        }
      }
      return mail( existing, callback, Attendee::NeedsAction, Scheduler::Declinecounter,
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

    bool handleClick( KMail::Interface::BodyPart *part,
                      const QString &path, KMail::Callback& c ) const
    {
      // filter out known paths that don't belong to this type of urlmanager.
      // kolab/issue4054 msg27201
      if ( path.contains( "addToAddressBook:" ) ) {
        return false;
      }

      if ( !CalHelper::hasWritableEventsFolders( "calendar", StdCalendar::self()->resourceManager() ) ) {
        KMessageBox::error(
          0,
          i18n( "You have no writable calendar folders for invitations, "
                "so storing or saving a response will not be possible.\n"
                "Please create at least 1 writable events calendar and re-sync." ) );
        return false;
      }


      // If the bodypart does not have a charset specified, we need to fall back to utf8,
      // not the KMail fallback encoding, so get the contents as binary and decode explicitly.
      QString iCal;
      if ( part->contentTypeParameter( "charset").isEmpty() ) {
        const QByteArray &ba = part->asBinary();
        iCal = QString::fromUtf8(ba);
      } else {
        iCal = part->asText();
      }
      Incidence *incidence = icalToString( iCal );
      if ( !incidence ) {
        KMessageBox::sorry(
          0,
          i18n( "The calendar invitation stored in this email message is broken in some way. "
                "Unable to continue." ) );
        return false;
      }

      bool result = false;
      if ( cancelPastInvites( incidence, path ) ) {
        return result;
      }

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
        incidence = icalToString( iCal );
        showCalendar( incidence->dtStart().date() );
      }
      if ( path == "reply" || path == "cancel" || path == "accept_counter" ) {
        // These should just be saved with their type as the dir
        const QString p = (path == "accept_counter" ? QString("reply") : path);
        if ( saveFile( "Receiver Not Searched", iCal, p ) ) {
          if ( c.deleteInvitationAfterReply() ) {
            ( new KMDeleteMsgCommand( c.getMsg()->getMsgSerNum() ) )->start();
          } else {
            c.updateReaderWindow();
          }
          result = true;
        }
      }
      if ( path == "record" ) {
        incidence = icalToString( iCal );

        int response =
          KMessageBox::questionYesNoCancel(
          0,
          i18n( "The organizer is not expecting a reply to this invitation "
                "but you can send them an email message if you desire.\n\n"
                "Would you like to send the organizer a message regarding this invitation?\n"
                "Press the [Cancel] button to cancel the recording operation." ),
          i18n( "Send Email to Organizer" ),
          KGuiItem( i18n( "Do Not Send" ) ),
          KGuiItem( i18n( "Send EMail" ) ) );

        QString summary;
        switch( response ) {
        case KMessageBox::Cancel:
          break;
        case KMessageBox::No: // means "send email"
          summary = incidence->summary();
          if ( !summary.isEmpty() ) {
            summary = i18n( "Re: %1" ).arg( summary );
          }

          KApplication::kApplication()->invokeMailer( incidence->organizer().email(), summary );
          //fall through
        case KMessageBox::Yes: // means "do not send"
          if ( saveFile( "Receiver Not Searched", iCal, QString( "reply" ) ) ) {
            if ( c.deleteInvitationAfterReply() ) {
              ( new KMDeleteMsgCommand( c.getMsg()->getMsgSerNum() ) )->start();
            } else {
              c.updateReaderWindow();
            }
            result = true;
          }
          if ( !incidence->dtStart().isValid() ) {
            // Should not happen according to RFC5546 every published
            // event should have a dtstart. But there are some clients
            // like Kontact in some versions that do this anyhow.
            // So we should try to handle it gracefully.
            if ( incidence->type() == "Todo" ) {
              Todo * const todo = dynamic_cast<Todo *>( incidence );

              // in RFC2445 VTODO's do not need to have a start date
              // this might be hitting us here
              if ( todo && todo->hasDueDate() ) {
                showCalendar( todo->dtDue().date() );
              }
            } else {
              // mmh no idea.
              kdWarning() << "Incidence has no start and no due something is wrong here." << endl;
            }
          } else {
            showCalendar( incidence->dtStart().date() );
          }
          break;
        }
      }

      if ( path == "delete" ) {
        ( new KMDeleteMsgCommand( c.getMsg()->getMsgSerNum() ) )->start();
        result = true;
      }

      if ( path.startsWith( "ATTACH:" ) ) {
        QString name = path.section( ':', 1, -1 );
        const QCString decodedName = KCodecs::base64Decode(name.utf8());
        name = QString::fromUtf8(decodedName.data(), decodedName.length());
        result = AttachmentHandler::view( 0, name, icalToMessage( iCal ) );
      }

      if ( result ) {
        // do not close the secondary window if an attachment was opened (kolab/issue4317)
        if ( !path.startsWith( "ATTACH:" ) ) {
          c.closeIfSecondaryWindow();
        }
      }
      return result;
    }

    bool handleContextMenuRequest( KMail::Interface::BodyPart *part,
                                   const QString &path,
                                   const QPoint &point ) const
    {
      QString name = path;
      if ( path.startsWith( "ATTACH:" ) ) {
        QString name = path.section( ':', 1, -1 );
        const QCString decodedName = KCodecs::base64Decode(name.utf8());
        name = QString::fromUtf8(decodedName.data(), decodedName.length());
      } else {
        return false; //because it isn't an attachment inviation
      }

      QString iCal;
      if ( part->contentTypeParameter( "charset").isEmpty() ) {
        const QByteArray &ba = part->asBinary();
        iCal = QString::fromUtf8( ba );
      } else {
        iCal = part->asText();
      }

      KPopupMenu *menu = new KPopupMenu();
      menu->insertItem( i18n( "Open Attachment" ), 0 );
      menu->insertItem( i18n( "Save Attachment As..." ), 1 );

      switch( menu->exec( point, 0 ) ) {
      case 0: // open
        AttachmentHandler::view( 0, name, icalToMessage( iCal ) );
        break;
      case 1: // save as
        AttachmentHandler::saveAs( 0, name, icalToMessage( iCal ) );
        break;
      default:
        break;
      }
      return true;
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
        if ( path == "record" )
          return i18n( "Record invitation into my calendar" );
        if ( path == "delete" )
          return i18n( "Move this invitation to my trash folder" );
        if ( path == "delegate" )
          return i18n( "Delegate invitation" );
        if ( path == "forward" )
          return i18n( "Forward invitation" );
        if ( path == "cancel" )
          return i18n( "Remove invitation from my calendar" );
        if ( path.startsWith( "ATTACH:" ) ) {
          QString name = path.section( ':', 1, -1 );
          const QCString decodedName = KCodecs::base64Decode(name.utf8());
          name = QString::fromUtf8(decodedName.data(), decodedName.length());
          return i18n( "Open attachment \"%1\"" ).arg( name );
        }
      }

      return QString::null;
    }
};

class Plugin : public KMail::Interface::BodyPartFormatterPlugin
{
  public:
    const KMail::Interface::BodyPartFormatter *bodyPartFormatter( int idx ) const
    {
      if ( idx == 0 || idx == 1 ) return new Formatter();
      else return 0;
    }

    const char *type( int idx ) const
    {
      if ( idx == 0 || idx == 1 ) return "text";
      else return 0;
    }

    const char *subtype( int idx ) const
    {
      if ( idx == 0 ) return "calendar";
      if ( idx == 1 ) return "x-vcalendar";
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
