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

#include "attachmentcontrollerbase.h"

#include "messagecomposer/attachment/attachmentmodel.h"
#include "messagecomposer/job/attachmentjob.h"
#include "messagecomposer/job/attachmentfrompublickeyjob.h"
#include "messagecomposer/composer/composer.h"

#include "messagecomposer/part/globalpart.h"
#include "messageviewer/viewer/editorwatcher.h"
#include "messageviewer/viewer/nodehelper.h"
#include "messageviewer/utils/util.h"

#include <AkonadiCore/itemfetchjob.h>
#include <kio/jobuidelegate.h>
#include <QIcon>

#include <QMenu>
#include <QPointer>

#include <QAction>
#include <KActionCollection>
#include <QDebug>
#include <KEncodingFileDialog>
#include <KFileDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <KMimeTypeTrader>
#include <QPushButton>
#include <KRun>
#include <QTemporaryFile>
#include <KFileItemActions>
#include <KActionMenu>

#include <libkleo/kleo/cryptobackendfactory.h>
#include <libkleo/ui/keyselectiondialog.h>

#include <messagecore/attachment/attachmentcompressjob.h>
#include <messagecore/attachment/attachmentfromfolderjob.h>
#include <messagecore/attachment/attachmentfrommimecontentjob.h>
#include <messagecore/attachment/attachmentfromurljob.h>
#include <messagecore/attachment/attachmentpropertiesdialog.h>
#include <settings/messagecomposersettings.h>
#include <KIO/Job>

#include <KMime/Content>
#include <QFileDialog>

using namespace MessageComposer;
using namespace MessageCore;

class MessageComposer::AttachmentControllerBase::Private
{
public:
    Private(AttachmentControllerBase *qq);
    ~Private();

    void attachmentRemoved(AttachmentPart::Ptr part);   // slot
    void compressJobResult(KJob *job);   // slot
    void loadJobResult(KJob *job);   // slot
    void openSelectedAttachments(); // slot
    void viewSelectedAttachments(); // slot
    void editSelectedAttachment(); // slot
    void editSelectedAttachmentWith(); // slot
    void removeSelectedAttachments(); // slot
    void saveSelectedAttachmentAs(); // slot
    void selectedAttachmentProperties(); // slot
    void editDone(MessageViewer::EditorWatcher *watcher);   // slot
    void attachPublicKeyJobResult(KJob *job);   // slot
    void slotAttachmentContentCreated(KJob *job);   // slot
    void addAttachmentPart(AttachmentPart::Ptr part);
    void selectedAllAttachment();
    void createOpenWithMenu(QMenu *topMenu, AttachmentPart::Ptr part);

    AttachmentControllerBase *const q;
    bool encryptEnabled;
    bool signEnabled;
    MessageComposer::AttachmentModel *model;
    QWidget *wParent;
    QHash<MessageViewer::EditorWatcher *, AttachmentPart::Ptr> editorPart;
    QHash<MessageViewer::EditorWatcher *, QTemporaryFile *> editorTempFile;

    AttachmentPart::List selectedParts;
    KActionCollection *mActionCollection;
    QAction *attachPublicKeyAction;
    QAction *attachMyPublicKeyAction;
    QAction *openContextAction;
    QAction *viewContextAction;
    QAction *editContextAction;
    QAction *editWithContextAction;
    QAction *removeAction;
    QAction *removeContextAction;
    QAction *saveAsAction;
    QAction *saveAsContextAction;
    QAction *propertiesAction;
    QAction *propertiesContextAction;
    QAction *addAction;
    QAction *addContextAction;
    QAction *selectAllAction;
    KActionMenu *attachmentMenu;
    QAction *addOwnVcardAction;

    // If part p is compressed, uncompressedParts[p] is the uncompressed part.
    QHash<AttachmentPart::Ptr, AttachmentPart::Ptr> uncompressedParts;
};

AttachmentControllerBase::Private::Private(AttachmentControllerBase *qq)
    : q(qq)
    , encryptEnabled(false)
    , signEnabled(false)
    , model(0)
    , wParent(0)
    , attachPublicKeyAction(0)
    , attachMyPublicKeyAction(0)
    , openContextAction(0)
    , viewContextAction(0)
    , editContextAction(0)
    , editWithContextAction(0)
    , removeAction(0)
    , removeContextAction(0)
    , saveAsAction(0)
    , saveAsContextAction(0)
    , propertiesAction(0)
    , propertiesContextAction(0)
    , addAction(0)
    , addContextAction(0)
    , selectAllAction(0)
    , attachmentMenu(0)
    , addOwnVcardAction(0)
{
}

