/*
  Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2010 Leo Franchi <lfranchi@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "composerviewbase.h"

#include "attachmentcontrollerbase.h"
#include "attachmentmodel.h"
#include "signaturecontroller.h"
#include <messagecore/stringutil.h>
#include <krichtextwidget.h>
#include <messageviewer/objecttreeemptysource.h>
#include <messageviewer/objecttreeparser.h>
#include "kmeditor.h"
#include <messagecore/attachmentcollector.h>
#include <messagecore/nodehelper.h>
#include <messageviewer/kcursorsaver.h>
#include <mailtransport/transportmanager.h>
#include <messagecomposer/recipientseditor.h>
#include <akonadi/collectioncombobox.h>
#include <KLocalizedString>
#include <KMessageBox>
#include "emailaddressresolvejob.h"
#include "keyresolver.h"
#include <kpimidentities/identitycombo.h>
#include <kpimidentities/identitymanager.h>
#include "kleo_util.h"
#include "infopart.h"
#include "composer.h"
#include <mailtransport/messagequeuejob.h>
#include <akonadi/kmime/specialmailcollections.h>
#include <akonadi/itemcreatejob.h>
#include <mailtransport/transportcombobox.h>
#include <QtCore/qtextcodec.h>
#include "globalpart.h"

Message::ComposerViewBase::ComposerViewBase ( QObject* parent )
 : QObject ( parent )
 , m_msg( KMime::Message::Ptr( new KMime::Message ) )
 , m_attachmentController( 0 )
 , m_attachmentModel( 0 )
 , m_signatureController( 0 )
 , m_recipientsEditor( 0 )
 , m_identityCombo( 0 )
 , m_identMan( 0 )
 , m_editor( 0 )
 , m_transport( 0 )
 , m_fcc( 0 )
 , m_sign( false )
 , m_encrypt( false )
 , m_neverEncrypt( false )
 , m_mdnRequested( false )
 , m_urgent( false )
 , m_cryptoMessageFormat( Kleo::AutoFormat )
 , m_pendingQueueJobs( 0 )
{
  m_charsets << "utf-8"; // default, so we have a backup in case client code forgot to set.
}

Message::ComposerViewBase::~ComposerViewBase()
{

}

// Checks if the mail is a HTML mail.
// The catch here is that encapsulated messages can also have a HTML part, so we make
// sure that only messages where the first HTML part is in the same multipart/alternative container
// as the frist plain text part are counted as HTML mail
bool Message::ComposerViewBase::isHTMLMail( KMime::Content *root )
{
  if ( !root )
    return false;

  using namespace MessageViewer;
  KMime::Content *firstTextPart = ObjectTreeParser::findType( root, "text/plain", true, true );
  KMime::Content *firstHtmlPart = ObjectTreeParser::findType( root, "text/html", true, true );
  if ( !firstTextPart || !firstHtmlPart )
    return false;

  KMime::Content *parent = firstTextPart->parent();
  if ( !parent || parent != firstHtmlPart->parent() )
    return false;

  if ( !parent->contentType()->isMultipart() ||
       parent->contentType()->subType() != "alternative" )
    return false;

  return true;
}

bool Message::ComposerViewBase::isComposing() const
{
  return !m_composers.isEmpty();
}

void Message::ComposerViewBase::setMessage ( const KMime::Message::Ptr& msg )
{

  foreach( KPIM::AttachmentPart::Ptr attachment, m_attachmentModel->attachments() )
    m_attachmentModel->removeAttachment( attachment );

  m_msg = msg;

  m_recipientsEditor->setRecipientString( m_msg->to()->mailboxes(), Recipient::To );
  m_recipientsEditor->setRecipientString( m_msg->cc()->mailboxes(), Recipient::Cc );
  m_recipientsEditor->setRecipientString( m_msg->bcc()->mailboxes(), Recipient::Bcc );
  m_recipientsEditor->setFocusBottom();

  // If we are loading from a draft, load unexpanded aliases as well
  if( m_msg->hasHeader( "X-KMail-UnExpanded-To" ) ) {
      QStringList spl = m_msg->headerByType( "X-KMail-UnExpanded-To" )->asUnicodeString().split( QLatin1String( "," ) );
      foreach( QString addr, spl )
        m_recipientsEditor->addRecipient( addr, Recipient::To );
  }
  if( m_msg->hasHeader( "X-KMail-UnExpanded-CC" ) ) {
      QStringList spl = m_msg->headerByType( "X-KMail-UnExpanded-CC" )->asUnicodeString().split( QLatin1String( "," ) );
      foreach( QString addr, spl )
        m_recipientsEditor->addRecipient( addr, Recipient::Cc );
  }
  if( m_msg->hasHeader( "X-KMail-UnExpanded-BCC" ) ) {
      QStringList spl = m_msg->headerByType( "X-KMail-UnExpanded-BCC" )->asUnicodeString().split( QLatin1String( "," ) );
      foreach( QString addr, spl )
        m_recipientsEditor->addRecipient( addr, Recipient::Bcc );
  }

  // First, we copy the message and then parse it to the object tree parser.
  // The otp gets the message text out of it, in textualContent(), and also decrypts
  // the message if necessary.
  KMime::Content *msgContent = new KMime::Content;
  msgContent->setContent( m_msg->encodedContent() );
  msgContent->parse();
  MessageViewer::EmptySource emptySource;
  MessageViewer::ObjectTreeParser otp( &emptySource );//All default are ok
  otp.parseObjectTree( msgContent );

  m_editor->setText( otp.textualContent() );

  // Load the attachments
  MessageCore::AttachmentCollector ac;
  ac.collectAttachmentsFrom( msgContent );
  for ( std::vector<KMime::Content*>::const_iterator it = ac.attachments().begin();
        it != ac.attachments().end() ; ++it ) {
    addAttachmentPart( *it );
  }
  
  QString transportName;
  if( m_msg->headerByType( "X-KMail-Transport" ) )
    transportName = m_msg->headerByType("X-KMail-Transport")->asUnicodeString();
  if ( !transportName.isEmpty() ) {
    MailTransport::Transport *transport = MailTransport::TransportManager::self()->transportByName( transportName );
    if ( transport )
      m_transport->setCurrentTransport( transport->id() );
  }

  // Set the HTML text and collect HTML images
  if ( isHTMLMail( m_msg.get() ) ) {
    KMime::Content *htmlNode = MessageViewer::ObjectTreeParser::findType( msgContent, "text/html", true, true );
    Q_ASSERT( htmlNode );
    KMime::Content *parentNode = htmlNode->parent();
    if ( parentNode && parentNode->contentType()->isMultipart() ) {
      emit enableHtml();

      const QByteArray htmlCharset = htmlNode->contentType()->charset();
      const QByteArray htmlBodyDecoded = htmlNode->decodedContent();
      const QTextCodec *codec = MessageViewer::NodeHelper::codecForName( htmlCharset );
      if ( codec ) {
        m_editor->setHtml( codec->toUnicode( htmlBodyDecoded ) );
      } else {
        m_editor->setHtml( QString::fromLocal8Bit( htmlBodyDecoded ) );
      }
    }
    collectImages( m_msg.get() );
  }
  
  if ( m_msg->headerByType( "X-KMail-CursorPos" ) ) {
    m_editor->setCursorPositionFromStart( m_msg->headerByType( "X-KMail-CursorPos" )->asUnicodeString().toInt() );
  }
}

void Message::ComposerViewBase::send ( MessageSender::SendMethod method, MessageSender::SaveIn saveIn )
{
  mSendMethod = method;
  mSaveIn = saveIn;

  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );

  m_msg->setHeader( new KMime::Headers::Generic( "X-KMail-Transport", m_msg.get(), m_transport->currentText(), "utf-8" ) );

  // Save the quote prefix which is used for this message. Each message can have
  // a different quote prefix, for example depending on the original sender.
  if ( m_editor->quotePrefixName().isEmpty() )
    m_msg->removeHeader( "X-KMail-QuotePrefix" );
  else
    m_msg->setHeader( new KMime::Headers::Generic("X-KMail-QuotePrefix", m_msg.get(), m_editor->quotePrefixName(), "utf-8" ) );

  if ( m_editor->isFormattingUsed() ) {
    kDebug() << "Html mode";
    m_msg->setHeader( new KMime::Headers::Generic("X-KMail-Markup", m_msg.get(), QLatin1String( "true" ), "utf-8" ) );
  } else {
    m_msg->removeHeader( "X-KMail-Markup" );
    kDebug() << "Plain text";
  }

  if ( m_editor->isFormattingUsed() && inlineSigningEncryptionSelected() ) {
    QString keepBtnText = m_encrypt ?
      m_sign ? i18n( "&Keep markup, do not sign/encrypt" )
      : i18n( "&Keep markup, do not encrypt" )
      : i18n( "&Keep markup, do not sign" );
    QString yesBtnText = m_encrypt ?
      m_sign ? i18n("Sign/Encrypt (delete markup)")
      : i18n( "Encrypt (delete markup)" )
      : i18n( "Sign (delete markup)" );
    int ret = KMessageBox::warningYesNoCancel( m_parentWidget,
                                               i18n("<qt><p>Inline signing/encrypting of HTML messages is not possible;</p>"
                                                    "<p>do you want to delete your markup?</p></qt>"),
                                               i18n("Sign/Encrypt Message?"),
                                               KGuiItem( yesBtnText ),
                                               KGuiItem( keepBtnText ) );
    if ( KMessageBox::Cancel == ret ) {
      return;
    }
    if ( KMessageBox::No == ret ) {
      m_encrypt = false;
      m_encrypt = false;
    } else {
      emit disableHtml( NoConfirmationNeeded );
    }
  }

  if ( m_neverEncrypt && saveIn != MessageSender::SaveInNone ) {
      // we can't use the state of the mail itself, to remember the
      // signing and encryption state, so let's add a header instead
    m_msg->setHeader( new KMime::Headers::Generic( "X-KMail-SignatureActionEnabled", m_msg.get(),
                                                   m_sign ? QLatin1String( "true" ): QLatin1String( "false" ), "utf-8" ) );
    m_msg->setHeader( new KMime::Headers::Generic( "X-KMail-EncryptActionEnabled", m_msg.get(),
                                                   m_encrypt ? QLatin1String( "true" ) : QLatin1String( "false" ), "utf-8" ) );
    m_msg->setHeader( new KMime::Headers::Generic( "X-KMail-CryptoMessageFormat", m_msg.get(), QString::number( m_cryptoMessageFormat ), "utf-8" ) );
  } else {
    m_msg->removeHeader( "X-KMail-SignatureActionEnabled" );
    m_msg->removeHeader( "X-KMail-EncryptActionEnabled" );
    m_msg->removeHeader( "X-KMail-CryptoMessageFormat" );
  }

  readyForSending();
}

void Message::ComposerViewBase::readyForSending()
{
  kDebug() << "Entering readyForSending";
  if( !m_msg ) {
    kDebug() << "m_msg == 0!";
    return;
  }

  if( !m_composers.isEmpty() ) {
    // This may happen if e.g. the autosave timer calls applyChanges.
    kDebug() << "Called while composer active; ignoring.";
    return;
  }

  // first, expand all addresses
  MessageComposer::EmailAddressResolveJob *job = new MessageComposer::EmailAddressResolveJob( this );
  job->setFrom( from() );
  job->setTo( m_recipientsEditor->recipientStringList( Recipient::To ) );
  job->setCc( m_recipientsEditor->recipientStringList( Recipient::Cc ) );
  job->setBcc( m_recipientsEditor->recipientStringList( Recipient::Bcc ) );
  connect( job, SIGNAL( result( KJob* ) ), SLOT( slotEmailAddressResolved( KJob* ) ) );
  job->start();
  
}

void Message::ComposerViewBase::slotEmailAddressResolved ( KJob* job )
{
  if ( job->error() ) {
    KMessageBox::sorry( m_parentWidget, i18n( "Expanding email addresses in message failed.\n"
                                    "%1\n",
                                    job->errorString() ),
                                    i18n( "Sending Message Failed" ) );
//     setEnabled( true );
    // TODO add string after string freeze!
    emit failed( QLatin1String( "" ) );
    return;
  }

  const MessageComposer::EmailAddressResolveJob *resolveJob = qobject_cast<MessageComposer::EmailAddressResolveJob*>( job );
  if( mSaveIn == MessageSender::SaveInNone ) {
    mExpandedFrom = resolveJob->expandedFrom();
    mExpandedTo = resolveJob->expandedTo();
    mExpandedCc = resolveJob->expandedCc();
    mExpandedBcc = resolveJob->expandedBcc();
 } else { // saved to draft, so keep the old values, not very nice.
    mExpandedFrom = from();
    foreach( const Recipient &r, m_recipientsEditor->recipients() ) {
      switch( r.type() ) {
        case Recipient::To: mExpandedTo << r.email(); break;
        case Recipient::Cc: mExpandedCc << r.email(); break;
        case Recipient::Bcc: mExpandedBcc << r.email(); break;
        case Recipient::Undefined: Q_ASSERT( !"Unknown recpient type!" ); break;
      }
    }
    QStringList unExpandedTo, unExpandedCc, unExpandedBcc;
    foreach( QString exp, resolveJob->expandedTo() ) {
      if( !mExpandedTo.contains( exp ) ) // this address was expanded, so save it explicitly
        unExpandedTo << exp;
    }
    foreach( QString exp, resolveJob->expandedCc() ) {
        if( !mExpandedCc.contains( exp ) )
        unExpandedCc << exp;
    }
    foreach( QString exp, resolveJob->expandedBcc() ) {
      if( !mExpandedBcc.contains( exp ) ) // this address was expanded, so save it explicitly
        unExpandedBcc << exp;
    }
    m_msg->setHeader( new KMime::Headers::Generic( "X-KMail-UnExpanded-To", m_msg.get(), unExpandedTo.join( QLatin1String( ", " ) ).toLatin1() ) );
    m_msg->setHeader( new KMime::Headers::Generic( "X-KMail-UnExpanded-CC", m_msg.get(), unExpandedCc.join( QLatin1String( ", " ) ).toLatin1() ) );
    m_msg->setHeader( new KMime::Headers::Generic( "X-KMail-UnExpanded-BCC", m_msg.get(), unExpandedBcc.join( QLatin1String( ", " ) ).toLatin1() ) );
  }

  // we first figure out if we need to create multiple messages with different crypto formats
  // if so, we create a composer per format
  // if we aren't signing or encrypting, this just returns a single empty message
  if( m_neverEncrypt && mSaveIn == MessageSender::SaveInNone ) {
    Message::Composer* composer = new Message::Composer;
    composer->setNoCrypto( true );
    m_composers.append( composer );
  } else {
    m_composers = generateCryptoMessages();
  }

  if( m_composers.isEmpty() ) {
    // TODO i18n after string freeze!
    emit failed( QLatin1String( "" ) );
    return;
  }
  
  // Compose each message and prepare it for queueing, sending, or storing
  foreach( Message::Composer* composer, m_composers ) {
    fillGlobalPart( composer->globalPart() );
    m_editor->fillComposerTextPart( composer->textPart() );
    fillInfoPart( composer->infoPart(), UseExpandedRecipients );

    composer->addAttachmentParts( m_attachmentModel->attachments() );

    connect( composer, SIGNAL( result( KJob*) ), this, SLOT( slotSendComposeResult( KJob* ) ) );
    composer->start();
    kDebug() << "Started a composer for sending!";

   }
}

QList< Message::Composer* > Message::ComposerViewBase::generateCryptoMessages ()
{
   QList< Message::Composer* > composers;

  kDebug() << "filling crypto info";
/*
  Kleo::KeyResolver* keyResolver = new Kleo::KeyResolver(  encryptToSelf(), showKeyApprovalDialog(),
                                                           GlobalSettings::self()->pgpAutoEncrypt(), cryptoMessageFormat(),
                                                           encryptKeyNearExpiryWarningThresholdInDays(),
                                                           signingKeyNearExpiryWarningThresholdInDays(),
                                                           encryptRootCertNearExpiryWarningThresholdInDays(),
                                                           signingRootCertNearExpiryWarningThresholdInDays(),
                                                           encryptChainCertNearExpiryWarningThresholdInDays(),
                                                           signingChainCertNearExpiryWarningThresholdInDays());
  */
  Kleo::KeyResolver* keyResolver = new Kleo::KeyResolver(  true /*encryptToSelf()*/,
                                                           false /*showKeyApprovalDialog()*/,
                                                           false /*GlobalSettings::self()->pgpAutoEncrypt()*/,
                                                           m_cryptoMessageFormat,
                                                           -1 /*encryptKeyNearExpiryWarningThresholdInDays()*/,
                                                           -1 /*signingKeyNearExpiryWarningThresholdInDays()*/,
                                                           -1 /*encryptRootCertNearExpiryWarningThresholdInDays()*/,
                                                           -1 /*signingRootCertNearExpiryWarningThresholdInDays()*/,
                                                           -1 /*encryptChainCertNearExpiryWarningThresholdInDays()*/,
                                                           -1 /*signingChainCertNearExpiryWarningThresholdInDays()*/ );

  const KPIMIdentities::Identity &id = m_identMan->identityForUoidOrDefault( m_identityCombo->currentIdentity() );

  QStringList encryptToSelfKeys;
  QStringList signKeys;

  bool signSomething = m_sign;
  foreach( KPIM::AttachmentPart::Ptr attachment, m_attachmentModel->attachments() ) {
    if( attachment->isSigned() )
      signSomething = true;
  }
  bool encryptSomething = m_encrypt;
  foreach( KPIM::AttachmentPart::Ptr attachment, m_attachmentModel->attachments() ) {
    if( attachment->isEncrypted() )
      encryptSomething = true;
  }

   if( !signSomething && !encryptSomething ) {
    composers.append( new Message::Composer() );
    return composers;
  }

  if( encryptSomething ) {
    if ( !id.pgpEncryptionKey().isEmpty() )
      encryptToSelfKeys.push_back( QLatin1String( id.pgpEncryptionKey() ) );
    if ( !id.smimeEncryptionKey().isEmpty() )
      encryptToSelfKeys.push_back( QLatin1String( id.smimeEncryptionKey() ) );
    if ( keyResolver->setEncryptToSelfKeys( encryptToSelfKeys ) != Kpgp::Ok ) {
      kDebug() << "Failed to set encryptoToSelf keys!";
      return composers;
    }
  }

  if( signSomething ) {
    if ( !id.pgpSigningKey().isEmpty() )
      signKeys.push_back( QLatin1String( id.pgpSigningKey() ) );
    if ( !id.smimeSigningKey().isEmpty() )
      signKeys.push_back( QLatin1String( id.smimeSigningKey() ) );
    if ( keyResolver->setSigningKeys( signKeys ) != Kpgp::Ok ) {
      kDebug() << "Failed to set signing keys!";
      return composers;
    }
  }

  QStringList recipients( mExpandedTo ), bcc( mExpandedBcc );
  recipients.append( mExpandedCc );

  keyResolver->setPrimaryRecipients( recipients );
  keyResolver->setSecondaryRecipients( bcc );

  if ( keyResolver->resolveAllKeys( signSomething, encryptSomething ) != Kpgp::Ok ) {
    /// TODO handle failure
    kDebug() << "failed to resolve keys! oh noes";
    return composers;
  }
  kDebug() << "done resolving keys:";

  Kleo::CryptoMessageFormat concreteEncryptFormat = Kleo::AutoFormat;
  if( encryptSomething ) {
    for ( unsigned int i = 0 ; i < numConcreteCryptoMessageFormats ; ++i ) {
      if ( keyResolver->encryptionItems( concreteCryptoMessageFormats[i] ).empty() )
        continue;

      if ( !(concreteCryptoMessageFormats[i] & m_cryptoMessageFormat) )
        continue;

      concreteEncryptFormat = concreteCryptoMessageFormats[i];

      std::vector<Kleo::KeyResolver::SplitInfo> encData = keyResolver->encryptionItems( concreteEncryptFormat );
      std::vector<Kleo::KeyResolver::SplitInfo>::iterator it;
      QList<QPair<QStringList, std::vector<GpgME::Key> > > data;
      for( it = encData.begin(); it != encData.end(); ++it ) {
        QPair<QStringList, std::vector<GpgME::Key> > p( it->recipients, it->keys );
        data.append( p );
        kDebug() << "got resolved keys for:" << it->recipients;
      }
      Message::Composer* composer =  new Message::Composer;

      composer->setEncryptionKeys( data );
      composer->setMessageCryptoFormat( concreteEncryptFormat );

      if( signSomething ) {
        // find signing keys for this format
        std::vector<GpgME::Key> signingKeys = keyResolver->signingKeys( concreteEncryptFormat );
        composer->setSigningKeys( signingKeys );
      }

      composer->setSignAndEncrypt( m_sign, m_encrypt );

      composers.append( composer );
    }
  } else if( signSomething ) { // just signing, so check sign prefs
    Kleo::CryptoMessageFormat concreteSignFormat = Kleo::AutoFormat;

    for ( unsigned int i = 0 ; i < numConcreteCryptoMessageFormats ; ++i ) {
      if ( keyResolver->encryptionItems( concreteCryptoMessageFormats[i] ).empty() )
        continue;

      if ( !(concreteCryptoMessageFormats[i] & m_cryptoMessageFormat) )
        continue;

      concreteSignFormat = concreteCryptoMessageFormats[i];
      std::vector<GpgME::Key> signingKeys = keyResolver->signingKeys( concreteEncryptFormat );

      Message::Composer* composer =  new Message::Composer;

      composer->setSigningKeys( signingKeys );
      composer->setMessageCryptoFormat( concreteSignFormat );
      composer->setSignAndEncrypt( m_sign, m_encrypt );

      composers.append( composer );
    }
  }

  if( composers.isEmpty() && ( signSomething || encryptSomething ) )
    Q_ASSERT_X( false, "ComposerViewBase::fillCryptoInfo" , "No concrete sign or encrypt method selected");

  return composers;
}

