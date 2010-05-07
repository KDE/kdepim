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
#include <messageviewer/webkitparthtmlwriter.h>
#include <messageviewer/globalsettings.h>

#include <kcal/calendarlocal.h>
#ifndef KDEPIM_NO_KRESOURCES
#include <kcal/calendarresources.h>
#include <kcal/calhelper.h>
#endif
#include <kcal/icalformat.h>
#include <kcal/attendee.h>
#include <kcal/incidence.h>
#include <kcal/incidenceformatter.h>

#include <akonadi/kcal/groupware.h>
#include <incidenceeditors/groupwareintegration.h>
#include <kmail/kmcommands.h>

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

#ifndef KDEPIM_NO_KRESOURCES
class CalendarManager
{
  public:
    CalendarManager();
    ~CalendarManager();
    static KCal::CalendarResources* calendar();

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

KCal::CalendarResources * CalendarManager::calendar()
{
  K_GLOBAL_STATIC(CalendarManager, _self);
  return _self->mCalendar;
}
#endif


class KMInvitationFormatterHelper : public KCal::InvitationFormatterHelper
{
  public:
    KMInvitationFormatterHelper( MessageViewer::Interface::BodyPart *bodyPart ) : mBodyPart( bodyPart ) {}
    virtual QString generateLinkURL( const QString &id ) { return mBodyPart->makeLink( id ); }
#ifndef KDEPIM_NO_KRESOURCES
    KCal::Calendar* calendar() const { return CalendarManager::calendar(); }
#else
    KCal::Calendar* calendar() const { return 0; }
#endif
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
      KMime::Message::Ptr message = bodyPart->item().payload<KMime::Message::Ptr>();
      QString html = IncidenceFormatter::formatICalInvitationNoHtml( source, &cl, &helper /*TODO enable when r1121244 is merged , message->sender()->asUnicodeString() */);

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

    bool mailICal( const Akonadi::Item& item, const QString &receiver, const QString &to, const QString &iCal,
                   const QString &subject, const QString &status,
                   bool delMessage ) const
    {
      kDebug() << "Mailing message:" << iCal;

      KMime::Message::Ptr msg( new KMime::Message );
      if ( MessageViewer::GlobalSettings::self()->exchangeCompatibleInvitations() ) {
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
        msg->subject()->fromUnicodeString( subject, "utf-8" );
      }
      msg->to()->fromUnicodeString( to, "utf-8" );
      msg->from()->fromUnicodeString( receiver, "utf-8" );
      if ( !MessageViewer::GlobalSettings::self()->exchangeCompatibleInvitations() ) {
        msg->contentType()->from7BitString( "text/calendar; method=reply; charset=\"utf-8\"" );
        msg->setBody( iCal.toUtf8() );
      }
      KMime::Content *body = new KMime::Content;
      body->contentType()->setMimeType( "text/calendar" );
      body->setBody( iCal.toUtf8() );
      msg->addContent( body );

#if 0
      if ( delMessage &&  GlobalSettings::self()->deleteInvitationEmailsAfterSendingReply()() )
        /* We want the triggering mail to be moved to the trash once this one
         * has been sent successfully. Set a link header which accomplishes that. */
        msg->link( mMsg, MessageStatus::statusDeleted() );
#endif

      // Try and match the receiver with an identity.
      // Setting the identity here is important, as that is used to select the correct
      // transport later
      const KPIMIdentities::Identity &identity = KPIMIdentities::IdentityManager().identityForAddress( findReceiver( item.payload<KMime::Message::Ptr>().get() ) );
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

      // Outlook will only understand the reply if the From: header is the
      // same as the To: header of the invitation message.
      if ( !MessageViewer::GlobalSettings::self()->legacyMangleFromToHeaders() ) {
        if ( identity != KPIMIdentities::Identity::null() ) {
          msg->from()->fromUnicodeString( identity.fullEmailAddr(), "utf-8" );
        }
        // Remove BCC from identity on ical invitations (https://intevation.de/roundup/kolab/issue474)
        msg->bcc()->clear();;
      }

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
      job->addressAttribute().setTo( QStringList() << to );
      job->transportAttribute().setTransportId( MailTransport::TransportManager::self()->defaultTransportId() );
      if( ! job->exec() ) {
        kWarning() << "Error queuing message in outbox:" << job->errorText();
        return false;
      }
#endif

      return true;
    }