AttachmentControllerBase::Private::~Private()
{
}

void AttachmentControllerBase::setSelectedParts(const AttachmentPart::List &selectedParts)
{
    d->selectedParts = selectedParts;
    const int selectedCount = selectedParts.count();
    const bool enableEditAction = (selectedCount == 1) &&
                                  (!selectedParts.first()->isMessageOrMessageCollection());

    d->openContextAction->setEnabled(selectedCount > 0);
    d->viewContextAction->setEnabled(selectedCount > 0);
    d->editContextAction->setEnabled(enableEditAction);
    d->editWithContextAction->setEnabled(enableEditAction);
    d->removeAction->setEnabled(selectedCount > 0);
    d->removeContextAction->setEnabled(selectedCount > 0);
    d->saveAsAction->setEnabled(selectedCount == 1);
    d->saveAsContextAction->setEnabled(selectedCount == 1);
    d->propertiesAction->setEnabled(selectedCount == 1);
    d->propertiesContextAction->setEnabled(selectedCount == 1);
}

void AttachmentControllerBase::Private::attachmentRemoved(AttachmentPart::Ptr part)
{
    if (uncompressedParts.contains(part)) {
        uncompressedParts.remove(part);
    }
}

void AttachmentControllerBase::Private::compressJobResult(KJob *job)
{
    if (job->error()) {
        KMessageBox::sorry(wParent, job->errorString(), i18n("Failed to compress attachment"));
        return;
    }

    Q_ASSERT(dynamic_cast<AttachmentCompressJob *>(job));
    AttachmentCompressJob *ajob = static_cast<AttachmentCompressJob *>(job);
    //AttachmentPart *originalPart = const_cast<AttachmentPart*>( ajob->originalPart() );
    AttachmentPart::Ptr originalPart = ajob->originalPart();
    AttachmentPart::Ptr compressedPart = ajob->compressedPart();

    if (ajob->isCompressedPartLarger()) {
        const int result = KMessageBox::questionYesNo(wParent,
                           i18n("The compressed attachment is larger than the original. "
                                "Do you want to keep the original one?"),
                           QString(/*caption*/),
                           KGuiItem(i18nc("Do not compress", "Keep")),
                           KGuiItem(i18n("Compress")));
        if (result == KMessageBox::Yes) {
            // The user has chosen to keep the uncompressed file.
            return;
        }
    }

    qDebug() << "Replacing uncompressed part in model.";
    uncompressedParts[ compressedPart ] = originalPart;
    bool ok = model->replaceAttachment(originalPart, compressedPart);
    if (!ok) {
        // The attachment was removed from the model while we were compressing.
        qDebug() << "Compressed a zombie.";
    }
}

void AttachmentControllerBase::Private::loadJobResult(KJob *job)
{
    if (job->error()) {
        KMessageBox::sorry(wParent, job->errorString(), i18n("Failed to attach file"));
        return;
    }

    Q_ASSERT(dynamic_cast<AttachmentLoadJob *>(job));
    AttachmentLoadJob *ajob = static_cast<AttachmentLoadJob *>(job);
    AttachmentPart::Ptr part = ajob->attachmentPart();
    q->addAttachment(part);
}

void AttachmentControllerBase::Private::openSelectedAttachments()
{
    Q_ASSERT(selectedParts.count() >= 1);
    foreach (AttachmentPart::Ptr part, selectedParts) {
        q->openAttachment(part);
    }
}

void AttachmentControllerBase::Private::viewSelectedAttachments()
{
    Q_ASSERT(selectedParts.count() >= 1);
    foreach (AttachmentPart::Ptr part, selectedParts) {
        q->viewAttachment(part);
    }
}

void AttachmentControllerBase::Private::editSelectedAttachment()
{
    Q_ASSERT( selectedParts.count() == 1 );
    q->editAttachment( selectedParts.first(), MessageViewer::EditorWatcher::NoOpenWithDialog );
}

void AttachmentControllerBase::Private::editSelectedAttachmentWith()
{
    Q_ASSERT( selectedParts.count() == 1 );
    q->editAttachment( selectedParts.first(), MessageViewer::EditorWatcher::OpenWithDialog );
}

void AttachmentControllerBase::Private::removeSelectedAttachments()
{
    Q_ASSERT(selectedParts.count() >= 1);
    foreach (AttachmentPart::Ptr part, selectedParts) {
        model->removeAttachment(part);
    }
}

void AttachmentControllerBase::Private::saveSelectedAttachmentAs()
{
    Q_ASSERT(selectedParts.count() == 1);
    q->saveAttachmentAs(selectedParts.first());
}

