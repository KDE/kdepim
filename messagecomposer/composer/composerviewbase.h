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

#ifndef COMPOSER_VIEW_BASE_H
#define COMPOSER_VIEW_BASE_H

#include "messagecomposer_export.h"
#include "sender/messagesender.h"
#include "messagecomposer/recipient/recipient.h"

#include <akonadi/collection.h>
#include <KDE/KMime/Message>

#include <QObject>
#include <KUrl>
#include <kleo/enum.h>

class QTimer;
class QAction;
class KJob;
class QWidget;

namespace Akonadi
{
  class CollectionComboBox;
}

namespace MailTransport
{
  class TransportComboBox;
class MessageQueueJob;
}

namespace KPIMIdentities
{
  class IdentityCombo;
  class Identity;
  class IdentityManager;
}

namespace Kleo
{
  class KeyResolver;
}

namespace MessageComposer {
class RecipientsEditor;
class KMeditor;
class InfoPart;
class GlobalPart;
class Composer;
class AttachmentControllerBase;
class AttachmentModel;
class SignatureController;
/**
 *
 */
class MESSAGECOMPOSER_EXPORT ComposerViewBase : public QObject
{
  Q_OBJECT
public:
  explicit ComposerViewBase ( QObject* parent = 0, QWidget *parentGui = 0 );
  virtual ~ComposerViewBase();

  enum Confirmation { LetUserConfirm, NoConfirmationNeeded };
  enum MissingAttachment { NoMissingAttachmentFound, FoundMissingAttachmentAndSending, FoundMissingAttachmentAndAddedAttachment, FoundMissingAttachmentAndCancel };

  enum FailedType { Sending, AutoSave };

  /**
   * Set the message to be opened in the composer window, and set the internal data structures to
   *  keep track of it.
   */
  void setMessage( const KMime::Message::Ptr& newMsg );

  void updateTemplate ( const KMime::Message::Ptr& msg );

  /**
   * Send the message with the specified method, saving it in the specified folder.
   */
  void send( MessageComposer::MessageSender::SendMethod method, MessageComposer::MessageSender::SaveIn saveIn );

  /**
   * Returns true if there is at least one composer job running.
   */
  bool isComposing() const;

  /**
   * Add the given attachment to the message.
   */
  void addAttachment( const KUrl &url, const QString &comment );
  void addAttachmentUrlSync ( const KUrl& url, const QString& comment );
  void addAttachment ( const QString& name, const QString& filename, const QString& charset, const QByteArray& data, const QByteArray& mimeType );
  void addAttachmentPart( KMime::Content* part );

  MessageComposer::Composer* createSimpleComposer();

  /**
    * Header fields in recipients editor.
    */
  QString to() const;
  QString cc() const;
  QString bcc() const;
  QString from() const;
  QString replyTo() const;
  QString subject() const;

  /**
   * The following are for setting the various options and widgets in the
   *  composer.
   */
  void setAttachmentModel( MessageComposer::AttachmentModel* model );
  MessageComposer::AttachmentModel* attachmentModel();

  void setAttachmentController( MessageComposer::AttachmentControllerBase* controller );
  MessageComposer::AttachmentControllerBase* attachmentController();

  void setRecipientsEditor( MessageComposer::RecipientsEditor* recEditor );
  MessageComposer::RecipientsEditor* recipientsEditor();

  void setSignatureController( MessageComposer::SignatureController* sigController );
  MessageComposer::SignatureController* signatureController();

  void setIdentityCombo( KPIMIdentities::IdentityCombo* identCombo );
  KPIMIdentities::IdentityCombo* identityCombo();

  void setIdentityManager( KPIMIdentities::IdentityManager* identMan );
  KPIMIdentities::IdentityManager* identityManager();

  void setEditor( MessageComposer::KMeditor* editor );
  MessageComposer::KMeditor* editor();

  void setTransportCombo( MailTransport::TransportComboBox* transpCombo );
  MailTransport::TransportComboBox* transportComboBox();

  void setFccCombo( Akonadi::CollectionComboBox* fcc );
  Akonadi::CollectionComboBox* fccCombo();
  void setFcc( const Akonadi::Collection& id );

  /**
   * Widgets for editing differ in client classes, so
   *  values are set before sending.
   */
  void setFrom( const QString& from );
  void setReplyTo( const QString& replyTo );
  void setSubject( const QString& subject );

  /**
   * The following are various settings the user can modify when composing a message. If they are not set,
   *  the default values will be used.
   */
  void setCryptoOptions( bool sign, bool encrypt, Kleo::CryptoMessageFormat format, bool neverEncryptDrafts = false );
  void setCharsets( const QList< QByteArray >& charsets );
  void setMDNRequested( bool mdnRequested );
  void setUrgent( bool urgent );

  void setAutoSaveInterval( int interval );
  void setCustomHeader( const QMap<QByteArray, QString>&customHeader );

  /**
   * Enables/disables autosaving depending on the value of the autosave
   * interval.
   */
  void updateAutoSave();

  /**
    * Sets the filename to use when autosaving something. This is used when the client recovers
    * the autosave files: It calls this method, so that the composer uses the same filename again.
    * That way, the recovered autosave file is properly cleaned up in cleanupAutoSave():
    */
  void setAutoSaveFileName( const QString &fileName );

  /**
    * Stop autosaving and delete the autosaved message.
    */
  void cleanupAutoSave();

  void setParentWidgetForGui( QWidget* );

