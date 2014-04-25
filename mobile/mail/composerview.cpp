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

#include "attachmenteditor.h"
#include "composerautoresizer.h"
#include "cryptoformatselectiondialog.h"
#include "declarativeidentitycombobox.h"
#include "declarativewidgetbase.h"
#include "mobilekernel.h"
#include "settings.h"
#include "snippetseditor.h"

#include <kpimidentities/identity.h>
#include <kpimidentities/identitycombo.h>
#include <kpimidentities/identitymanager.h>
#include <mailtransport/messagequeuejob.h>
#include <mailtransport/transportcombobox.h>
#include <mailtransport/transportmanager.h>
#include <messageviewer/settings/globalsettings.h>
#include <messageviewer/viewer/objecttreeemptysource.h>
#include <messageviewer/viewer/objecttreeparser.h>
#include <messagecomposer/composer/kmeditor.h>
#include <messagecomposer/composer/signaturecontroller.h>
#include <messagecomposer/composer/composer.h>
#include <messagecomposer/part/globalpart.h>
#include <messagecomposer/part/infopart.h>
#include <messagecomposer/part/textpart.h>
#include <messagecomposer/job/emailaddressresolvejob.h>
#include <messagecomposer/attachment/attachmentcontrollerbase.h>
#include <messagecomposer/attachment/attachmentmodel.h>
#include <messagecomposer/utils/kleo_util.h>
#include <messagecomposer/settings/messagecomposersettings.h>
#include <messagecomposer/recipient/recipientseditor.h>
#include <messagecomposer/utils/util.h>
#include <AkonadiWidgets/collectioncombobox.h>

#include <klocalizedstring.h>
#include <KDebug>
#include <KIcon>
#include <KAction>
#include <KMessageBox>
#include <KCMultiDialog>
#include <KNotification>

#include <QtCore/QTimer>
#include <qdeclarativecontext.h>
#include <qdeclarativeengine.h>
#include <qplatformdefs.h>

class DeclarativeEditor : public DeclarativeWidgetBase<MessageComposer::KMeditor, ComposerView, &ComposerView::setEditor>
{
   Q_OBJECT
   Q_PROPERTY( int availableScreenHeight READ availableScreenHeight WRITE setAvailableScreenHeight )
  public:
    int availableScreenHeight() { return widget()->property( "availableScreenHeight" ).toInt(); }
    void setAvailableScreenHeight( int height ) { widget()->setProperty( "availableScreenHeight", height ); }
};

typedef DeclarativeWidgetBase<MessageComposer::RecipientsEditor, ComposerView, &ComposerView::setRecipientsEditor> DeclarativeRecipientsEditor;

QML_DECLARE_TYPE( DeclarativeEditor )
QML_DECLARE_TYPE( DeclarativeIdentityComboBox )
QML_DECLARE_TYPE( DeclarativeRecipientsEditor )

ComposerView::ComposerView(QWidget* parent) :
  KDeclarativeFullScreenView( QLatin1String( "kmail-composer" ), parent ),
  m_composerBase( 0 ),
  m_jobCount( 0 ),
  m_sign( false ),
  m_encrypt( false ),
  m_busy( false ),
  m_draft( false ),
  m_urgent( false ),
  m_mdnRequested( Settings::self()->composerRequestMDN() ),
  m_cryptoFormat( Kleo::AutoFormat ),
  m_presetIdentity( 0 ),
  m_mayAutoSign( true )
{
  setSubject( QString() );
  setAttribute(Qt::WA_DeleteOnClose);
}