    bool mail( const Akonadi::Item &item, Incidence* incidence, const QString &status,
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
        KMime::Message::Ptr message = item.payload<KMime::Message::Ptr>();
        KPIMUtils::extractEmailAddressAndName( message->sender()->asUnicodeString(), temail, tname );
        incidence->setOrganizer( Person( tname, temail ) );
      }

      QString recv = to;
      if ( recv.isEmpty() )
        recv = incidence->organizer().fullName();
      return mailICal( item, receiver, recv, msg, subject, status, type != Forward );
    }

    void ensureKorganizerRunning( bool switchTo ) const
    {
      QString error;
      QString dbusService;
      int result = KDBusServiceStarter::self()->findServiceFor( "DBUS/Organizer", QString(), &error, &dbusService );
      if ( result == 0 ) {
        // OK, so korganizer (or kontact) is running. Now ensure the object we want is available
        // [that's not the case when kontact was already running, but korganizer not loaded into it...]
        QDBusInterface iface( "org.kde.korganizer", "/korganizer_PimApplication", "org.kde.KUniqueApplication" );
        if ( iface.isValid() ) {
          if ( switchTo ) {
            iface.call( "newInstance" ); // activate korganizer window
          }
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
      if ( !IncidenceEditors::GroupwareIntegration::isActive() ) {
        IncidenceEditors::GroupwareIntegration::activate();
      }
      return Akonadi::Groupware::instance()->handleInvitation( receiver, iCal, type );
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

      if ( askForComment( status ) ) {
        bool ok = false;
        QString comment = KInputDialog::getMultiLineText( i18n("Reaction to Invitation"),
            i18n("Comment:"), QString(), &ok );
        if ( !ok )
          return true;
        if ( !comment.isEmpty() ) {
          if ( MessageViewer::GlobalSettings::self()->outlookCompatibleInvitationReplyComments() ) {
            incidence->setDescription( comment );
          } else {
            incidence->addComment( comment );
          }
        }
      }

      // First, save it for KOrganizer to handle
      QString dir = directoryForStatus( status );
      if ( dir.isEmpty() ) {
        return true; // unknown status
      }
      if ( status != Attendee::Delegated ) {
        // we do that below for delegated incidences
        saveFile( receiver, iCal, dir );
      }

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
        ok =  mail( part->item(), incidence, dir, iTIPReply, receiver );

        // check if we need to inform our delegator about this as well
        if ( newMyself && (status == Attendee::Accepted || status == Attendee::Declined) && !delegator.isEmpty() ) {
          if ( delegatorRSVP || status == Attendee::Declined )
            ok = mail( part->item(), incidence, dir, iTIPReply, receiver, delegator );
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
          ok = mail( part->item(), incidence, dir, iTIPReply, receiver );
        }
      } else {
#if 0 // TODO: port to Akonadi
        if ( MessageViewer::GlobalSettings::self()->deleteInvitationEmailsAfterSendingReply() )
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

        ok = mail( part->item(), incidence, dir, iTIPRequest, receiver, delegateString, Delegation );
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
        file->setAutoRemove( false );
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
        file->setAutoRemove( false );
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
      ensureKorganizerRunning( true );
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
      if ( askForComment( Attendee::Declined ) ) {
        bool ok = false;
        QString comment = KInputDialog::getMultiLineText( i18n("Decline Counter Proposal"),
            i18n("Comment:"), QString(), &ok );
        if ( !ok )
          return true;
        if ( !comment.isEmpty() ) {
          if ( MessageViewer::GlobalSettings::self()->outlookCompatibleInvitationReplyComments() ) {
            incidence->setDescription( comment );
          } else {
            incidence->addComment( comment );
          }
        }
      }
      return mail( part->item(), incidence, "declinecounter", KCal::iTIPDeclineCounter,
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

    bool handleClick( MessageViewer::Interface::BodyPart *part,
                      const QString &path ) const
    {
#ifndef KDEPIM_NO_KRESOURCES
      if ( !CalHelper::hasMyWritableEventsFolders( "calendar" ) ) {
        KMessageBox::error(
          0,
          i18n( "You have no writable calendar folders for invitations, "
                "so storing or saving a response will not be possible.\n"
                "Please create at least 1 writable events calendar and re-sync." ) );
        return false;
      }
#endif

      Incidence *incidence;
      QString iCal;
      QString summary;
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
        incidence = icalToString( iCal );
        AttendeeSelector dlg;
        if ( dlg.exec() == QDialog::Rejected )
          return true;
        QString fwdTo = dlg.attendees().join( ", " );
        if ( fwdTo.isEmpty() )
          return true;
        result = mail( part->item(), incidence, "forward", iTIPRequest, receiver, fwdTo, Forward );
      }
      if ( path == "check_calendar" ) {
        incidence = icalToString( iCal );
        showCalendar( incidence->dtStart().date() );
        return true;
      }
      if ( path == "reply" || path == "cancel" || path == "accept_counter" ) {
        // These should just be saved with their type as the dir
        const QString p = (path == "accept_counter" ? QString("reply") : path);
        if ( saveFile( "Receiver Not Searched", iCal, p ) ) {
#if 0 // TODO port to Akonadi
          if ( MessageViewer::GlobalSettings::self()->deleteInvitationEmailsAfterSendingReply() )
            c.deleteInvitation();
#else
          kDebug() << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif
          result = true;
        }
      }
      if ( path == "record" ) {
        incidence = icalToString( iCal );

        int response =
          KMessageBox::questionYesNoCancel(
          0,
          i18nc( "@info",
                 "The organizer is not expecting a reply to this invitation "
                 "but you can send them an email message if you desire.\n\n"
                 "Would you like to send the organizer a message regarding this invitation?\n"
                 "Press the [Cancel] button to cancel the recording operation." ),
          i18nc( "@title:window", "Send Email to Organizer" ),
          KGuiItem( i18n( "Do Not Send" ) ),
          KGuiItem( i18n( "Send EMail" ) ) );

        switch( response ) {
        case KMessageBox::Cancel:
          break;
        case KMessageBox::No: // means "send email"
          summary = incidence->summary();
          if ( !summary.isEmpty() ) {
            summary = i18n( "Re: %1", summary );
          }

          KToolInvocation::invokeMailer( incidence->organizer().email(), summary );
          //fall through
        case KMessageBox::Yes: // means "do not send"
          if ( saveFile( "Receiver Not Searched", iCal, QString( "reply" ) ) ) {
#if 0 // TODO port to Akonadi
            if ( MessageViewer::GlobalSettings::self()->deleteInvitationEmailsAfterSendingReply() ) {
              ( new KMTrashMsgCommand( c.getMsg()->getMsgSerNum() ) )->start();
              result = true;
            }
#endif
          }
          showCalendar( incidence->dtStart().date() );
          break;
        }
      }

#if 0 // TODO port to Akonadi
      if ( path == "delete" ) {
        ( new KMTrashMsgCommand( c.getMsg()->getMsgSerNum() ) )->start();
        result = true;
      }
#else
      kWarning() << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif

      if ( path.startsWith( QLatin1String( "ATTACH:" ) ) ) {
        QString name = path;
        name.remove( QRegExp( "^ATTACH:" ) );
        result = openAttachment( name, iCal );
      }

#if 0 // TODO port to Akonadi
      if ( result ) {
         // do not close the secondary window if an attachment was opened (kolab/issue4317)
         if ( !path.startsWith( "ATTACH:" ) ) {
             c.closeIfSecondaryWindow();
        }
      }
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
      delete menu;
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
        if ( path.startsWith( QLatin1String( "ATTACH:" ) ) ) {
          QString name = path;
          return i18n( "Open attachment \"%1\"",
                       name.remove( QRegExp( "^ATTACH:" ) ) );
        }
      }

      return QString();
    }

    bool askForComment( KCal::Attendee::PartStat status ) const
    {
      if ( ( status != KCal::Attendee::Accepted
              && MessageViewer::GlobalSettings::self()->askForCommentWhenReactingToInvitation()
              == MessageViewer::GlobalSettings::EnumAskForCommentWhenReactingToInvitation::AskForAllButAcceptance )
          || MessageViewer::GlobalSettings::self()->askForCommentWhenReactingToInvitation()
          == MessageViewer::GlobalSettings::EnumAskForCommentWhenReactingToInvitation::AlwaysAsk )
          return true;
      return false;
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
