/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

#include "composerview.h"

#include "declarativeeditor.h"
#include "declarativeidentitycombobox.h"
#include "declarativerecipientseditor.h"

#include <kpimidentities/identity.h>
#include <kpimidentities/identitycombo.h>
#include <kpimidentities/identitymanager.h>
#include <mailtransport/messagequeuejob.h>
#include <mailtransport/transportmanager.h>
#include <messageviewer/objecttreeemptysource.h>
#include <messageviewer/objecttreeparser.h>
#include <messagecomposer/kmeditor.h>
#include <messagecomposer/signaturecontroller.h>
#include <messagecomposer/composer.h>
#include <messagecomposer/globalpart.h>
#include <messagecomposer/infopart.h>
#include <messagecomposer/textpart.h>
#include <messagecomposer/emailaddressresolvejob.h>
#include <messagecomposer/attachmentcontrollerbase.h>

#include <klocalizedstring.h>
#include <KDebug>
#include <KIcon>
#include <KAction>
#include <KMessageBox>
#include <KCMultiDialog>

#include <qdeclarativecontext.h>
#include <qdeclarativeengine.h>
#include <KFileDialog>
#include <messagecomposer/attachmentmodel.h>

QML_DECLARE_TYPE( DeclarativeEditor )
QML_DECLARE_TYPE( DeclarativeIdentityComboBox )

ComposerView::ComposerView(QWidget* parent) :
  KDeclarativeFullScreenView( QLatin1String( "kmail-composer" ), parent ),
  m_identityCombo( 0 ),
  m_editor( 0 ),
  m_attachmentController( 0 )
{
  setSubject( QString() );

  qmlRegisterType<DeclarativeEditor>( "org.kde.messagecomposer", 4, 5, "Editor" );
  qmlRegisterType<DeclarativeIdentityComboBox>( "org.kde.kpimidentities", 4, 5, "IdentityComboBox" );
  qmlRegisterType<DeclarativeRecipientsEditor>( "org.kde.messagecomposer", 4, 5, "RecipientsEditor" );

  // TODO: Really make this application-global;
  mActionCollection = new KActionCollection( this );
  KAction *action = mActionCollection->addAction( "add_attachment" );
  action->setText( i18n( "Add Attachment" ) );
  action->setIcon( KIcon( "list-add" ) );
  connect(action, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), SLOT(addAttachment()));

  engine()->rootContext()->setContextProperty( "application", QVariant::fromValue( static_cast<QObject*>( this ) ) );
  connect( this, SIGNAL(statusChanged(QDeclarativeView::Status)), SLOT(qmlLoaded(QDeclarativeView::Status)) );

  m_attachmentModel = new Message::AttachmentModel(this);
  engine()->rootContext()->setContextProperty( "attachmentModel", QVariant::fromValue( static_cast<QObject*>( m_attachmentModel ) ) );
  m_attachmentController = new Message::AttachmentControllerBase(m_attachmentModel, this, mActionCollection);

}

void ComposerView::qmlLoaded ( QDeclarativeView::Status status )
{
  if ( status != QDeclarativeView::Ready )
    return;

  Q_ASSERT( m_identityCombo );
  Q_ASSERT( m_editor );
  Q_ASSERT( m_recipientsEditor );

  kDebug() << m_identityCombo;
  kDebug() << m_editor;

  Message::SignatureController *signatureController = new Message::SignatureController( this );
  signatureController->setEditor( m_editor );
  signatureController->setIdentityCombo( m_identityCombo );
  signatureController->applyCurrentSignature();

  if ( m_message )
    setMessageInternal( m_message );
}

void ComposerView::setMessage(const KMime::Message::Ptr& msg)
{
  m_message = msg;
  foreach(KPIM::AttachmentPart::Ptr attachment, m_attachmentModel->attachments())
    m_attachmentModel->removeAttachment(attachment);

  foreach(KMime::Content *attachment, msg->attachments())
  {
    KPIM::AttachmentPart::Ptr part( new KPIM::AttachmentPart );
    if( attachment->contentType()->mimeType() == "multipart/digest" ||
        attachment->contentType()->mimeType() == "message/rfc822" ) {
      // if it is a digest or a full message, use the encodedContent() of the attachment,
      // which already has the proper headers
      part->setData( attachment->encodedContent() );
      part->setMimeType( attachment->contentType()->mimeType() );
      part->setName( attachment->contentDisposition()->parameter( QLatin1String("name") ) );
    } else {
      part->setName( attachment->contentDescription()->asUnicodeString() );
      part->setFileName( attachment->contentDisposition()->filename() );
      part->setMimeType( attachment->contentType()->mimeType() );
      part->setData( attachment->decodedContent() );
    }
    m_attachmentController->addAttachment( part );
    m_attachmentModel->addAttachment(part);
  }

  if ( status() == QDeclarativeView::Ready )
    setMessageInternal( msg );
}