void AttachmentControllerBase::Private::selectedAttachmentProperties()
{
    Q_ASSERT(selectedParts.count() == 1);
    q->attachmentProperties(selectedParts.first());
}

void AttachmentControllerBase::Private::editDone(MessageViewer::EditorWatcher *watcher)
{
    AttachmentPart::Ptr part = editorPart.take(watcher);
    Q_ASSERT(part);
    QTemporaryFile *tempFile = editorTempFile.take(watcher);
    Q_ASSERT(tempFile);

    if (watcher->fileChanged()) {
        qDebug() << "File has changed.";

        // Read the new data and update the part in the model.
        tempFile->reset();
        QByteArray data = tempFile->readAll();
        part->setData(data);
        model->updateAttachment(part);
    }

    delete tempFile;
    // The watcher deletes itself.
}

void AttachmentControllerBase::Private::createOpenWithMenu(QMenu *topMenu, AttachmentPart::Ptr part)
{
    const QString contentTypeStr = QString::fromLatin1(part->mimeType());
    const KService::List offers = KFileItemActions::associatedApplications(QStringList() << contentTypeStr, QString());
    if (!offers.isEmpty()) {
        QMenu *menu = topMenu;
        QActionGroup *actionGroup = new QActionGroup(menu);
        connect(actionGroup, &QActionGroup::triggered, q, &AttachmentControllerBase::slotOpenWithAction);

        if (offers.count() > 1) { // submenu 'open with'
            menu = new QMenu(i18nc("@title:menu", "&Open With"), topMenu);
            menu->menuAction()->setObjectName(QLatin1String("openWith_submenu")); // for the unittest
            topMenu->addMenu(menu);
        }
        //qDebug() << offers.count() << "offers" << topMenu << menu;

        KService::List::ConstIterator it = offers.constBegin();
        KService::List::ConstIterator end = offers.constEnd();
        for (; it != end; ++it) {
            QAction *act = MessageViewer::Util::createAppAction(*it,
                           // no submenu -> prefix single offer
                           menu == topMenu, actionGroup, menu);
            menu->addAction(act);
        }

        QString openWithActionName;
        if (menu != topMenu) { // submenu
            menu->addSeparator();
            openWithActionName = i18nc("@action:inmenu Open With", "&Other...");
        } else {
            openWithActionName = i18nc("@title:menu", "&Open With...");
        }
        QAction *openWithAct = new QAction(menu);
        openWithAct->setText(openWithActionName);
        QObject::connect(openWithAct, &QAction::triggered, q, &AttachmentControllerBase::slotOpenWithDialog);
        menu->addAction(openWithAct);
    } else { // no app offers -> Open With...
        QAction *act = new QAction(topMenu);
        act->setText(i18nc("@title:menu", "&Open With..."));
        QObject::connect(act, &QAction::triggered, q, &AttachmentControllerBase::slotOpenWithDialog);
        topMenu->addAction(act);
    }
}

void AttachmentControllerBase::exportPublicKey(const QString &fingerprint)
{
    if (fingerprint.isEmpty() || !Kleo::CryptoBackendFactory::instance()->openpgp()) {
        qWarning() << "Tried to export key with empty fingerprint, or no OpenPGP.";
        Q_ASSERT(false);   // Can this happen?
        return;
    }

    MessageComposer::AttachmentFromPublicKeyJob *ajob = new MessageComposer::AttachmentFromPublicKeyJob(fingerprint, this);
    connect(ajob, SIGNAL(result(KJob*)), this, SLOT(attachPublicKeyJobResult(KJob*)));
    ajob->start();
}

void AttachmentControllerBase::Private::attachPublicKeyJobResult(KJob *job)
{
    // The only reason we can't use loadJobResult() and need a separate method
    // is that we want to show the proper caption ("public key" instead of "file")...

    if (job->error()) {
        KMessageBox::sorry(wParent, job->errorString(), i18n("Failed to attach public key"));
        return;
    }

    Q_ASSERT(dynamic_cast<MessageComposer::AttachmentFromPublicKeyJob *>(job));
    MessageComposer::AttachmentFromPublicKeyJob *ajob = static_cast<MessageComposer::AttachmentFromPublicKeyJob *>(job);
    AttachmentPart::Ptr part = ajob->attachmentPart();
    q->addAttachment(part);
}