  /**
   * Check if the mail has references to attachments, but no attachments are added to it.
   * If missing attachments are found, a dialog to add new attachments is shown.
   * @param attachmentKeywords a list with the keywords that indicate an attachment should be present
   * @return NoMissingAttachmentFound, if there is attachment in email
   *         FoundMissingAttachmentAndCancelSending, if mail might miss attachment but sending
   *         FoundMissingAttachmentAndAddedAttachment, if mail might miss attachment and we added an attachment
   *         FoundMissingAttachmentAndCancel, if mail might miss attachment and cancel sending
   */
  ComposerViewBase::MissingAttachment checkForMissingAttachments( const QStringList &attachmentKeywords ) ;

  bool hasMissingAttachments( const QStringList& attachmentKeywords );

public slots:

  void identityChanged( const KPIMIdentities::Identity &ident, const KPIMIdentities::Identity &oldIdent, bool msgCleared = false);

  /**
   * Save the message.
   */
  void autoSaveMessage();

signals:
  /**
   * Message sending completed successfully.
   */
  void sentSuccessfully();
  /**
   * Message sending failed with given error message.
   */
  void failed( const QString& errorMessage, MessageComposer::ComposerViewBase::FailedType type = Sending );

  /**
   * The composer was modified. This can happen behind the users' back
   *  when, for example, and autosaved message was recovered.
   */
  void modified( bool isModified );

  /**
   * Enabling or disabling HTML in the editor is affected
   *  by various client options, so when that would otherwise happen,
   *  hand it off to the client to enact it for real.
   */
  void disableHtml( MessageComposer::ComposerViewBase::Confirmation );
  void enableHtml();

private slots:
  void slotEmailAddressResolved( KJob* );
  void slotSendComposeResult( KJob* );
  void slotQueueResult( KJob *job );
  void slotCreateItemResult( KJob * );
  void slotAutoSaveComposeResult( KJob *job );
  void slotFccCollectionCheckResult( KJob *job );
  void slotSaveMessage( KJob *job );

private:
  Akonadi::Collection defaultSpecialTarget() const;
  /**
  * Searches the mime tree, where root is the root node, for embedded images,
  * extracts them froom the body and adds them to the editor.
  */
  void collectImages( KMime::Content* root );
  bool inlineSigningEncryptionSelected();
  /**
    * Applies the user changes to the message object of the composer
    * and signs/encrypts the message if activated.
    * Disables the controls of the composer window.
    */
  void readyForSending();

  enum RecipientExpansion { UseExpandedRecipients, UseUnExpandedRecipients };
  QList< MessageComposer::Composer* > generateCryptoMessages();
  void fillGlobalPart( MessageComposer::GlobalPart *globalPart );
  void fillInfoPart( MessageComposer::InfoPart *part, RecipientExpansion expansion );
  void queueMessage( KMime::Message::Ptr message, MessageComposer::Composer* composer );
  void saveMessage( KMime::Message::Ptr message, MessageComposer::MessageSender::SaveIn saveIn );
  void fillQueueJobHeaders( MailTransport::MessageQueueJob* qjob, KMime::Message::Ptr message, const MessageComposer::InfoPart* infoPart );
  QStringList cleanEmailList( const QStringList& emails );
  void saveRecentAddresses( KMime::Message::Ptr ptr );
  void updateRecipients( const KPIMIdentities::Identity &ident, const KPIMIdentities::Identity &oldIdent, MessageComposer::Recipient::Type type );

  void markAllAttachmentsForSigning(bool sign);
  void markAllAttachmentsForEncryption(bool encrypt);
  bool determineWhetherToSign(bool doSignCompletely , Kleo::KeyResolver *keyResolver, bool signSomething, bool & result);
  bool determineWhetherToEncrypt(bool doEncryptCompletely , Kleo::KeyResolver *keyResolver, bool encryptSomething, bool signSomething, bool & result);

  /**
  * Writes out autosave data to the disk from the KMime::Message message.
  * Also appends the msgNum to the filename as a message can have a number of
  * KMime::Messages
  */
  void writeAutoSaveToDisk( const KMime::Message::Ptr& message );

  /**
    * Returns the autosave interval in milliseconds (as needed for QTimer).
    */
  int autoSaveInterval() const;

  /**
    * Initialize autosaving (timer and filename).
    */
  void initAutoSave();


  KMime::Message::Ptr m_msg;
  MessageComposer::AttachmentControllerBase* m_attachmentController;
  MessageComposer::AttachmentModel* m_attachmentModel;
  MessageComposer::SignatureController* m_signatureController;
  MessageComposer::RecipientsEditor * m_recipientsEditor;
  KPIMIdentities::IdentityCombo *m_identityCombo;
  KPIMIdentities::IdentityManager* m_identMan;
  MessageComposer::KMeditor* m_editor;
  MailTransport::TransportComboBox* m_transport;
  Akonadi::CollectionComboBox* m_fccCombo;
  Akonadi::Collection m_fccCollection;
  QWidget* m_parentWidget;

  // List of active composer jobs. For example, saving as draft, autosaving and printing
  // all create a composer, which is added to this list as long as it is active.
  // Used mainly to prevent closing the window if a composer is active
  QList< MessageComposer::Composer* > m_composers;

  bool m_sign, m_encrypt, m_neverEncrypt, m_mdnRequested, m_urgent;
  Kleo::CryptoMessageFormat m_cryptoMessageFormat;
  QString mExpandedFrom, m_from, m_replyTo, m_subject;
  QStringList mExpandedTo, mExpandedCc, mExpandedBcc;
  QList< QByteArray > m_charsets;
  QMap<QByteArray, QString> m_customHeader;

  int m_pendingQueueJobs;

  QTimer *m_autoSaveTimer;
  QString m_autoSaveUUID;
  bool m_autoSaveErrorShown; // Stops an error message being shown every time autosave is executed.
  int m_autoSaveInterval;

  MessageComposer::MessageSender::SendMethod mSendMethod;
  MessageComposer::MessageSender::SaveIn mSaveIn;
};

} // namespace

#endif
