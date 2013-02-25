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
#include "kmeditor.h"
#include "emailaddressresolvejob.h"
#include "keyresolver.h"
#include "globalpart.h"
#include "kleo_util.h"
#include "infopart.h"
#include "composer.h"
#include "util.h"

#include <messageviewer/objecttreeemptysource.h>
#include <messageviewer/objecttreeparser.h>
#include <messagecore/messagehelpers.h>
#include <messagecore/stringutil.h>
#include <mailtransport/transportcombobox.h>
#include <mailtransport/messagequeuejob.h>
#include <akonadi/kmime/specialmailcollections.h>
#include <akonadi/itemcreatejob.h>
#include <akonadi/collectionfetchjob.h>
#include <kpimidentities/identitycombo.h>
#include <messagecore/attachmentcollector.h>
#include <messagecore/nodehelper.h>
#ifndef QT_NO_CURSOR
#include <messageviewer/kcursorsaver.h>
#endif
#include <mailtransport/transportmanager.h>
#include <messagecomposer/recipientseditor.h>
#include <akonadi/collectioncombobox.h>
#include <kpimidentities/identitymanager.h>
#include <kpimutils/email.h>

#include <KSaveFile>
#include <KLocalizedString>
#include <KMessageBox>
#include <krichtextwidget.h>
#include <KStandardDirs>

#include <QDir>
#include <QTimer>
#include <QUuid>
#include <QtCore/QTextCodec>
#include <recentaddresses.h>
#include "messagecomposersettings.h"
#include "messagehelper.h"
#include "util.h"

static QStringList encodeIdn( const QStringList &emails )
{
  QStringList encoded;
  foreach ( const QString &email, emails )
    encoded << KPIMUtils::normalizeAddressesAndEncodeIdn( email );

  return encoded;
}

Message::ComposerViewBase::ComposerViewBase ( QObject* parent, QWidget *parentGui)
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
 , m_fccCombo( 0 )
 , m_parentWidget( parentGui )
 , m_sign( false )
 , m_encrypt( false )
 , m_neverEncrypt( false )
 , m_mdnRequested( false )
 , m_urgent( false )
 , m_cryptoMessageFormat( Kleo::AutoFormat )
 , m_pendingQueueJobs( 0 )
 , m_autoSaveTimer( 0 )
 , m_autoSaveErrorShown( false )
 , m_autoSaveInterval( 1 * 1000 * 60 ) // default of 1 min
{
  m_charsets << "utf-8"; // default, so we have a backup in case client code forgot to set.

  initAutoSave();

}

Message::ComposerViewBase::~ComposerViewBase()
{

}

bool Message::ComposerViewBase::isComposing() const
{
  return !m_composers.isEmpty();
}