static QTemporaryFile *dumpAttachmentToTempFile(const AttachmentPart::Ptr part)   // local
{
    QTemporaryFile *file = new QTemporaryFile;
    if (!file->open()) {
        qCritical() << "Could not open tempfile" << file->fileName();
        delete file;
        return 0;
    }
    if (file->write(part->data()) == -1) {
        qCritical() << "Could not dump attachment to tempfile.";
        delete file;
        return 0;
    }
    file->flush();
    return file;
}

AttachmentControllerBase::AttachmentControllerBase(MessageComposer::AttachmentModel *model, QWidget *wParent, KActionCollection *actionCollection)
    : QObject(wParent)
    , d(new Private(this))
{
    d->model = model;
    connect(model, &MessageComposer::AttachmentModel::attachUrlsRequested, this, &AttachmentControllerBase::addAttachments);
    connect(model, SIGNAL(attachmentRemoved(MessageCore::AttachmentPart::Ptr)),
            this, SLOT(attachmentRemoved(MessageCore::AttachmentPart::Ptr)));
    connect(model, SIGNAL(attachmentCompressRequested(MessageCore::AttachmentPart::Ptr,bool)),
            this, SLOT(compressAttachment(MessageCore::AttachmentPart::Ptr,bool)));
    connect(model, &MessageComposer::AttachmentModel::encryptEnabled, this, &AttachmentControllerBase::setEncryptEnabled);
    connect(model, &MessageComposer::AttachmentModel::signEnabled, this, &AttachmentControllerBase::setSignEnabled);

    d->wParent = wParent;
    d->mActionCollection = actionCollection;
}

AttachmentControllerBase::~AttachmentControllerBase()
{
    delete d;
}

void AttachmentControllerBase::createActions()
{
    // Create the actions.
    d->attachPublicKeyAction = new QAction(i18n("Attach &Public Key..."), this);
    connect(d->attachPublicKeyAction, SIGNAL(triggered(bool)),
            this, SLOT(showAttachPublicKeyDialog()));

    d->attachMyPublicKeyAction = new QAction(i18n("Attach &My Public Key"), this);
    connect(d->attachMyPublicKeyAction, &QAction::triggered, this, &AttachmentControllerBase::attachMyPublicKey);

    d->attachmentMenu = new KActionMenu(QIcon::fromTheme(QLatin1String("mail-attachment")), i18n("Attach"), this);
    connect(d->attachmentMenu, &KActionMenu::triggered, this, &AttachmentControllerBase::showAddAttachmentDialog);

    d->attachmentMenu->setDelayed(true);

    d->addAction = new QAction(QIcon::fromTheme(QLatin1String("mail-attachment")), i18n("&Attach File..."), this);
    d->addAction->setIconText(i18n("Attach"));
    d->addContextAction = new QAction(QIcon::fromTheme(QLatin1String("mail-attachment")),
                                      i18n("Add Attachment..."), this);
    connect(d->addAction, &QAction::triggered, this, &AttachmentControllerBase::showAddAttachmentDialog);
    connect(d->addContextAction, &QAction::triggered, this, &AttachmentControllerBase::showAddAttachmentDialog);

    d->addOwnVcardAction = new QAction(i18n("Attach Own vCard"), this);
    d->addOwnVcardAction->setIconText(i18n("Own vCard"));
    d->addOwnVcardAction->setCheckable(true);
    connect(d->addOwnVcardAction, &QAction::triggered, this, &AttachmentControllerBase::addOwnVcard);

    d->attachmentMenu->addAction(d->addAction);
    d->attachmentMenu->addSeparator();
    d->attachmentMenu->addAction(d->addOwnVcardAction);

    d->removeAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("&Remove Attachment"), this);
    d->removeContextAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Remove"), this);     // FIXME need two texts. is there a better way?
    connect(d->removeAction, SIGNAL(triggered(bool)), this, SLOT(removeSelectedAttachments()));
    connect(d->removeContextAction, SIGNAL(triggered(bool)), this, SLOT(removeSelectedAttachments()));

    d->openContextAction = new QAction(i18nc("to open", "Open"), this);
    connect(d->openContextAction, SIGNAL(triggered(bool)), this, SLOT(openSelectedAttachments()));

    d->viewContextAction = new QAction(i18nc("to view", "View"), this);
    connect(d->viewContextAction, SIGNAL(triggered(bool)), this, SLOT(viewSelectedAttachments()));

    d->editContextAction = new QAction(i18nc("to edit", "Edit"), this);
    connect(d->editContextAction, SIGNAL(triggered(bool)), this, SLOT(editSelectedAttachment()));

    d->editWithContextAction = new QAction(i18n("Edit With..."), this);
    connect(d->editWithContextAction, SIGNAL(triggered(bool)), this, SLOT(editSelectedAttachmentWith()));

    d->saveAsAction = new QAction(QIcon::fromTheme(QLatin1String("document-save-as")),
                                  i18n("&Save Attachment As..."), this);
    d->saveAsContextAction = new QAction(QIcon::fromTheme(QLatin1String("document-save-as")),
                                         i18n("Save As..."), this);
    connect(d->saveAsAction, SIGNAL(triggered(bool)),
            this, SLOT(saveSelectedAttachmentAs()));
    connect(d->saveAsContextAction, SIGNAL(triggered(bool)),
            this, SLOT(saveSelectedAttachmentAs()));

    d->propertiesAction = new QAction(i18n("Attachment Pr&operties..."), this);
    d->propertiesContextAction = new QAction(i18n("Properties"), this);
    connect(d->propertiesAction, SIGNAL(triggered(bool)),
            this, SLOT(selectedAttachmentProperties()));
    connect(d->propertiesContextAction, SIGNAL(triggered(bool)),
            this, SLOT(selectedAttachmentProperties()));

    d->selectAllAction = new QAction(i18n("Select All"), this);
    connect(d->selectAllAction, SIGNAL(triggered(bool)),
            this, SIGNAL(selectedAllAttachment()));

    // Insert the actions into the composer window's menu.
    KActionCollection *collection = d->mActionCollection;
    collection->addAction(QLatin1String("attach_public_key"), d->attachPublicKeyAction);
    collection->addAction(QLatin1String("attach_my_public_key"), d->attachMyPublicKeyAction);
    collection->addAction(QLatin1String("attach"), d->addAction);
    collection->addAction(QLatin1String("remove"), d->removeAction);
    collection->addAction(QLatin1String("attach_save"), d->saveAsAction);
    collection->addAction(QLatin1String("attach_properties"), d->propertiesAction);
    collection->addAction(QLatin1String("select_all_attachment"), d->selectAllAction);
    collection->addAction(QLatin1String("attach_menu"), d->attachmentMenu);
    collection->addAction(QLatin1String("attach_own_vcard"), d->addOwnVcardAction);

    setSelectedParts(AttachmentPart::List());
    emit actionsCreated();
}

