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

#include <messagecomposer/messagesender.h>
#include <messagecomposer/composerviewbase.h>
#include <KActionCollection>
#include <KMime/Message>

class KJob;

namespace KPIMIdentities
{
  class IdentityCombo;
}

namespace Message
{
  class KMeditor;
  class AttachmentModel;
  class AttachmentControllerBase;
}

namespace MessageComposer
{
  class RecipientsEditor;
}

/** The new KMMainWidget ;-) */
class ComposerView : public KDeclarativeFullScreenView
{
  Q_OBJECT
  Q_PROPERTY( QString subject READ subject WRITE setSubject NOTIFY changed )
  Q_PROPERTY( bool busy READ busy WRITE setBusy NOTIFY busyChanged)

  public:
    explicit ComposerView(QWidget* parent = 0);

    void setIdentityCombo( KPIMIdentities::IdentityCombo* combo ) { m_composerBase->setIdentityCombo( combo ); }
    void setEditor( Message::KMeditor* editor ) { m_composerBase->setEditor( editor ); }
    void setRecipientsEditor( MessageComposer::RecipientsEditor *editor ) { m_composerBase->setRecipientsEditor( editor ); }

    QString subject() const;
    void setSubject( const QString &subject );

    void setMessage( const KMime::Message::Ptr &msg );

    bool busy() const;
    void setBusy(bool busy);

  public slots:
    /// Send clicked in the user interface
    void send( MessageSender::SaveIn saveIn = MessageSender::SaveInNone );
    QObject* getAction( const QString &name ) const;
    void configureIdentity();
    void configureTransport();
    void slotSendSuccessful();

  signals:
    void changed();
    void busyChanged();

  private slots:
    void qmlLoaded ( QDeclarativeView::Status );
    void addAttachment();

    void signEmail( bool sign ) { m_sign = sign; }
    void encryptEmail( bool encrypt ) { m_encrypt = encrypt; }

  private:
    Message::ComposerViewBase* m_composerBase;
    QString m_subject;
    KMime::Message::Ptr m_message;
    int m_jobCount;
    bool m_sign;
    bool m_encrypt;
    bool m_busy;
};

#endif