void Message::ComposerViewBase::fillGlobalPart ( Message::GlobalPart* globalPart )
{
  globalPart->setParentWidgetForGui( m_parentWidget );
  globalPart->setCharsets( m_charsets );
  globalPart->setMDNRequested( m_mdnRequested );
}

void Message::ComposerViewBase::fillInfoPart ( Message::InfoPart* infoPart, Message::ComposerViewBase::RecipientExpansion expansion )
{
   // TODO splitAddressList and expandAliases ugliness should be handled by a
  // special AddressListEdit widget... (later: see RecipientsEditor)

  if ( m_fcc ) {
    infoPart->setFcc( QString::number( m_fcc->currentCollection().id() ) );
  }

  infoPart->setTransportId( m_transport->currentTransportId() );
  infoPart->setReplyTo( replyTo() );
  if ( expansion == UseExpandedRecipients ) {
    infoPart->setFrom( mExpandedFrom );
    infoPart->setTo( mExpandedTo );
    infoPart->setCc( mExpandedCc );
    infoPart->setBcc( mExpandedBcc );
  } else {
    infoPart->setFrom( from() );
    infoPart->setTo( m_recipientsEditor->recipientStringList( Recipient::To ) );
    infoPart->setCc( m_recipientsEditor->recipientStringList( Recipient::Cc ) );
    infoPart->setBcc( m_recipientsEditor->recipientStringList( Recipient::Bcc ) );
  }
  infoPart->setSubject( subject() );
  infoPart->setUserAgent( QLatin1String( "KMail" ) );
  infoPart->setUrgent( m_urgent );

  KMime::Headers::Base::List extras;
  if( m_msg->headerByType( "X-KMail-SignatureActionEnabled" ) )
    extras << m_msg->headerByType( "X-KMail-SignatureActionEnabled" );
  if( m_msg->headerByType( "X-KMail-EncryptActionEnabled" ) )
    extras << m_msg->headerByType( "X-KMail-EncryptActionEnabled" );
  if( m_msg->headerByType( "X-KMail-CryptoMessageFormat" ) )
    extras << m_msg->headerByType( "X-KMail-CryptoMessageFormat" );
  if( m_msg->headerByType( "X-KMail-UnExpanded-To" ) )
    extras << m_msg->headerByType( "X-KMail-UnExpanded-To" );
  if( m_msg->headerByType( "X-KMail-UnExpanded-CC" ) )
    extras << m_msg->headerByType( "X-KMail-UnExpanded-CC" );
  if( m_msg->headerByType( "X-KMail-UnExpanded-BCC" ) )
    extras << m_msg->headerByType( "X-KMail-UnExpanded-BCC" );

  infoPart->setExtraHeaders( extras );
}

