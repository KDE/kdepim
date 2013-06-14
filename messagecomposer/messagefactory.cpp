/* -*- mode: C++; c-file-style: "gnu" -*-
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2010 Leo Franchi <lfranchi@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "messagefactory.h"

#include "messagecomposersettings.h"
#include "messagecomposer/utils/util.h"

#include <akonadi/item.h>
#ifndef QT_NO_CURSOR
#include <messageviewer/utils/kcursorsaver.h>
#endif
#include <kpimidentities/identitymanager.h>
#include <kpimidentities/identity.h>

#include <akonadi/kmime/messagestatus.h>
#include <kmime/kmime_dateformatter.h>
#include <KPIMUtils/Email>
#include <messagecore/mailinglist.h>
#include <messagecore/helpers/messagehelpers.h>
#include <messagecore/utils/stringutil.h>
#include "messagehelper.h"
#include "templateparser/templateparser.h"
#include <KLocalizedString>
#include <kcharsets.h>
#include <QTextCodec>

using namespace MessageComposer;

namespace KMime {
namespace Types {
static bool operator==( const KMime::Types::Mailbox &left, const KMime::Types::Mailbox &right )
{
  return (left.addrSpec().asString() == right.addrSpec().asString());
}
}
}

/**
 * Strips all the user's addresses from an address list. This is used
 * when replying.
 */
static KMime::Types::Mailbox::List stripMyAddressesFromAddressList( const KMime::Types::Mailbox::List& list, const KPIMIdentities::IdentityManager *manager )
{
  KMime::Types::Mailbox::List addresses( list );
  for ( KMime::Types::Mailbox::List::Iterator it = addresses.begin(); it != addresses.end(); ) {
    if ( manager->thatIsMe( MessageCore::StringUtil::mailboxListToUnicodeString( KMime::Types::Mailbox::List() << *it ) ) ) {
      it = addresses.erase( it );
    } else {
      ++it;
    }
  }

  return addresses;
}

MessageFactory::MessageFactory( const KMime::Message::Ptr& origMsg, Akonadi::Item::Id id, const Akonadi::Collection& col )
  : m_identityManager( 0 )
  , m_origMsg( origMsg )
  , m_origId( 0 )
  , m_parentFolderId( 0 )
  , m_collection( col )
  , m_replyStrategy( MessageComposer::ReplySmart )
  , m_quote( true )
  , m_allowDecryption( true )
  , m_id ( id )
{

}

MessageFactory::~MessageFactory()
{
}

