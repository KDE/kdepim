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
#include "messagesender.h"
#include "recipient.h"

#include <KDE/KMime/Message>

#include <QObject>
#include <KUrl>
#include <kleo/enum.h>

class QAction;
class KJob;
class QWidget;

namespace Akonadi
{
  class CollectionComboBox;
  class Collection;
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

namespace MessageComposer
{
  class RecipientsEditor;
}

namespace Message {

class AttachmentControllerBase;


class SignatureController;


class AttachmentControllerBase;


class AttachmentModel;


class SignatureController;


class InfoPart;
class GlobalPart;
class Composer;
class KMeditor;
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
  explicit ComposerViewBase ( QObject* parent = 0 );
  virtual ~ComposerViewBase();

  enum Confirmation { LetUserConfirm, NoConfirmationNeeded };
  
  /**
   * Set the message to be opened in the composer window, and set the internal data structures to
   *  keep track of it.
   */
  void setMessage( const KMime::Message::Ptr& newMsg );

  /**
   * Send the message with the specified method, saving it in the specified folder.
   */
  void send( MessageSender::SendMethod method, MessageSender::SaveIn saveIn );

  /**
   * Returns true if there is at least one composer job running.
   */
  bool isComposing() const;
  
  /**
   * Add the given attachment to the message.
   */
  void addAttachment( const KUrl &url, const QString &comment );
  void addAttachment ( const QString& name, const QString& filename, const QString& charset, const QByteArray& data, const QByteArray& mimeType );
  void addAttachmentPart( KMime::Content* part );

  Composer* createSimpleComposer();
  
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
  void setAttachmentModel( AttachmentModel* model );
  AttachmentModel* attachmentModel();

  void setAttachmentController( AttachmentControllerBase* controller );
  AttachmentControllerBase* attachmentController();
  
  void setRecipientsEditor( MessageComposer::RecipientsEditor* recEditor );
  MessageComposer::RecipientsEditor* recipientsEditor();

  void setSignatureController( SignatureController* sigController );
  SignatureController* signatureController();
  
  void setIdentityCombo( KPIMIdentities::IdentityCombo* identCombo );
  KPIMIdentities::IdentityCombo* identityCombo();
  void identityChanged( const KPIMIdentities::Identity &ident, const KPIMIdentities::Identity &oldIdent );

  void setIdentityManager( KPIMIdentities::IdentityManager* identMan );
  KPIMIdentities::IdentityManager* identityManager();
  
  void setEditor( Message::KMeditor* editor );
  Message::KMeditor* editor();

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
  
  void setParentWidgetForGui( QWidget* );

signals:
  /**
   * Message sending completed successfully.
   */
  void sentSuccessfully();
  /**
   * Message sending failed with given error message.
   */
  void failed( const QString& errorMessage );

  /**
   * Enabling or disabling HTML in the editor is affected
   *  by various client options, so when that would otherwise happen,
   *  hand it off to the client to enact it for real.
   */
  void disableHtml( Message::ComposerViewBase::Confirmation );
  void enableHtml();
  
private slots:
  void slotEmailAddressResolved( KJob* );
  void slotSendComposeResult( KJob* );
  void slotQueueResult( KJob *job );
private:
  bool isHTMLMail( KMime::Content* root );
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
  QList< Message::Composer* > generateCryptoMessages();
  void fillGlobalPart( Message::GlobalPart *globalPart );
  void fillInfoPart( Message::InfoPart *part, RecipientExpansion expansion );
  void queueMessage( KMime::Message::Ptr message, Composer* composer );
  void saveMessage( KMime::Message::Ptr message, MessageSender::SaveIn saveIn );
  void fillQueueJobHeaders( MailTransport::MessageQueueJob* qjob, KMime::Message::Ptr message, const Message::InfoPart* infoPart );
  void slotCreateItemResult( KJob *job );

  KMime::Message::Ptr m_msg;
  AttachmentControllerBase* m_attachmentController;
  AttachmentModel* m_attachmentModel;
  SignatureController* m_signatureController;
  MessageComposer::RecipientsEditor* m_recipientsEditor;
  KPIMIdentities::IdentityCombo *m_identityCombo;
  KPIMIdentities::IdentityManager* m_identMan;
  Message::KMeditor* m_editor;
  MailTransport::TransportComboBox* m_transport;
  Akonadi::CollectionComboBox* m_fcc;
  QWidget* m_parentWidget;
  
  // List of active composer jobs. For example, saving as draft, autosaving and printing
  // all create a composer, which is added to this list as long as it is active.
  // Used mainly to prevent closing the window if a composer is active
  QList< Message::Composer* > m_composers;

  bool m_sign, m_encrypt, m_neverEncrypt, m_mdnRequested, m_urgent;
  Kleo::CryptoMessageFormat m_cryptoMessageFormat;
  QString mExpandedFrom, m_from, m_replyTo, m_subject;
  QStringList mExpandedTo, mExpandedCc, mExpandedBcc;
  QList< QByteArray > m_charsets;
  int m_pendingQueueJobs;
  
  MessageSender::SendMethod mSendMethod;
  MessageSender::SaveIn mSaveIn;
};

} // namespace 

#endif
