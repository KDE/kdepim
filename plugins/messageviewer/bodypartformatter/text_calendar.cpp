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

#include <messageviewer/interfaces/bodypartformatter.h>
#include <messageviewer/interfaces/bodypart.h>
#include <messageviewer/interfaces/bodyparturlhandler.h>
#include <messageviewer/khtmlparthtmlwriter.h>

#include <kcal/calendarlocal.h>
#include <kcal/calendarresources.h>
#include <kcal/icalformat.h>
#include <kcal/attendee.h>
#include <kcal/incidence.h>
#include <kcal/incidenceformatter.h>

#include <kpimutils/email.h>
#include <kpimidentities/identity.h>
#include <kpimidentities/identitymanager.h>

#include <kmime/kmime_content.h>
#include <kmime/kmime_message.h>
#include <mailtransport/messagequeuejob.h>
#include <mailtransport/transportmanager.h>

#include <kglobal.h>
#include <kfiledialog.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kstringhandler.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kdbusservicestarter.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ksystemtimezone.h>
#include <ktemporaryfile.h>
#include <kmimetype.h>
#include <kmenu.h>
#include <krun.h>
#include <ktoolinvocation.h>
#include <kio/netaccess.h>

#include <QUrl>
#include <QDir>
#include <QTextStream>

#include <kdemacros.h>
#include "calendarinterface.h"

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
};

CalendarManager::CalendarManager()
{
  mCalendar = new CalendarResources( KSystemTimeZones::local() );
  mCalendar->readConfig();
  mCalendar->load();
  bool multipleKolabResources = false;
  CalendarResourceManager *mgr = mCalendar->resourceManager();
  for ( CalendarResourceManager::ActiveIterator it = mgr->activeBegin(); it != mgr->activeEnd(); ++it ) {
    if ( (*it)->type() == "imap" || (*it)->type() == "kolab" ) {
      const QStringList subResources = (*it)->subresources();
      QSet<QString> prefixSet;
      for ( QStringList::ConstIterator subIt = subResources.constBegin(); subIt != subResources.constEnd(); ++subIt ) {
        if ( !(*subIt).contains( "/.INBOX.directory/" ) )
          // we don't care about shared folders
          continue;
        prefixSet.insert( (*subIt).left( (*subIt).indexOf( "/.INBOX.directory/" ) ) );
      }
      if ( prefixSet.count() > 1 )
        multipleKolabResources = true;
    }
  }
  if ( multipleKolabResources ) {
    kDebug() << "disabling calendar lookup because multiple active Kolab resources";
    delete mCalendar;
    mCalendar = 0;
  }
}

CalendarManager::~CalendarManager()
{
  delete mCalendar;
}

KCal::Calendar* CalendarManager::calendar()
{
  K_GLOBAL_STATIC(CalendarManager, _self);
  return _self->mCalendar;
}


class KMInvitationFormatterHelper : public KCal::InvitationFormatterHelper
{
  public:
    KMInvitationFormatterHelper( MessageViewer::Interface::BodyPart *bodyPart ) : mBodyPart( bodyPart ) {}
    virtual QString generateLinkURL( const QString &id ) { return mBodyPart->makeLink( id ); }
    KCal::Calendar* calendar() const { return CalendarManager::calendar(); }
  private:
    MessageViewer::Interface::BodyPart *mBodyPart;
};

class Formatter : public MessageViewer::Interface::BodyPartFormatter
{
  public:
    Result format( MessageViewer::Interface::BodyPart *bodyPart,
                   MessageViewer::HtmlWriter *writer ) const
    {
      if ( !writer )
        // Guard against crashes in createReply()
        return Ok;
      CalendarLocal cl( KSystemTimeZones::local() );
      KMInvitationFormatterHelper helper( bodyPart );
      QString source;
      /* If the bodypart does not have a charset specified, we need to fall
         back to utf8, not the KMail fallback encoding, so get the contents
         as binary and decode explicitly. */
      if ( bodyPart->contentTypeParameter( "charset").isEmpty() ) {
        const QByteArray &ba = bodyPart->asBinary();
        source = QString::fromUtf8(ba);
      } else {
        source = bodyPart->asText();
      }
      QString html = IncidenceFormatter::formatICalInvitationNoHtml( source, &cl, &helper );

      if ( html.isEmpty() ) return AsIcon;
      writer->queue( html );

      return Ok;
    }
};