MessageFactory::MessageReply MessageFactory::createReply()
{
  KMime::Message::Ptr msg( new KMime::Message );
  QString mailingListStr;
  QByteArray refStr, headerName;
  bool replyAll = true;
  KMime::Types::Mailbox::List toList;
  KMime::Types::Mailbox::List replyToList;

  const uint originalIdentity = identityUoid( m_origMsg );
  MessageHelper::initFromMessage( msg, m_origMsg, m_identityManager, originalIdentity );
  MessageCore::MailingList::name( m_origMsg, headerName, mailingListStr );
  replyToList = m_origMsg->replyTo()->mailboxes();

  msg->contentType()->setCharset( "utf-8" );

  if ( m_origMsg->headerByType( "List-Post" ) &&
       m_origMsg->headerByType( "List-Post" )->asUnicodeString().contains( QLatin1String( "mailto:" ), Qt::CaseInsensitive ) ) {

    const QString listPost = m_origMsg->headerByType( "List-Post" )->asUnicodeString();
    QRegExp rx( QLatin1String( "<mailto:([^@>]+)@([^>]+)>" ), Qt::CaseInsensitive );
    if ( rx.indexIn( listPost, 0 ) != -1 ) // matched
      m_mailingListAddresses << MessageCore::StringUtil::mailboxFromUnicodeString( rx.cap( 1 ) + QLatin1Char( '@' ) + rx.cap( 2 ) );
  }

  switch ( m_replyStrategy ) {
    case MessageComposer::ReplySmart:
      {
        if ( m_origMsg->headerByType( "Mail-Followup-To" ) ) {
          toList << MessageCore::StringUtil::mailboxListFrom7BitString( m_origMsg->headerByType( "Mail-Followup-To" )->as7BitString( false ) );
        } else if ( !replyToList.isEmpty() ) {
          toList = replyToList;
          // use the ReplyAll template only when it's a reply to a mailing list
          if ( m_mailingListAddresses.isEmpty() )
            replyAll = false;
        } else if ( !m_mailingListAddresses.isEmpty() ) {
          toList = (KMime::Types::Mailbox::List() << m_mailingListAddresses[ 0 ]);
        } else {

          // doesn't seem to be a mailing list, reply to From: address
          toList = m_origMsg->from()->mailboxes();

          if ( m_identityManager->thatIsMe( MessageCore::StringUtil::mailboxListToUnicodeString( toList ) ) ) {
            // sender seems to be one of our own identities, so we assume that this
            // is a reply to a "sent" mail where the users wants to add additional
            // information for the recipient.
            toList = m_origMsg->to()->mailboxes();
          }

          replyAll = false;
        }
        // strip all my addresses from the list of recipients
        const KMime::Types::Mailbox::List recipients = toList;

        toList = stripMyAddressesFromAddressList( recipients, m_identityManager );

        // ... unless the list contains only my addresses (reply to self)
        if ( toList.isEmpty() && !recipients.isEmpty() )
          toList << recipients.first();
      }
      break;
    case MessageComposer::ReplyList:
      {
        if ( m_origMsg->headerByType( "Mail-Followup-To" ) ) {
          toList << MessageCore::StringUtil::mailboxFrom7BitString( m_origMsg->headerByType( "Mail-Followup-To" )->as7BitString( false ) );
        } else if ( !m_mailingListAddresses.isEmpty() ) {
          toList << m_mailingListAddresses[ 0 ];
        } else if ( !replyToList.isEmpty() ) {
          // assume a Reply-To header mangling mailing list
          toList = replyToList;
        }

        // strip all my addresses from the list of recipients
        const KMime::Types::Mailbox::List recipients = toList;

        toList = stripMyAddressesFromAddressList( recipients, m_identityManager );
      }
      break;
    case MessageComposer::ReplyAll:
      {
        KMime::Types::Mailbox::List recipients;
        KMime::Types::Mailbox::List ccRecipients;

        // add addresses from the Reply-To header to the list of recipients
        if ( !replyToList.isEmpty() ) {
          recipients = replyToList;

          // strip all possible mailing list addresses from the list of Reply-To addresses
          foreach ( const KMime::Types::Mailbox &mailbox, m_mailingListAddresses ) {
            foreach ( const KMime::Types::Mailbox &recipient, recipients ) {
              if ( mailbox == recipient )
                recipients.removeAll( recipient );
            }
          }
        }

        if ( !m_mailingListAddresses.isEmpty() ) {
          // this is a mailing list message
          if ( recipients.isEmpty() && !m_origMsg->from()->asUnicodeString().isEmpty() ) {
            // The sender didn't set a Reply-to address, so we add the From
            // address to the list of CC recipients.
            ccRecipients += m_origMsg->from()->mailboxes();
            kDebug() << "Added" << m_origMsg->from()->asUnicodeString() << "to the list of CC recipients";
          }

          // if it is a mailing list, add the posting address
          recipients.prepend( m_mailingListAddresses[ 0 ] );
        } else {
          // this is a normal message
          if ( recipients.isEmpty() && !m_origMsg->from()->asUnicodeString().isEmpty() ) {
            // in case of replying to a normal message only then add the From
            // address to the list of recipients if there was no Reply-to address
            recipients += m_origMsg->from()->mailboxes();
            kDebug() << "Added" << m_origMsg->from()->asUnicodeString() << "to the list of recipients";
          }
        }

        // strip all my addresses from the list of recipients
        toList = stripMyAddressesFromAddressList( recipients, m_identityManager );

        // merge To header and CC header into a list of CC recipients
        if ( !m_origMsg->cc()->asUnicodeString().isEmpty() || !m_origMsg->to()->asUnicodeString().isEmpty() ) {
          KMime::Types::Mailbox::List list;
          if ( !m_origMsg->to()->asUnicodeString().isEmpty() )
            list += m_origMsg->to()->mailboxes();
          if ( !m_origMsg->cc()->asUnicodeString().isEmpty() )
            list += m_origMsg->cc()->mailboxes();

          foreach ( const KMime::Types::Mailbox &mailbox, list ) {
            if ( !recipients.contains( mailbox ) &&
                 !ccRecipients.contains( mailbox ) ) {
              ccRecipients += mailbox;
              kDebug() << "Added" << mailbox.prettyAddress() <<"to the list of CC recipients";
            }
          }
        }

        if ( !ccRecipients.isEmpty() ) {
          // strip all my addresses from the list of CC recipients
          ccRecipients = stripMyAddressesFromAddressList( ccRecipients, m_identityManager );

          // in case of a reply to self, toList might be empty. if that's the case
          // then propagate a cc recipient to To: (if there is any).
          if ( toList.isEmpty() && !ccRecipients.isEmpty() ) {
            toList << ccRecipients[ 0 ];
            ccRecipients.pop_front();
          }

          foreach ( const KMime::Types::Mailbox &mailbox, ccRecipients )
            msg->cc()->addAddress( mailbox );
        }

        if ( toList.isEmpty() && !recipients.isEmpty() ) {
          // reply to self without other recipients
          toList << recipients[ 0 ];
        }
      }
      break;
    case MessageComposer::ReplyAuthor:
      {
        if ( !replyToList.isEmpty() ) {
          KMime::Types::Mailbox::List recipients = replyToList;

          // strip the mailing list post address from the list of Reply-To
          // addresses since we want to reply in private
          foreach ( const KMime::Types::Mailbox &mailbox, m_mailingListAddresses ) {
            foreach ( const KMime::Types::Mailbox &recipient, recipients ) {
              if ( mailbox == recipient )
                recipients.removeAll( recipient );
            }
          }

          if ( !recipients.isEmpty() ) {
            toList = recipients;
          } else {
            // there was only the mailing list post address in the Reply-To header,
            // so use the From address instead
            toList = m_origMsg->from()->mailboxes();
          }
        } else if ( !m_origMsg->from()->asUnicodeString().isEmpty() ) {
          toList = m_origMsg->from()->mailboxes();
        }

        replyAll = false;
      }
      break;
    case MessageComposer::ReplyNone:
      // the addressees will be set by the caller
      break;
  }

  foreach ( const KMime::Types::Mailbox &mailbox, toList )
    msg->to()->addAddress( mailbox );

  refStr = getRefStr( m_origMsg );
  if ( !refStr.isEmpty() )
    msg->references()->fromUnicodeString( QString::fromLocal8Bit( refStr ), "utf-8" );

  //In-Reply-To = original msg-id
  msg->inReplyTo()->from7BitString( m_origMsg->messageID()->as7BitString( false ) );

  msg->subject()->fromUnicodeString( MessageHelper::replySubject( m_origMsg ), "utf-8" );

  // If the reply shouldn't be blank, apply the template to the message
  if ( m_quote ) {
    TemplateParser::TemplateParser parser( msg, (replyAll ? TemplateParser::TemplateParser::ReplyAll : TemplateParser::TemplateParser::Reply ) );
    parser.setIdentityManager( m_identityManager );
    parser.setCharsets( MessageComposerSettings::self()->preferredCharsets() );
    parser.setWordWrap( MessageComposerSettings::wordWrap(), MessageComposerSettings::lineWrapWidth() );
    if ( MessageComposer::MessageComposerSettings::quoteSelectionOnly() ) {
      parser.setSelection( m_selection );
    }
    parser.setAllowDecryption( m_allowDecryption );
    if ( !m_template.isEmpty() )
      parser.process( m_template, m_origMsg );
    else
      parser.process( m_origMsg, m_collection );
  }

  applyCharset( msg );

  MessageCore::Util::addLinkInformation( msg, m_id, Akonadi::MessageStatus::statusReplied() );
  if ( m_parentFolderId > 0 ) {
    KMime::Headers::Generic *header = new KMime::Headers::Generic( "X-KMail-Fcc", msg.get(), QString::number( m_parentFolderId ), "utf-8" );
    msg->setHeader( header );
  }

  if ( m_origMsg->hasHeader( "X-KMail-EncryptActionEnabled" ) &&
       m_origMsg->headerByType( "X-KMail-EncryptActionEnabled" )->as7BitString() == "true" ) {
    msg->setHeader( new KMime::Headers::Generic( "X-KMail-EncryptActionEnabled", msg.get(), QLatin1String( "true" ), "utf-8" ) );
  }
  msg->assemble();

  MessageReply reply;
  reply.msg = msg;
  reply.replyAll = replyAll;
  return reply;
}



