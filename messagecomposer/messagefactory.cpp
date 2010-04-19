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

#include "messageinfo.h"
#include "messagecomposersettings.h"

#include <akonadi/item.h>
#include <messageviewer/kcursorsaver.h>
#include <messageviewer/objecttreeparser.h>
#include <kpimidentities/identitymanager.h>
#include <kpimidentities/identity.h>

#include <messagecore/messagestatus.h>
#include <kmime/kmime_dateformatter.h>
#include <KPIMUtils/Email>
#include <messagecore/stringutil.h>
#include "messagehelper.h"
#include "templateparser/templateparser.h"
#include <messagecore/mailinglist-magic.h>
#include <KLocalizedString>


MessageFactory::MessageFactory( const KMime::Message::Ptr& origMsg, Akonadi::Item::Id id )
  : m_identityManager( 0 )
  , m_origMsg( KMime::Message::Ptr() )
  , m_origId( 0 )
  , m_parentFolderId( 0 )
  , m_replyStrategy( MessageComposer::ReplySmart )
  , m_quote( true )
  , m_allowDecryption( true )
  , m_id ( id )
{
  m_origMsg = origMsg;
}

MessageFactory::~MessageFactory()
{
}

MessageFactory::MessageReply MessageFactory::createReply()
{
  KMime::Message::Ptr msg( new KMime::Message );
  QString str, mailingListStr, replyToStr, toStr;
  QByteArray refStr, headerName;
  bool replyAll = true;

  MessageHelper::initFromMessage( msg, m_origMsg, m_identityManager );
  MessageCore::MailingList::name( m_origMsg, headerName, mailingListStr );
  replyToStr = m_origMsg->replyTo()->asUnicodeString();

  msg->contentType()->setCharset(QString::fromLatin1("utf-8").toLatin1());

  if ( m_origMsg->headerByType("List-Post") && m_origMsg->headerByType("List-Post")->asUnicodeString().contains( QString::fromLatin1("mailto:"), Qt::CaseInsensitive ) ) {
    QString listPost = m_origMsg->headerByType("List-Post")->asUnicodeString();
    QRegExp rx( QString::fromLatin1("<mailto:([^@>]+)@([^>]+)>"), Qt::CaseInsensitive );
    if ( rx.indexIn( listPost, 0 ) != -1 ) // matched
      m_mailingListAddresses << rx.cap(1) + QChar::fromLatin1('@') + rx.cap(2);
  }

  switch( m_replyStrategy ) {
    case MessageComposer::ReplySmart : {
    if ( m_origMsg->headerByType( "Mail-Followup-To" ) ) {
      toStr = m_origMsg->headerByType( "Mail-Followup-To" )->asUnicodeString();
    }
    else if ( !replyToStr.isEmpty() ) {
      toStr = replyToStr;
      // use the ReplyAll template only when it's a reply to a mailing list
      if ( m_mailingListAddresses.isEmpty() )
        replyAll = false;
    }
    else if ( !m_mailingListAddresses.isEmpty() ) {
      toStr = m_mailingListAddresses[0];
    }
    else {

      // doesn't seem to be a mailing list, reply to From: address
      toStr = m_origMsg->from()->asUnicodeString();

      if( m_identityManager->thatIsMe( KPIMUtils::extractEmailAddress( toStr ) ) ) {
        // sender seems to be one of our own identities, so we assume that this
        // is a reply to a "sent" mail where the users wants to add additional
        // information for the recipient.
        toStr = m_origMsg->to()->asUnicodeString();
      }

      replyAll = false;
    }
    // strip all my addresses from the list of recipients
    QStringList recipients = KPIMUtils::splitAddressList( toStr );
    toStr = MessageCore::StringUtil::stripMyAddressesFromAddressList( recipients, m_identityManager ).join(QString::fromLatin1(", "));
    // ... unless the list contains only my addresses (reply to self)
    if ( toStr.isEmpty() && !recipients.isEmpty() )
      toStr = recipients[0];

    break;
  }
  case MessageComposer::ReplyList : {
    if ( m_origMsg->headerByType( "Mail-Followup-To" ) ) {
      toStr = m_origMsg->headerByType( "Mail-Followup-To" )->asUnicodeString();
    }
    else if ( !m_mailingListAddresses.isEmpty() ) {
      toStr = m_mailingListAddresses[0];
    }
    else if ( !replyToStr.isEmpty() ) {
      // assume a Reply-To header mangling mailing list
      toStr = replyToStr;
    }
    // strip all my addresses from the list of recipients
    QStringList recipients = KPIMUtils::splitAddressList( toStr );
    toStr = MessageCore::StringUtil::stripMyAddressesFromAddressList( recipients, m_identityManager ).join(QString::fromLatin1(", "));

    break;
  }
  case MessageComposer::ReplyAll : {
    QStringList recipients;
    QStringList ccRecipients;

    // add addresses from the Reply-To header to the list of recipients
    if( !replyToStr.isEmpty() ) {
      recipients += KPIMUtils::splitAddressList( replyToStr );
      // strip all possible mailing list addresses from the list of Reply-To
      // addresses
      for ( QStringList::const_iterator it = m_mailingListAddresses.constBegin();
            it != m_mailingListAddresses.constEnd();
            ++it ) {
        recipients = MessageCore::StringUtil::stripAddressFromAddressList( *it, recipients );
      }
    }

    if ( !m_mailingListAddresses.isEmpty() ) {
      // this is a mailing list message
      if ( recipients.isEmpty() && !m_origMsg->from()->asUnicodeString().isEmpty() ) {
        // The sender didn't set a Reply-to address, so we add the From
        // address to the list of CC recipients.
        ccRecipients += m_origMsg->from()->asUnicodeString();
        kDebug() << "Added" << m_origMsg->from()->asUnicodeString() <<"to the list of CC recipients";
      }
      // if it is a mailing list, add the posting address
      recipients.prepend( m_mailingListAddresses[0] );
    }
    else {
      // this is a normal message
      if ( recipients.isEmpty() && !m_origMsg->from()->asUnicodeString().isEmpty() ) {
        // in case of replying to a normal message only then add the From
        // address to the list of recipients if there was no Reply-to address
        recipients +=  m_origMsg->from()->asUnicodeString();
        kDebug() << "Added" <<  m_origMsg->from()->asUnicodeString() <<"to the list of recipients";
      }
    }

    // strip all my addresses from the list of recipients
    toStr = MessageCore::StringUtil::stripMyAddressesFromAddressList( recipients, m_identityManager ).join(QString::fromLatin1(", "));

    // merge To header and CC header into a list of CC recipients
    if( !m_origMsg->cc()->asUnicodeString().isEmpty() || !m_origMsg->to()->asUnicodeString().isEmpty() ) {
      QStringList list;
      if (!m_origMsg->to()->asUnicodeString().isEmpty())
        list += KPIMUtils::splitAddressList(m_origMsg->to()->asUnicodeString());
      if (!m_origMsg->cc()->asUnicodeString().isEmpty())
        list += KPIMUtils::splitAddressList(m_origMsg->cc()->asUnicodeString());
      for( QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it ) {
        if(    !MessageCore::StringUtil::addressIsInAddressList( *it, recipients )
            && !MessageCore::StringUtil::addressIsInAddressList( *it, ccRecipients ) ) {
          ccRecipients += *it;
          kDebug() << "Added" << *it <<"to the list of CC recipients";
        }
      }
    }

    if ( !ccRecipients.isEmpty() ) {
      // strip all my addresses from the list of CC recipients
      ccRecipients = MessageCore::StringUtil::stripMyAddressesFromAddressList( ccRecipients, m_identityManager );

      // in case of a reply to self toStr might be empty. if that's the case
      // then propagate a cc recipient to To: (if there is any).
      if ( toStr.isEmpty() && !ccRecipients.isEmpty() ) {
        toStr = ccRecipients[0];
        ccRecipients.pop_front();
      }

      msg->cc()->fromUnicodeString( ccRecipients.join(QString::fromLatin1(", ")), "utf-8" );
    }

    if ( toStr.isEmpty() && !recipients.isEmpty() ) {
      // reply to self without other recipients
      toStr = recipients[0];
    }
    break;
  }
  case MessageComposer::ReplyAuthor : {
    if ( !replyToStr.isEmpty() ) {
      QStringList recipients = KPIMUtils::splitAddressList( replyToStr );
      // strip the mailing list post address from the list of Reply-To
      // addresses since we want to reply in private
      for ( QStringList::const_iterator it = m_mailingListAddresses.constBegin();
            it != m_mailingListAddresses.constEnd();
            ++it ) {
        recipients = MessageCore::StringUtil::stripAddressFromAddressList( *it, recipients );
      }
      if ( !recipients.isEmpty() ) {
        toStr = recipients.join(QString::fromLatin1(", "));
      }
      else {
        // there was only the mailing list post address in the Reply-To header,
        // so use the From address instead
        toStr =  m_origMsg->from()->asUnicodeString();
      }
    }
    else if ( ! m_origMsg->from()->asUnicodeString().isEmpty() ) {
      toStr = m_origMsg->from()->asUnicodeString();
    }
    replyAll = false;
    break;
  }
  case MessageComposer::ReplyNone : {
    // the addressees will be set by the caller
  }
  }

  msg->to()->fromUnicodeString(toStr, "utf-8");

  refStr = getRefStr( m_origMsg );
  if (!refStr.isEmpty())
    msg->references()->fromUnicodeString (QString::fromLocal8Bit(refStr), QString::fromLatin1("utf-8").toLatin1() );
  //In-Reply-To = original msg-id
  msg->inReplyTo()->fromUnicodeString( m_origMsg->messageID()->asUnicodeString(), "utf-8" );

  msg->subject()->fromUnicodeString( MessageHelper::replySubject( m_origMsg ), "utf-8" );

  // If the reply shouldn't be blank, apply the template to the message
  if ( m_quote ) {
    TemplateParser::TemplateParser parser( msg, (replyAll ? TemplateParser::TemplateParser::ReplyAll : TemplateParser::TemplateParser::Reply ) );
    parser.setIdentityManager( m_identityManager );
    if ( MessageComposer::MessageComposerSettings::quoteSelectionOnly() ) {
      parser.setSelection( m_selection );
    }
    parser.setAllowDecryption( m_allowDecryption );
    if ( !m_template.isEmpty() )
      parser.process( m_template, m_origMsg );
    else
      parser.process( m_origMsg );
  }
  link( msg, m_id, KPIM::MessageStatus::statusReplied() );
  if ( m_parentFolderId > 0 ) {
    KMime::Headers::Generic *header = new KMime::Headers::Generic( "X-KMail-Fcc", msg.get(), QString::number( m_parentFolderId ), "utf-8" );
    msg->setHeader( header );
  }
#if 0
  // replies to an encrypted message should be encrypted as well
  if ( encryptionState() == KMMsgPartiallyEncrypted ||
       encryptionState() == KMMsgFullyEncrypted ) {
    msg->setEncryptionState( KMMsgFullyEncrypted );
  }

#else //TODO port to akonadi
  kDebug() << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif

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
    MessageHelper::initFromMessage( msg, m_origMsg, m_identityManager );
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
    MessageHelper::initFromMessage( msg, m_origMsg, m_identityManager );

    // restore the content type, MessageHelper::initFromMessage() sets the contents type to
    // text/plain, via initHeader(), for unclear reasons
    msg->contentType()->fromUnicodeString( oldContentType, "utf-8" );
    msg->assemble();
  }

  msg->subject()->fromUnicodeString( MessageHelper::forwardSubject( m_origMsg ), "utf-8" );

  TemplateParser::TemplateParser parser( msg, TemplateParser::TemplateParser::Forward );
  parser.setIdentityManager( m_identityManager );
  if ( !m_template.isEmpty() )
    parser.process( m_template, m_origMsg );
  else
    parser.process( m_origMsg );

  link( msg, m_id, KPIM::MessageStatus::statusForwarded() );
  return msg;
}