static Incidence *icalToString( const QString& iCal )
{
  CalendarLocal calendar( KSystemTimeZones::local() ) ;
  ICalFormat format;
  ScheduleMessage *message =
    format.parseScheduleMessage( &calendar, iCal );
  if ( !message )
    //TODO: Error message?
    return 0;
  return dynamic_cast<Incidence*>( message->event() );
}

class UrlHandler : public MessageViewer::Interface::BodyPartURLHandler
{
  public:
    UrlHandler()
    {
      kDebug() <<"UrlHandler() (iCalendar)";
    }

    Attendee *findMyself( Incidence* incidence, const QString& receiver ) const
    {
      Attendee::List attendees = incidence->attendees();
      Attendee::List::ConstIterator it;
      Attendee* myself = 0;
      // Find myself. There will always be all attendees listed, even if
      // only I need to answer it.
      for ( it = attendees.constBegin(); it != attendees.constEnd(); ++it ) {
        // match only the email part, not the name
        if( KPIMUtils::compareEmail( (*it)->email(), receiver, false ) ) {
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
      for ( it = attendees.constBegin(); it != attendees.constEnd(); ++it ) {
        if ( it == attendees.constBegin() ) {
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
      for ( it = attendees.constBegin(); it != attendees.constEnd(); ++it ) {
        if ( it == attendees.constBegin() ) {
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

    static Attachment *findAttachment( const QString &name, const QString &iCal )
    {
      Incidence *incidence = icalToString( iCal );

      // get the attachment by name from the incidence
      Attachment::List as = incidence->attachments();
      Attachment *a = 0;
      if ( as.count() > 0 ) {
        Attachment::List::ConstIterator it;
        for ( it = as.constBegin(); it != as.constEnd(); ++it ) {
          if ( (*it)->label() == name ) {
            a = *it;
            break;
          }
        }
      }

      if ( !a ) {
        KMessageBox::error(
          0,
          i18n("No attachment named \"%1\" found in the invitation.", name ) );
        return 0;
      }

      if ( a->isUri() ) {
        if ( !KIO::NetAccess::exists( a->uri(), KIO::NetAccess::SourceSide, 0 ) ) {
          KMessageBox::information(
            0,
            i18n( "The invitation attachment \"%1\" is a web link that "
                  "is inaccessible from this computer. Please ask the event "
                  "organizer to resend the invitation with this attachment "
                  "stored inline instead of a link.",
                  KUrl::fromPercentEncoding( a->uri().toLatin1() ) ) );
          return 0;
        }
      }
      return a;
    }

    static QString findReceiver( KMime::Content *node )
    {
      if ( !node || !node->topLevel() )
        return QString();

      QString receiver;
      KPIMIdentities::IdentityManager* im = new KPIMIdentities::IdentityManager( true );

      KMime::Types::Mailbox::List addrs;
      if ( node->topLevel()->header<KMime::Headers::To>() )
        addrs = node->topLevel()->header<KMime::Headers::To>()->mailboxes();
      int found = 0;
      for ( QList< KMime::Types::Mailbox >::const_iterator it = addrs.constBegin(); it != addrs.constEnd(); ++it ) {
        if ( im->identityForAddress( (*it).address() ) != KPIMIdentities::Identity::null() ) {
          // Ok, this could be us
          ++found;
          receiver = (*it).address();
        }
      }

      KMime::Types::Mailbox::List ccaddrs;
      if ( node->topLevel()->header<KMime::Headers::Cc>() )
        ccaddrs = node->topLevel()->header<KMime::Headers::Cc>()->mailboxes();
      for ( QList< KMime::Types::Mailbox >::const_iterator it = ccaddrs.constBegin(); it != ccaddrs.constEnd(); ++it ) {
        if ( im->identityForAddress( (*it).address() ) != KPIMIdentities::Identity::null() ) {
          // Ok, this could be us
          ++found;
          receiver = (*it).address();
        }
      }

      if ( found != 1 ) {
        QStringList possibleAddrs;
        bool ok;
        QString selectMessage;
        if ( found == 0 ) {
          selectMessage = i18n("<qt>None of your identities match the "
                              "receiver of this message,<br />please "
                              "choose which of the following addresses "
                              "is yours, if any, or select one of your "
                              "identities to use in the reply:</qt>");
          possibleAddrs += im->allEmails();
        } else {
          selectMessage = i18n("<qt>Several of your identities match the "
                              "receiver of this message,<br />please "
                              "choose which of the following addresses "
                              "is yours:</qt>");
          foreach ( const KMime::Types::Mailbox &mbx, addrs )
            possibleAddrs.append( mbx.address() );
          foreach ( const KMime::Types::Mailbox &mbx, ccaddrs )
            possibleAddrs.append( mbx.address() );
        }

        // select default identity by default
        const QString defaultAddr = im->defaultIdentity().emailAddr();
        const int defaultIndex = qMax( 0, possibleAddrs.indexOf( defaultAddr ) );

        receiver = KInputDialog::getItem(
          i18n( "Select Address" ),
          selectMessage,
          possibleAddrs, defaultIndex, false, &ok, 0 );
        if ( !ok ) {
          receiver.clear();
        }
      }

      return receiver;
    }

    Attendee* setStatusOnMyself( Incidence* incidence, Attendee* myself,
                            Attendee::PartStat status, const QString &receiver ) const
    {
      Attendee* newMyself = 0;
      QString name;
      QString email;
      KPIMUtils::extractEmailAddressAndName( receiver, email, name );
      if ( name.isEmpty() && myself ) name = myself->name();
      if ( email.isEmpty()&& myself ) email = myself->email();
      Q_ASSERT( !email.isEmpty() ); // delivery must be possible
      newMyself = new Attendee( name,
                                email,
                                true, // RSVP, otherwise we would not be here
                                status,
                                myself ? myself->role() : heuristicalRole( incidence ),
                                myself ? myself->uid() : QString::null );	//krazy:exclude=nullstrassign for old broken gcc
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

    bool mailICal( const QString &receiver, const QString &to, const QString &iCal,
                   const QString &subject, const QString &status,
                   bool delMessage ) const
    {
      kDebug() << "Mailing message:" << iCal;

      KMime::Message::Ptr msg( new KMime::Message );
#if 0 //TODO finish porting
      if ( GlobalSettings::self()->exchangeCompatibleInvitations() ) {
        msg->subject()->fromUnicodeString( status, "utf-8" );
        QString tsubject = subject;
        tsubject.remove( i18n( "Answer: " ) );
        if ( status == QLatin1String( "cancel" ) ) {
          msg->subject()->fromUnicodeString( i18nc( "Not able to attend.", "Declined: %1", tsubject ), "utf-8" );
        } else if ( status == QLatin1String("tentative") ) {
          msg->subject()->fromUnicodeString( i18nc( "Unsure if it is possible to attend.", "Tentative: %1", tsubject ), "utf-8" );
        } else if ( status == QLatin1String("accepted") ) {
          msg->subject()->fromUnicodeString( i18nc( "Accepted the invitation.", "Accepted: %1", tsubject ), "utf-8" );
        } else {
          msg->subject()->fromUnicodeString( subject, "utf-8" );
        }
      } else {
#endif
        msg->subject()->fromUnicodeString( subject, "utf-8" );
#if 0
      }
#endif
      msg->to()->fromUnicodeString( to, "utf-8" );
      msg->from()->fromUnicodeString( receiver, "utf-8" );
#if 0
      if ( !GlobalSettings::self()->exchangeCompatibleInvitations() ) {
        msg->setHeaderField( "Content-Type",
                             "text/calendar; method=reply; charset=\"utf-8\"" );
        msg->setBody( iCal.toUtf8() );
      }
#else
      KMime::Content *body = new KMime::Content;
      body->contentType()->setMimeType( "text/calendar" );
      body->setBody( iCal.toUtf8() );
      msg->addContent( body );
#endif

#if 0
      if ( delMessage && deleteInvitationAfterReply() )
        /* We want the triggering mail to be moved to the trash once this one
         * has been sent successfully. Set a link header which accomplishes that. */
        msg->link( mMsg, MessageStatus::statusDeleted() );
#endif

      // Try and match the receiver with an identity.
      // Setting the identity here is important, as that is used to select the correct
      // transport later
#if 0
      const KPIMIdentities::Identity &identity = KPIMIdentities::IdentityManager().identityForAddress( receiver() );
#else
      const KPIMIdentities::Identity &identity = KPIMIdentities::IdentityManager().defaultIdentity();
#endif
      const bool nullIdentity = ( identity == KPIMIdentities::Identity::null() );
      if ( !nullIdentity ) {
        KMime::Headers::Generic *x_header = new KMime::Headers::Generic(
          "X-KMail-Identity", msg.get(), QByteArray::number( identity.uoid() )
        );
        msg->setHeader( x_header );
      }

#if 0 // Shouldn't be necessary anymore
      const bool identityHasTransport = !identity.transport().isEmpty();
      if ( !nullIdentity && identityHasTransport )
        msg->setHeaderField( "X-KMail-Transport", identity.transport() );
      else if ( !nullIdentity && identity.isDefault() )
        msg->setHeaderField( "X-KMail-Transport", TransportManager::self()->defaultTransportName() );
      else {
        const QString transport = askForTransport( nullIdentity );
        if ( transport.isEmpty() )
          return false; // user canceled transport selection dialog
        msg->setHeaderField( "X-KMail-Transport", transport );
      }
#endif

#if 0 //TODO: finish port
      // Outlook will only understand the reply if the From: header is the
      // same as the To: header of the invitation message.
      if ( !GlobalSettings::self()->legacyMangleFromToHeaders() ) {
        if ( identity != KPIMIdentities::Identity::null() ) {
          msg->setFrom( identity.fullEmailAddr() );
        }
        // Remove BCC from identity on ical invitations (https://intevation.de/roundup/kolab/issue474)
        msg->setBcc( "" );
      }
#endif

#if 0 // For now assume automatic sending
      KMail::Composer *cWin = KMail::makeComposer();
      cWin->ignoreStickyFields();
      cWin->setMsg( msg, false /* mayAutoSign */ );
      // cWin->setCharset( "", true );
      cWin->disableWordWrap();
      cWin->setSigningAndEncryptionDisabled( true );
      if ( GlobalSettings::self()->exchangeCompatibleInvitations() ) {
        // For Exchange, send ical as attachment, with proper
        // parameters
        msg->setSubject( status );
        msg->setCharset( "utf-8" );
        KMMessagePart *msgPart = new KMMessagePart;
        msgPart->setName( "cal.ics" );
        // msgPart->setCteStr( attachCte ); // "base64" ?
        msgPart->setBodyEncoded( iCal.toUtf8() );
        msgPart->setTypeStr( "text" );
        msgPart->setSubtypeStr( "calendar" );
        msgPart->setParameter( "method", "reply" );
        cWin->addAttach( msgPart );
      }

      cWin->forceDisableHtml();
      cWin->disableRecipientNumberCheck();
      cWin->disableForgottenAttachmentsCheck();
      if ( GlobalSettings::self()->automaticSending() ) {
        cWin->setAttribute( Qt::WA_DeleteOnClose );
        cWin->slotSendNow();
      } else {
        cWin->show();
      }
#else
      msg->assemble();

      MailTransport::MessageQueueJob *job = new MailTransport::MessageQueueJob;
      job->setMessage( msg );
      job->setTo( QStringList() << to );
      job->setTransportId( MailTransport::TransportManager::self()->defaultTransportId() );
      if( ! job->exec() ) {
        kWarning() << "Error queuing message in outbox:" << job->errorText();
        return false;
      }
#endif

      return true;
    }

    bool mail( Incidence* incidence, /* TODO: port me!  MessageViewer::Callback& callback,*/ const QString &status,
               iTIPMethod method = iTIPReply, const QString &receiver = QString(), const QString &to = QString(),
               MailType type = Answer ) const
    {
      //status is accepted/tentative/declined
      ICalFormat format;
      format.setTimeSpec( KSystemTimeZones::local() );
      QString msg = format.createScheduleMessage( incidence, method );
      QString summary = incidence->summary();
      if ( summary.isEmpty() )
        summary = i18n( "Incidence with no summary" );
      QString subject;
      switch ( type ) {
        case Answer:
          subject = i18n( "Answer: %1" , summary );
          break;
        case Delegation:
          subject = i18n( "Delegated: %1", summary );
          break;
        case Forward:
          subject = i18n( "Forwarded: %1", summary );
          break;
        case DeclineCounter:
          subject = i18n( "Declined Counter Proposal: %1", summary );
          break;
      }

      // Set the organizer to the sender, if the ORGANIZER hasn't been set.
      if ( incidence->organizer().isEmpty() ) {
        QString tname, temail;
        KPIMUtils::extractEmailAddressAndName( /* TODO: port me! callback.sender()*/ QString(), temail, tname );
        incidence->setOrganizer( Person( tname, temail ) );
      }

      QString recv = to;
      if ( recv.isEmpty() )
        recv = incidence->organizer().fullName();
      return mailICal( receiver, recv, msg, subject, status, type != Forward );
    }

    void ensureKorganizerRunning() const
    {
      QString error;
      QString dbusService;
      int result = KDBusServiceStarter::self()->findServiceFor( "DBUS/Organizer", QString(), &error, &dbusService );
      if ( result == 0 ) {
        // OK, so korganizer (or kontact) is running. Now ensure the object we want is available
        // [that's not the case when kontact was already running, but korganizer not loaded into it...]
        QDBusInterface iface( "org.kde.korganizer", "/korganizer_PimApplication", "org.kde.KUniqueApplication" );
        if ( iface.isValid() ) {
          iface.call( "newInstance" );
          QDBusReply<bool> r = iface.call( "load" );
          if ( !r.isValid() || !r.value() ) {
            kWarning() << "Loading korganizer failed: " << iface.lastError().message();
          }
        } else {
          kWarning() << "Couldn't obtain korganizer D-Bus interface" << iface.lastError().message();
        }

        // We don't do anything with it, we just need it to be running so that it handles
        // the incoming directory.
      }
      else
        kWarning() <<"Couldn't start DBUS/Organizer:" << dbusService << error;
    }

    bool saveFile( const QString& receiver, const QString& iCal,
                   const QString& type ) const
    {
      kDebug() << receiver << iCal << type;
      KTemporaryFile file;
      file.setPrefix(KStandardDirs::locateLocal( "data", "korganizer/income." + type + '/', true));
      file.setAutoRemove(false);
      if ( !file.open() ) {
        KMessageBox::error( 0, i18n("Could not save file to KOrganizer") );
        return false;
      }
      QTextStream ts ( &file );
      ts.setCodec("UTF-8");
      ts << receiver << '\n' << iCal;
      ts.flush();
      file.flush();
      file.close();

      // Now ensure that korganizer is running; otherwise start it, to prevent surprises
      // (https://intevation.de/roundup/kolab/issue758)
      ensureKorganizerRunning();

      return true;
    }

    bool handleInvitation( const QString& iCal, Attendee::PartStat status,
                           MessageViewer::Interface::BodyPart *part ) const
    {
      bool ok = true;
      const QString receiver = findReceiver( part->content() );
      kDebug() << receiver;

      if ( receiver.isEmpty() )
        // Must be some error. Still return true though, since we did handle it
        return true;

      // get comment for tentative acceptance
      Incidence* incidence = icalToString( iCal );

      if ( /* TODO: port me! callback.askForComment( status )*/ false ) {
        bool ok = false;
        QString comment = KInputDialog::getMultiLineText( i18n("Reaction to Invitation"),
            i18n("Comment:"), QString(), &ok );
        if ( !ok )
          return true;
        if ( !comment.isEmpty() ) {
          if ( /* TODO: port me! callback.outlookCompatibleInvitationReplyComments() */ false ) {
            incidence->setDescription( comment );
          } else {
            incidence->addComment( comment );
          }
        }
      }

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
      bool delegatorRSVP = false;
      if ( status == Attendee::Delegated ) {
        DelegateSelector dlg;
        if ( dlg.exec() == QDialog::Rejected )
          return true;
        delegateString = dlg.delegate();
        delegatorRSVP = dlg.rsvp();
        if ( delegateString.isEmpty() )
          return true;
        if ( KPIMUtils::compareEmail( delegateString, incidence->organizer().email(), false ) ) {
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
          if( KPIMUtils::compareEmail( (*it)->fullName(), myself->delegator(), false ) && (*it)->status() == Attendee::Delegated ) {
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
        ok =  mail( incidence, /* TODO: port me! callback,*/ dir, iTIPReply, receiver );

        // check if we need to inform our delegator about this as well
        if ( newMyself && (status == Attendee::Accepted || status == Attendee::Declined) && !delegator.isEmpty() ) {
          if ( delegatorRSVP || status == Attendee::Declined )
            ok = mail( incidence, /* TODO: port me! callback,*/ dir, iTIPReply, receiver, delegator );
        }

      } else if ( !myself && (status != Attendee::Declined) ) {
        // forwarded invitation
        Attendee* newMyself = 0;
        QString name;
        QString email;
        KPIMUtils::extractEmailAddressAndName( receiver, email, name );
        if ( !email.isEmpty() ) {
          newMyself = new Attendee( name,
                                    email,
                                    true, // RSVP, otherwise we would not be here
                                    status,
                                    heuristicalRole( incidence ),
                                    QString() );
          incidence->clearAttendees();
          incidence->addAttendee( newMyself );
          ok = mail( incidence, /* TODO: port me! callback,*/ dir, iTIPReply, receiver );
        }
      } else {
#if 0 // TODO: port to Akonadi
        if ( callback.deleteInvitationAfterReply() )
          callback.deleteInvitation();
#else
        kDebug() << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif

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
        KPIMUtils::extractEmailAddressAndName( delegateString, email, name );
        Attendee *delegate = new Attendee( name, email, true );
        delegate->setDelegator( receiver );
        incidence->addAttendee( delegate );

        ICalFormat format;
        format.setTimeSpec( KSystemTimeZones::local() );
        QString iCal = format.createScheduleMessage( incidence, iTIPRequest );
        saveFile( receiver, iCal, dir );

        ok = mail( incidence, /* TODO: port me! callback,*/ dir, iTIPRequest, receiver, delegateString, Delegation );
      }
      return ok;
    }

    bool openAttachment( const QString &name, const QString &iCal ) const
    {
      Attachment *a = findAttachment( name, iCal );
      if ( !a ) {
        return false;
      }

      if ( a->isUri() ) {
        KToolInvocation::invokeBrowser( a->uri() );
      } else {
        // put the attachment in a temporary file and launch it
        KTemporaryFile *file = new KTemporaryFile();
        QStringList patterns = KMimeType::mimeType( a->mimeType() )->patterns();
        if ( !patterns.empty() ) {
          file->setSuffix( QString( patterns.first() ).remove( '*' ) );
        }
        file->open();
        file->setPermissions( QFile::ReadUser );
        file->write( QByteArray::fromBase64( a->data() ) );
        file->close();

        bool stat = KRun::runUrl( KUrl( file->fileName() ), a->mimeType(), 0, true );
        delete file;
        return stat;
      }
      return true;
    }

    bool saveAsAttachment( const QString &name, const QString &iCal ) const
    {
      Attachment *a = findAttachment( name, iCal );
      if ( !a ) {
        return false;
      }

      // get the saveas file name
      QString saveAsFile =
        KFileDialog::getSaveFileName( name,
                                      QString(), 0,
                                      i18n( "Save Invitation Attachment" ));

      if ( saveAsFile.isEmpty() ||
           ( QFile( saveAsFile ).exists() &&
             ( KMessageBox::warningContinueCancel(
               0,
               i18nc( "@info",
                      "File <filename>%1</filename> exists.<nl/> Do you want to replace it?",
                      saveAsFile ) ) != KMessageBox::Continue ) ) ) {
        return false;
      }

      bool stat = false;
      if ( a->isUri() ) {
        // save the attachment url
        stat = KIO::NetAccess::file_copy( a->uri(), KUrl( saveAsFile ) );
      } else {
        // put the attachment in a temporary file and save it
        KTemporaryFile *file = new KTemporaryFile();
        QStringList patterns = KMimeType::mimeType( a->mimeType() )->patterns();
        if ( !patterns.empty() ) {
          file->setSuffix( QString( patterns.first() ).remove( '*' ) );
        }
        file->open();
        file->setPermissions( QFile::ReadUser );
        file->write( QByteArray::fromBase64( a->data() ) );
        file->close();

        stat = KIO::NetAccess::file_copy( KUrl( file->fileName() ), KUrl( saveAsFile ) );

        delete file;
      }
      return stat;
    }

    void showCalendar( const QDate &date ) const
    {
      ensureKorganizerRunning();
      QDBusInterface *kontact = new QDBusInterface( "org.kde.kontact", "/KontactInterface", "org.kde.kontact.KontactInterface", QDBusConnection::sessionBus() );
      if ( kontact->isValid() )
        kontact->call( "selectPlugin", "kontact_korganizerplugin" );
      delete kontact;

      OrgKdeKorganizerCalendarInterface *iface = new OrgKdeKorganizerCalendarInterface( "org.kde.korganizer", "/Calendar", QDBusConnection::sessionBus(), 0 );
      iface->showEventView();
      iface->showDate( date );
      delete iface;
    }

    bool handleIgnore( const QString& /* TODO port me! , MessageViewer::Callback& c */ ) const
    {
#if 0 // TODO port to Akonadi
      // simply move the message to trash
      c.deleteInvitation();
#else
      kDebug() << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif
      return true;
    }

    bool handleDeclineCounter( const QString &iCal, MessageViewer::Interface::BodyPart* part ) const
    {
      const QString receiver = findReceiver( part->content() );
      if ( receiver.isEmpty() )
        return true;
      Incidence* incidence = icalToString( iCal );
      if ( /* TODO: port me! callback.askForComment( Attendee::Declined )*/ true ) {
        bool ok = false;
        QString comment = KInputDialog::getMultiLineText( i18n("Decline Counter Proposal"),
            i18n("Comment:"), QString(), &ok );
        if ( !ok )
          return true;
        if ( !comment.isEmpty() ) {
          if ( /* TODO: port me! callback.outlookCompatibleInvitationReplyComments()*/ false ) {
            incidence->setDescription( comment );
          } else {
            incidence->addComment( comment );
          }
        }
      }
      return mail( incidence, /* TODO: port me! callback,*/ "declinecounter", KCal::iTIPDeclineCounter,
                   receiver, QString(), DeclineCounter );
    }

    bool counterProposal( const QString &iCal, MessageViewer::Interface::BodyPart* part ) const
    {
      const QString receiver = findReceiver( part->content() );
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
        foreach( const QString& subResource, subResources ) {
          if ( !(*it)->subresourceActive( subResource ) )
            continue;
          return true;
        }
      }
      return false;
    }

    bool handleClick( MessageViewer::Interface::BodyPart *part,
                      const QString &path ) const
    {
      if ( !hasWritableCalendars() ) {
        KMessageBox::error( 0, i18n("No writable calendar found.") );
        return false;
      }

      QString iCal;
      /* If the bodypart does not have a charset specified, we need to fall back
         to utf8, not the KMail fallback encoding, so get the contents as binary
         and decode explicitly. */
      if ( part->contentTypeParameter( "charset").isEmpty() ) {
        const QByteArray &ba = part->asBinary();
        iCal = QString::fromUtf8(ba);
      } else {
        iCal = part->asText();
      }
      bool result = false;
      if ( path == "accept" )
        result = handleInvitation( iCal, Attendee::Accepted, part );
      if ( path == "accept_conditionally" )
        result = handleInvitation( iCal, Attendee::Tentative, part );
      if ( path == "counter" )
        result = counterProposal( iCal, part );
      if ( path == "ignore" )
        result = handleIgnore( iCal/*,TODO port me! c*/ );
      if ( path == "decline" )
        result = handleInvitation( iCal, Attendee::Declined, part );
      if ( path == "decline_counter" ) {
        result = handleDeclineCounter( iCal, part );
      }
      if ( path == "delegate" )
        result = handleInvitation( iCal, Attendee::Delegated, part );
      if ( path == "forward" ) {
        const QString receiver = findReceiver( part->content() );
        Incidence* incidence = icalToString( iCal );
        AttendeeSelector dlg;
        if ( dlg.exec() == QDialog::Rejected )
          return true;
        QString fwdTo = dlg.attendees().join( ", " );
        if ( fwdTo.isEmpty() )
          return true;
        result = mail( incidence, /*TODO port me! c,*/ "forward", iTIPRequest, receiver, fwdTo, Forward );
      }
      if ( path == "check_calendar" ) {
        Incidence* incidence = icalToString( iCal );
        showCalendar( incidence->dtStart().date() );
        return true;
      }
      if ( path == "reply" || path == "cancel" || path == "accept_counter" ) {
        // These should just be saved with their type as the dir
        const QString p = (path == "accept_counter" ? QString("reply") : path);
        if ( saveFile( "Receiver Not Searched", iCal, p ) ) {
#if 0 // TODO port to Akonadi
          if ( c.deleteInvitationAfterReply() )
            c.deleteInvitation();
#else
          kDebug() << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif
          result = true;
        }
      }
      if ( path.startsWith( QLatin1String( "ATTACH:" ) ) ) {
        QString name = path;
        name.remove( QRegExp( "^ATTACH:" ) );
        result = openAttachment( name, iCal );
      }

#if 0 // TODO port to Akonadi
      if ( result )
        c.closeIfSecondaryWindow();
#else
      kDebug() << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif
      return result;
    }

    bool handleContextMenuRequest( MessageViewer::Interface::BodyPart *part,
                                   const QString &path,
                                   const QPoint &point ) const
    {
      QString name = path;
      if ( path.startsWith( QLatin1String( "ATTACH:" ) ) ) {
        name.remove( QRegExp( "^ATTACH:" ) );
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

      KMenu *menu = new KMenu();
      QAction *open =
        menu->addAction( KIcon( "document-open" ), i18n( "Open Attachment" ) );
      QAction *saveas =
        menu->addAction( KIcon( "document-save-as" ), i18n( "Save Attachment As..." ) );

      QAction *a = menu->exec( point, 0 );
      if ( a == open ) {
        openAttachment( name, iCal );
      } else if ( a == saveas ) {
        saveAsAttachment( name, iCal );
      }
      return true;
    }

    QString statusBarMessage( MessageViewer::Interface::BodyPart *,
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
        if ( path.startsWith( QLatin1String( "ATTACH:" ) ) ) {
          QString name = path;
          return i18n( "Open attachment \"%1\"",
                       name.remove( QRegExp( "^ATTACH:" ) ) );
        }
      }

      return QString();
    }
};

class Plugin : public MessageViewer::Interface::BodyPartFormatterPlugin
{
  public:
    const MessageViewer::Interface::BodyPartFormatter *bodyPartFormatter( int idx ) const
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

    const MessageViewer::Interface::BodyPartURLHandler * urlHandler( int idx ) const
    {
      if ( idx == 0 ) return new UrlHandler();
      else return 0;
    }
};

}

extern "C"
KDE_EXPORT MessageViewer::Interface::BodyPartFormatterPlugin *
messageviewer_bodypartformatter_text_calendar_create_bodypart_formatter_plugin()
{
  KGlobal::locale()->insertCatalog( "messageviewer_text_calendar_plugin" );
  return new Plugin();
}