KMime::Message::Ptr MessageFactory::createForward()
{
  KMime::Message::Ptr msg( new KMime::Message );

  // This is a non-multipart, non-text mail (e.g. text/calendar). Construct
  // a multipart/mixed mail and add the original body as an attachment.
  if ( !m_origMsg->contentType()->isMultipart() &&
      ( !m_origMsg->contentType()->isText() ||
      ( m_origMsg->contentType()->isText() && m_origMsg->contentType()->subType() != "html"
        && m_origMsg->contentType()->subType() != "plain" ) ) ) {
    const uint originalIdentity = identityUoid( m_origMsg );
    MessageHelper::initFromMessage( msg, m_origMsg, m_identityManager, originalIdentity );
    msg->removeHeader("Content-Type");
    msg->removeHeader("Content-Transfer-Encoding");

    msg->contentType()->setMimeType( "multipart/mixed" );

  //TODO: Andras: somebody should check if this is correct. :)
    // empty text part
    KMime::Content *msgPart = new KMime::Content;
    msgPart->contentType()->setMimeType("text/plain");
    msg->addContent( msgPart );

    // the old contents of the mail
    KMime::Content *secondPart = new KMime::Content;
    secondPart->contentType()->setMimeType( m_origMsg->contentType()->mimeType() );
    secondPart->setBody( m_origMsg->body() );
    // use the headers of the original mail
    secondPart->setHead( m_origMsg->head() );
    msg->addContent( secondPart );
    msg->assemble();
  }

  // Normal message (multipart or text/plain|html)
  // Just copy the message, the template parser will do the hard work of
  // replacing the body text in TemplateParser::addProcessedBodyToMessage()
  else {
//TODO Check if this is ok
    msg->setHead( m_origMsg->head() );
    msg->setBody( m_origMsg->body() );
    QString oldContentType = msg->contentType()->asUnicodeString();
    const uint originalIdentity = identityUoid( m_origMsg );
    MessageHelper::initFromMessage( msg, m_origMsg, m_identityManager, originalIdentity );

    // restore the content type, MessageHelper::initFromMessage() sets the contents type to
    // text/plain, via initHeader(), for unclear reasons
    msg->contentType()->fromUnicodeString( oldContentType, "utf-8" );
    msg->assemble();
  }

  msg->subject()->fromUnicodeString( MessageHelper::forwardSubject( m_origMsg ), "utf-8" );

  TemplateParser::TemplateParser parser( msg, TemplateParser::TemplateParser::Forward );
  parser.setIdentityManager( m_identityManager );
  parser.setCharsets( MessageComposerSettings::self()->preferredCharsets() );
  if ( !m_template.isEmpty() )
    parser.process( m_template, m_origMsg );
  else
    parser.process( m_origMsg, m_collection );

  applyCharset( msg );

  MessageCore::Util::addLinkInformation( msg, m_id, Akonadi::MessageStatus::statusForwarded() );
  msg->assemble();
  return msg;
}