void Message::ComposerViewBase::slotSendComposeResult( KJob* job )
{
  kDebug() << "compose job might have error error" << job->error() << "errorString" << job->errorString();
  Q_ASSERT( dynamic_cast< Message::Composer* >( job ) );
  Composer* composer = static_cast< Message::Composer* >( job );

  Q_ASSERT( m_composers.contains( composer ) );

  if( composer->error() == Message::Composer::NoError ) {
    // The messages were composed successfully.
    kDebug() << "NoError.";
    for( int i = 0; i < composer->resultMessages().size(); ++i ) {
      if ( mSaveIn == MessageSender::SaveInNone ) {
        queueMessage( composer->resultMessages().at( i ), composer );
      } else {
        saveMessage( composer->resultMessages().at( i ), mSaveIn );
      }
    }
  } else if( composer->error() == Message::Composer::UserCancelledError ) {
    // The job warned the user about something, and the user chose to return
    // to the message.  Nothing to do.
    kDebug() << "UserCancelledError.";
    emit failed( QLatin1String( "" ) /* TODO string after freeze */ );
  } else {
    kDebug() << "other Error.";
    QString msg;
    if( composer->error() == Message::Composer::BugError ) {
      msg = i18n( "Error composing message:\n\n%1\n\nPlease report this bug.", job->errorString() );
    } else {
      msg = i18n( "Error composing message:\n\n%1", job->errorString() );
    }
    KMessageBox::sorry( m_parentWidget, msg, i18n( "Composer" ) );
    emit failed( QLatin1String( "" ) /* TODO string after freeze */ );
  }

  m_composers.removeAll( composer );
}

