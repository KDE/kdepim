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

namespace MessageComposer {
  class RecipientsEditor;
}

/** The new KMMainWidget ;-) */
class ComposerView : public KDeclarativeFullScreenView
{
  Q_OBJECT
  Q_PROPERTY( QString subject READ subject WRITE setSubject NOTIFY changed )

  public:
    explicit ComposerView(QWidget* parent = 0);

    void setIdentityCombo( KPIMIdentities::IdentityCombo* combo ) { m_identityCombo = combo; }
    void setEditor( Message::KMeditor* editor ) { m_editor = editor; }
    void setRecipientsEditor( MessageComposer::RecipientsEditor *editor ) { m_recipientsEditor = editor; }

    QString subject() const;
    void setSubject( const QString &subject );

    KActionCollection* actionCollection() const;

    void setMessage( const KMime::Message::Ptr &msg );

  public slots:
    /// Send clicked in the user interface
    void send();
    QObject* getAction( const QString &name ) const;
    void configureIdentity();
    void configureTransport();

  signals:
    void changed();

  private:
    void setMessageInternal( const KMime::Message::Ptr &msg );
    void expandAddresses();

  private slots:
    void qmlLoaded ( QDeclarativeView::Status );
    void addressExpansionResult( KJob *job );
    void composerResult( KJob* job );
    void sendResult( KJob* job );
    void addAttachment();

  private:
    KPIMIdentities::IdentityCombo *m_identityCombo;
    Message::KMeditor *m_editor;
    MessageComposer::RecipientsEditor *m_recipientsEditor;
    Message::AttachmentModel *m_attachmentModel;
    Message::AttachmentControllerBase *m_attachmentController;
    QString m_subject;
    KActionCollection *mActionCollection;
    KMime::Message::Ptr m_message;
    int m_jobCount;
};

#endif