QPair< KMime::Message::Ptr, QList< KMime::Content* > > MessageFactory::createAttachedForward(const QList< Akonadi::Item >& items)
{
  
  // create forwarded message with original message as attachment
  // remove headers that shouldn't be forwarded
  KMime::Message::Ptr msg( new KMime::Message );
  QList< KMime::Content* > attachments;

  const int numberOfItems( items.count() ); 
  if( numberOfItems >= 2 ) {
    // don't respect X-KMail-Identity headers because they might differ for
    // the selected mails
    MessageHelper::initHeader( msg, m_identityManager, m_origId );
  } else if( numberOfItems == 1 ) {
    KMime::Message::Ptr firstMsg = MessageCore::Util::message( items.first() );
    const uint originalIdentity = identityUoid( firstMsg );
    MessageHelper::initFromMessage( msg, firstMsg, m_identityManager, originalIdentity );
    msg->subject()->fromUnicodeString( MessageHelper::forwardSubject( firstMsg ),"utf-8" );
  }

  MessageHelper::setAutomaticFields( msg, true );
#ifndef QT_NO_CURSOR
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
#endif
  if ( numberOfItems == 0 ) {
    attachments << createForwardAttachmentMessage( m_origMsg );
    MessageCore::Util::addLinkInformation( msg, m_id, Akonadi::MessageStatus::statusForwarded() );
  }
  else {
    // iterate through all the messages to be forwarded
    foreach ( const Akonadi::Item& item, items ) {
      attachments << createForwardAttachmentMessage(MessageCore::Util::message( item ));
      MessageCore::Util::addLinkInformation( msg, item.id(), Akonadi::MessageStatus::statusForwarded() );
    }
  }

  applyCharset( msg );

  //msg->assemble();
  return QPair< KMime::Message::Ptr, QList< KMime::Content* > >( msg, QList< KMime::Content* >() << attachments );
}

KMime::Content *MessageFactory::createForwardAttachmentMessage(const KMime::Message::Ptr& fwdMsg)
{
  // remove headers that shouldn't be forwarded
  MessageCore::StringUtil::removePrivateHeaderFields( fwdMsg );
  fwdMsg->removeHeader("Bcc");
  fwdMsg->assemble();
  // set the part
  KMime::Content *msgPart = new KMime::Content( fwdMsg.get() );
  msgPart->contentType()->setMimeType( "message/rfc822" );
  
  msgPart->contentDisposition()->setParameter( QLatin1String( "filename" ), i18n( "forwarded message" ) );
  msgPart->contentDisposition()->setDisposition( KMime::Headers::CDinline );
  msgPart->contentDescription()->fromUnicodeString( fwdMsg->from()->asUnicodeString() + QLatin1String( ": " ) + fwdMsg->subject()->asUnicodeString(), "utf-8" );
  msgPart->setBody( fwdMsg->encodedContent() );
  msgPart->assemble();
  
#if 0
  // THIS HAS TO BE AFTER setCte()!!!!
  msgPart->setCharset( "" );
#else
  kDebug() << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif
  MessageCore::Util::addLinkInformation( fwdMsg, m_origId, Akonadi::MessageStatus::statusForwarded() );
  return msgPart;
}
  


KMime::Message::Ptr MessageFactory::createResend()
{
  KMime::Message::Ptr msg( new KMime::Message );
  msg->setContent( m_origMsg->encodedContent() );
  msg->parse();
  msg->removeHeader( "Message-Id" );
  uint originalIdentity = identityUoid( m_origMsg );

  // Set the identity from above
  KMime::Headers::Generic *header = new KMime::Headers::Generic( "X-KMail-Identity", msg.get(), QString::number( originalIdentity ), "utf-8" );
  msg->setHeader( header );

  // Restore the original bcc field as this is overwritten in applyIdentity
  msg->bcc( m_origMsg->bcc() );
  return msg;
}