void Message::ComposerViewBase::queueMessage( KMime::Message::Ptr message, Message::Composer* composer )
{
  const Message::InfoPart *infoPart = composer->infoPart();
  MailTransport::MessageQueueJob *qjob = new MailTransport::MessageQueueJob( this );
  qjob->setMessage( message );
  qjob->transportAttribute().setTransportId( infoPart->transportId() );
  if( mSendMethod == MessageSender::SendLater )
    qjob->dispatchModeAttribute().setDispatchMode( MailTransport::DispatchModeAttribute::Manual );

  if ( !infoPart->fcc().isEmpty() ) {
    qjob->sentBehaviourAttribute().setSentBehaviour(
                      MailTransport::SentBehaviourAttribute::MoveToCollection );
    const Akonadi::Collection sentCollection( infoPart->fcc().toInt() );
    qjob->sentBehaviourAttribute().setMoveToCollection( sentCollection );
  } else {
    qjob->sentBehaviourAttribute().setSentBehaviour(
           MailTransport::SentBehaviourAttribute::MoveToDefaultSentCollection );
  }

  fillQueueJobHeaders( qjob, message, infoPart );
  connect( qjob, SIGNAL( result(KJob*) ), this, SLOT( slotQueueResult( KJob* ) ) );
  m_pendingQueueJobs++;
  qjob->start();

  kDebug() << "Queued a message.";
}


