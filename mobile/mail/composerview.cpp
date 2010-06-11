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

#include "global.h"
#include "declarativewidgetbase.h"
#include "declarativeidentitycombobox.h"

#include <kpimidentities/identity.h>
#include <kpimidentities/identitycombo.h>
#include <kpimidentities/identitymanager.h>
#include <mailtransport/messagequeuejob.h>
#include <mailtransport/transportcombobox.h>
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
#include <messagecomposer/attachmentmodel.h>
#include <messagecomposer/keyresolver.h>
#include <messagecomposer/kleo_util.h>
#include <messagecomposer/recipientseditor.h>
#include <akonadi/collectioncombobox.h>

#include <klocalizedstring.h>
#include <KDebug>
#include <KIcon>
#include <KAction>
#include <KMessageBox>
#include <KCMultiDialog>
#include <KFileDialog>

#include <qdeclarativecontext.h>
#include <qdeclarativeengine.h>

typedef DeclarativeWidgetBase<Message::KMeditor, ComposerView, &ComposerView::setEditor> DeclarativeEditor;
typedef DeclarativeWidgetBase<MessageComposer::RecipientsEditor, ComposerView, &ComposerView::setRecipientsEditor> DeclarativeRecipientsEditor;

QML_DECLARE_TYPE( DeclarativeEditor )
QML_DECLARE_TYPE( DeclarativeIdentityComboBox )
QML_DECLARE_TYPE( DeclarativeRecipientsEditor )

ComposerView::ComposerView(QWidget* parent) :
  KDeclarativeFullScreenView( QLatin1String( "kmail-composer" ), parent ),
  m_composerBase( 0 ),
  m_jobCount( 0 ),
  m_sign( false ),
  m_encrypt( false )
{
  setSubject( QString() );

  qmlRegisterType<DeclarativeEditor>( "org.kde.messagecomposer", 4, 5, "Editor" );
  qmlRegisterType<DeclarativeIdentityComboBox>( "org.kde.kpimidentities", 4, 5, "IdentityComboBox" );
  qmlRegisterType<DeclarativeRecipientsEditor>( "org.kde.messagecomposer", 4, 5, "RecipientsEditor" );

  // TODO: Really make this application-global;
  KAction *action = actionCollection()->addAction( "add_attachment" );
  action->setText( i18n( "Add Attachment" ) );
  action->setIcon( KIcon( "list-add" ) );
  connect(action, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), SLOT(addAttachment()));

  engine()->rootContext()->setContextProperty( "application", QVariant::fromValue( static_cast<QObject*>( this ) ) );
  connect( this, SIGNAL(statusChanged(QDeclarativeView::Status)), SLOT(qmlLoaded(QDeclarativeView::Status)) );


  m_composerBase = new Message::ComposerViewBase( this );
  m_composerBase->setIdentityManager( Global::identityManager() );

  // Temporarily only in c++, use from QML when ready.
  MailTransport::TransportComboBox* transportCombo = new  MailTransport::TransportComboBox( this );
  transportCombo->hide();
  m_composerBase->setTransportCombo( transportCombo );

  /*
  Akonadi::CollectionComboBox* fcc = new Akonadi::CollectionComboBox( this );
  fcc->setMimeTypeFilter( QStringList()<< "message/rfc822" );
  fcc->setAccessRightsFilter( Akonadi::Collection::CanCreateItem );
  fcc->setToolTip( i18n( "Select the sent-mail folder where a copy of this message will be saved" ) );
  fcc->hide();
  m_composerBase->setFccCombo( fcc );
  */
  /*
  connect( m_composerBase, SIGNAL( disableHtml( Message::ComposerViewBase::Confirmation ) ),
           this, SLOT( disableHtml( Message::ComposerViewBase::Confirmation ) ) );

  connect( m_composerBase, SIGNAL( enableHtml() ),
           this, SLOT( enableHtml() ) );
  connect( m_composerBase, SIGNAL( failed( QString) ), this, SLOT( slotSendFailed( QString ) ) ); */
  connect( m_composerBase, SIGNAL( sentSuccessfully() ), this, SLOT( slotSendSuccessful() ) );

  
  Message::AttachmentModel* attachmentModel = new Message::AttachmentModel(this);
  engine()->rootContext()->setContextProperty( "attachmentModel", QVariant::fromValue( static_cast<QObject*>( attachmentModel ) ) );
  Message::AttachmentControllerBase* attachmentController = new Message::AttachmentControllerBase(attachmentModel, this, actionCollection());
  m_composerBase->setAttachmentModel( attachmentModel );
  m_composerBase->setAttachmentController( attachmentController );
    
  action = actionCollection()->addAction("sign_email");
  action->setText( i18n( "Sign" ) );
  action->setIcon( KIcon( "document-sign" ) );
  action->setCheckable(true);
  connect(action, SIGNAL(triggered(bool)), SLOT(signEmail(bool)));

  action = actionCollection()->addAction("encrypt_email");
  action->setText( i18n( "Encrypt" ) );
  action->setIcon( KIcon( "mail-encrypt" ) );
  action->setCheckable(true);
  connect(action, SIGNAL(triggered(bool)), SLOT(encryptEmail(bool)));
}