KMime::Message::Ptr MessageFactory::createRedirect( const QString &toStr, int transportId, const QString& fcc, int identity )
{
  if ( !m_origMsg )
    return KMime::Message::Ptr();

  // copy the message 1:1
  KMime::Message::Ptr msg( new KMime::Message );
  msg->setContent( m_origMsg->encodedContent() );
  msg->parse();

  uint id = identity;
  if(id == -1) {
    QString strId = msg->headerByType( "X-KMail-Identity" ) ? msg->headerByType( "X-KMail-Identity" )->asUnicodeString().trimmed() : QString::fromLocal8Bit("");
    if ( !strId.isEmpty())
      id = strId.toUInt();
  }
  const KPIMIdentities::Identity & ident =
    m_identityManager->identityForUoidOrDefault( id );

  // X-KMail-Redirect-From: content
  QString strByWayOf = QString::fromLocal8Bit("%1 (by way of %2 <%3>)")
    .arg( m_origMsg->from()->asUnicodeString() )
    .arg( ident.fullName() )
    .arg( ident.primaryEmailAddress() );

  // Resent-From: content
  QString strFrom = QString::fromLocal8Bit("%1 <%2>")
    .arg( ident.fullName() )
    .arg( ident.primaryEmailAddress() );

  // format the current date to be used in Resent-Date:
  QString newDate = KDateTime::currentLocalDateTime().toString( KDateTime::RFCDateDay );

  // prepend Resent-*: headers (c.f. RFC2822 3.6.6)
  QString msgIdSuffix;
  if ( MessageComposer::MessageComposerSettings::useCustomMessageIdSuffix() ) {
    msgIdSuffix = MessageComposer::MessageComposerSettings::customMsgIDSuffix();
  }
  KMime::Headers::Generic *header =
      new KMime::Headers::Generic( "Resent-Message-ID", msg.get(),
                                   MessageCore::StringUtil::generateMessageId(
                                       msg->sender()->asUnicodeString(), msgIdSuffix ), "utf-8" );
  msg->setHeader( header );

  header = new KMime::Headers::Generic( "Resent-Date", msg.get(), newDate, "utf-8" );
  msg->setHeader( header );

  header = new KMime::Headers::Generic( "Resent-From", msg.get(), strFrom, "utf-8" );
  msg->setHeader( header );

  KMime::Headers::To* headerT = new KMime::Headers::To( msg.get(), toStr, "utf-8" );
  msg->setHeader( headerT );

  header = new KMime::Headers::Generic( "Resent-To", msg.get(), toStr, "utf-8" );
  msg->setHeader( header );

  if( msg->cc( false ) ) {
    header = new KMime::Headers::Generic( "Resent-Cc", msg.get(), m_origMsg->cc()->asUnicodeString(), "utf-8" );
    msg->setHeader( header );
  }

  if( msg->bcc( false ) ) {
    header = new KMime::Headers::Generic( "Resent-Bcc", msg.get(), m_origMsg->bcc()->asUnicodeString(), "utf-8" );
    msg->setHeader( header );
  }

  header = new KMime::Headers::Generic( "X-KMail-Redirect-From", msg.get(), strByWayOf, "utf-8" );
  msg->setHeader( header );
  header = new KMime::Headers::Generic( "X-KMail-Recipients", msg.get(), toStr, "utf-8" );
  msg->setHeader( header );


  if ( transportId != -1 ) {
    header = new KMime::Headers::Generic( "X-KMail-Transport", msg.get(), QString::number( transportId ), "utf-8" );
    msg->setHeader( header );
  }

  if ( !fcc.isEmpty() ) {
    header = new KMime::Headers::Generic( "X-KMail-Fcc", msg.get(), fcc, "utf-8" );
    msg->setHeader( header );    
  }

  const bool fccIsDisabled = ident.disabledFcc();
  if (fccIsDisabled) {
      KMime::Headers::Generic *header = new KMime::Headers::Generic( "X-KMail-FccDisabled", msg.get(), QLatin1String("true"), "utf-8" );
      msg->setHeader( header );
  } else {
      msg->removeHeader( "X-KMail-FccDisabled" );
  }


  msg->assemble();

  MessageCore::Util::addLinkInformation( msg, m_id, Akonadi::MessageStatus::statusForwarded() );
  return msg;
}


KMime::Message::Ptr MessageFactory::createDeliveryReceipt()
{
  QString str, receiptTo;
  KMime::Message::Ptr receipt;

  receiptTo = m_origMsg->headerByType("Disposition-Notification-To") ? m_origMsg->headerByType("Disposition-Notification-To")->asUnicodeString() : QString::fromLatin1("");
  if ( receiptTo.trimmed().isEmpty() )
    return  KMime::Message::Ptr();
  receiptTo.remove( QChar::fromLatin1('\n') );

  receipt =  KMime::Message::Ptr( new KMime::Message );
  const uint originalIdentity = identityUoid( m_origMsg );
  MessageHelper::initFromMessage( receipt, m_origMsg, m_identityManager, originalIdentity );
  receipt->to()->fromUnicodeString( receiptTo, QString::fromLatin1("utf-8").toLatin1() );
  receipt->subject()->fromUnicodeString( i18n("Receipt: ") + m_origMsg->subject()->asUnicodeString(), "utf-8");

  str  = QString::fromLatin1("Your message was successfully delivered.");
  str += QString::fromLatin1("\n\n---------- Message header follows ----------\n");
  str += QString::fromLatin1(m_origMsg->head());
  str += QString::fromLatin1("--------------------------------------------\n");
  // Conversion to toLatin1 is correct here as Mail headers should contain
  // ascii only
  receipt->setBody(str.toLatin1());
  MessageHelper::setAutomaticFields( receipt );
  receipt->assemble();

  return receipt;
}