void Message::ComposerViewBase::slotQueueResult( KJob *job )
{
  m_pendingQueueJobs--;
  kDebug() << "mPendingQueueJobs" << m_pendingQueueJobs;
  Q_ASSERT( m_pendingQueueJobs >= 0 );

  if( job->error() ) {
    kDebug() << "Failed to queue a message:" << job->errorString();
    // There is not much we can do now, since all the MessageQueueJobs have been
    // started.  So just wait for them to finish.
    // TODO show a message box or something
    KMessageBox::sorry( m_parentWidget,
                        QString( QLatin1String( "<qt><p>%1</p><br />%2</qt>" ) ).arg( i18n("There was an error trying to queue the "
                                                                            "message for sending. The error was:" ) )
                                                               .arg( job->errorString() ),
                       i18n("Error Queueing Message") );
    if( m_pendingQueueJobs == 0 )
      emit failed( QLatin1String( "" ) );
  }

  if( m_pendingQueueJobs == 0 ) {
    emit sentSuccessfully();
  }
}

void Message::ComposerViewBase::fillQueueJobHeaders( MailTransport::MessageQueueJob* qjob, KMime::Message::Ptr message, const Message::InfoPart* infoPart )
{
  qjob->addressAttribute().setFrom( infoPart->from() );

  if( m_editor && !infoPart->bcc().isEmpty() ) // have to deal with multiple message contents
  {
    // if the bcc isn't empty, then we send it to the bcc because this is the bcc-only encrypted body
    if( !message->bcc()->addresses().isEmpty() ) {
      QStringList bcc;
      foreach( QByteArray address, message->bcc()->addresses()  ) {
        bcc << QString::fromUtf8( address );
      }
      kDebug() << "sending with-bcc encr mail to a secondary recipient:" << bcc;
      qjob->addressAttribute().setTo( bcc );
    } else {
      // the main mail in the encrypted set, just don't set the bccs here
      qjob->addressAttribute().setTo( infoPart->to() );
      qjob->addressAttribute().setCc( infoPart->cc() );

      kDebug() << "sending with-bcc encrypted mail to orig recipients:" <<infoPart->to() << infoPart->cc();

    }
  } else {
    // continue as normal
    kDebug() << "no bccs";
    qjob->addressAttribute().setTo( infoPart->to() );
    qjob->addressAttribute().setCc( infoPart->cc() );
    qjob->addressAttribute().setBcc( infoPart->bcc() );
  }
}