void AttachmentControllerBase::setEncryptEnabled(bool enabled)
{
    d->encryptEnabled = enabled;
}

void AttachmentControllerBase::setSignEnabled(bool enabled)
{
    d->signEnabled = enabled;
}

void AttachmentControllerBase::compressAttachment(AttachmentPart::Ptr part, bool compress)
{
    if (compress) {
        qDebug() << "Compressing part.";

        AttachmentCompressJob *ajob = new AttachmentCompressJob(part, this);
        connect(ajob, SIGNAL(result(KJob*)), this, SLOT(compressJobResult(KJob*)));
        ajob->start();
    } else {
        qDebug() << "Uncompressing part.";

        // Replace the compressed part with the original uncompressed part, and delete
        // the compressed part.
        AttachmentPart::Ptr originalPart = d->uncompressedParts.take(part);
        Q_ASSERT(originalPart);   // Found in uncompressedParts.
        bool ok = d->model->replaceAttachment(part, originalPart);
        Q_ASSERT(ok);
        Q_UNUSED(ok);
    }
}

void AttachmentControllerBase::showContextMenu()
{
    emit refreshSelection();

    const int numberOfParts(d->selectedParts.count());
    QMenu *menu = new QMenu;

    const bool enableEditAction = (numberOfParts == 1) &&
                                  (!d->selectedParts.first()->isMessageOrMessageCollection());

    if (numberOfParts > 0) {
        if (numberOfParts == 1) {
            d->createOpenWithMenu(menu, d->selectedParts.first());
        } else {
            menu->addAction(d->openContextAction);
        }
        menu->addAction(d->viewContextAction);
    }
    if (enableEditAction) {
        menu->addAction(d->editWithContextAction);
        menu->addAction(d->editContextAction);
    }
    if (numberOfParts > 0) {
        menu->addAction(d->removeContextAction);
    }
    if (numberOfParts == 1) {
        menu->addAction(d->saveAsContextAction);
        menu->addAction(d->propertiesContextAction);
    }

    const int nbAttachment = d->model->rowCount();
    if (nbAttachment != numberOfParts) {
        menu->addSeparator();
        menu->addAction(d->selectAllAction);
    }
    if (numberOfParts == 0) {
        menu->addSeparator();
        menu->addAction(d->addContextAction);
    }
    menu->exec( QCursor::pos() );
    delete menu;
}

void AttachmentControllerBase::slotOpenWithDialog()
{
    openWith();
}