void Message::ComposerViewBase::setMessage ( const KMime::Message::Ptr& msg )
{

  foreach( MessageCore::AttachmentPart::Ptr attachment, m_attachmentModel->attachments() )
    m_attachmentModel->removeAttachment( attachment );

  m_msg = msg;
  m_recipientsEditor->clear();
  m_recipientsEditor->setRecipientString( m_msg->to()->mailboxes(), MessageComposer::Recipient::To );
  m_recipientsEditor->setRecipientString( m_msg->cc()->mailboxes(), MessageComposer::Recipient::Cc );
  m_recipientsEditor->setRecipientString( m_msg->bcc()->mailboxes(), MessageComposer::Recipient::Bcc );
  m_recipientsEditor->setFocusBottom();

  // If we are loading from a draft, load unexpanded aliases as well
  if( m_msg->hasHeader( "X-KMail-UnExpanded-To" ) ) {
      const QStringList spl = m_msg->headerByType( "X-KMail-UnExpanded-To" )->asUnicodeString().split( QLatin1String( "," ) );
      foreach( const QString& addr, spl )
        m_recipientsEditor->addRecipient( addr, MessageComposer::Recipient::To );
  }
  if( m_msg->hasHeader( "X-KMail-UnExpanded-CC" ) ) {
      const QStringList spl = m_msg->headerByType( "X-KMail-UnExpanded-CC" )->asUnicodeString().split( QLatin1String( "," ) );
      foreach( const QString& addr, spl )
        m_recipientsEditor->addRecipient( addr, MessageComposer::Recipient::Cc );
  }
  if( m_msg->hasHeader( "X-KMail-UnExpanded-BCC" ) ) {
      const QStringList spl = m_msg->headerByType( "X-KMail-UnExpanded-BCC" )->asUnicodeString().split( QLatin1String( "," ) );
      foreach( const QString& addr, spl )
        m_recipientsEditor->addRecipient( addr, MessageComposer::Recipient::Bcc );
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

  // Load the attachments
  MessageCore::AttachmentCollector ac;
  ac.collectAttachmentsFrom( msgContent );
  std::vector<KMime::Content*>::const_iterator end( ac.attachments().end() );
  for ( std::vector<KMime::Content*>::const_iterator it = ac.attachments().begin();
        it != end ; ++it ) {
    addAttachmentPart( *it );
  }

  int transportId = -1;
  if ( m_msg->headerByType( "X-KMail-Transport" ) )
    transportId = m_msg->headerByType( "X-KMail-Transport" )->asUnicodeString().toInt();

  const MailTransport::Transport *transport = MailTransport::TransportManager::self()->transportById( transportId );
  if ( transport )
    m_transport->setCurrentTransport( transport->id() );

  // Set the HTML text and collect HTML images
  if ( !otp.htmlContent().isEmpty() ) {
    m_editor->setHtml( otp.htmlContent() );
    emit enableHtml();
    collectImages( m_msg.get() );
  } else {
    m_editor->setPlainText( otp.plainTextContent() );
  }

  if ( m_msg->headerByType( "X-KMail-CursorPos" ) ) {
    m_editor->setCursorPositionFromStart( m_msg->headerByType( "X-KMail-CursorPos" )->asUnicodeString().toInt() );
  }
  delete msgContent;
}

void Message::ComposerViewBase::updateTemplate ( const KMime::Message::Ptr& msg )
{
  // First, we copy the message and then parse it to the object tree parser.
  // The otp gets the message text out of it, in textualContent(), and also decrypts
  // the message if necessary.
  KMime::Content *msgContent = new KMime::Content;
  msgContent->setContent( msg->encodedContent() );
  msgContent->parse();
  MessageViewer::EmptySource emptySource;
  MessageViewer::ObjectTreeParser otp( &emptySource );//All default are ok
  otp.parseObjectTree( msgContent );
  // Set the HTML text and collect HTML images
  if ( !otp.htmlContent().isEmpty() ) {
    m_editor->setHtml( otp.htmlContent() );
    emit enableHtml();
    collectImages( msg.get() );
  } else {
    m_editor->setPlainText( otp.plainTextContent() );
  }

  if ( msg->headerByType( "X-KMail-CursorPos" ) ) {
    m_editor->setCursorPositionFromStart( m_msg->headerByType( "X-KMail-CursorPos" )->asUnicodeString().toInt() );
  }
  delete msgContent;
}

void Message::ComposerViewBase::send ( MessageSender::SendMethod method, MessageSender::SaveIn saveIn )
{
  mSendMethod = method;
  mSaveIn = saveIn;

#ifndef QT_NO_CURSOR
  MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
#endif

  const KPIMIdentities::Identity identity = identityManager()->identityForUoid( m_identityCombo->currentIdentity() );

  if(identity.attachVcard() && m_attachmentController->attachOwnVcard()) {
    const QString vcardFileName = identity.vCardFile();
    if(!vcardFileName.isEmpty()) {
      m_attachmentController->addAttachmentUrlSync(KUrl(vcardFileName));
    }
  }
  m_msg->setHeader( new KMime::Headers::Generic( "X-KMail-Transport", m_msg.get(), QString::number(m_transport->currentTransportId()), "utf-8" ) );

  m_msg->setHeader( new KMime::Headers::Generic( "X-KMail-Fcc", m_msg.get(), QString::number( m_fccCollection.id() ) , "utf-8" ) );
  m_msg->setHeader( new KMime::Headers::Generic( "X-KMail-Identity", m_msg.get(), QString::number( identity.uoid() ), "utf-8" ));

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
      m_sign = false;
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

  if( mSendMethod == MessageSender::SendImmediate )
    Message::Util::sendMailDispatcherIsOnline( m_parentWidget );

  readyForSending();
}

void Message::ComposerViewBase::setCustomHeader( const QMap<QByteArray, QString>&customHeader )
{
  m_customHeader = customHeader;
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
  job->setTo( m_recipientsEditor->recipientStringList( MessageComposer::Recipient::To ) );
  job->setCc( m_recipientsEditor->recipientStringList( MessageComposer::Recipient::Cc ) );
  job->setBcc( m_recipientsEditor->recipientStringList( MessageComposer::Recipient::Bcc ) );
  connect( job, SIGNAL(result(KJob*)), SLOT(slotEmailAddressResolved(KJob*)) );
  job->start();
}

void Message::ComposerViewBase::slotEmailAddressResolved ( KJob* job )
{
  if ( job->error() ) {
    qWarning() << "An error occured while resolving the email addresses:" << job->errorString();
    // This error could be caused by a broken search infrastructure, so we ignore it for now
    // to not block sending emails completly.
  }
  bool autoResize = true;
  const MessageComposer::EmailAddressResolveJob *resolveJob = qobject_cast<MessageComposer::EmailAddressResolveJob*>( job );
  if( mSaveIn == MessageSender::SaveInNone ) {
    mExpandedFrom = resolveJob->expandedFrom();
    mExpandedTo = resolveJob->expandedTo();
    mExpandedCc = resolveJob->expandedCc();
    mExpandedBcc = resolveJob->expandedBcc();
  } else { // saved to draft, so keep the old values, not very nice.
    mExpandedFrom = from();
    foreach( const MessageComposer::Recipient::Ptr &r, m_recipientsEditor->recipients() ) {
      switch( r->type() ) {
        case MessageComposer::Recipient::To: mExpandedTo << r->email(); break;
        case MessageComposer::Recipient::Cc: mExpandedCc << r->email(); break;
        case MessageComposer::Recipient::Bcc: mExpandedBcc << r->email(); break;
        case MessageComposer::Recipient::Undefined: Q_ASSERT( !"Unknown recpient type!" ); break;
      }
    }
    QStringList unExpandedTo, unExpandedCc, unExpandedBcc;
    foreach( const QString &exp, resolveJob->expandedTo() ) {
      if( !mExpandedTo.contains( exp ) ) // this address was expanded, so save it explicitly
        unExpandedTo << exp;
    }
    foreach( const QString& exp, resolveJob->expandedCc() ) {
        if( !mExpandedCc.contains( exp ) )
        unExpandedCc << exp;
    }
    foreach( const QString& exp, resolveJob->expandedBcc() ) {
      if( !mExpandedBcc.contains( exp ) ) // this address was expanded, so save it explicitly
        unExpandedBcc << exp;
    }
    m_msg->setHeader( new KMime::Headers::Generic( "X-KMail-UnExpanded-To", m_msg.get(), unExpandedTo.join( QLatin1String( ", " ) ).toLatin1() ) );
    m_msg->setHeader( new KMime::Headers::Generic( "X-KMail-UnExpanded-CC", m_msg.get(), unExpandedCc.join( QLatin1String( ", " ) ).toLatin1() ) );
    m_msg->setHeader( new KMime::Headers::Generic( "X-KMail-UnExpanded-BCC", m_msg.get(), unExpandedBcc.join( QLatin1String( ", " ) ).toLatin1() ) );
    autoResize = false;
  }

  Q_ASSERT(m_composers.isEmpty()); //composers should be empty. The caller of this function
                                   //checks for emptyness before calling it
                                   //so just ensure it actually is empty
                                   //and document it
  // we first figure out if we need to create multiple messages with different crypto formats
  // if so, we create a composer per format
  // if we aren't signing or encrypting, this just returns a single empty message
  if( m_neverEncrypt && mSaveIn != MessageSender::SaveInNone ) {
    Message::Composer* composer = new Message::Composer;
    composer->setNoCrypto( true );
    m_composers.append( composer );
  } else {
    m_composers = generateCryptoMessages();
  }

  if( m_composers.isEmpty() ) {
    emit failed( i18n( "It was not possible to create a message composer." ) );
    return;
  }

  bool autoresizeImage = false;
  if(autoResize && MessageComposer::MessageComposerSettings::self()->autoResizeImageEnabled()) {
      bool hasImage = false;
      foreach( MessageCore::AttachmentPart::Ptr part, m_attachmentModel->attachments() ) {
          if ( part->mimeType() == "image/gif" ||
               part->mimeType() == "image/jpeg" ||
               part->mimeType() == "image/png" ) {
              hasImage = true;
              break;
          }
      }
      if(hasImage && MessageComposer::MessageComposerSettings::self()->askBeforeResizing()) {
          const int rc = KMessageBox::warningYesNo( m_parentWidget,i18n("Do you want to resize images?"),
                                                    i18n("Auto Resize Images"), KStandardGuiItem::yes(), KStandardGuiItem::no());
          if(rc == KMessageBox::Yes) {
              autoresizeImage = true;
          }
      } else {
          autoresizeImage = false;
      }
  }
  // Compose each message and prepare it for queueing, sending, or storing
  foreach( Message::Composer* composer, m_composers ) {
    fillGlobalPart( composer->globalPart() );
    m_editor->fillComposerTextPart( composer->textPart() );
    fillInfoPart( composer->infoPart(), UseExpandedRecipients );

    composer->addAttachmentParts( m_attachmentModel->attachments(), autoresizeImage );

    connect( composer, SIGNAL(result(KJob*)), this, SLOT(slotSendComposeResult(KJob*)) );
    composer->start();
    kDebug() << "Started a composer for sending!";

   }
}

namespace {

 // helper methods for reading encryption settings

inline int encryptKeyNearExpiryWarningThresholdInDays() {
  if ( ! MessageComposer::MessageComposerSettings::self()->cryptoWarnWhenNearExpire() ) {
    return -1;
  }
  const int num =
  MessageComposer::MessageComposerSettings::self()->cryptoWarnEncrKeyNearExpiryThresholdDays();
  return qMax( 1, num );
}

inline int signingKeyNearExpiryWarningThresholdInDays()
{
  if ( ! MessageComposer::MessageComposerSettings::self()->cryptoWarnWhenNearExpire() ) {
    return -1;
  }
  const int num =
  MessageComposer::MessageComposerSettings::self()->cryptoWarnSignKeyNearExpiryThresholdDays();
  return qMax( 1, num );
}

inline int encryptRootCertNearExpiryWarningThresholdInDays()
{
  if ( ! MessageComposer::MessageComposerSettings::self()->cryptoWarnWhenNearExpire() ) {
    return -1;
  }
  const int num =
  MessageComposer::MessageComposerSettings::self()->cryptoWarnEncrRootNearExpiryThresholdDays();
  return qMax( 1, num );
}

inline int signingRootCertNearExpiryWarningThresholdInDays() {
  if ( ! MessageComposer::MessageComposerSettings::self()->cryptoWarnWhenNearExpire() ) {
    return -1;
  }
  const int num =
  MessageComposer::MessageComposerSettings::self()->cryptoWarnSignRootNearExpiryThresholdDays();
  return qMax( 1, num );
}

inline int encryptChainCertNearExpiryWarningThresholdInDays()
{
  if ( ! MessageComposer::MessageComposerSettings::self()->cryptoWarnWhenNearExpire() ) {
    return -1;
  }
  const int num =
  MessageComposer::MessageComposerSettings::self()->cryptoWarnEncrChaincertNearExpiryThresholdDays();
  return qMax( 1, num );
}

inline int signingChainCertNearExpiryWarningThresholdInDays()
{
  if ( ! MessageComposer::MessageComposerSettings::self()->cryptoWarnWhenNearExpire() ) {
    return -1;
  }
  const int num =
  MessageComposer::MessageComposerSettings::self()->cryptoWarnSignChaincertNearExpiryThresholdDays();;
  return qMax( 1, num );
}

inline bool encryptToSelf()
{
  // return !Kpgp::Module::getKpgp() || Kpgp::Module::getKpgp()->encryptToSelf();
  return MessageComposer::MessageComposerSettings::self()->cryptoEncryptToSelf();
}

inline bool showKeyApprovalDialog()
{
  return MessageComposer::MessageComposerSettings::self()->cryptoShowKeysForApproval();
}

} // nameless namespace

QList< Message::Composer* > Message::ComposerViewBase::generateCryptoMessages ()
{

  kDebug() << "filling crypto info";
  Kleo::KeyResolver* keyResolver = new Kleo::KeyResolver(  encryptToSelf(),
                                                           showKeyApprovalDialog(),
                                                           MessageComposer::MessageComposerSettings::self()->pgpAutoEncrypt(),
                                                           m_cryptoMessageFormat,
                                                           encryptKeyNearExpiryWarningThresholdInDays(),
                                                           signingKeyNearExpiryWarningThresholdInDays(),
                                                           encryptRootCertNearExpiryWarningThresholdInDays(),
                                                           signingRootCertNearExpiryWarningThresholdInDays(),
                                                           encryptChainCertNearExpiryWarningThresholdInDays(),
                                                           signingChainCertNearExpiryWarningThresholdInDays() );

  const KPIMIdentities::Identity &id = m_identMan->identityForUoidOrDefault( m_identityCombo->currentIdentity() );

  QStringList encryptToSelfKeys;
  QStringList signKeys;

  bool signSomething = m_sign;
  bool doSignCompletely = m_sign;
  bool encryptSomething = m_encrypt;
  bool doEncryptCompletely = m_encrypt;
  foreach( MessageCore::AttachmentPart::Ptr attachment, m_attachmentModel->attachments() ) {
    if( attachment->isSigned() ) {
      signSomething = true;
    } else {
      doEncryptCompletely = false;
    }
    if( attachment->isEncrypted() ) {
      encryptSomething = true;
    } else {
      doSignCompletely = false;
    }
  }

  if( encryptSomething ) {
    if ( !id.pgpEncryptionKey().isEmpty() )
      encryptToSelfKeys.push_back( QLatin1String( id.pgpEncryptionKey() ) );
    if ( !id.smimeEncryptionKey().isEmpty() )
      encryptToSelfKeys.push_back( QLatin1String( id.smimeEncryptionKey() ) );
    if ( keyResolver->setEncryptToSelfKeys( encryptToSelfKeys ) != Kpgp::Ok ) {
      kDebug() << "Failed to set encryptoToSelf keys!";
      return QList< Message::Composer* >();
    }
  }

  if( signSomething ) {
    if ( !id.pgpSigningKey().isEmpty() )
      signKeys.push_back( QLatin1String( id.pgpSigningKey() ) );
    if ( !id.smimeSigningKey().isEmpty() )
      signKeys.push_back( QLatin1String( id.smimeSigningKey() ) );
    if ( keyResolver->setSigningKeys( signKeys ) != Kpgp::Ok ) {
      kDebug() << "Failed to set signing keys!";
      return QList< Message::Composer* >();
    }
  }

  QStringList recipients( mExpandedTo ), bcc( mExpandedBcc );
  recipients.append( mExpandedCc );

  keyResolver->setPrimaryRecipients( recipients );
  keyResolver->setSecondaryRecipients( bcc );

  bool result = true;
  signSomething = determineWhetherToSign( doSignCompletely, keyResolver,signSomething, result );
  if(!result) {
      /// TODO handle failure
      kDebug() << "failed to resolve keys! oh noes";
      emit failed( i18n( "Failed to resolve keys. Please report a bug." ) );
      return QList< Message::Composer*>();
  }

  encryptSomething = determineWhetherToEncrypt( doEncryptCompletely,keyResolver,encryptSomething, signSomething, result );
  if(!result) {
      /// TODO handle failure
      kDebug() << "failed to resolve keys! oh noes";
      emit failed( i18n( "Failed to resolve keys. Please report a bug." ) );
      return QList< Message::Composer*>();
  }

  if( !signSomething && !encryptSomething ) {
    return QList< Message::Composer* >() << new Message::Composer();
  }


  if ( keyResolver->resolveAllKeys( signSomething, encryptSomething ) != Kpgp::Ok ) {
    /// TODO handle failure
    kDebug() << "failed to resolve keys! oh noes";
    emit failed( i18n( "Failed to resolve keys. Please report a bug." ) );
    return QList< Message::Composer*>();
  }
  kDebug() << "done resolving keys:";

  QList< Message::Composer* > composers;

  if( encryptSomething ) {
    Kleo::CryptoMessageFormat concreteEncryptFormat = Kleo::AutoFormat;
    for ( unsigned int i = 0 ; i < numConcreteCryptoMessageFormats ; ++i ) {
      if ( keyResolver->encryptionItems( concreteCryptoMessageFormats[i] ).empty() )
        continue;

      if ( !(concreteCryptoMessageFormats[i] & m_cryptoMessageFormat) )
        continue;

      concreteEncryptFormat = concreteCryptoMessageFormats[i];

      std::vector<Kleo::KeyResolver::SplitInfo> encData = keyResolver->encryptionItems( concreteEncryptFormat );
      std::vector<Kleo::KeyResolver::SplitInfo>::iterator it;
      std::vector<Kleo::KeyResolver::SplitInfo>::iterator end( encData.end() );
      QList<QPair<QStringList, std::vector<GpgME::Key> > > data;
      for( it = encData.begin(); it != end; ++it ) {
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
      concreteSignFormat = concreteCryptoMessageFormats[i];
      if ( keyResolver->encryptionItems( concreteSignFormat ).empty() )
        continue;
      if ( !(concreteSignFormat & m_cryptoMessageFormat) )
        continue;

      std::vector<GpgME::Key> signingKeys = keyResolver->signingKeys( concreteSignFormat );

      Message::Composer* composer =  new Message::Composer;

      composer->setSigningKeys( signingKeys );
      composer->setMessageCryptoFormat( concreteSignFormat );
      composer->setSignAndEncrypt( m_sign, m_encrypt );

      composers.append( composer );
    }
  } else if( !signSomething && !encryptSomething ) {
     Message::Composer* composer =  new Message::Composer;
     composers.append( composer );
     //If we canceled sign or encrypt be sure to change status in attachment.
     markAllAttachmentsForSigning(false);
     markAllAttachmentsForSigning(false);
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

  if ( m_fccCombo ) {
    infoPart->setFcc( QString::number( m_fccCombo->currentCollection().id() ) );
  } else {
    if ( m_fccCollection.isValid() ) {
      infoPart->setFcc( QString::number( m_fccCollection.id() ) );
    }
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
    infoPart->setTo( m_recipientsEditor->recipientStringList( MessageComposer::Recipient::To ) );
    infoPart->setCc( m_recipientsEditor->recipientStringList( MessageComposer::Recipient::Cc ) );
    infoPart->setBcc( m_recipientsEditor->recipientStringList( MessageComposer::Recipient::Bcc ) );
  }
  infoPart->setSubject( subject() );
  infoPart->setUserAgent( QLatin1String( "KMail" ) );
  infoPart->setUrgent( m_urgent );

  if ( m_msg->inReplyTo() )
    infoPart->setInReplyTo( m_msg->inReplyTo()->asUnicodeString() );

  if ( m_msg->references() )
    infoPart->setReferences( m_msg->references()->asUnicodeString() );

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
  if( m_msg->headerByType( "Organization" ) )
    extras << m_msg->headerByType( "Organization" );
  if( m_msg->headerByType( "X-KMail-Identity" ) )
    extras << m_msg->headerByType( "X-KMail-Identity" );
  if( m_msg->headerByType( "X-KMail-Transport" ) )
    extras << m_msg->headerByType( "X-KMail-Transport" );
  if( m_msg->headerByType( "X-KMail-Fcc" ) )
    extras << m_msg->headerByType( "X-KMail-Fcc" );
  if( m_msg->headerByType( "X-KMail-Drafts" ) )
    extras << m_msg->headerByType( "X-KMail-Drafts" );
  if( m_msg->headerByType( "X-KMail-Templates" ) )
    extras << m_msg->headerByType( "X-KMail-Templates" );
  if( m_msg->headerByType( "X-KMail-Link-Message" ) )
    extras << m_msg->headerByType( "X-KMail-Link-Message" );
  if( m_msg->headerByType( "X-KMail-Link-Type" ) )
    extras << m_msg->headerByType( "X-KMail-Link-Type" );
  if( m_msg->headerByType( "X-Face" ) )
    extras << m_msg->headerByType( "X-Face" );
  infoPart->setExtraHeaders( extras );
}

void Message::ComposerViewBase::slotSendComposeResult( KJob* job )
{
  kDebug() << "compose job might have error error" << job->error() << "errorString" << job->errorString();
  Q_ASSERT( dynamic_cast< Message::Composer* >( job ) );
  Composer* composer = static_cast< Message::Composer* >( job );

  if( composer->error() == Message::Composer::NoError ) {
    Q_ASSERT( m_composers.contains( composer ) );
    // The messages were composed successfully.
    kDebug() << "NoError.";
    const int numberOfMessage( composer->resultMessages().size() );
    for( int i = 0; i < numberOfMessage; ++i ) {
      if ( mSaveIn == MessageSender::SaveInNone ) {
        queueMessage( composer->resultMessages().at( i ), composer );
      } else {
        saveMessage( composer->resultMessages().at( i ), mSaveIn );
      }
    }
    saveRecentAddresses( composer->resultMessages().at( 0 ) );
  } else if( composer->error() == Message::Composer::UserCancelledError ) {
    // The job warned the user about something, and the user chose to return
    // to the message.  Nothing to do.
    kDebug() << "UserCancelledError.";
    emit failed( i18n( "Job cancelled by the user" ) );
  } else {
    kDebug() << "other Error.";
    QString msg;
    if( composer->error() == Message::Composer::BugError ) {
      msg = i18n( "Could not compose message: %1 \n Please report this bug.", job->errorString() );
    } else {
      msg = i18n( "Could not compose message: %1", job->errorString() );
    }
    emit failed( msg );
  }

  m_composers.removeAll( composer );
}

void Message::ComposerViewBase::saveRecentAddresses( KMime::Message::Ptr msg )
{
  foreach( const QByteArray& address, msg->to()->addresses() )
    KPIM::RecentAddresses::self( MessageComposer::MessageComposerSettings::self()->config() )->add( QLatin1String( address ) );
  foreach( const QByteArray& address, msg->cc()->addresses() )
    KPIM::RecentAddresses::self( MessageComposer::MessageComposerSettings::self()->config() )->add( QLatin1String( address ) );
  foreach( const QByteArray& address, msg->bcc()->addresses() )
    KPIM::RecentAddresses::self( MessageComposer::MessageComposerSettings::self()->config() )->add( QLatin1String( address ) );
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
    qjob->sentBehaviourAttribute().setSentBehaviour( MailTransport::SentBehaviourAttribute::MoveToCollection );

    const Akonadi::Collection sentCollection( infoPart->fcc().toLongLong() );
    qjob->sentBehaviourAttribute().setMoveToCollection( sentCollection );
  } else {
    qjob->sentBehaviourAttribute().setSentBehaviour(
           MailTransport::SentBehaviourAttribute::MoveToDefaultSentCollection );
  }
  Message::Util::addSendReplyForwardAction(message, qjob);

  fillQueueJobHeaders( qjob, message, infoPart );

  MessageCore::StringUtil::removePrivateHeaderFields( message, false );

  QMapIterator<QByteArray, QString> customHeader(m_customHeader);
  while (customHeader.hasNext()) {
     customHeader.next();
     message->setHeader( new KMime::Headers::Generic( customHeader.key(), message.get(), customHeader.value(),"utf-8") );
  }
  message->assemble();

  connect( qjob, SIGNAL(result(KJob*)), this, SLOT(slotQueueResult(KJob*)) );
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
    QString msg = i18n( "There were problems trying to queue the message for sending: %1",
                        job->errorString() );

    if( m_pendingQueueJobs == 0 ) {
      emit failed( msg );
      return;
    }
  }

  if( m_pendingQueueJobs == 0 ) {
    emit sentSuccessfully();
  }
}

void Message::ComposerViewBase::fillQueueJobHeaders( MailTransport::MessageQueueJob* qjob, KMime::Message::Ptr message, const Message::InfoPart* infoPart )
{
  MailTransport::Transport *transport = MailTransport::TransportManager::self()->transportById( infoPart->transportId() );
  if ( transport && transport->specifySenderOverwriteAddress() )
    qjob->addressAttribute().setFrom( KPIMUtils::extractEmailAddress( KPIMUtils::normalizeAddressesAndEncodeIdn( transport->senderOverwriteAddress() ) ) );
  else
    qjob->addressAttribute().setFrom( KPIMUtils::extractEmailAddress( KPIMUtils::normalizeAddressesAndEncodeIdn( infoPart->from() ) ) );
  // if this header is not empty, it contains the real recipient of the message, either the primary or one of the
  //  secondary recipients. so we set that to the transport job, while leaving the message itself alone.
  if( message->hasHeader( "X-KMail-EncBccRecipients" ) ) {
    KMime::Headers::Base* realTo = message->headerByType( "X-KMail-EncBccRecipients" );
    qjob->addressAttribute().setTo( cleanEmailList( encodeIdn( realTo->asUnicodeString().split( QLatin1String( "%" ) ) ) ) );
    message->removeHeader( "X-KMail-EncBccRecipients" );
    message->assemble();
    kDebug() << "sending with-bcc encr mail to a/n recipient:" <<  qjob->addressAttribute().to();
  } else {
    qjob->addressAttribute().setTo( cleanEmailList( encodeIdn( infoPart->to() ) ) );
    qjob->addressAttribute().setCc( cleanEmailList( encodeIdn( infoPart->cc() ) ) );
    qjob->addressAttribute().setBcc( cleanEmailList( encodeIdn( infoPart->bcc() ) ) );
  }
}

void Message::ComposerViewBase::initAutoSave()
{
  kDebug() << "initalising autosave";

  // Ensure that the autosave directory exsits.
  QDir dataDirectory( KStandardDirs::locateLocal( "data", QLatin1String( "kmail2/" ) ) );
  if( !dataDirectory.exists( QLatin1String( "autosave" ) ) ) {
    kDebug() << "Creating autosave directory.";
    dataDirectory.mkdir( QLatin1String( "autosave" ) );
  }

  // Construct a file name
  if ( m_autoSaveUUID.isEmpty() ) {
    m_autoSaveUUID = QUuid::createUuid().toString();
  }

  updateAutoSave();
}


void Message::ComposerViewBase::updateAutoSave()
{
  if ( m_autoSaveInterval == 0 ) {
    delete m_autoSaveTimer; m_autoSaveTimer = 0;
  } else {
    if ( !m_autoSaveTimer ) {
      m_autoSaveTimer = new QTimer( this );
      if ( m_parentWidget )
        connect( m_autoSaveTimer, SIGNAL(timeout()),
                 m_parentWidget, SLOT(autoSaveMessage()) );
      else
        connect( m_autoSaveTimer, SIGNAL(timeout()),
                 this, SLOT(autoSaveMessage()) );

    }
    m_autoSaveTimer->start( m_autoSaveInterval );
  }
}

void Message::ComposerViewBase::cleanupAutoSave()
{
  delete m_autoSaveTimer; m_autoSaveTimer = 0;
  if ( !m_autoSaveUUID.isEmpty() ) {

    kDebug() << "deleting autosave files" << m_autoSaveUUID;

    // Delete the autosave files
    QDir autoSaveDir( KStandardDirs::locateLocal( "data", QLatin1String( "kmail2/" ) ) + QLatin1String( "autosave" ) );

    // Filter out only this composer window's autosave files
    QStringList autoSaveFilter;
    autoSaveFilter << m_autoSaveUUID + QLatin1String( "*" );
    autoSaveDir.setNameFilters( autoSaveFilter );

    // Return the files to be removed
    QStringList autoSaveFiles = autoSaveDir.entryList();
    kDebug() << "There are" << autoSaveFiles.count() << "to be deleted.";

    // Delete each file
    foreach( const QString &file, autoSaveFiles ) {
      autoSaveDir.remove( file );
    }
    m_autoSaveUUID.clear();
  }
}

//-----------------------------------------------------------------------------
void Message::ComposerViewBase::autoSaveMessage()
{
  kDebug() << "Autosaving message";

  if ( m_autoSaveTimer ) {
    m_autoSaveTimer->stop();
  }

  if( !m_composers.isEmpty() ) {
    // This may happen if e.g. the autosave timer calls applyChanges.
    kDebug() << "Called while composer active; ignoring.";
    return;
  }

  Message::Composer * const composer = createSimpleComposer();
  composer->setAutoSave( true );
  m_composers.append( composer );
  connect( composer, SIGNAL(result(KJob*)), this, SLOT(slotAutoSaveComposeResult(KJob*)) );
  composer->start();
}

void Message::ComposerViewBase::setAutoSaveFileName( const QString &fileName )
{
  m_autoSaveUUID = fileName;

  emit modified( true );
}

void Message::ComposerViewBase::slotAutoSaveComposeResult( KJob *job )
{
  using Message::Composer;

  Q_ASSERT( dynamic_cast< Composer* >( job ) );
  Composer* composer = static_cast< Composer* >( job );

  if( composer->error() == Composer::NoError ) {
    Q_ASSERT( m_composers.contains( composer ) );

    // The messages were composed successfully. Only save the first message, there should
    // only be one anyway, since crypto is disabled.
    kDebug() << "NoError.";
    writeAutoSaveToDisk( composer->resultMessages().first() );
    Q_ASSERT( composer->resultMessages().size() == 1 );

    if( m_autoSaveInterval > 0 ) {
      updateAutoSave();
    }
  } else if( composer->error() == Message::Composer::UserCancelledError ) {
    // The job warned the user about something, and the user chose to return
    // to the message.  Nothing to do.
    kDebug() << "UserCancelledError.";
    emit failed( i18n( "Job cancelled by the user" ), AutoSave );
  } else {
    kDebug() << "other Error.";
    emit failed( i18n( "Could not autosave message: %1", job->errorString() ), AutoSave );
  }

  m_composers.removeAll( composer );
}

void Message::ComposerViewBase::writeAutoSaveToDisk( const KMime::Message::Ptr& message )
{
  const QString filename = KStandardDirs::locateLocal( "data", QLatin1String( "kmail2/" ) ) + QLatin1String( "autosave/" ) +
    m_autoSaveUUID;
  KSaveFile file( filename );
  QString errorMessage;
  kDebug() << "Writing message to disk as" << filename;

  if( file.open() ) {
    file.setPermissions( QFile::ReadUser | QFile::WriteUser );

    if( file.write( message->encodedContent() ) !=
        static_cast<qint64>( message->encodedContent().size() ) ) {
      errorMessage = i18n( "Could not write all data to file." );
    }
    else {
      if( !file.finalize() ) {
        errorMessage = i18n( "Could not finalize the file." );
      }
    }
  }
  else {
    errorMessage = i18n( "Could not open file." );
  }

  if ( !errorMessage.isEmpty() ) {
    kWarning() << "Auto saving failed:" << errorMessage << file.errorString();
    if ( !m_autoSaveErrorShown ) {
      KMessageBox::sorry( m_parentWidget, i18n( "Autosaving the message as %1 failed.\n"
                                      "%2\n"
                                      "Reason: %3",
                                      filename,
                                      errorMessage,
                                      file.errorString() ),
                          i18n( "Autosaving Message Failed" ) );

      // Error dialog shown, hide the errors the next time
      m_autoSaveErrorShown = true;
    }
  }
  else {
    // No error occurred, the next error should be shown again
    m_autoSaveErrorShown = false;
  }
}

void Message::ComposerViewBase::saveMessage( KMime::Message::Ptr message, MessageSender::SaveIn saveIn )
{
  Akonadi::Collection target;
  const KPIMIdentities::Identity identity = identityManager()->identityForUoid( m_identityCombo->currentIdentity() );
  message->date()->setDateTime( KDateTime::currentLocalDateTime() );
  message->assemble();

  Akonadi::Item item;
  item.setMimeType( QLatin1String( "message/rfc822" ) );
  item.setPayload( message );
  if ( !identity.isNull() ) { // we have a valid identity
    if ( saveIn == MessageSender::SaveInTemplates ) {
      if ( !identity.templates().isEmpty() ) { // the user has specified a custom templates collection
        target = Akonadi::Collection( identity.templates().toLongLong() );
      }
    } else {
      if ( !identity.drafts().isEmpty() ) { // the user has specified a custom drafts collection
        target = Akonadi::Collection( identity.drafts().toLongLong() );
      }
    }
    Akonadi::CollectionFetchJob *saveMessageJob = new Akonadi::CollectionFetchJob( target, Akonadi::CollectionFetchJob::Base );
    saveMessageJob->setProperty( "Akonadi::Item" , QVariant::fromValue( item )  );
    QObject::connect( saveMessageJob, SIGNAL(result(KJob*)), this, SLOT(slotSaveMessage(KJob*)) );
  } else {
    // preinitialize with the default collections
    if ( saveIn == MessageSender::SaveInTemplates ) {
      target = Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::Templates );
    } else {
      target = Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::Drafts );
    }
    Akonadi::ItemCreateJob *create = new Akonadi::ItemCreateJob( item, target, this );
    connect( create, SIGNAL(result(KJob*)), this, SLOT(slotCreateItemResult(KJob*)) );
    m_pendingQueueJobs++;
  }
}