void ComposerView::doDelayedInit()
{
  kDebug();
  qmlRegisterType<DeclarativeEditor>( "org.kde.messagecomposer", 4, 5, "Editor" );
  qmlRegisterType<DeclarativeIdentityComboBox>( "org.kde.kpimidentities", 4, 5, "IdentityComboBox" );
  qmlRegisterType<DeclarativeRecipientsEditor>( "org.kde.messagecomposer", 4, 5, "RecipientsEditor" );

  engine()->rootContext()->setContextProperty( QLatin1String("application"), QVariant::fromValue( static_cast<QObject*>( this ) ) );
  connect( this, SIGNAL(statusChanged(QDeclarativeView::Status)), SLOT(qmlLoaded(QDeclarativeView::Status)) );

  m_snippetsEditor = new SnippetsEditor( actionCollection(), this );
  engine()->rootContext()->setContextProperty( QLatin1String("snippetsEditor"), m_snippetsEditor );
  engine()->rootContext()->setContextProperty( QLatin1String("snippetsModel"), m_snippetsEditor->model() );

  // ### TODO: make this happens later to show the composer as fast as possible
  m_composerBase = new MessageComposer::ComposerViewBase( this );
  m_composerBase->setIdentityManager( MobileKernel::self()->identityManager() );

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


  connect( m_composerBase, SIGNAL(disableHtml(MessageComposer::ComposerViewBase::Confirmation)),
           this, SLOT(disableHtml(MessageComposer::ComposerViewBase::Confirmation)) );
  connect( m_composerBase, SIGNAL(enableHtml()),this, SLOT(enableHtml()) );

  connect( m_composerBase, SIGNAL(sentSuccessfully()), this, SLOT(sendSuccessful()) );
  connect( m_composerBase, SIGNAL(failed(QString)), this, SLOT(failed(QString)) );
  connect( m_composerBase, SIGNAL(sentSuccessfully()), this, SLOT(success()) );

  MessageComposer::AttachmentModel* attachmentModel = new MessageComposer::AttachmentModel(this);
  engine()->rootContext()->setContextProperty( QLatin1String("attachmentModel"), QVariant::fromValue( static_cast<QObject*>( attachmentModel ) ) );
  MessageComposer::AttachmentControllerBase* attachmentController = new MessageComposer::AttachmentControllerBase(attachmentModel, this, actionCollection());
  attachmentController->createActions();
  m_composerBase->setAttachmentModel( attachmentModel );
  m_composerBase->setAttachmentController( attachmentController );

  AttachmentEditor *attachmentEditor = new AttachmentEditor( actionCollection(), attachmentModel, attachmentController, this );
  engine()->rootContext()->setContextProperty( QLatin1String("attachmentEditor"), attachmentEditor );

  KAction *action = actionCollection()->addAction(QLatin1String("sign_email"));
  action->setText( i18n( "Sign" ) );
  action->setIcon( KIcon( QLatin1String("document-sign") ) );
  action->setCheckable(true);
  connect(action, SIGNAL(triggered(bool)), SLOT(signEmail(bool)));

  action = actionCollection()->addAction(QLatin1String("encrypt_email"));
  action->setText( i18n( "Encrypt" ) );
  action->setIcon( KIcon( QLatin1String("mail-encrypt") ) );
  action->setCheckable(true);
  connect(action, SIGNAL(triggered(bool)), SLOT(encryptEmail(bool)));

  action = actionCollection()->addAction( QLatin1String("send_later") );
  action->setText( i18n( "Send Later" ) );
  connect( action, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), SLOT(sendLater()) );

  action = actionCollection()->addAction(QLatin1String("save_in_drafts"));
  action->setText( i18n( "Save As Draft" ) );
  action->setIcon( KIcon( QLatin1String("document-save" )) );
  connect(action, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), SLOT(saveDraft()));

  action = actionCollection()->addAction(QLatin1String("save_as_template"));
  action->setText( i18n( "Save As Template" ) );
  connect(action, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), SLOT(saveAsTemplate()));

  action = actionCollection()->addAction(QLatin1String("composer_clean_spaces"));
  action->setText( i18n( "Clean Spaces" ) );

  action = actionCollection()->addAction( QLatin1String("composer_add_quote_char") );
  action->setText( i18n( "Add Quote Characters" ) );

  action = actionCollection()->addAction( QLatin1String("composer_remove_quote_char") );
  action->setText( i18n( "Remove Quote Characters" ) );

  action = actionCollection()->addAction( QLatin1String("composer_spell_check") );
  action->setText( i18n( "Check Spelling" ) );

  action = actionCollection()->addAction( QLatin1String("composer_search") );
  action->setText( i18n( "Search in Email" ) );

  action = actionCollection()->addAction( QLatin1String("composer_search_next") );
  action->setText( i18n( "Continue Search" ) );

  action = actionCollection()->addAction( QLatin1String("composer_replace") );
  action->setText( i18n( "Replace" ) );

  action = actionCollection()->addAction( QLatin1String("composer_append_signature") );
  action->setText( i18n( "Append Signature" ) );

  action = actionCollection()->addAction( QLatin1String("composer_prepend_signature") );
  action->setText( i18n( "Prepend Signature" ) );

  action = actionCollection()->addAction( QLatin1String("composer_insert_signature") );
  action->setText( i18n( "Insert Signature at Cursor Position" ) );

  action = actionCollection()->addAction( QLatin1String("options_mark_as_urgent") );
  action->setText( i18n( "Urgent" ) );
  action->setCheckable( true );
  connect( action, SIGNAL(triggered(bool)), SLOT(urgentEmail(bool)) );

  action = actionCollection()->addAction( QLatin1String("options_request_mdn") );
  action->setText( i18n( "Request Notification" ) );
  action->setCheckable( true );
  action->setChecked( m_mdnRequested );
  connect( action, SIGNAL(triggered(bool)), SLOT(requestMdn(bool)) );

  action = actionCollection()->addAction( QLatin1String("options_wordwrap") );
  action->setText( i18n( "Wordwrap" ) );
  action->setCheckable( true );
  action->setChecked( MessageComposer::MessageComposerSettings::self()->wordWrap() );
  connect( action, SIGNAL(triggered(bool)), SLOT(toggleAutomaticWordWrap(bool)) );

  action = actionCollection()->addAction( QLatin1String("options_fixedfont") );
  action->setText( i18n( "Use Fixed Font" ) );
  action->setCheckable( true );
  action->setChecked( MessageViewer::GlobalSettings::self()->useFixedFont() );
  connect( action, SIGNAL(triggered(bool)), SLOT(toggleUseFixedFont(bool)) );

  action = actionCollection()->addAction( QLatin1String("options_set_cryptoformat"));
  action->setText( i18n( "Crypto Message Format" ) );
  connect( action, SIGNAL(triggered(bool)), SLOT(setCryptoFormat()) );

  actionCollection()->action( QLatin1String("attach_public_key") )->setText( i18n( "Attach Public Key" ) );
  actionCollection()->action( QLatin1String("composer_insert_signature") )->setText( i18n( "Insert Signature at Cursor Position" ) );
}