KMime::Message::Ptr MessageFactory::createResend()
{
  KMime::Message::Ptr msg( new KMime::Message );
  MessageHelper::initFromMessage( msg, m_origMsg, m_identityManager );
  msg->setContent( m_origMsg->encodedContent() );
  msg->removeHeader( "Message-Id" );
  uint originalIdentity = identityUoid( m_origMsg );

  // Remove all unnecessary headers
  msg->removeHeader("Bcc");
  msg->removeHeader( "Cc" );
  msg->removeHeader( "To" );
  msg->removeHeader( "Subject" );
  // Set the identity from above
  KMime::Headers::Generic *header = new KMime::Headers::Generic( "X-KMail-Identity", msg.get(), QString::number( originalIdentity ), "utf-8" );
  msg->setHeader( header );

  // Restore the original bcc field as this is overwritten in applyIdentity
  msg->bcc( m_origMsg->bcc() );
  return msg;
}

KMime::Message::Ptr MessageFactory::createRedirect( const QString &toStr )
{
  if ( !m_origMsg )
    return KMime::Message::Ptr();

  // copy the message 1:1
  KMime::Message::Ptr msg( new KMime::Message );
  msg->setContent( m_origMsg->encodedContent() );

  uint id = 0;
  QString strId = msg->headerByType( "X-KMail-Identity" ) ? msg->headerByType( "X-KMail-Identity" )->asUnicodeString().trimmed() : QString::fromLocal8Bit("");
  if ( !strId.isEmpty())
    id = strId.toUInt();
  const KPIMIdentities::Identity & ident =
    m_identityManager->identityForUoidOrDefault( id );

  // X-KMail-Redirect-From: content
  QString strByWayOf = QString::fromLocal8Bit("%1 (by way of %2 <%3>)")
    .arg( m_origMsg->from()->asUnicodeString() )
    .arg( ident.fullName() )
    .arg( ident.emailAddr() );

  // Resent-From: content
  QString strFrom = QString::fromLocal8Bit("%1 <%2>")
    .arg( ident.fullName() )
    .arg( ident.emailAddr() );

  // format the current date to be used in Resent-Date:
  QString origDate = msg->date()->asUnicodeString();
  msg->date()->setDateTime( KDateTime::currentLocalDateTime() );
  QString newDate = msg->date()->asUnicodeString();

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

  header = new KMime::Headers::Generic( "Resent-To", msg.get(), toStr, "utf-8" );
  msg->setHeader( header );
  header = new KMime::Headers::Generic( "Resent-To", msg.get(), strFrom, "utf-8" );
  msg->setHeader( header );

  header = new KMime::Headers::Generic( "X-KMail-Redirect-From", msg.get(), strByWayOf, "utf-8" );
  msg->setHeader( header );
  header = new KMime::Headers::Generic( "X-KMail-Recipients", msg.get(), toStr, "utf-8" );
  msg->setHeader( header );

  link( msg, m_id, KPIM::MessageStatus::statusForwarded() );
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
  MessageHelper::initFromMessage( receipt, m_origMsg, m_identityManager );
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
                                                QList<KMime::MDN::DispositionModifier> m )
{
  // RFC 2298: At most one MDN may be issued on behalf of each
  // particular recipient by their user agent.  That is, once an MDN
  // has been issued on behalf of a recipient, no further MDNs may be
  // issued on behalf of that recipient, even if another disposition
  // is performed on the message.
//#define MDN_DEBUG 1
#ifndef MDN_DEBUG
  if ( MessageInfo::instance()->mdnSentState( m_origMsg.get() ) != KMMsgMDNStateUnknown &&
       MessageInfo::instance()->mdnSentState( m_origMsg.get() ) != KMMsgMDNNone )
    return KMime::Message::Ptr();
#else
  char st[2]; st[0] = (char)MessageInfo::instance()->mdnSentState( m_origMsg ); st[1] = 0;
  kDebug() << "mdnSentState() == '" << st <<"'";
#endif

  // RFC 2298: An MDN MUST NOT be generated in response to an MDN.
  if ( MessageViewer::ObjectTreeParser::findType( m_origMsg.get(), "message",
                       "disposition-notification", true, true ) ) {
    MessageInfo::instance()->setMDNSentState( m_origMsg.get(), KMMsgMDNIgnore );
    return KMime::Message::Ptr();
  }

  // extract where to send to:
  QString receiptTo = m_origMsg->headerByType("Disposition-Notification-To") ? m_origMsg->headerByType("Disposition-Notification-To")->asUnicodeString() : QString::fromLatin1("");
  if ( receiptTo.trimmed().isEmpty() ) return KMime::Message::Ptr();
  receiptTo.remove( QChar::fromLatin1('\n') );


  QString special; // fill in case of error, warning or failure

  // extract where to send from:
  QString finalRecipient = m_identityManager->identityForUoidOrDefault( identityUoid( m_origMsg ) ).fullEmailAddr();

  //
  // Generate message:
  //

  KMime::Message::Ptr receipt( new KMime::Message() );
  MessageHelper::initFromMessage( receipt, m_origMsg, m_identityManager );
  receipt->contentType()->from7BitString( "multipart/report" );
  receipt->removeHeader("Content-Transfer-Encoding");
  // Modify the ContentType directly (replaces setAutomaticFields(true))
  receipt->contentType()->setParameter( QString::fromLatin1("report-type"), QString::fromLatin1("disposition-notification") );


  QString description = replaceHeadersInString( m_origMsg, KMime::MDN::descriptionFor( d, m ) );

  // text/plain part:
  KMime::Content* firstMsgPart = new KMime::Content( m_origMsg.get() );
  firstMsgPart->contentType()->setMimeType( "text/plain" );
  firstMsgPart->setBody( description.toUtf8() );
  receipt->addContent( firstMsgPart );

  // message/disposition-notification part:
  KMime::Content* secondMsgPart = new KMime::Content( m_origMsg.get() );
  secondMsgPart->contentType()->setMimeType( "message/disposition-notification" );
  //secondMsgPart.setCharset( "us-ascii" );
  //secondMsgPart.setCteStr( "7bit" );
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
  receipt->subject()->from7BitString( "Message Disposition Notification" );
  KMime::Headers::Generic *header = new KMime::Headers::Generic( "In-Reply-To", receipt.get(), m_origMsg->messageID()->as7BitString() );
  receipt->setHeader( header );

  receipt->references()->from7BitString( getRefStr( m_origMsg ) );

  receipt->assemble();

  kDebug() << "final message:" + receipt->encodedContent();

  //
  // Set "MDN sent" status:
  //
  KMMsgMDNSentState state = KMMsgMDNStateUnknown;
  switch ( d ) {
  case KMime::MDN::Displayed:   state = KMMsgMDNDisplayed;  break;
  case KMime::MDN::Deleted:     state = KMMsgMDNDeleted;    break;
  case KMime::MDN::Dispatched:  state = KMMsgMDNDispatched; break;
  case KMime::MDN::Processed:   state = KMMsgMDNProcessed;  break;
  case KMime::MDN::Denied:      state = KMMsgMDNDenied;     break;
  case KMime::MDN::Failed:      state = KMMsgMDNFailed;     break;
  };
  MessageInfo::instance()->setMDNSentState( m_origMsg.get(), state );

  receipt->assemble();
  return receipt;
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

void MessageFactory::setMailingListAddresses( const QStringList& listAddresses )
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

bool MessageFactory::MDNConfirmMultipleRecipients( KMime::Message::Ptr msg )
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

bool MessageFactory::MDNReturnPathEmpty( KMime::Message::Ptr msg )
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

bool MessageFactory::MDNReturnPathNotInRecieptTo( KMime::Message::Ptr msg )
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

bool MessageFactory::MDNMDNUnknownOption( KMime::Message::Ptr msg )
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

void MessageFactory::link( const KMime::Message::Ptr &msg, Akonadi::Item::Id id, const KPIM::MessageStatus& aStatus )
{
  Q_ASSERT( aStatus.isReplied() || aStatus.isForwarded() || aStatus.isDeleted() );

  QString message = msg->headerByType( "X-KMail-Link-Message" ) ? msg->headerByType( "X-KMail-Link-Message" )->asUnicodeString() : QString();
  if ( !message.isEmpty() )
    message += QChar::fromLatin1(',');
  QString type = msg->headerByType( "X-KMail-Link-Type" ) ? msg->headerByType( "X-KMail-Link-Type" )->asUnicodeString(): QString();
  if ( !type.isEmpty() )
    type += QChar::fromLatin1(',');

  message += QString::number( id );
  if ( aStatus.isReplied() )
    type += QString::fromLatin1("reply");
  else if ( aStatus.isForwarded() )
    type += QString::fromLatin1("forward");
  else if ( aStatus.isDeleted() )
    type += QString::fromLatin1("deleted");

  KMime::Headers::Generic *header = new KMime::Headers::Generic( "X-KMail-Link-Message", msg.get(), message, QString::fromLatin1("utf-8").toLatin1() );
  msg->setHeader( header );

  header = new KMime::Headers::Generic( "X-KMail-Link-Type", msg.get(), type, QString::fromLatin1("utf-8").toLatin1() );
  msg->setHeader( header );
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


QByteArray MessageFactory::getRefStr( const KMime::Message::Ptr &msg )
{
  QByteArray firstRef, lastRef, refStr, retRefStr;
  int i, j;

  refStr = msg->headerByType("References") ? msg->headerByType("References")->as7BitString().trimmed() : "";

  if (refStr.isEmpty())
    return msg->messageID()->as7BitString();

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

  retRefStr += msg->messageID()->as7BitString();
  return retRefStr;
}