void Message::ComposerViewBase::slotSaveMessage( KJob* job )
{
  Akonadi::Collection target;
  Akonadi::Item item = job->property( "Akonadi::Item" ).value<Akonadi::Item>();
  if( job->error() ) {
    target = defaultSpecialTarget();
  } else {
    const Akonadi::CollectionFetchJob *fetchJob = qobject_cast<Akonadi::CollectionFetchJob*>( job );
    if(fetchJob->collections().isEmpty())
      target = defaultSpecialTarget();
    else
      target = fetchJob->collections().first();
  }
  Akonadi::ItemCreateJob *create = new Akonadi::ItemCreateJob( item, target, this );
  connect( create, SIGNAL(result(KJob*)), this, SLOT(slotCreateItemResult(KJob*)) );
  m_pendingQueueJobs++;
}

Akonadi::Collection Message::ComposerViewBase::defaultSpecialTarget() const
{
  Akonadi::Collection target;
  if ( mSaveIn == MessageSender::SaveInTemplates ) {
    target = Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::Templates );
  } else {
    target = Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::Drafts );
  }
  return target;
}

void Message::ComposerViewBase::slotCreateItemResult( KJob *job )
{
  m_pendingQueueJobs--;
  kDebug() << "mPendingCreateItemJobs" << m_pendingQueueJobs;
  Q_ASSERT( m_pendingQueueJobs >= 0 );

  if( job->error() ) {
    kWarning() << "Failed to save a message:" << job->errorString();
    emit failed( i18n( "Failed to save the message: %1", job->errorString() ) );
    return;
  }

  if( m_pendingQueueJobs == 0 ) {
    emit sentSuccessfully();
  }
}