void Message::ComposerViewBase::saveMessage( KMime::Message::Ptr message, MessageSender::SaveIn saveIn )
{
  Akonadi::Collection target;

  if ( saveIn == MessageSender::SaveInTemplates ) {
    target = Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::Templates );
  } else {
    target = Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::Drafts );
  }

  if ( !target.isValid() ) {
    // TODO: Show an error message to the user
    kWarning() << "No default collection for" << saveIn;
    emit failed( QLatin1String( "" ) /* TODO error string after freeze" */ );
//     setEnabled( true );
    return;
  }

  // Store when the draft or template got saved.
  message->date()->setDateTime( KDateTime::currentLocalDateTime() );
  message->assemble();

  Akonadi::Item item;
  item.setMimeType( QLatin1String( "message/rfc822" ) );
  item.setPayload( message );
  Akonadi::ItemCreateJob *create = new Akonadi::ItemCreateJob( item, target, this );
  connect( create, SIGNAL( result( KJob* ) ), this, SLOT( slotCreateItemResult(KJob*) ) );
  m_pendingQueueJobs++;
}


void Message::ComposerViewBase::slotCreateItemResult( KJob *job )
{
  m_pendingQueueJobs--;
  kDebug() << "mPendingCreateItemJobs" << m_pendingQueueJobs;
  Q_ASSERT( m_pendingQueueJobs >= 0 );

  if( job->error() ) {
    // TODO: Show an error message to the user
    kWarning() << "Failed to save a message:" << job->errorString();
    emit failed( QLatin1String( "" ) /* TODO error string after freeze" */ );
    return;
  }

  if( m_pendingQueueJobs == 0 ) {
    emit sentSuccessfully();
  }
}