void AttachmentControllerBase::slotOpenWithAction(QAction *act)
{
    KService::Ptr app = act->data().value<KService::Ptr>();
    Q_ASSERT(d->selectedParts.count() == 1);

    openWith(app);
}

void AttachmentControllerBase::openWith(KService::Ptr offer)
{
    QTemporaryFile *tempFile = dumpAttachmentToTempFile(d->selectedParts.first());
    if (!tempFile) {
        KMessageBox::sorry(d->wParent,
                           i18n("KMail was unable to write the attachment to a temporary file."),
                           i18n("Unable to open attachment"));
        return;
    }
    QList<QUrl> lst;
    QUrl url = QUrl::fromLocalFile(tempFile->fileName());
    lst.append(url);
    bool result = false;
    if (offer) {
        result = KRun::run(*offer, lst, d->wParent, false);
    } else {
        result = KRun::displayOpenWithDialog(lst, d->wParent, false);
    }
    if (!result) {
        delete tempFile;
        tempFile = 0;
    } else {
        // The file was opened.  Delete it only when the composer is closed
        // (and this object is destroyed).
        tempFile->setParent(this);   // Manages lifetime.
    }
}

void AttachmentControllerBase::openAttachment(AttachmentPart::Ptr part)
{
    QTemporaryFile *tempFile = dumpAttachmentToTempFile(part);
    if (!tempFile) {
        KMessageBox::sorry(d->wParent,
                           i18n("KMail was unable to write the attachment to a temporary file."),
                           i18n("Unable to open attachment"));
        return;
    }

    bool success = KRun::runUrl(QUrl::fromLocalFile(tempFile->fileName()),
                                QString::fromLatin1(part->mimeType()),
                                d->wParent,
                                true /*tempFile*/,
                                false /*runExecutables*/);
    if (!success) {
        if (!KMimeTypeTrader::self()->preferredService(QString::fromLatin1(part->mimeType())).data()) {
            // KRun showed an Open-With dialog, and it was canceled.
        } else {
            // KRun failed.
            KMessageBox::sorry(d->wParent,
                               i18n("KMail was unable to open the attachment."),
                               i18n("Unable to open attachment"));
        }
        delete tempFile;
        tempFile = 0;
    } else {
        // The file was opened.  Delete it only when the composer is closed
        // (and this object is destroyed).
        tempFile->setParent(this);   // Manages lifetime.
    }
}

void AttachmentControllerBase::viewAttachment(AttachmentPart::Ptr part)
{
    MessageComposer::Composer *composer = new MessageComposer::Composer;
    composer->globalPart()->setFallbackCharsetEnabled(true);
    MessageComposer::AttachmentJob *attachmentJob = new MessageComposer::AttachmentJob(part, composer);
    connect(attachmentJob, SIGNAL(result(KJob*)),
            this, SLOT(slotAttachmentContentCreated(KJob*)));
    attachmentJob->start();
}

void AttachmentControllerBase::Private::slotAttachmentContentCreated(KJob *job)
{
    if (!job->error()) {
        const MessageComposer::AttachmentJob *const attachmentJob = dynamic_cast<MessageComposer::AttachmentJob *>(job);
        Q_ASSERT(attachmentJob);
        emit q->showAttachment(attachmentJob->content(), QByteArray());
    } else {
        // TODO: show warning to the user
        qWarning() << "Error creating KMime::Content for attachment:" << job->errorText();
    }
}

void AttachmentControllerBase::editAttachment( AttachmentPart::Ptr part, MessageViewer::EditorWatcher::OpenWithOption openWithOption )
{
    QTemporaryFile *tempFile = dumpAttachmentToTempFile(part);
    if (!tempFile) {
        KMessageBox::sorry(d->wParent,
                           i18n("KMail was unable to write the attachment to a temporary file."),
                           i18n("Unable to edit attachment"));
        return;
    }

    MessageViewer::EditorWatcher *watcher = new MessageViewer::EditorWatcher(
        QUrl::fromLocalFile(tempFile->fileName()),
        QString::fromLatin1(part->mimeType()), openWithOption,
        this, d->wParent);
    connect(watcher, SIGNAL(editDone(MessageViewer::EditorWatcher*)),
            this, SLOT(editDone(MessageViewer::EditorWatcher*)));

    if (watcher->start()) {
        // The attachment is being edited.
        // We will clean things up in editDone().
        d->editorPart[ watcher ] = part;
        d->editorTempFile[ watcher ] = tempFile;

        // Delete the temp file if the composer is closed (and this object is destroyed).
        tempFile->setParent(this);   // Manages lifetime.
    } else {
        qWarning() << "Could not start EditorWatcher.";
        delete watcher;
        delete tempFile;
    }
}