KMime::Message::Ptr MessageFactory::createMDN( KMime::MDN::ActionMode a,
                                                KMime::MDN::DispositionType d,
                                                KMime::MDN::SendingMode s,
                                                int mdnQuoteOriginal,
                                                const QList<KMime::MDN::DispositionModifier>& m )
{
  // extract where to send to:
  QString receiptTo = m_origMsg->headerByType("Disposition-Notification-To") ? m_origMsg->headerByType("Disposition-Notification-To")->asUnicodeString() : QString::fromLatin1("");
  if( receiptTo.trimmed().isEmpty() ) return KMime::Message::Ptr( new KMime::Message );
  receiptTo.remove( QChar::fromLatin1('\n') );


  QString special; // fill in case of error, warning or failure

  // extract where to send from:
  QString finalRecipient = m_identityManager->identityForUoidOrDefault( identityUoid( m_origMsg ) ).fullEmailAddr();

  //
  // Generate message:
  //

  KMime::Message::Ptr receipt( new KMime::Message() );
  const uint originalIdentity = identityUoid( m_origMsg );
  MessageHelper::initFromMessage( receipt, m_origMsg, m_identityManager, originalIdentity );
  receipt->contentType()->from7BitString( "multipart/report" );
  receipt->contentType()->setBoundary( KMime::multiPartBoundary() );
  receipt->contentType()->setCharset( "us-ascii" );
  receipt->removeHeader("Content-Transfer-Encoding");
  // Modify the ContentType directly (replaces setAutomaticFields(true))
  receipt->contentType()->setParameter( QString::fromLatin1("report-type"), QString::fromLatin1("disposition-notification") );


  QString description = replaceHeadersInString( m_origMsg, KMime::MDN::descriptionFor( d, m ) );

  // text/plain part:
  KMime::Content* firstMsgPart = new KMime::Content( m_origMsg.get() );
  firstMsgPart->contentType()->setMimeType( "text/plain" );
  firstMsgPart->contentType()->setCharset( "utf-8" );
  firstMsgPart->contentTransferEncoding()->from7BitString( "7bit" );
  firstMsgPart->setBody( description.toUtf8() );
  receipt->addContent( firstMsgPart );

  // message/disposition-notification part:
  KMime::Content* secondMsgPart = new KMime::Content( m_origMsg.get() );
  secondMsgPart->contentType()->setMimeType( "message/disposition-notification" );
  //secondMsgPart.setCharset( "us-ascii" );

  secondMsgPart->contentTransferEncoding()->from7BitString( "7bit" );
  secondMsgPart->setBody( KMime::MDN::dispositionNotificationBodyContent(
                            finalRecipient,
                            m_origMsg->headerByType("Original-Recipient") ? m_origMsg->headerByType("Original-Recipient")->as7BitString() : "",
                            m_origMsg->messageID()->as7BitString(), /* Message-ID */
                            d, a, s, m, special ) );
  receipt->addContent( secondMsgPart );

  if ( mdnQuoteOriginal < 0 || mdnQuoteOriginal > 2 ) mdnQuoteOriginal = 0;
  /* 0=> Nothing, 1=>Full Message, 2=>HeadersOnly*/

  KMime::Content* thirdMsgPart = new KMime::Content( m_origMsg.get() );
  switch ( mdnQuoteOriginal ) {
  case 1:
    thirdMsgPart->contentType()->setMimeType( "message/rfc822" );
    thirdMsgPart->setBody( MessageCore::StringUtil::asSendableString( m_origMsg ) );
    receipt->addContent( thirdMsgPart );
    break;
  case 2:
    thirdMsgPart->contentType()->setMimeType( "text/rfc822-headers" );
    thirdMsgPart->setBody( MessageCore::StringUtil::headerAsSendableString( m_origMsg ) );
    receipt->addContent( thirdMsgPart );
    break;
  case 0:
  default:
    break;
  };

  receipt->to()->fromUnicodeString( receiptTo, "utf-8" );
  //Laurent: We don't translate subject ?
  receipt->subject()->from7BitString( "Message Disposition Notification" );
  KMime::Headers::InReplyTo *header = new KMime::Headers::InReplyTo( receipt.get(), m_origMsg->messageID()->asUnicodeString(), "utf-8" );
  receipt->setHeader( header );

  receipt->references()->from7BitString( getRefStr( m_origMsg ) );

  receipt->assemble();


  kDebug() << "final message:" + receipt->encodedContent();

  receipt->assemble();
  return receipt;
}

QPair< KMime::Message::Ptr, KMime::Content* > MessageFactory::createForwardDigestMIME( const QList< Akonadi::Item >& items )
{
  KMime::Message::Ptr msg( new KMime::Message );
  KMime::Content* digest = new KMime::Content( msg.get() );

  QString mainPartText = i18n("\nThis is a MIME digest forward. The content of the"
                         " message is contained in the attachment(s).\n\n\n");

  digest->contentType()->setMimeType( "multipart/digest" );
  digest->contentType()->setBoundary( KMime::multiPartBoundary() );
  digest->contentDescription()->fromUnicodeString( QString::fromLatin1("Digest of %1 messages.").arg( items.count() ), "utf8" );
  digest->contentDisposition()->setFilename( QLatin1String( "digest" ) );
  digest->fromUnicodeString( mainPartText );

  int id = 0;
  foreach ( const Akonadi::Item& item, items ) {
    KMime::Message::Ptr fMsg = MessageCore::Util::message( item );
    if( id == 0 && fMsg->hasHeader( "X-KMail-Identity" ) )
      id = fMsg->headerByType( "X-KMail-Identity" )->asUnicodeString().toInt();

    MessageCore::StringUtil::removePrivateHeaderFields( fMsg );
    fMsg->removeHeader("Bcc");
    fMsg->assemble();
    KMime::Content* part = new KMime::Content( digest );

    part->contentType()->setMimeType( "message/rfc822" );
    part->contentType()->setCharset( fMsg->contentType()->charset() );
    part->contentID()->setIdentifier( fMsg->contentID()->identifier() );
    part->contentDescription()->fromUnicodeString( fMsg->contentDescription()->asUnicodeString(), "utf8" );
    part->contentDisposition()->setParameter( QLatin1String( "name" ), i18n( "forwarded message" ) );
    part->fromUnicodeString( QString::fromLatin1( fMsg->encodedContent() ) );
    part->assemble();
    MessageCore::Util::addLinkInformation( msg, item.id(), Akonadi::MessageStatus::statusForwarded() );
    digest->addContent( part );
  }
  digest->assemble();

  id = m_folderId;
  MessageHelper::initHeader( msg, m_identityManager, id );

//   kDebug() << "digest:" << digest->contents().size() << digest->encodedContent();

  return QPair< KMime::Message::Ptr, KMime::Content* >( msg, digest );
}


