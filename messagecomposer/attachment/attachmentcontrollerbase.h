/*
 * This file is part of KMail.
 * Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>
 *
 * Parts based on KMail code by:
 * Various authors.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KMAIL_ATTACHMENTCONTROLLERBASE_H
#define KMAIL_ATTACHMENTCONTROLLERBASE_H

#include <QtCore/QObject>

#include <KDE/KUrl>

#include <KPIMIdentities/Identity>
#include <messagecore/attachment/attachmentpart.h>
#include <akonadi/item.h>
#include <KJob>
#include <KService>
#include "messagecomposer_export.h"
class KActionCollection;
class QAction;
namespace MessageCore {
class AttachmentFromUrlBaseJob;
}

namespace MessageComposer {
class AttachmentModel;
class MESSAGECOMPOSER_EXPORT AttachmentControllerBase : public QObject
{
    Q_OBJECT

public:
    AttachmentControllerBase( MessageComposer::AttachmentModel *model, QWidget *wParent, KActionCollection *actionCollection );
    ~AttachmentControllerBase();

    void createActions();

    // TODO dnd stuff...

    void setSelectedParts( const MessageCore::AttachmentPart::List &selectedParts );

    void setAttachOwnVcard(bool attachVcard);
    bool attachOwnVcard() const;
    void setIdentityHasOwnVcard(bool state);

public slots:
    /// model sets these
    void setEncryptEnabled( bool enabled );
    void setSignEnabled( bool enabled );
    /// compression is async...
    void compressAttachment( MessageCore::AttachmentPart::Ptr part, bool compress );
    void showContextMenu();
    void openAttachment( MessageCore::AttachmentPart::Ptr part );
    void viewAttachment( MessageCore::AttachmentPart::Ptr part );
    void editAttachment( MessageCore::AttachmentPart::Ptr part, bool openWith = false );
    void editAttachmentWith( MessageCore::AttachmentPart::Ptr part );
    void saveAttachmentAs( MessageCore::AttachmentPart::Ptr part );
    void attachmentProperties( MessageCore::AttachmentPart::Ptr part );
    void showAddAttachmentDialog();
    /// sets sign, encrypt, shows properties dialog if so configured
    void addAttachment( MessageCore::AttachmentPart::Ptr part );
    void addAttachment( const KUrl &url );
    void addAttachmentUrlSync(const KUrl &url);
    void addAttachments( const KUrl::List &urls );
    void showAttachPublicKeyDialog();
    virtual void attachMyPublicKey() {}

signals:
    void actionsCreated();
    void refreshSelection();
    void showAttachment( KMime::Content *content, const QByteArray &charset );
    void selectedAllAttachment();
    void addOwnVcard(bool);
    void fileAttached();

protected:
    void exportPublicKey( const QString &fingerprint );
    void enableAttachPublicKey( bool enable );
    void enableAttachMyPublicKey( bool enable );
    void byteArrayToRemoteFile(const QByteArray &aData, const KUrl &aURL, bool overwrite = false);
    void openWith(KService::Ptr offer = KService::Ptr());
private:
    MessageCore::AttachmentFromUrlBaseJob * createAttachmentJob(const KUrl &url);
private slots:
    void slotPutResult(KJob *job);
    void slotOpenWithDialog();
    void slotOpenWithAction(QAction*act);

private:
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT( d, void attachmentRemoved( MessageCore::AttachmentPart::Ptr ) )
    Q_PRIVATE_SLOT( d, void compressJobResult( KJob* ) )
    Q_PRIVATE_SLOT( d, void loadJobResult( KJob* ) )
    Q_PRIVATE_SLOT( d, void openSelectedAttachments() )
    Q_PRIVATE_SLOT( d, void viewSelectedAttachments() )
    Q_PRIVATE_SLOT( d, void editSelectedAttachment() )
    Q_PRIVATE_SLOT( d, void editSelectedAttachmentWith() )
    Q_PRIVATE_SLOT( d, void removeSelectedAttachments() )
    Q_PRIVATE_SLOT( d, void saveSelectedAttachmentAs() )
    Q_PRIVATE_SLOT( d, void selectedAttachmentProperties() )
    Q_PRIVATE_SLOT( d, void editDone( MessageViewer::EditorWatcher* ) )
    Q_PRIVATE_SLOT( d, void attachPublicKeyJobResult( KJob* ) )
    Q_PRIVATE_SLOT( d, void slotAttachmentContentCreated( KJob * ) )
};

} //

#endif // KMAIL_ATTACHMENTCONTROLLERBASE_H