void ComposerView::setIdentityCombo( KPIMIdentities::IdentityCombo* combo )
{
  m_composerBase->setIdentityCombo( combo );

  if ( m_presetIdentity != 0 ) {
    m_currentIdentity = m_presetIdentity;
    m_composerBase->identityCombo()->setCurrentIdentity( m_presetIdentity );
  } else {
    m_currentIdentity = m_composerBase->identityCombo()->currentIdentity();
  }

  connect( combo, SIGNAL(identityChanged(uint)), SLOT(identityChanged(uint)) );
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
  Q_ASSERT( m_composerBase->attachmentModel() );
  Q_ASSERT( m_composerBase->attachmentController() );

//   kDebug() << m_identityCombo;
//   kDebug() << m_editor;

  MessageComposer::SignatureController *signatureController = new MessageComposer::SignatureController( this );
  signatureController->setEditor( m_composerBase->editor() );
  signatureController->setIdentityCombo( m_composerBase->identityCombo() );
  signatureController->suspend(); // ComposerView::identityChanged will update the signature
  m_composerBase->setSignatureController( signatureController );

  if ( MessageComposer::MessageComposerSettings::self()->autoTextSignature() == QLatin1String( "auto" ) && m_mayAutoSign ) {
    if ( MessageComposer::MessageComposerSettings::self()->prependSignature() ) {
      QTimer::singleShot( 0, m_composerBase->signatureController(), SLOT(prependSignature()) );
    } else {
      QTimer::singleShot( 0, m_composerBase->signatureController(), SLOT(appendSignature()) );
    }
  }

  connect( actionCollection()->action( QLatin1String("composer_clean_spaces") ), SIGNAL(triggered(bool)), signatureController, SLOT(cleanSpace()) );
  connect( actionCollection()->action( QLatin1String("composer_append_signature") ), SIGNAL(triggered(bool)), signatureController, SLOT(appendSignature()) );
  connect( actionCollection()->action( QLatin1String("composer_prepend_signature") ), SIGNAL(triggered(bool)), signatureController, SLOT(prependSignature()) );
  connect( actionCollection()->action( QLatin1String("composer_insert_signature") ), SIGNAL(triggered(bool)), signatureController, SLOT(insertSignatureAtCursor()) );

  toggleAutomaticWordWrap( actionCollection()->action( QLatin1String("options_wordwrap") )->isChecked() );
  toggleUseFixedFont( actionCollection()->action( QLatin1String("options_fixedfont") )->isChecked() );

  m_composerBase->recipientsEditor()->setAutoResizeView( true );

  connect( m_composerBase->recipientsEditor(), SIGNAL(lineAdded(KPIM::MultiplyingLine*)),
           SIGNAL(recipientsCountChanged()) );
  connect( m_composerBase->recipientsEditor(), SIGNAL(lineDeleted(int)),
           SIGNAL(recipientsCountChanged()) );

  m_snippetsEditor->setEditor( m_composerBase->editor(), "insertPlainText", SIGNAL(insertSnippet()) );

  if ( m_message )
    setMessage( m_message, m_mayAutoSign );

  connect( MailTransport::TransportManager::self(), SIGNAL(transportsChanged()), SLOT(transportsChanged()) );
}