void Message::ComposerViewBase::addAttachment ( const KUrl& url, const QString& comment )
{
  Q_UNUSED( comment );
  kDebug() << "adding attachment with url:" << url;
  m_attachmentController->addAttachment( url );
}

void Message::ComposerViewBase::addAttachmentUrlSync ( const KUrl& url, const QString& comment )
{
  Q_UNUSED( comment );
  kDebug() << "adding attachment with url:" << url;
  m_attachmentController->addAttachmentUrlSync( url );
}


void Message::ComposerViewBase::addAttachment ( const QString& name, const QString& filename, const QString& charset, const QByteArray& data, const QByteArray& mimeType )
{
  MessageCore::AttachmentPart::Ptr attachment = MessageCore::AttachmentPart::Ptr( new MessageCore::AttachmentPart() );
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
  MessageCore::AttachmentPart::Ptr part( new MessageCore::AttachmentPart );
  if( partToAttach->contentType()->mimeType() == "multipart/digest" ||
      partToAttach->contentType()->mimeType() == "message/rfc822" ) {
    // if it is a digest or a full message, use the encodedContent() of the attachment,
    // which already has the proper headers
    part->setData( partToAttach->encodedContent() );
  } else {
    part->setData( partToAttach->decodedContent() );
  }
  part->setMimeType( partToAttach->contentType()->mimeType() );
  if ( partToAttach->contentDescription( false ) ) {
    part->setDescription( partToAttach->contentDescription()->asUnicodeString() );
  }
  if ( partToAttach->contentType( false ) ) {
    if ( partToAttach->contentType()->hasParameter( QLatin1String( "name" ) ) ) {
      part->setName( partToAttach->contentType()->parameter( QLatin1String( "name" ) ) );
    }
  }
  if ( partToAttach->contentDisposition( false ) ) {
    part->setFileName( partToAttach->contentDisposition()->filename() );
    part->setInline( partToAttach->contentDisposition()->disposition() == KMime::Headers::CDinline );
  }
  if ( part->name().isEmpty() && !part->fileName().isEmpty() ) {
    part->setName( part->fileName() );
  }
  if ( part->fileName().isEmpty() && !part->name().isEmpty() ) {
    part->setFileName( part->name() );
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
QString Message::ComposerViewBase::to() const
{
  return Message::Util::cleanedUpHeaderString( m_recipientsEditor->recipientString( MessageComposer::Recipient::To ) );
}

//-----------------------------------------------------------------------------
QString Message::ComposerViewBase::cc() const
{
  return Message::Util::cleanedUpHeaderString( m_recipientsEditor->recipientString( MessageComposer::Recipient::Cc ) );
}

//-----------------------------------------------------------------------------
QString Message::ComposerViewBase::bcc() const
{
  return Message::Util::cleanedUpHeaderString( m_recipientsEditor->recipientString( MessageComposer::Recipient::Bcc ) );
}

QString Message::ComposerViewBase::from() const
{
  return Message::Util::cleanedUpHeaderString( m_from );
}

QString Message::ComposerViewBase::replyTo() const
{
  return Message::Util::cleanedUpHeaderString( m_replyTo );
}

QString Message::ComposerViewBase::subject() const
{
  return Message::Util::cleanedUpHeaderString( m_subject );
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

void Message::ComposerViewBase::updateRecipients( const KPIMIdentities::Identity &ident, const KPIMIdentities::Identity &oldIdent, MessageComposer::Recipient::Type type )
{
  QString oldIdentList;
  QString newIdentList;
  if ( type == MessageComposer::Recipient::Bcc ) {
    oldIdentList = oldIdent.bcc();
    newIdentList = ident.bcc();

  } else if ( type == MessageComposer::Recipient::Cc ) {
    oldIdentList = oldIdent.cc();
    newIdentList = ident.cc();
  } else {
    return;
  }

  if ( oldIdentList != newIdentList ) {
    const KMime::Types::Mailbox::List oldRecipients = MessageCore::StringUtil::mailboxListFromUnicodeString( oldIdentList );
    foreach ( const KMime::Types::Mailbox &recipient, oldRecipients ) {
      m_recipientsEditor->removeRecipient( MessageCore::StringUtil::mailboxListToUnicodeString( KMime::Types::Mailbox::List() << recipient ),
                                           type );
    }

    const KMime::Types::Mailbox::List newRecipients = MessageCore::StringUtil::mailboxListFromUnicodeString( newIdentList );
    foreach ( const KMime::Types::Mailbox &recipient, newRecipients ) {
      m_recipientsEditor->addRecipient( MessageCore::StringUtil::mailboxListToUnicodeString( KMime::Types::Mailbox::List() << recipient ),
                                        type );
    }
    m_recipientsEditor->setFocusBottom();
  }
}

void Message::ComposerViewBase::identityChanged ( const KPIMIdentities::Identity &ident, const KPIMIdentities::Identity &oldIdent, bool msgCleared )
{
  updateRecipients( ident, oldIdent, MessageComposer::Recipient::Bcc );
  updateRecipients( ident, oldIdent, MessageComposer::Recipient::Cc );

  KPIMIdentities::Signature oldSig = const_cast<KPIMIdentities::Identity&>
                                               ( oldIdent ).signature();
  KPIMIdentities::Signature newSig = const_cast<KPIMIdentities::Identity&>
                                               ( ident ).signature();
  //replace existing signatures
  const bool replaced = editor()->replaceSignature( oldSig, newSig );
  // Just append the signature if there was no old signature
  if ( !replaced && ( msgCleared || oldSig.rawText().isEmpty() ) ) {
    signatureController()->applySignature( newSig );
  }
  const QString vcardFileName = ident.vCardFile();
  attachmentController()->setIdentityHasOwnVcard(!vcardFileName.isEmpty());
  attachmentController()->setAttachOwnVcard(ident.attachVcard());

  m_editor->setAutocorrectionLanguage(ident.autocorrectionLanguage());
}

void Message::ComposerViewBase::setEditor ( Message::KMeditor* editor )
{
  m_editor = editor;

  m_editor->setRichTextSupport( KRichTextWidget::FullTextFormattingSupport |
                               KRichTextWidget::FullListSupport |
                               KRichTextWidget::SupportDirection |
                               KRichTextWidget::SupportAlignment |
                               KRichTextWidget::SupportRuleLine |
                               KRichTextWidget::SupportHyperlinks );
  m_editor->enableImageActions();
  m_editor->enableEmoticonActions();
  m_editor->enableInsertHtmlActions();
  m_editor->enableInsertTableActions();

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
  if ( m_fccCombo ) {
    m_fccCombo->setDefaultCollection( fccCollection );
  } else {
    m_fccCollection = fccCollection;
  }
  Akonadi::CollectionFetchJob * const checkFccCollectionJob =
    new Akonadi::CollectionFetchJob( fccCollection, Akonadi::CollectionFetchJob::Base );
  connect( checkFccCollectionJob, SIGNAL(result(KJob*)),
           SLOT(slotFccCollectionCheckResult(KJob*)) );
}

void Message::ComposerViewBase::slotFccCollectionCheckResult( KJob* job )
{
  if( job->error() ) {
    const Akonadi::Collection sentMailCol =
      Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::SentMail );
    if ( m_fccCombo ) {
      m_fccCombo->setDefaultCollection( sentMailCol );
    } else {
      m_fccCollection = sentMailCol;
    }
  }
}

void Message::ComposerViewBase::setFccCombo ( Akonadi::CollectionComboBox* fcc )
{
  m_fccCombo = fcc;
}

Akonadi::CollectionComboBox* Message::ComposerViewBase::fccCombo()
{
  return m_fccCombo;
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

void Message::ComposerViewBase::setAutoSaveInterval( int interval )
{
  m_autoSaveInterval = interval;
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

QStringList Message::ComposerViewBase::cleanEmailList(const QStringList& emails)
{
  QStringList clean;
  foreach( const QString& email, emails )
    clean << KPIMUtils::extractEmailAddress( email );
  return clean;
}

int Message::ComposerViewBase::autoSaveInterval() const
{
  return m_autoSaveInterval;
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

bool Message::ComposerViewBase::hasMissingAttachments( const QStringList& attachmentKeywords )
{
  if ( attachmentKeywords.isEmpty() )
    return false;
  if ( m_attachmentModel->rowCount() > 0 ) {
    return false;
  }

  QStringList attachWordsList = attachmentKeywords;

  QRegExp rx ( QString::fromLatin1("\\b") +
               attachWordsList.join( QString::fromLatin1("\\b|\\b") ) +
               QString::fromLatin1("\\b") );
  rx.setCaseSensitivity( Qt::CaseInsensitive );

  // check whether the subject contains one of the attachment key words
  // unless the message is a reply or a forwarded message
  const QString subj = subject();
  bool gotMatch = ( MessageHelper::stripOffPrefixes( subj ) == subj ) && ( rx.indexIn( subj ) >= 0 );

  if ( !gotMatch ) {
    // check whether the non-quoted text contains one of the attachment key
    // words
    QRegExp quotationRx( QString::fromLatin1("^([ \\t]*([|>:}#]|[A-Za-z]+>))+") );
    QTextDocument *doc = m_editor->document();
    QTextBlock end( doc->end() );
    for ( QTextBlock it = doc->begin(); it != end; it = it.next() ) {
      const QString line = it.text();
      gotMatch = ( quotationRx.indexIn( line ) < 0 ) &&
                 ( rx.indexIn( line ) >= 0 );
      if ( gotMatch ) {
        break;
      }
    }
  }

  if ( !gotMatch )
    return false;
  return true;
}

Message::ComposerViewBase::MissingAttachment Message::ComposerViewBase::checkForMissingAttachments( const QStringList& attachmentKeywords )
{
  if(!hasMissingAttachments( attachmentKeywords )) {
    return NoMissingAttachmentFound;
  }
  int rc = KMessageBox::warningYesNoCancel( m_editor,
                                            i18n("The message you have composed seems to refer to an "
                                                "attached file but you have not attached anything.\n"
                                                "Do you want to attach a file to your message?"),
                                            i18n("File Attachment Reminder"),
                                            KGuiItem(i18n("&Attach File...")),
                                            KGuiItem(i18n("&Send as Is")) );
  if ( rc == KMessageBox::Cancel )
    return FoundMissingAttachmentAndCancel;
  if ( rc == KMessageBox::Yes ) {
    m_attachmentController->showAddAttachmentDialog();
    return FoundMissingAttachmentAndAddedAttachment;
  }

  return FoundMissingAttachmentAndSending;
}



void Message::ComposerViewBase::markAllAttachmentsForSigning(bool sign)
{
  foreach( MessageCore::AttachmentPart::Ptr attachment, m_attachmentModel->attachments() ) {
    if( attachment->isSigned() ) {
       attachment->setSigned(sign);
    }
  }
}

void Message::ComposerViewBase::markAllAttachmentsForEncryption(bool encrypt)
{
  foreach( MessageCore::AttachmentPart::Ptr attachment, m_attachmentModel->attachments() ) {
    if( attachment->isEncrypted() ) {
        attachment->setEncrypted(encrypt);
    }
  }
}

bool Message::ComposerViewBase::determineWhetherToSign( bool doSignCompletely, Kleo::KeyResolver* keyResolver, bool signSomething, bool & result )
{
    bool sign = false;
    switch ( keyResolver->checkSigningPreferences( signSomething ) ) {
    case Kleo::DoIt:
        if ( !signSomething ) {
            markAllAttachmentsForSigning( true );
            return true;
        }
        sign = true;
        break;
    case Kleo::DontDoIt:
        sign = false;
        break;
    case Kleo::AskOpportunistic:
        assert( 0 );
    case Kleo::Ask:
    {
        // the user wants to be asked or has to be asked
#ifndef QT_NO_CURSOR
        MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
#endif
        const QString msg = i18n("Examination of the recipient's signing preferences "
                                 "yielded that you be asked whether or not to sign "
                                 "this message.\n"
                                 "Sign this message?");
        switch ( KMessageBox::questionYesNoCancel( m_parentWidget, msg,
                                                   i18n("Sign Message?"),
                                                   KGuiItem( i18nc("to sign","&Sign") ),
                                                   KGuiItem( i18n("Do &Not Sign") ) ) ) {
        case KMessageBox::Cancel:
            result = false;
            return false;
        case KMessageBox::Yes:
            markAllAttachmentsForSigning( true );
            return true;
        case KMessageBox::No:
            markAllAttachmentsForSigning( false );
            return false;
        }
    }
        break;
    case Kleo::Conflict:
    {
        // warn the user that there are conflicting signing preferences
#ifndef QT_NO_CURSOR
        MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
#endif
        const QString msg = i18n("There are conflicting signing preferences "
                                 "for these recipients.\n"
                                 "Sign this message?");
        switch ( KMessageBox::warningYesNoCancel( m_parentWidget, msg,
                                                  i18n("Sign Message?"),
                                                  KGuiItem( i18nc("to sign","&Sign") ),
                                                  KGuiItem( i18n("Do &Not Sign") ) ) ) {
        case KMessageBox::Cancel:
            result = false;
            return false;
        case KMessageBox::Yes:
            markAllAttachmentsForSigning( true );
            return true;
        case KMessageBox::No:
            markAllAttachmentsForSigning( false );
            return false;
        }
    }
        break;
    case Kleo::Impossible:
    {
#ifndef QT_NO_CURSOR
        MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
#endif
        const QString msg = i18n("You have requested to sign this message, "
                                 "but no valid signing keys have been configured "
                                 "for this identity.");
        if ( KMessageBox::warningContinueCancel( m_parentWidget, msg,
                                                 i18n("Send Unsigned?"),
                                                 KGuiItem( i18n("Send &Unsigned") ) )
             == KMessageBox::Cancel ) {
            result = false;
            return false;
        } else {
            markAllAttachmentsForSigning( false );
            return false;
        }
    }
    }

    if ( !sign || !doSignCompletely ) {
        if ( MessageComposer::MessageComposerSettings::self()->cryptoWarningUnsigned() ) {
#ifndef QT_NO_CURSOR
        MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
#endif
            const QString msg = sign && !doSignCompletely ?
                        i18n("Some parts of this message will not be signed.\n"
                             "Sending only partially signed messages might violate site policy.\n"
                             "Sign all parts instead?") // oh, I hate this...
                      : i18n("This message will not be signed.\n"
                             "Sending unsigned message might violate site policy.\n"
                             "Sign message instead?"); // oh, I hate this...
            const QString buttonText = sign && !doSignCompletely
                    ? i18n("&Sign All Parts") : i18n("&Sign");
            switch ( KMessageBox::warningYesNoCancel( m_parentWidget, msg,
                                                      i18n("Unsigned-Message Warning"),
                                                      KGuiItem( buttonText ),
                                                      KGuiItem( i18n("Send &As Is") ) ) ) {
            case KMessageBox::Cancel:
                result = false;
                return false;
            case KMessageBox::Yes:
                markAllAttachmentsForSigning( true );
                return true;
            case KMessageBox::No:
                return sign || doSignCompletely;
            }
        }
    }
    return sign || doSignCompletely;
}

bool Message::ComposerViewBase::determineWhetherToEncrypt( bool doEncryptCompletely, Kleo::KeyResolver* keyResolver, bool encryptSomething, bool signSomething, bool & result )
{
    bool encrypt = false;
    bool opportunistic = false;
    switch ( keyResolver->checkEncryptionPreferences( encryptSomething ) ) {
    case Kleo::DoIt:
        if ( !encryptSomething ) {
            markAllAttachmentsForEncryption( true );
            return true;
        }
        encrypt = true;
        break;
    case Kleo::DontDoIt:
        encrypt = false;
        break;
    case Kleo::AskOpportunistic:
        opportunistic = true;
        // fall through...
    case Kleo::Ask:
    {
        // the user wants to be asked or has to be asked
#ifndef QT_NO_CURSOR
        MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
#endif
        const QString msg = opportunistic
                ? i18n("Valid trusted encryption keys were found for all recipients.\n"
                       "Encrypt this message?")
                : i18n("Examination of the recipient's encryption preferences "
                       "yielded that you be asked whether or not to encrypt "
                       "this message.\n"
                       "Encrypt this message?");
        switch ( KMessageBox::questionYesNoCancel( m_parentWidget, msg,
                                                   i18n("Encrypt Message?"),
                                                   KGuiItem( signSomething
                                                             ? i18n("Sign && &Encrypt")
                                                             : i18n("&Encrypt") ),
                                                   KGuiItem( signSomething
                                                             ? i18n("&Sign Only")
                                                             : i18n("&Send As-Is") ) ) ) {
        case KMessageBox::Cancel:
            result = false;
            return false;
        case KMessageBox::Yes:
            markAllAttachmentsForEncryption( true );
            return true;
        case KMessageBox::No:
            markAllAttachmentsForEncryption( false );
            return false;
        }
    }
        break;
    case Kleo::Conflict:
    {
        // warn the user that there are conflicting encryption preferences
#ifndef QT_NO_CURSOR
        MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
#endif
        const QString msg = i18n("There are conflicting encryption preferences "
                                 "for these recipients.\n"
                                 "Encrypt this message?");
        switch ( KMessageBox::warningYesNoCancel( m_parentWidget, msg,
                                                  i18n("Encrypt Message?"),
                                                  KGuiItem( i18n("&Encrypt") ),
                                                  KGuiItem( i18n("Do &Not Encrypt")) ) ) {
        case KMessageBox::Cancel:
            result = false;
            return false;
        case KMessageBox::Yes:
            markAllAttachmentsForEncryption( true );
            return true;
        case KMessageBox::No:
            markAllAttachmentsForEncryption( false );
            return false;
        }
    }
        break;
    case Kleo::Impossible:
    {
#ifndef QT_NO_CURSOR
        MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
#endif
        const QString msg = i18n("You have requested to encrypt this message, "
                                 "and to encrypt a copy to yourself, "
                                 "but no valid trusted encryption keys have been "
                                 "configured for this identity.");
        if ( KMessageBox::warningContinueCancel( m_parentWidget, msg,
                                                 i18n("Send Unencrypted?"),
                                                 KGuiItem( i18n("Send &Unencrypted") ) )
             == KMessageBox::Cancel ) {
            result = false;
            return false;
        } else {
            markAllAttachmentsForEncryption( false );
            return false;
        }
    }
    }

    if ( !encrypt || !doEncryptCompletely ) {
        if ( MessageComposer::MessageComposerSettings::self()->cryptoWarningUnencrypted() ) {
#ifndef QT_NO_CURSOR
            MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
#endif
            const QString msg = !doEncryptCompletely ?
                        i18n("Some parts of this message will not be encrypted.\n"
                             "Sending only partially encrypted messages might violate "
                             "site policy and/or leak sensitive information.\n"
                             "Encrypt all parts instead?") // oh, I hate this...
                      : i18n("This message will not be encrypted.\n"
                             "Sending unencrypted messages might violate site policy and/or "
                             "leak sensitive information.\n"
                             "Encrypt messages instead?"); // oh, I hate this...
            const QString buttonText = !doEncryptCompletely
                    ? i18n("&Encrypt All Parts") : i18n("&Encrypt");
            switch ( KMessageBox::warningYesNoCancel( m_parentWidget, msg,
                                                      i18n("Unencrypted Message Warning"),
                                                      KGuiItem( buttonText ),
                                                      KGuiItem( signSomething
                                                                ? i18n("&Sign Only")
                                                                : i18n("&Send As-Is")) ) ) {
            case KMessageBox::Cancel:
                result = false;
                return false;
            case KMessageBox::Yes:
                markAllAttachmentsForEncryption( true );
                return true;
            case KMessageBox::No:
                return encrypt || doEncryptCompletely;
            }
        }
    }

    return encrypt || doEncryptCompletely;
}

#include "composerviewbase.moc"