void ComposerView::setMessageInternal(const KMime::Message::Ptr& msg)
{
  // ### duplication with KMComposeWin

  m_subject = msg->subject()->asUnicodeString();

  m_recipientsEditor->setRecipientString( msg->to()->mailboxes(), Recipient::To );
  m_recipientsEditor->setRecipientString( msg->cc()->mailboxes(), Recipient::Cc );
  m_recipientsEditor->setRecipientString( msg->bcc()->mailboxes(), Recipient::Bcc );

  // First, we copy the message and then parse it to the object tree parser.
  // The otp gets the message text out of it, in textualContent(), and also decrypts
  // the message if necessary.
  KMime::Content *msgContent = new KMime::Content;
  msgContent->setContent( msg->encodedContent() );
  msgContent->parse();
  MessageViewer::EmptySource emptySource;
  MessageViewer::ObjectTreeParser otp( &emptySource );//All default are ok
  otp.parseObjectTree( msgContent );

  // Set the editor text and charset
  m_editor->setText( otp.textualContent() );

  emit changed();
}


void ComposerView::send()
{
  kDebug();
  expandAddresses();
}

void ComposerView::expandAddresses()
{
  // TODO share this with kmcomposewin.cpp
  MessageComposer::EmailAddressResolveJob *job = new MessageComposer::EmailAddressResolveJob( this );
  job->setFrom( "volker@kdab.com" ); // TODO: retrieve from identity
  job->setTo( m_recipientsEditor->recipientStringList( Recipient::To ) );
  job->setCc( m_recipientsEditor->recipientStringList( Recipient::Cc ) );
  job->setBcc( m_recipientsEditor->recipientStringList( Recipient::Bcc ) );
  connect( job, SIGNAL(result(KJob*)), SLOT(addressExpansionResult(KJob*)) );
  job->start();
}

void ComposerView::addressExpansionResult(KJob* job)
{
  if ( job->error() ) {
    kDebug() << job->error() << job->errorText();
    return;
  }

  const MessageComposer::EmailAddressResolveJob *resolveJob = qobject_cast<MessageComposer::EmailAddressResolveJob*>( job );

  // ### temporary, more code can be shared with kmail here
  Message::Composer* composer = new Message::Composer( this );
  composer->globalPart()->setCharsets( QList<QByteArray>() << "utf-8" );
  composer->globalPart()->setParentWidgetForGui( this );
  composer->infoPart()->setSubject( subject() );
  composer->infoPart()->setTo( resolveJob->expandedTo() );
  composer->infoPart()->setCc( resolveJob->expandedCc() );
  composer->infoPart()->setBcc( resolveJob->expandedBcc() );
  composer->infoPart()->setFrom( resolveJob->expandedFrom() );
  composer->infoPart()->setUserAgent( "KMail Mobile" );
  m_editor->fillComposerTextPart( composer->textPart() );
  composer->addAttachmentParts( m_attachmentModel->attachments() );
  connect( composer, SIGNAL(result(KJob*)), SLOT(composerResult(KJob*)) );
  composer->start();
}

void ComposerView::composerResult ( KJob* job )
{
  kDebug() << job->error() << job->errorText();
  if ( !job->error() ) {
    Message::Composer *composer = qobject_cast<Message::Composer*>( job );
    Q_ASSERT( composer );
    const Message::InfoPart *infoPart = composer->infoPart();
    MailTransport::MessageQueueJob *qjob = new MailTransport::MessageQueueJob( this );
    qjob->transportAttribute().setTransportId( MailTransport::TransportManager::self()->defaultTransportId() );
    qjob->setMessage( composer->resultMessages().first() );
    qjob->addressAttribute().setTo( infoPart->to() );
    qjob->addressAttribute().setCc( infoPart->cc() );
    qjob->addressAttribute().setBcc( infoPart->bcc() );
    connect( qjob, SIGNAL(result(KJob*)), SLOT(sendResult(KJob*)) );
    qjob->start();
  }
}

void ComposerView::sendResult ( KJob* job )
{
  kDebug() << job->error() << job->errorText();
  if ( !job->error() )
    deleteLater();
}

QString ComposerView::subject() const
{
  return m_subject;
}

void ComposerView::setSubject ( const QString& subject )
{
  m_subject = subject;
  if ( !subject.isEmpty() )
    setWindowTitle( subject );
  else
    setWindowTitle( i18n( "New mail" ) );
}

KActionCollection* ComposerView::actionCollection() const
{
  return mActionCollection;
}

QObject* ComposerView::getAction( const QString &name ) const
{
  kDebug() << mActionCollection << mActionCollection->action( name );
  return mActionCollection->action( name );
}

void ComposerView::configureIdentity()
{
  KCMultiDialog dlg;
  dlg.addModule( "kcm_kpimidentities" );
  dlg.exec();

}

void ComposerView::configureTransport()
{
  KCMultiDialog dlg;
  dlg.addModule( "kcm_mailtransport" );
  dlg.exec();
}

void ComposerView::addAttachment()
{
  KUrl url = KFileDialog::getOpenUrl();
  if (!url.isEmpty())
    m_attachmentController->addAttachment(url);
}

#include "composerview.moc"