void ComposerView::setMessage(const KMime::Message::Ptr& msg, bool mayAutoSign)
{
  m_message = msg;
  m_mayAutoSign = mayAutoSign;
  if ( status() != QDeclarativeView::Ready )
    return;

  m_subject = msg->subject()->asUnicodeString();
  m_composerBase->setMessage( msg );

  //###: See comment in setAutoSaveFileName
  if ( !m_fileName.isEmpty() )
    m_composerBase->setAutoSaveFileName( m_fileName );

  emit changed();
}

void ComposerView::send( MessageComposer::MessageSender::SendMethod method, MessageComposer::MessageSender::SaveIn saveIn )
{
  kDebug();

  if ( !m_composerBase->editor()->checkExternalEditorFinished() )
    return;

  if ( m_composerBase->recipientsEditor()->recipients().isEmpty()
    &&  saveIn != MessageComposer::MessageSender::SaveInDrafts && saveIn != MessageComposer::MessageSender::SaveInTemplates ) {
      KMessageBox::sorry( this,
                          i18n("You should specify at least one recipient for this message."),
                          i18n("No recipients found"));
      return;
  }

  if ( m_subject.isEmpty() && saveIn != MessageComposer::MessageSender::SaveInDrafts && saveIn != MessageComposer::MessageSender::SaveInTemplates ) {
      const int rc = KMessageBox::questionYesNo( this,
                                                 i18n("You did not specify a subject. Do you want to send the message without specifying one?"),
                                                 i18n("No subject"));
      if ( rc == KMessageBox::No) {
          return;
      }
  }

  m_composerBase->setSubject( m_subject ); //needed by checkForMissingAttachments

  if ( Settings::self()->composerDetectMissingAttachments() && m_composerBase->checkForMissingAttachments( MessageComposer::Util::AttachmentKeywords() ) ) {
    return;
  }

  setBusy(true);

  const KPIMIdentities::Identity identity = m_composerBase->identityManager()->identityForUoidOrDefault( m_composerBase->identityCombo()->currentIdentity() );
  m_composerBase->setFrom( identity.fullEmailAddr() );
  m_composerBase->setReplyTo( identity.replyToAddr() );

  if ( !identity.fcc().isEmpty() ) {
    const Akonadi::Collection customSentFolder( identity.fcc().toLongLong() );
    m_composerBase->setFcc( customSentFolder );
  }

  m_composerBase->setCryptoOptions( m_sign, m_encrypt, m_cryptoFormat );

  // Default till UI exists
  //  m_composerBase->setCharsets( );
  m_composerBase->setUrgent( m_urgent );
  m_composerBase->setMDNRequested( m_mdnRequested );

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

bool ComposerView::busy() const
{
    return m_busy;
}

void ComposerView::setBusy(bool busy)
{
    if (m_busy == busy)
        return;

    m_busy = busy;
    emit busyChanged();
}

QObject* ComposerView::getAction( const QString &name ) const
{
  kDebug() << actionCollection() << actionCollection()->action( name );
  return actionCollection()->action( name );
}

void ComposerView::configureIdentity()
{
  KCMultiDialog dlg;
  dlg.addModule( QLatin1String("kcm_kpimidentities") );
  dlg.currentPage()->setHeader( QLatin1String( "" ) ); // hide header to save space
  dlg.setButtons( KDialog::Ok | KDialog::Cancel );
  dlg.exec();
}

void ComposerView::sendSuccessful()
{
  // Removed successfully sent messages from autosave
  m_composerBase->cleanupAutoSave();
  deleteLater();
}

void ComposerView::configureTransport()
{
  KCMultiDialog dlg;
  dlg.addModule( QLatin1String("kcm_mailtransport") );
  dlg.currentPage()->setHeader( QLatin1String( "" ) ); // hide header to save space
  dlg.setButtons( KDialog::Ok | KDialog::Cancel );
  dlg.exec();
}

void ComposerView::addAttachment(KMime::Content* part)
{
  if ( part ) {
    m_composerBase->addAttachmentPart( part );
  }
}

void ComposerView::success()
{
  if (m_draft) {
    m_draft = false;
    return;
  }
}

void ComposerView::failed( const QString &errorMessage )
{
  QPixmap pix = KIcon(QLatin1String("kmail-mobile")).pixmap(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
  KNotification *notify = new KNotification(QLatin1String("sendfailed"));
  notify->setComponentData(KComponentData("kmail-mobile"));
  notify->setPixmap(pix);
  notify->setText(i18nc("Notification when there was an error while trying to send an email",
                        "Error while trying to send email. %1", errorMessage));
  notify->sendEvent();
  setBusy( false );
}

void ComposerView::transportsChanged()
{
  if ( m_composerBase->transportComboBox() )
    m_composerBase->transportComboBox()->setCurrentTransport( MailTransport::TransportManager::self()->defaultTransportId() );
}

void ComposerView::identityChanged( uint newIdentity )
{
  const KPIMIdentities::Identity identity = MobileKernel::self()->identityManager()->identityForUoid( newIdentity );
  const KPIMIdentities::Identity oldIdentity = MobileKernel::self()->identityManager()->identityForUoid( m_currentIdentity );
  m_composerBase->identityChanged( identity, oldIdentity );

  if ( !identity.isNull() && !identity.transport().isEmpty() ) {
    if ( m_composerBase->transportComboBox() )
      m_composerBase->transportComboBox()->setCurrentTransport( identity.transport().toInt() );
  }

  m_currentIdentity = newIdentity;
}

void ComposerView::setEditor( MessageComposer::KMeditor* editor )
{
    new ComposerAutoResizer(editor);
    m_composerBase->setEditor( editor );
    m_composerBase->editor()->createActions( actionCollection() );
    m_composerBase->editor()->setAutocorrection(MobileKernel::self()->composerAutoCorrection());
    connect( actionCollection()->action( QLatin1String("composer_add_quote_char") ), SIGNAL(triggered(bool)), m_composerBase->editor(), SLOT(slotAddQuotes()) );
    connect( actionCollection()->action( QLatin1String("composer_remove_quote_char") ), SIGNAL(triggered(bool)), m_composerBase->editor(), SLOT(slotRemoveQuotes()) );
    connect( actionCollection()->action( QLatin1String("composer_spell_check") ), SIGNAL(triggered(bool)), m_composerBase->editor(), SLOT(checkSpelling()) );
    connect( actionCollection()->action( QLatin1String("composer_search") ), SIGNAL(triggered(bool)), m_composerBase->editor(), SLOT(slotFind()) );
    connect( actionCollection()->action( QLatin1String("composer_search_next") ), SIGNAL(triggered(bool)), m_composerBase->editor(), SLOT(slotFindNext()) );
    connect( actionCollection()->action( QLatin1String("composer_replace") ), SIGNAL(triggered(bool)), m_composerBase->editor(), SLOT(slotReplace()) );
}

void ComposerView::setRecipientsEditor( MessageComposer::RecipientsEditor *editor )
{
  m_composerBase->setRecipientsEditor( editor );
}

void ComposerView::closeEvent( QCloseEvent * event )
{
  if ( m_composerBase->editor()->document()->isModified() || m_composerBase->recipientsEditor()->isModified() || !m_subject.isEmpty() ) {
    const QString saveButton = i18n("&Save as Draft");
    const QString saveText = i18n("Save this message in the Drafts folder. ");

    const int rc = KMessageBox::warningYesNoCancel( this,
                                                    i18n("Do you want to save the message for later or discard it?"),
                                                    i18n("Close Composer"),
                                                    KGuiItem(saveButton, QLatin1String("document-save"), QString(), saveText),
                                                    KStandardGuiItem::discard(),
                                                    KStandardGuiItem::cancel() );

    if ( rc == KMessageBox::Yes ) {
      connect( m_composerBase, SIGNAL(sentSuccessfully()), this, SLOT(deleteLater()) );
      saveDraft();
      event->ignore();
      return;
    } else if (rc == KMessageBox::Cancel ) {
      event->ignore();
      return;
    } else {
      // remove autosaves if the message was discarded
      m_composerBase->cleanupAutoSave();
    }
  } else {
    // discard empty autosave files as well
    m_composerBase->cleanupAutoSave();
  }

  event->accept();
}

void ComposerView::sendLater()
{
  const MessageComposer::MessageSender::SendMethod method = MessageComposer::MessageSender::SendLater;
  const MessageComposer::MessageSender::SaveIn saveIn = MessageComposer::MessageSender::SaveInNone;
  send ( method, saveIn );
}

void ComposerView::saveDraft()
{
  const MessageComposer::MessageSender::SendMethod method = MessageComposer::MessageSender::SendLater;
  const MessageComposer::MessageSender::SaveIn saveIn = MessageComposer::MessageSender::SaveInDrafts;
  m_draft = true;
  send ( method, saveIn );
}

void ComposerView::saveAsTemplate()
{
  const MessageComposer::MessageSender::SendMethod method = MessageComposer::MessageSender::SendLater;
  const MessageComposer::MessageSender::SaveIn saveIn = MessageComposer::MessageSender::SaveInTemplates;
  send ( method, saveIn );
}

bool ComposerView::isSigned() const
{
  return m_sign;
}

bool ComposerView::isEncrypted() const
{
  return m_encrypt;
}

bool ComposerView::tooManyRecipients() const
{
  const int threshold = Settings::self()->recipientThreshold();
  return (recipientsCount() > threshold);
}

int ComposerView::recipientsCount() const
{
  if ( !Settings::self()->tooManyRecipients() )
    return 0;

  if ( !m_composerBase->recipientsEditor() )
    return 0;

  return m_composerBase->recipientsEditor()->recipients().count();
}

void ComposerView::setIdentity( uint identity )
{
  // cache the value here, because the QML identity combobox has not been created yet
  m_presetIdentity = identity;
}

void ComposerView::signEmail( bool sign )
{
  m_sign = sign;
  emit cryptoStateChanged();
}

void ComposerView::encryptEmail( bool encrypt )
{
  m_encrypt = encrypt;
  emit cryptoStateChanged();
}

void ComposerView::toggleUseFixedFont( bool use )
{
  m_composerBase->editor()->setFontForWholeText( use ? KGlobalSettings::fixedFont() : KGlobalSettings::generalFont() );
}

void ComposerView::toggleAutomaticWordWrap( bool use )
{
  if ( use )
    m_composerBase->editor()->enableWordWrap( MessageComposer::MessageComposerSettings::self()->lineWrapWidth() );
  else
    m_composerBase->editor()->disableWordWrap();
}

void ComposerView::setCryptoFormat()
{
  CryptoFormatSelectionDialog dlg( this );
  dlg.setCryptoFormat( m_cryptoFormat );

  if ( dlg.exec() )
    m_cryptoFormat = dlg.cryptoFormat();
}

void ComposerView::enableHtml()
{
  m_composerBase->editor()->enableRichTextMode();
  m_composerBase->editor()->updateActionStates();
  m_composerBase->editor()->setActionsEnabled( true );
}

void ComposerView::disableHtml( MessageComposer::ComposerViewBase::Confirmation confirmation )
{
  if ( confirmation == MessageComposer::ComposerViewBase::LetUserConfirm && m_composerBase->editor()->isFormattingUsed() ) {
    int choice = KMessageBox::warningContinueCancel( this, i18n( "Turning HTML mode off "
        "will cause the text to lose the formatting. Are you sure?" ),
        i18n( "Lose the formatting?" ), KGuiItem( i18n( "Lose Formatting" ) ), KStandardGuiItem::cancel(),
              QLatin1String("LoseFormattingWarning") );
    if ( choice != KMessageBox::Continue ) {
      enableHtml();
      return;
    }
  }

  m_composerBase->editor()->switchToPlainText();
  m_composerBase->editor()->setActionsEnabled( false );
}

void ComposerView::setAutoSaveFileName(const QString &fileName)
{
  m_fileName = fileName;
  //###: the idea is to set the filename directly in ComposerViewBase,
  // but it is not working as expected yet.
  //m_composerBase->setAutoSaveFileName( fileName );
}


#include "composerview.moc"