void Message::ComposerViewBase::addAttachment ( const KUrl& url, const QString& comment )
{
  kDebug() << "adding attachment with url:" << url;
  m_attachmentController->addAttachment( url );
}

void Message::ComposerViewBase::addAttachment ( const QString& name, const QString& filename, const QString& charset, const QByteArray& data, const QByteArray& mimeType )
{
  KPIM::AttachmentPart::Ptr attachment = KPIM::AttachmentPart::Ptr( new KPIM::AttachmentPart() );
  if( !data.isEmpty() ) {
    attachment->setName( name );
    attachment->setFileName( filename );
    attachment->setData( data );
    attachment->setCharset( charset.toLatin1() );
    attachment->setMimeType( mimeType );
    // TODO what about the other fields?

    m_attachmentController->addAttachment( attachment);
  }
}

void Message::ComposerViewBase::addAttachmentPart ( KMime::Content* partToAttach )
{
  KPIM::AttachmentPart::Ptr part( new KPIM::AttachmentPart );
  if( partToAttach->contentType()->mimeType() == "multipart/digest" ||
      partToAttach->contentType()->mimeType() == "message/rfc822" ) {
    // if it is a digest or a full message, use the encodedContent() of the attachment,
    // which already has the proper headers
    part->setData( partToAttach->encodedContent() );
    part->setMimeType( partToAttach->contentType()->mimeType() );
    part->setName( partToAttach->contentDisposition()->parameter( QLatin1String("name") ) );
  } else {
    part->setName( partToAttach->contentDescription()->asUnicodeString() );
    part->setFileName( partToAttach->contentDisposition()->filename() );
    part->setMimeType( partToAttach->contentType()->mimeType() );
    part->setData( partToAttach->decodedContent() );
  }
  m_attachmentController->addAttachment( part );
}

Message::Composer* Message::ComposerViewBase::createSimpleComposer() {
  Message::Composer* composer = new Message::Composer;
  fillGlobalPart( composer->globalPart() );
  m_editor->fillComposerTextPart( composer->textPart() );
  fillInfoPart( composer->infoPart(), UseUnExpandedRecipients );
  composer->addAttachmentParts( m_attachmentModel->attachments() );
  return composer;
}

//-----------------------------------------------------------------------------
static QString cleanedUpHeaderString( const QString &s )
{
  // remove invalid characters from the header strings
  QString res( s );
  res.remove( QChar::fromLatin1( '\r' ) );
  res.replace( QChar::fromLatin1( '\n' ), QString::fromLatin1( " " ) );
  return res.trimmed();
}

//-----------------------------------------------------------------------------
QString Message::ComposerViewBase::to() const
{
  return cleanedUpHeaderString( m_recipientsEditor->recipientString( Recipient::To ) );
}

//-----------------------------------------------------------------------------
QString Message::ComposerViewBase::cc() const
{
  return cleanedUpHeaderString( m_recipientsEditor->recipientString( Recipient::Cc ) );
}

//-----------------------------------------------------------------------------
QString Message::ComposerViewBase::bcc() const
{
  return cleanedUpHeaderString( m_recipientsEditor->recipientString( Recipient::Bcc ) );
}

QString Message::ComposerViewBase::from() const
{
  return cleanedUpHeaderString( m_from );
}

QString Message::ComposerViewBase::replyTo() const
{
  return cleanedUpHeaderString( m_replyTo );
}

QString Message::ComposerViewBase::subject() const
{
  return cleanedUpHeaderString( m_subject );
}

void Message::ComposerViewBase::setParentWidgetForGui ( QWidget* w )
{
  m_parentWidget = w;
}

void Message::ComposerViewBase::setAttachmentController( Message::AttachmentControllerBase* controller )
{
  m_attachmentController = controller;
}

Message::AttachmentControllerBase* Message::ComposerViewBase::attachmentController()
{
  return m_attachmentController;
}

void Message::ComposerViewBase::setAttachmentModel( Message::AttachmentModel* model )
{
  m_attachmentModel = model;
}

Message::AttachmentModel* Message::ComposerViewBase::attachmentModel()
{
  return m_attachmentModel;
}