void AttachmentControllerBase::editAttachmentWith(AttachmentPart::Ptr part)
{
    editAttachment( part, MessageViewer::EditorWatcher::OpenWithDialog );
}

void AttachmentControllerBase::saveAttachmentAs(AttachmentPart::Ptr part)
{
    QString pname = part->name();
    if (pname.isEmpty()) {
        pname = i18n("unnamed");
    }

    KUrl url = KFileDialog::getSaveUrl(pname,
                                       QString(/*filter*/), d->wParent,
                                       i18n("Save Attachment As"));

    if (url.isEmpty()) {
        qDebug() << "Save Attachment As dialog canceled.";
        return;
    }

    byteArrayToRemoteFile(part->data(), url);
}

void AttachmentControllerBase::byteArrayToRemoteFile(const QByteArray &aData, const KUrl &aURL, bool overwrite)
{
    KIO::StoredTransferJob *job = KIO::storedPut(aData, aURL, -1, overwrite ? KIO::Overwrite : KIO::DefaultFlags);
    connect(job, &KIO::StoredTransferJob::result, this, &AttachmentControllerBase::slotPutResult);
}

void AttachmentControllerBase::slotPutResult(KJob *job)
{
    KIO::StoredTransferJob *_job = qobject_cast<KIO::StoredTransferJob *>(job);

    if (job->error()) {
        if (job->error() == KIO::ERR_FILE_ALREADY_EXIST) {
            if (KMessageBox::warningContinueCancel(0,
                                                   i18n("File %1 exists.\nDo you want to replace it?", _job->url().toLocalFile()), i18n("Save to File"), KGuiItem(i18n("&Replace")))
                    == KMessageBox::Continue) {
                byteArrayToRemoteFile(_job->data(), _job->url(), true);
            }
        } else {
            KJobUiDelegate *ui = static_cast<KIO::Job *>(job)->ui();
            ui->showErrorMessage();
        }
    }
}

void AttachmentControllerBase::attachmentProperties(AttachmentPart::Ptr part)
{
    QPointer<AttachmentPropertiesDialog> dialog = new AttachmentPropertiesDialog(
        part, false, d->wParent);

    dialog->setEncryptEnabled(d->encryptEnabled);
    dialog->setSignEnabled(d->signEnabled);

    if (dialog->exec() && dialog) {
        d->model->updateAttachment(part);
    }
    delete dialog;
}

void AttachmentControllerBase::showAddAttachmentDialog()
{
#ifndef KDEPIM_MOBILE_UI
    KEncodingFileDialog::Result result = KEncodingFileDialog::getOpenUrlsAndEncoding(QString(),
                                         QUrl(),
                                         QString(),
                                         d->wParent,
                                         i18n("Attach File"));
    if (!result.URLs.isEmpty()) {
        const QString encoding = MessageViewer::NodeHelper::fixEncoding(result.encoding);
        const int numberOfFiles(result.URLs.count());
        for (int i = 0; i < numberOfFiles; ++i) {
            const QUrl url = result.URLs.at(i);
            QUrl urlWithEncoding = url;
            //QT4 urlWithEncoding.setFileEncoding( encoding );
            if (KMimeType::findByUrl(urlWithEncoding)->name() == QLatin1String("inode/directory")) {
                const int rc = KMessageBox::warningYesNo(d->wParent, i18n("Do you really want to attach this directory \"%1\" ?", url.toLocalFile()), i18n("Attach directory"));
                if (rc == KMessageBox::Yes) {
                    addAttachment(urlWithEncoding);
                }
            } else {
                addAttachment(urlWithEncoding);
            }
        }

    }

#if 0 //QT5
    QPointer<KEncodingFileDialog> dialog = new KEncodingFileDialog(
        QUrl(/*startDir*/), QString(/*encoding*/), QString(/*filter*/),
        i18n("Attach File"), QFileDialog::Other, d->wParent);

    //dialog->okButton()->setGuiItem( KGuiItem( i18n("&Attach"), QLatin1String( "document-open" ) ) );
    //dialog->setMode( KFile::Files|KFile::Directory );
    if (dialog->exec() == KDialog::Accepted && dialog) {
        const KUrl::List files = dialog->selectedUrls();
        const QString encoding = MessageViewer::NodeHelper::fixEncoding(dialog->selectedEncoding());
        const int numberOfFiles(files.count());
        for (int i = 0; i < numberOfFiles; ++i) {
            const KUrl url = files.at(i);
            KUrl urlWithEncoding = url;
            urlWithEncoding.setFileEncoding(encoding);
            if (KMimeType::findByUrl(urlWithEncoding)->name() == QLatin1String("inode/directory")) {
                const int rc = KMessageBox::warningYesNo(d->wParent, i18n("Do you really want to attach this directory \"%1\" ?", url.toLocalFile()), i18n("Attach directory"));
                if (rc == KMessageBox::Yes) {
                    addAttachment(urlWithEncoding);
                }
            } else {
                addAttachment(urlWithEncoding);
            }
        }
    }
    delete dialog;
#endif
#else
    // use native dialog, while being much simpler, it actually fits on the screen much better than our own monster dialog
    const QString fileName = QFileDialog::getOpenFileName(d->wParent, i18n("Attach File") ,  QUrl(), QString());
    if (!fileName.isEmpty()) {
        addAttachment(QUrl::fromLocalFile(fileName));
    }
#endif
}

