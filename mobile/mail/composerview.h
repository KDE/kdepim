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

#ifndef COMPOSERVIEW_H
#define COMPOSERVIEW_H

#include "kdeclarativefullscreenview.h"
#include <messagecomposer/sender/messagesender.h>
#include <messagecomposer/composer/composerviewbase.h>
#include <KMime/Message>


namespace KIdentityManagement
{
  class IdentityCombo;
}

namespace Message
{
  class KMeditor;
}

namespace MessageComposer
{
  class RecipientsEditor;
}

class SnippetsEditor;

/** The new KMMainWidget ;-) */
class ComposerView : public KDeclarativeFullScreenView
{
  Q_OBJECT
  Q_PROPERTY( QString subject READ subject WRITE setSubject NOTIFY changed )
  Q_PROPERTY( bool busy READ busy WRITE setBusy NOTIFY busyChanged )
  Q_PROPERTY( bool isSigned READ isSigned NOTIFY cryptoStateChanged )
  Q_PROPERTY( bool isEncrypted READ isEncrypted NOTIFY cryptoStateChanged )
  Q_PROPERTY( bool tooManyRecipients READ tooManyRecipients NOTIFY recipientsCountChanged )
  Q_PROPERTY( int recipientsCount READ recipientsCount NOTIFY recipientsCountChanged )

  public:
    explicit ComposerView(QWidget* parent = 0);

    void setIdentityCombo( KIdentityManagement::IdentityCombo* combo );

    void setEditor( MessageComposer::KMeditor* editor );
    void setRecipientsEditor( MessageComposer::RecipientsEditor *editor );

    QString subject() const;
    void setSubject( const QString &subject );

    bool busy() const;
    void setBusy(bool busy);
    void setAutoSaveFileName(const QString &fileName);

    bool isSigned() const;
    bool isEncrypted() const;

    bool tooManyRecipients() const;
    int recipientsCount() const;

    void setIdentity( uint );

  public slots:
    void setMessage( const KMime::Message::Ptr &msg, bool mayAutoSign = true );
    QObject* getAction( const QString &name ) const;

    /// Send clicked in the user interface
    void send( MessageComposer::MessageSender::SendMethod method = MessageComposer::MessageSender::SendDefault,
               MessageComposer::MessageSender::SaveIn saveIn = MessageComposer::MessageSender::SaveInNone );

    void configureIdentity();
    void configureTransport();
    void sendSuccessful();

    void enableHtml();
    void disableHtml( MessageComposer::ComposerViewBase::Confirmation confirmation );
    void addAttachment( KMime::Content* part );

  signals:
    void changed();
    void busyChanged();
    void cryptoStateChanged();
    void recipientsCountChanged();

  private slots:
    void qmlLoaded ( QDeclarativeView::Status );
    void success();
    void failed( const QString &errorMessage );
    void transportsChanged();
    void identityChanged( uint );

    void signEmail( bool sign );
    void encryptEmail( bool encrypt );
    void urgentEmail ( bool urgent ) { m_urgent = urgent; }
    void requestMdn( bool value ) { m_mdnRequested = value; }

    void toggleUseFixedFont( bool );
    void toggleAutomaticWordWrap( bool );
    void setCryptoFormat();

    void sendLater();
    void saveDraft();
    void saveAsTemplate();

 protected:
    void doDelayedInit();
    void closeEvent ( QCloseEvent * event );

  private:
    MessageComposer::ComposerViewBase* m_composerBase;
    QString m_subject;
    KMime::Message::Ptr m_message;
    int m_jobCount;
    bool m_sign;
    bool m_encrypt;
    bool m_busy;
    bool m_draft;
    bool m_urgent;
    bool m_mdnRequested;
    QString m_fileName;
    SnippetsEditor *m_snippetsEditor;
    Kleo::CryptoMessageFormat m_cryptoFormat;
    uint m_presetIdentity;
    uint m_currentIdentity;
    bool m_mayAutoSign;
};

#endif