void Message::ComposerViewBase::setRecipientsEditor ( MessageComposer::RecipientsEditor* recEditor )
{
  m_recipientsEditor = recEditor;
}

MessageComposer::RecipientsEditor* Message::ComposerViewBase::recipientsEditor()
{
  return m_recipientsEditor;
}

void Message::ComposerViewBase::setSignatureController(Message::SignatureController* sigController)
{
  m_signatureController = sigController;
}

Message::SignatureController* Message::ComposerViewBase::signatureController()
{
  return m_signatureController;
}

void Message::ComposerViewBase::setIdentityCombo ( KPIMIdentities::IdentityCombo* identCombo )
{
  m_identityCombo = identCombo;
}

KPIMIdentities::IdentityCombo* Message::ComposerViewBase::identityCombo()
{
  return m_identityCombo;
}

void Message::ComposerViewBase::identityChanged ( const KPIMIdentities::Identity &ident, const KPIMIdentities::Identity &oldIdent )
{
  if ( oldIdent.bcc() != ident.bcc() ) {
    m_recipientsEditor->removeRecipient( oldIdent.bcc(), Recipient::Bcc );
    m_recipientsEditor->addRecipient( ident.bcc(), Recipient::Bcc );
    m_recipientsEditor->setFocusBottom();
  }

}

void Message::ComposerViewBase::setEditor ( Message::KMeditor* editor )
{
  m_editor = editor;

  m_editor->setRichTextSupport( KRichTextWidget::FullTextFormattingSupport |
                               KRichTextWidget::FullListSupport |
                               KRichTextWidget::SupportAlignment |
                               KRichTextWidget::SupportRuleLine |
                               KRichTextWidget::SupportHyperlinks |
                               KRichTextWidget::SupportAlignment );
  m_editor->enableImageActions();

  m_editor->document()->setModified( false );

}

Message::KMeditor* Message::ComposerViewBase::editor()
{
  return m_editor;
}

void Message::ComposerViewBase::setTransportCombo ( MailTransport::TransportComboBox* transpCombo )
{
  m_transport = transpCombo;
}

MailTransport::TransportComboBox* Message::ComposerViewBase::transportComboBox()
{
  return m_transport;
}


void Message::ComposerViewBase::setIdentityManager ( KPIMIdentities::IdentityManager* identMan )
{
  m_identMan = identMan;
}

KPIMIdentities::IdentityManager* Message::ComposerViewBase::identityManager()
{
  return m_identMan;
}


void Message::ComposerViewBase::setFcc ( const Akonadi::Collection& fccCollection )
{
  if ( m_fcc ) {
    m_fcc->setDefaultCollection( fccCollection );
  }
}


void Message::ComposerViewBase::setFccCombo ( Akonadi::CollectionComboBox* fcc )
{
  m_fcc = fcc;
}

Akonadi::CollectionComboBox* Message::ComposerViewBase::fccCombo()
{

  return m_fcc;
}

void Message::ComposerViewBase::setFrom(const QString& from)
{
  m_from = from;
}

void Message::ComposerViewBase::setReplyTo(const QString& replyTo)
{
  m_replyTo = replyTo;
}

void Message::ComposerViewBase::setSubject(const QString& subject)
{
  m_subject = subject;
}


void Message::ComposerViewBase::setCryptoOptions ( bool sign, bool encrypt, Kleo::CryptoMessageFormat format, bool neverEncryptDrafts )
{
  m_sign = sign;
  m_encrypt = encrypt;
  m_cryptoMessageFormat = format;
  m_neverEncrypt = neverEncryptDrafts;
}

void Message::ComposerViewBase::setCharsets( const QList< QByteArray >& charsets )
{
  m_charsets = charsets;
}

void Message::ComposerViewBase::setMDNRequested( bool mdnRequested )
{
  m_mdnRequested = mdnRequested;
}

void Message::ComposerViewBase::setUrgent( bool urgent )
{
  m_urgent = urgent;
}

//-----------------------------------------------------------------------------
void Message::ComposerViewBase::collectImages( KMime::Content *root )
{
  if ( KMime::Content * n = MessageViewer::ObjectTreeParser::findType( root, "multipart/alternative", true, true ) ) {
    KMime::Content *parentnode = n->parent();
    if ( parentnode &&
         parentnode->contentType()->isMultipart() &&
         parentnode->contentType()->subType() == "related" ) {
      KMime::Content *node = MessageCore::NodeHelper::nextSibling( n );
      while ( node ) {
        if ( node->contentType()->isImage() ) {
          kDebug() << "found image in multipart/related : " << node->contentType()->name();
          QImage img;
          img.loadFromData( node->decodedContent() );
          m_editor->loadImage( img, QString::fromLatin1( "cid:" + node->contentID()->identifier() ),
                              node->contentType()->name() );
        }
        node = MessageCore::NodeHelper::nextSibling( node );
      }
    }
  }
}


//-----------------------------------------------------------------------------
bool Message::ComposerViewBase::inlineSigningEncryptionSelected()
{
  if ( !m_sign && !m_encrypt ) {
    return false;
  }
  return m_cryptoMessageFormat == Kleo::InlineOpenPGPFormat;
}