void AttachmentControllerBase::addAttachment(AttachmentPart::Ptr part)
{
    part->setEncrypted(d->model->isEncryptSelected());
    part->setSigned(d->model->isSignSelected());
    d->model->addAttachment(part);

    if (MessageComposer::MessageComposerSettings::self()->showMessagePartDialogOnAttach()) {
        attachmentProperties(part);
    }
    emit fileAttached();
}

MessageCore::AttachmentFromUrlBaseJob *AttachmentControllerBase::createAttachmentJob(const KUrl &url)
{
    MessageCore::AttachmentFromUrlBaseJob *ajob = 0;
    if (KMimeType::findByUrl(url)->name() == QLatin1String("inode/directory")) {
        qDebug() << "Creating attachment from folder";
        ajob = new AttachmentFromFolderJob(url, this);
    } else {
        ajob = new AttachmentFromUrlJob(url, this);
        qDebug() << "Creating attachment from file";
    }
    if (MessageComposer::MessageComposerSettings::maximumAttachmentSize() > 0) {
        ajob->setMaximumAllowedSize(MessageComposer::MessageComposerSettings::maximumAttachmentSize());
    }
    return ajob;
}

void AttachmentControllerBase::addAttachmentUrlSync(const KUrl &url)
{
    MessageCore::AttachmentFromUrlBaseJob *ajob = createAttachmentJob(url);
    if (ajob->exec()) {
        AttachmentPart::Ptr part = ajob->attachmentPart();
        addAttachment(part);
    } else {
        if (ajob->error()) {
            KMessageBox::sorry(d->wParent, ajob->errorString(), i18n("Failed to attach file"));
        }
    }
}

void AttachmentControllerBase::addAttachment(const KUrl &url)
{
    MessageCore::AttachmentFromUrlBaseJob *ajob = createAttachmentJob(url);
    connect(ajob, SIGNAL(result(KJob*)), this, SLOT(loadJobResult(KJob*)));
    ajob->start();
}

void AttachmentControllerBase::addAttachments(const KUrl::List &urls)
{
    foreach (const KUrl &url, urls) {
        addAttachment(url);
    }
}

void AttachmentControllerBase::showAttachPublicKeyDialog()
{
    using Kleo::KeySelectionDialog;
    QPointer<KeySelectionDialog> dialog = new KeySelectionDialog(
        i18n("Attach Public OpenPGP Key"),
        i18n("Select the public key which should be attached."),
        std::vector<GpgME::Key>(),
        KeySelectionDialog::PublicKeys | KeySelectionDialog::OpenPGPKeys,
        false /* no multi selection */,
        false /* no remember choice box */,
        d->wParent, "attach public key selection dialog");

    if (dialog->exec() == QDialog::Accepted && dialog) {
        exportPublicKey(dialog->fingerprint());
    }
    delete dialog;
}

void AttachmentControllerBase::enableAttachPublicKey(bool enable)
{
    d->attachPublicKeyAction->setEnabled(enable);
}

void AttachmentControllerBase::enableAttachMyPublicKey(bool enable)
{
    d->attachMyPublicKeyAction->setEnabled(enable);
}

void AttachmentControllerBase::setAttachOwnVcard(bool attachVcard)
{
    d->addOwnVcardAction->setChecked(attachVcard);
}

bool AttachmentControllerBase::attachOwnVcard() const
{
    return  d->addOwnVcardAction->isChecked();
}

void AttachmentControllerBase::setIdentityHasOwnVcard(bool state)
{
    d->addOwnVcardAction->setEnabled(state);
}

#include "moc_attachmentcontrollerbase.cpp"