void MessageFactory::setIdentityManager( KPIMIdentities::IdentityManager* ident)
{
  m_identityManager = ident;
}

void MessageFactory::setMessageItemID( Akonadi::Entity::Id id )
{
  m_origId = id;
}

void MessageFactory::setReplyStrategy( MessageComposer::ReplyStrategy replyStrategy  )
{
  m_replyStrategy = replyStrategy;
}

void MessageFactory::setSelection( const QString& selection )
{
  m_selection = selection;
}

void MessageFactory::setQuote( bool quote )
{
  m_quote = quote;
}

void MessageFactory::setAllowDecryption( bool allowD )
{
  m_allowDecryption = allowD;
}

void MessageFactory::setTemplate( const QString& templ )
{
  m_template = templ;
}

void MessageFactory::setMailingListAddresses( const KMime::Types::Mailbox::List& listAddresses )
{
  m_mailingListAddresses << listAddresses;
}

void MessageFactory::setFolderIdentity( Akonadi::Entity::Id folderIdentityId )
{
  m_folderId = folderIdentityId;
}

void MessageFactory::putRepliesInSameFolder( Akonadi::Entity::Id parentColId )
{
  m_parentFolderId = parentColId;
}

bool MessageFactory::MDNRequested(const KMime::Message::Ptr& msg)
{
  // extract where to send to:
  QString receiptTo = msg->headerByType("Disposition-Notification-To") ? msg->headerByType("Disposition-Notification-To")->asUnicodeString() : QString::fromLatin1("");
  if ( receiptTo.trimmed().isEmpty() ) return false;
  receiptTo.remove( QChar::fromLatin1('\n') );
  return !receiptTo.isEmpty();
}


bool MessageFactory::MDNConfirmMultipleRecipients( const KMime::Message::Ptr& msg )
{
  // extract where to send to:
  QString receiptTo = msg->headerByType("Disposition-Notification-To") ? msg->headerByType("Disposition-Notification-To")->asUnicodeString() : QString::fromLatin1("");
  if ( receiptTo.trimmed().isEmpty() ) return false;
  receiptTo.remove( QChar::fromLatin1('\n') );

   // RFC 2298: [ Confirmation from the user SHOULD be obtained (or no
  // MDN sent) ] if there is more than one distinct address in the
  // Disposition-Notification-To header.
  kDebug() << "KPIMUtils::splitAddressList(receiptTo):" // krazy:exclude=kdebug
           << KPIMUtils::splitAddressList(receiptTo).join(QString::fromLatin1("\n"));

  return KPIMUtils::splitAddressList(receiptTo).count() > 1;
}

bool MessageFactory::MDNReturnPathEmpty( const KMime::Message::Ptr& msg )
{
  // extract where to send to:
  QString receiptTo = msg->headerByType("Disposition-Notification-To") ? msg->headerByType("Disposition-Notification-To")->asUnicodeString() : QString::fromLatin1("");
  if ( receiptTo.trimmed().isEmpty() ) return false;
  receiptTo.remove( QChar::fromLatin1('\n') );

  // RFC 2298: MDNs SHOULD NOT be sent automatically if the address in
  // the Disposition-Notification-To header differs from the address
  // in the Return-Path header. [...] Confirmation from the user
  // SHOULD be obtained (or no MDN sent) if there is no Return-Path
  // header in the message [...]
  KMime::Types::AddrSpecList returnPathList = MessageHelper::extractAddrSpecs( msg, "Return-Path");
  QString returnPath = returnPathList.isEmpty() ? QString()
    : returnPathList.front().localPart + QChar::fromLatin1('@') + returnPathList.front().domain;
  kDebug() << "clean return path:" << returnPath;
  return returnPath.isEmpty();
}

bool MessageFactory::MDNReturnPathNotInRecieptTo( const  KMime::Message::Ptr& msg )
{
  // extract where to send to:
  QString receiptTo = msg->headerByType("Disposition-Notification-To") ? msg->headerByType("Disposition-Notification-To")->asUnicodeString() : QString::fromLatin1("");
  if ( receiptTo.trimmed().isEmpty() ) return false;
  receiptTo.remove( QChar::fromLatin1('\n') );

  // RFC 2298: MDNs SHOULD NOT be sent automatically if the address in
  // the Disposition-Notification-To header differs from the address
  // in the Return-Path header. [...] Confirmation from the user
  // SHOULD be obtained (or no MDN sent) if there is no Return-Path
  // header in the message [...]
  KMime::Types::AddrSpecList returnPathList = MessageHelper::extractAddrSpecs( msg, QString::fromLatin1("Return-Path").toLatin1());
  QString returnPath = returnPathList.isEmpty() ? QString()
    : returnPathList.front().localPart + QChar::fromLatin1('@') + returnPathList.front().domain;
  kDebug() << "clean return path:" << returnPath;
  return !receiptTo.contains( returnPath, Qt::CaseSensitive );
}