void ComposerView::qmlLoaded ( QDeclarativeView::Status status )
{
  if ( status != QDeclarativeView::Ready )
    return;

  Q_ASSERT( m_composerBase );
  Q_ASSERT( m_composerBase->editor() );
  Q_ASSERT( m_composerBase->identityCombo()  );
  Q_ASSERT( m_composerBase->recipientsEditor()  );
  Q_ASSERT( m_composerBase->transportComboBox()  );

//   kDebug() << m_identityCombo;
//   kDebug() << m_editor;

  Message::SignatureController *signatureController = new Message::SignatureController( this );
  signatureController->setEditor( m_composerBase->editor() );
  signatureController->setIdentityCombo( m_composerBase->identityCombo() );
  signatureController->applyCurrentSignature();
  m_composerBase->setSignatureController( signatureController );

  m_composerBase->recipientsEditor()->setCompletionMode( KGlobalSettings::CompletionAuto );

  if ( m_message )
    setMessage( m_message );
}

void ComposerView::setMessage(const KMime::Message::Ptr& msg)
{
  if ( status() != QDeclarativeView::Ready )
    return;
  
  m_message = msg;
  m_subject = msg->subject()->asUnicodeString();
  m_composerBase->setMessage( msg );
  emit changed();
}

void ComposerView::send( MessageSender::SaveIn saveIn )
{
  kDebug();
  // TODO no send later support in UI atm, so hard code
  MessageSender::SendMethod method = MessageSender::SendDefault;
  const KPIMIdentities::Identity identity = m_composerBase->identityManager()->identityForUoidOrDefault( m_composerBase->identityCombo()->currentIdentity() );
  m_composerBase->setFrom( identity.fullEmailAddr() );
  m_composerBase->setReplyTo( identity.replyToAddr() );
  m_composerBase->setSubject( m_subject );

  m_composerBase->setCryptoOptions( m_sign, m_encrypt, Kleo::AutoFormat );
  
  /* Default till UI exists
  m_composerBase->setCharsets( );
  m_composerBase->setUrgent( );
  m_composerBase->setMDNRequested( ); */

  m_composerBase->send( method, saveIn );
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

QObject* ComposerView::getAction( const QString &name ) const
{
  kDebug() << actionCollection() << actionCollection()->action( name );
  return actionCollection()->action( name );
}

void ComposerView::configureIdentity()
{
  KCMultiDialog dlg;
  dlg.addModule( "kcm_kpimidentities" );
  dlg.exec();

}

void ComposerView::slotSendSuccessful() {
  deleteLater();
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
    m_composerBase->addAttachment( url, QString() );
}

#include "composerview.moc"