bool MessageFactory::MDNMDNUnknownOption( const KMime::Message::Ptr& msg )
{

  // RFC 2298: An importance of "required" indicates that
  // interpretation of the parameter is necessary for proper
  // generation of an MDN in response to this request.  If a UA does
  // not understand the meaning of the parameter, it MUST NOT generate
  // an MDN with any disposition type other than "failed" in response
  // to the request.
  QString notificationOptions = msg->headerByType("Disposition-Notification-Options") ? msg->headerByType("Disposition-Notification-Options")->asUnicodeString() : QString::fromLatin1("");
  if ( notificationOptions.contains( QString::fromLatin1("required"), Qt::CaseSensitive ) ) {
    // ### hacky; should parse...
    // There is a required option that we don't understand. We need to
    // ask the user what we should do:
    return true;
  }
  return false;
}


uint MessageFactory::identityUoid( const KMime::Message::Ptr &msg )
{
  QString idString;
  if ( msg->headerByType("X-KMail-Identity") )
    idString = msg->headerByType("X-KMail-Identity")->asUnicodeString().trimmed();
  bool ok = false;
  int id = idString.toUInt( &ok );

  if ( !ok || id == 0 )
    id = m_identityManager->identityForAddress( msg->to()->asUnicodeString() + QString::fromLatin1(", ") + msg->cc()->asUnicodeString() ).uoid();

  if ( id == 0 && m_folderId > 0 ) {
    id = m_folderId;
  }
  return id;
}


QString MessageFactory::replaceHeadersInString( const KMime::Message::Ptr &msg, const QString & s )
{
    QString result = s;
    QRegExp rx( QString::fromLatin1("\\$\\{([a-z0-9-]+)\\}"), Qt::CaseInsensitive );
    Q_ASSERT( rx.isValid() );

    QRegExp rxDate( QString::fromLatin1("\\$\\{date\\}") );
    Q_ASSERT( rxDate.isValid() );

    kDebug() << "creating mdn date:" << msg->date()->dateTime().dateTime().toTime_t() << KMime::DateFormatter::formatDate(
        KMime::DateFormatter::Localized, msg->date()->dateTime().dateTime().toTime_t() );
    QString sDate = KMime::DateFormatter::formatDate(
        KMime::DateFormatter::Localized, msg->date()->dateTime().dateTime().toTime_t() );

    int idx = 0;
    if( ( idx = rxDate.indexIn( result, idx ) ) != -1  ) {
      result.replace( idx, rxDate.matchedLength(), sDate );
    }

    idx = 0;
    while ( ( idx = rx.indexIn( result, idx ) ) != -1 ) {
      QString replacement = msg->headerByType( rx.cap(1).toLatin1() ) ? msg->headerByType( rx.cap(1).toLatin1() )->asUnicodeString() : QString::fromLatin1("");
      result.replace( idx, rx.matchedLength(), replacement );
      idx += replacement.length();
    }
    return result;
}

void MessageFactory::applyCharset( const KMime::Message::Ptr msg )
{
  if ( MessageComposer::MessageComposerSettings::forceReplyCharset() ) {
    // first convert the body from its current encoding to unicode representation
    QTextCodec *bodyCodec = KGlobal::charsets()->codecForName( QString::fromLatin1( msg->contentType()->charset() ) );
    if ( !bodyCodec )
      bodyCodec = KGlobal::charsets()->codecForName( QLatin1String( "UTF-8" ) );

    const QString body = bodyCodec->toUnicode( msg->body() );

    // then apply the encoding of the original message
    msg->contentType()->setCharset( m_origMsg->contentType()->charset() );

    QTextCodec *codec = KGlobal::charsets()->codecForName( QString::fromLatin1( msg->contentType()->charset() ) );
    if ( !codec ) {
      kError() << "Could not get text codec for charset" << msg->contentType()->charset();
    } else if ( !codec->canEncode( body ) ) { // charset can't encode body, fall back to preferred
      const QStringList charsets = MessageComposer::MessageComposerSettings::preferredCharsets();

      QList<QByteArray> chars;
      foreach ( const QString &charset, charsets )
        chars << charset.toLatin1();

      QByteArray fallbackCharset = MessageComposer::Util::selectCharset( chars, body );
      if ( fallbackCharset.isEmpty() ) // UTF-8 as fall-through
        fallbackCharset = "UTF-8";

      codec = KGlobal::charsets()->codecForName( QString::fromLatin1( fallbackCharset ) );
      msg->setBody( codec->fromUnicode( body ) );
    } else {
      msg->setBody( codec->fromUnicode( body ) );
    }
  }
}


QByteArray MessageFactory::getRefStr( const KMime::Message::Ptr &msg )
{
  QByteArray firstRef, lastRef, refStr, retRefStr;
  int i, j;

  refStr = msg->headerByType("References") ? msg->headerByType("References")->as7BitString().trimmed() : "";

  if (refStr.isEmpty())
    return msg->messageID()->as7BitString( false );

  i = refStr.indexOf('<');
  j = refStr.indexOf('>');
  firstRef = refStr.mid(i, j-i+1);
  if (!firstRef.isEmpty())
    retRefStr = firstRef + ' ';

  i = refStr.lastIndexOf('<');
  j = refStr.lastIndexOf('>');

  lastRef = refStr.mid(i, j-i+1);
  if (!lastRef.isEmpty() && lastRef != firstRef)
    retRefStr += lastRef + ' ';

  retRefStr += msg->messageID()->as7BitString( false );
  return retRefStr;
}
