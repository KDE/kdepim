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

#include <akonadi/itemfetchjob.h>
#include <kio/jobuidelegate.h>

#include <QMenu>
#include <QPointer>

#include <KAction>
#include <KActionCollection>
#include <KDebug>
#include <KEncodingFileDialog>
#include <KFileDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <KMimeTypeTrader>
#include <KPushButton>
#include <KRun>
#include <KTemporaryFile>
#include <KFileItemActions>
#include <KActionMenu>

#include <kpimutils/kfileio.h>

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

using namespace MessageComposer;
using namespace MessageCore;

class MessageComposer::AttachmentControllerBase::Private
{
public:
    Private( AttachmentControllerBase *qq );
    ~Private();

    void attachmentRemoved( AttachmentPart::Ptr part ); // slot
    void compressJobResult( KJob *job ); // slot
    void loadJobResult( KJob *job ); // slot
    void openSelectedAttachments(); // slot
    void viewSelectedAttachments(); // slot
    void editSelectedAttachment(); // slot
    void editSelectedAttachmentWith(); // slot
    void removeSelectedAttachments(); // slot
    void saveSelectedAttachmentAs(); // slot
    void selectedAttachmentProperties(); // slot
    void editDone( MessageViewer::EditorWatcher *watcher ); // slot
    void attachPublicKeyJobResult( KJob *job ); // slot
    void slotAttachmentContentCreated( KJob *job ); // slot
    void addAttachmentPart( AttachmentPart::Ptr part );
    void selectedAllAttachment();
    void createOpenWithMenu( QMenu *topMenu, AttachmentPart::Ptr part );

    AttachmentControllerBase *const q;
    bool encryptEnabled;
    bool signEnabled;
    MessageComposer::AttachmentModel *model;
    QWidget *wParent;
    QHash<MessageViewer::EditorWatcher*,AttachmentPart::Ptr> editorPart;
    QHash<MessageViewer::EditorWatcher*,KTemporaryFile*> editorTempFile;

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

AttachmentControllerBase::Private::Private( AttachmentControllerBase *qq )
    : q( qq )
    , encryptEnabled( false )
    , signEnabled( false )
    , model( 0 )
    , wParent( 0 )
    , attachPublicKeyAction( 0 )
    , attachMyPublicKeyAction( 0 )
    , openContextAction( 0 )
    , viewContextAction( 0 )
    , editContextAction( 0 )
    , editWithContextAction( 0 )
    , removeAction( 0 )
    , removeContextAction( 0 )
    , saveAsAction( 0 )
    , saveAsContextAction( 0 )
    , propertiesAction( 0 )
    , propertiesContextAction( 0 )
    , addAction( 0 )
    , addContextAction( 0 )
    , selectAllAction( 0 )
    , attachmentMenu( 0 )
    , addOwnVcardAction( 0 )
{
}

AttachmentControllerBase::Private::~Private()
{
}

void AttachmentControllerBase::setSelectedParts( const AttachmentPart::List &selectedParts)
{
    d->selectedParts = selectedParts;
    const int selectedCount = selectedParts.count();
    const bool enableEditAction = (selectedCount == 1) &&
            ( !selectedParts.first()->isMessageOrMessageCollection() );

    d->openContextAction->setEnabled( selectedCount > 0 );
    d->viewContextAction->setEnabled( selectedCount > 0 );
    d->editContextAction->setEnabled( enableEditAction );
    d->editWithContextAction->setEnabled( enableEditAction );
    d->removeAction->setEnabled( selectedCount > 0 );
    d->removeContextAction->setEnabled( selectedCount > 0 );
    d->saveAsAction->setEnabled( selectedCount == 1 );
    d->saveAsContextAction->setEnabled( selectedCount == 1 );
    d->propertiesAction->setEnabled( selectedCount == 1 );
    d->propertiesContextAction->setEnabled( selectedCount == 1 );
}

void AttachmentControllerBase::Private::attachmentRemoved( AttachmentPart::Ptr part )
{
    if( uncompressedParts.contains( part ) ) {
        uncompressedParts.remove( part );
    }
}

void AttachmentControllerBase::Private::compressJobResult( KJob *job )
{
    if( job->error() ) {
        KMessageBox::sorry( wParent, job->errorString(), i18n( "Failed to compress attachment" ) );
        return;
    }

    Q_ASSERT( dynamic_cast<AttachmentCompressJob*>( job ) );
    AttachmentCompressJob *ajob = static_cast<AttachmentCompressJob*>( job );
    //AttachmentPart *originalPart = const_cast<AttachmentPart*>( ajob->originalPart() );
    AttachmentPart::Ptr originalPart = ajob->originalPart();
    AttachmentPart::Ptr compressedPart = ajob->compressedPart();

    if( ajob->isCompressedPartLarger() ) {
        const int result = KMessageBox::questionYesNo( wParent,
                                                       i18n( "The compressed attachment is larger than the original. "
                                                             "Do you want to keep the original one?" ),
                                                       QString( /*caption*/ ),
                                                       KGuiItem( i18nc( "Do not compress", "Keep" ) ),
                                                       KGuiItem( i18n( "Compress" ) ) );
        if( result == KMessageBox::Yes ) {
            // The user has chosen to keep the uncompressed file.
            return;
        }
    }

    kDebug() << "Replacing uncompressed part in model.";
    uncompressedParts[ compressedPart ] = originalPart;
    bool ok = model->replaceAttachment( originalPart, compressedPart );
    if( !ok ) {
        // The attachment was removed from the model while we were compressing.
        kDebug() << "Compressed a zombie.";
    }
}

void AttachmentControllerBase::Private::loadJobResult( KJob *job )
{
    if( job->error() ) {
        KMessageBox::sorry( wParent, job->errorString(), i18n( "Failed to attach file" ) );
        return;
    }

    Q_ASSERT( dynamic_cast<AttachmentLoadJob*>( job ) );
    AttachmentLoadJob *ajob = static_cast<AttachmentLoadJob*>( job );
    AttachmentPart::Ptr part = ajob->attachmentPart();
    q->addAttachment( part );
}

void AttachmentControllerBase::Private::openSelectedAttachments()
{
    Q_ASSERT( selectedParts.count() >= 1 );
    foreach( AttachmentPart::Ptr part, selectedParts ) {
        q->openAttachment( part );
    }
}

void AttachmentControllerBase::Private::viewSelectedAttachments()
{
    Q_ASSERT( selectedParts.count() >= 1 );
    foreach( AttachmentPart::Ptr part, selectedParts ) {
        q->viewAttachment( part );
    }
}

void AttachmentControllerBase::Private::editSelectedAttachment()
{
    Q_ASSERT( selectedParts.count() == 1 );
    q->editAttachment( selectedParts.first(), false /*openWith*/ );
    // TODO nicer api would be enum { OpenWithDialog, NoOpenWithDialog }
}

void AttachmentControllerBase::Private::editSelectedAttachmentWith()
{
    Q_ASSERT( selectedParts.count() == 1 );
    q->editAttachment( selectedParts.first(), true /*openWith*/ );
}

void AttachmentControllerBase::Private::removeSelectedAttachments()
{
    Q_ASSERT( selectedParts.count() >= 1 );
    foreach( AttachmentPart::Ptr part, selectedParts ) {
        model->removeAttachment( part );
    }
}

void AttachmentControllerBase::Private::saveSelectedAttachmentAs()
{
    Q_ASSERT( selectedParts.count() == 1 );
    q->saveAttachmentAs( selectedParts.first() );
}

void AttachmentControllerBase::Private::selectedAttachmentProperties()
{
    Q_ASSERT( selectedParts.count() == 1 );
    q->attachmentProperties( selectedParts.first() );
}

void AttachmentControllerBase::Private::editDone( MessageViewer::EditorWatcher *watcher )
{
    AttachmentPart::Ptr part = editorPart.take( watcher );
    Q_ASSERT( part );
    KTemporaryFile *tempFile = editorTempFile.take( watcher );
    Q_ASSERT( tempFile );

    if( watcher->fileChanged() ) {
        kDebug() << "File has changed.";

        // Read the new data and update the part in the model.
        tempFile->reset();
        QByteArray data = tempFile->readAll();
        part->setData( data );
        model->updateAttachment( part );
    }

    delete tempFile;
    // The watcher deletes itself.
}


void AttachmentControllerBase::Private::createOpenWithMenu( QMenu *topMenu, AttachmentPart::Ptr part )
{
    const QString contentTypeStr = QString::fromLatin1(part->mimeType());
    const KService::List offers = KFileItemActions::associatedApplications(QStringList()<<contentTypeStr, QString() );
    if (!offers.isEmpty()) {
        QMenu* menu = topMenu;
        QActionGroup *actionGroup = new QActionGroup( menu );
        connect( actionGroup, SIGNAL(triggered(QAction*)), q, SLOT(slotOpenWithAction(QAction*)) );

        if (offers.count() > 1) { // submenu 'open with'
            menu = new QMenu(i18nc("@title:menu", "&Open With"), topMenu);
            menu->menuAction()->setObjectName(QLatin1String("openWith_submenu")); // for the unittest
            topMenu->addMenu(menu);
        }
        //kDebug() << offers.count() << "offers" << topMenu << menu;

        KService::List::ConstIterator it = offers.constBegin();
        KService::List::ConstIterator end = offers.constEnd();
        for(; it != end; ++it) {
            KAction* act = MessageViewer::Util::createAppAction(*it,
                                                                // no submenu -> prefix single offer
                                                                menu == topMenu, actionGroup,menu);
            menu->addAction(act);
        }

        QString openWithActionName;
        if (menu != topMenu) { // submenu
            menu->addSeparator();
            openWithActionName = i18nc("@action:inmenu Open With", "&Other...");
        } else {
            openWithActionName = i18nc("@title:menu", "&Open With...");
        }
        KAction *openWithAct = new KAction(menu);
        openWithAct->setText(openWithActionName);
        QObject::connect(openWithAct, SIGNAL(triggered()), q, SLOT(slotOpenWithDialog()));
        menu->addAction(openWithAct);
    }
    else { // no app offers -> Open With...
        KAction *act = new KAction(topMenu);
        act->setText(i18nc("@title:menu", "&Open With..."));
        QObject::connect(act, SIGNAL(triggered()), q, SLOT(slotOpenWithDialog()));
        topMenu->addAction(act);
    }
}



void AttachmentControllerBase::exportPublicKey( const QString &fingerprint )
{
    if( fingerprint.isEmpty() || !Kleo::CryptoBackendFactory::instance()->openpgp() ) {
        kWarning() << "Tried to export key with empty fingerprint, or no OpenPGP.";
        Q_ASSERT( false ); // Can this happen?
        return;
    }

    MessageComposer::AttachmentFromPublicKeyJob *ajob = new MessageComposer::AttachmentFromPublicKeyJob( fingerprint, this );
    connect( ajob, SIGNAL(result(KJob*)), this, SLOT(attachPublicKeyJobResult(KJob*)) );
    ajob->start();
}

void AttachmentControllerBase::Private::attachPublicKeyJobResult( KJob *job )
{
    // The only reason we can't use loadJobResult() and need a separate method
    // is that we want to show the proper caption ("public key" instead of "file")...

    if( job->error() ) {
        KMessageBox::sorry( wParent, job->errorString(), i18n( "Failed to attach public key" ) );
        return;
    }

    Q_ASSERT( dynamic_cast<MessageComposer::AttachmentFromPublicKeyJob*>( job ) );
    MessageComposer::AttachmentFromPublicKeyJob *ajob = static_cast<MessageComposer::AttachmentFromPublicKeyJob*>( job );
    AttachmentPart::Ptr part = ajob->attachmentPart();
    q->addAttachment( part );
}


static KTemporaryFile *dumpAttachmentToTempFile( const AttachmentPart::Ptr part ) // local
{
    KTemporaryFile *file = new KTemporaryFile;
    if( !file->open() ) {
        kError() << "Could not open tempfile" << file->fileName();
        delete file;
        return 0;
    }
    if( file->write( part->data() ) == -1 ) {
        kError() << "Could not dump attachment to tempfile.";
        delete file;
        return 0;
    }
    file->flush();
    return file;
}



AttachmentControllerBase::AttachmentControllerBase( MessageComposer::AttachmentModel *model, QWidget *wParent, KActionCollection *actionCollection )
    : QObject( wParent )
    , d( new Private( this ) )
{
    d->model = model;
    connect( model, SIGNAL(attachUrlsRequested(KUrl::List)), this, SLOT(addAttachments(KUrl::List)) );
    connect( model, SIGNAL(attachmentRemoved(MessageCore::AttachmentPart::Ptr)),
             this, SLOT(attachmentRemoved(MessageCore::AttachmentPart::Ptr)) );
    connect( model, SIGNAL(attachmentCompressRequested(MessageCore::AttachmentPart::Ptr,bool)),
             this, SLOT(compressAttachment(MessageCore::AttachmentPart::Ptr,bool)) );
    connect( model, SIGNAL(encryptEnabled(bool)), this, SLOT(setEncryptEnabled(bool)) );
    connect( model, SIGNAL(signEnabled(bool)), this, SLOT(setSignEnabled(bool)) );

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
    d->attachPublicKeyAction = new KAction( i18n( "Attach &Public Key..." ), this );
    connect( d->attachPublicKeyAction, SIGNAL(triggered(bool)),
             this, SLOT(showAttachPublicKeyDialog()) );

    d->attachMyPublicKeyAction = new KAction( i18n( "Attach &My Public Key" ), this );
    connect( d->attachMyPublicKeyAction, SIGNAL(triggered(bool)), this, SLOT(attachMyPublicKey()) );

    d->attachmentMenu = new KActionMenu( KIcon( QLatin1String( "mail-attachment" ) ), i18n( "Attach" ), this );
    connect( d->attachmentMenu, SIGNAL(triggered(bool)), this, SLOT(showAddAttachmentDialog()) );

    d->attachmentMenu->setDelayed(true);

    d->addAction = new KAction( KIcon( QLatin1String( "mail-attachment" ) ), i18n( "&Attach File..." ), this );
    d->addAction->setIconText( i18n( "Attach" ) );
    d->addContextAction = new KAction( KIcon( QLatin1String( "mail-attachment" ) ),
                                       i18n( "Add Attachment..." ), this );
    connect( d->addAction, SIGNAL(triggered(bool)), this, SLOT(showAddAttachmentDialog()) );
    connect( d->addContextAction, SIGNAL(triggered(bool)), this, SLOT(showAddAttachmentDialog()) );

    d->addOwnVcardAction = new KAction( i18n("Attach Own Vcard"),this );
    d->addOwnVcardAction->setIconText( i18n( "Own Vcard" ) );
    d->addOwnVcardAction->setCheckable(true);
    connect(d->addOwnVcardAction, SIGNAL(triggered(bool)), this, SIGNAL(addOwnVcard(bool)));

    d->attachmentMenu->addAction(d->addAction);
    d->attachmentMenu->addSeparator();
    d->attachmentMenu->addAction(d->addOwnVcardAction);

    d->removeAction = new KAction( KIcon(QLatin1String("edit-delete")), i18n( "&Remove Attachment" ), this );
    d->removeContextAction = new KAction( KIcon(QLatin1String("edit-delete")), i18n( "Remove" ), this ); // FIXME need two texts. is there a better way?
    connect( d->removeAction, SIGNAL(triggered(bool)), this, SLOT(removeSelectedAttachments()) );
    connect( d->removeContextAction, SIGNAL(triggered(bool)), this, SLOT(removeSelectedAttachments()) );

    d->openContextAction = new KAction( i18nc( "to open", "Open" ), this );
    connect( d->openContextAction, SIGNAL(triggered(bool)), this, SLOT(openSelectedAttachments()) );

    d->viewContextAction = new KAction( i18nc( "to view", "View" ), this );
    connect( d->viewContextAction, SIGNAL(triggered(bool)), this, SLOT(viewSelectedAttachments()) );

    d->editContextAction = new KAction( i18nc( "to edit", "Edit" ), this );
    connect( d->editContextAction, SIGNAL(triggered(bool)), this, SLOT(editSelectedAttachment()) );

    d->editWithContextAction = new KAction( i18n( "Edit With..." ), this );
    connect( d->editWithContextAction, SIGNAL(triggered(bool)), this, SLOT(editSelectedAttachmentWith()) );

    d->saveAsAction = new KAction( KIcon( QLatin1String( "document-save-as" ) ),
                                   i18n( "&Save Attachment As..." ), this );
    d->saveAsContextAction = new KAction( KIcon( QLatin1String( "document-save-as" ) ),
                                          i18n( "Save As..." ), this );
    connect( d->saveAsAction, SIGNAL(triggered(bool)),
             this, SLOT(saveSelectedAttachmentAs()) );
    connect( d->saveAsContextAction, SIGNAL(triggered(bool)),
             this, SLOT(saveSelectedAttachmentAs()) );

    d->propertiesAction = new KAction( i18n( "Attachment Pr&operties..." ), this );
    d->propertiesContextAction = new KAction( i18n( "Properties" ), this );
    connect( d->propertiesAction, SIGNAL(triggered(bool)),
             this, SLOT(selectedAttachmentProperties()) );
    connect( d->propertiesContextAction, SIGNAL(triggered(bool)),
             this, SLOT(selectedAttachmentProperties()) );

    d->selectAllAction = new KAction( i18n("Select All"), this);
    connect( d->selectAllAction, SIGNAL(triggered(bool)),
             this, SIGNAL(selectedAllAttachment()) );

    // Insert the actions into the composer window's menu.
    KActionCollection *collection = d->mActionCollection;
    collection->addAction( QLatin1String( "attach_public_key" ), d->attachPublicKeyAction );
    collection->addAction( QLatin1String( "attach_my_public_key" ), d->attachMyPublicKeyAction );
    collection->addAction( QLatin1String( "attach" ), d->addAction );
    collection->addAction( QLatin1String( "remove" ), d->removeAction );
    collection->addAction( QLatin1String( "attach_save" ), d->saveAsAction );
    collection->addAction( QLatin1String( "attach_properties" ), d->propertiesAction );
    collection->addAction( QLatin1String( "select_all_attachment"), d->selectAllAction);
    collection->addAction( QLatin1String( "attach_menu"), d->attachmentMenu );
    collection->addAction( QLatin1String( "attach_own_vcard"), d->addOwnVcardAction );

    setSelectedParts( AttachmentPart::List());
    emit actionsCreated();
}

void AttachmentControllerBase::setEncryptEnabled( bool enabled )
{
    d->encryptEnabled = enabled;
}

void AttachmentControllerBase::setSignEnabled( bool enabled )
{
    d->signEnabled = enabled;
}

void AttachmentControllerBase::compressAttachment( AttachmentPart::Ptr part, bool compress )
{
    if( compress ) {
        kDebug() << "Compressing part.";

        AttachmentCompressJob *ajob = new AttachmentCompressJob( part, this );
        connect( ajob, SIGNAL(result(KJob*)), this, SLOT(compressJobResult(KJob*)) );
        ajob->start();
    } else {
        kDebug() << "Uncompressing part.";

        // Replace the compressed part with the original uncompressed part, and delete
        // the compressed part.
        AttachmentPart::Ptr originalPart = d->uncompressedParts.take( part );
        Q_ASSERT( originalPart ); // Found in uncompressedParts.
        bool ok = d->model->replaceAttachment( part, originalPart );
        Q_ASSERT( ok );
        Q_UNUSED( ok );
    }
}

void AttachmentControllerBase::showContextMenu()
{
    emit refreshSelection();


    const int numberOfParts(d->selectedParts.count());
    QMenu *menu = new QMenu;

    const bool enableEditAction = (numberOfParts == 1) &&
            ( !d->selectedParts.first()->isMessageOrMessageCollection() );

    if(numberOfParts>0) {
        if(numberOfParts == 1)
            d->createOpenWithMenu(menu, d->selectedParts.first());
        else
            menu->addAction(d->openContextAction);
        menu->addAction(d->viewContextAction);
    }
    if(enableEditAction) {
        menu->addAction(d->editWithContextAction);
        menu->addAction(d->editContextAction);
    }
    if(numberOfParts>0) {
        menu->addAction(d->removeContextAction);
    }
    if(numberOfParts == 1) {
        menu->addAction(d->saveAsContextAction);
        menu->addAction(d->propertiesContextAction);
    }

    menu->addSeparator();
    menu->addAction(d->selectAllAction);
    menu->addSeparator();
    menu->addAction(d->addContextAction);

    menu->exec( QCursor::pos() );
    delete menu;
}

void AttachmentControllerBase::slotOpenWithDialog()
{
    openWith();
}

void AttachmentControllerBase::slotOpenWithAction(QAction*act)
{
    KService::Ptr app = act->data().value<KService::Ptr>();
    Q_ASSERT( d->selectedParts.count() == 1 );

    openWith(app);
}

void AttachmentControllerBase::openWith(KService::Ptr offer)
{
    KTemporaryFile *tempFile = dumpAttachmentToTempFile( d->selectedParts.first() );
    if( !tempFile ) {
        KMessageBox::sorry( d->wParent,
                            i18n( "KMail was unable to write the attachment to a temporary file." ),
                            i18n( "Unable to open attachment" ) );
        return;
    }
    KUrl::List lst;
    KUrl url = KUrl::fromPath(tempFile->fileName());
    lst.append( url );
    bool result = false;
    if(offer) {
        result = KRun::run( *offer, lst, d->wParent, false );
    } else {
        result = KRun::displayOpenWithDialog( lst, d->wParent, false );
    }
    if ( !result ) {
        delete tempFile;
        tempFile = 0;
    } else {
        // The file was opened.  Delete it only when the composer is closed
        // (and this object is destroyed).
        tempFile->setParent( this ); // Manages lifetime.
    }
}

void AttachmentControllerBase::openAttachment( AttachmentPart::Ptr part )
{
    KTemporaryFile *tempFile = dumpAttachmentToTempFile( part );
    if( !tempFile ) {
        KMessageBox::sorry( d->wParent,
                            i18n( "KMail was unable to write the attachment to a temporary file." ),
                            i18n( "Unable to open attachment" ) );
        return;
    }

    bool success = KRun::runUrl( KUrl::fromPath( tempFile->fileName() ),
                                 QString::fromLatin1( part->mimeType() ),
                                 d->wParent,
                                 true /*tempFile*/,
                                 false /*runExecutables*/ );
    if( !success ) {
        if( KMimeTypeTrader::self()->preferredService( QString::fromLatin1( part->mimeType() ) ).isNull() ) {
            // KRun showed an Open-With dialog, and it was canceled.
        } else {
            // KRun failed.
            KMessageBox::sorry( d->wParent,
                                i18n( "KMail was unable to open the attachment." ),
                                i18n( "Unable to open attachment" ) );
        }
        delete tempFile;
        tempFile = 0;
    } else {
        // The file was opened.  Delete it only when the composer is closed
        // (and this object is destroyed).
        tempFile->setParent( this ); // Manages lifetime.
    }
}

void AttachmentControllerBase::viewAttachment( AttachmentPart::Ptr part )
{
    MessageComposer::Composer *composer = new MessageComposer::Composer;
    composer->globalPart()->setFallbackCharsetEnabled( true );
    MessageComposer::AttachmentJob *attachmentJob = new MessageComposer::AttachmentJob( part, composer );
    connect( attachmentJob, SIGNAL(result(KJob*)),
             this, SLOT(slotAttachmentContentCreated(KJob*)) );
    attachmentJob->start();
}

void AttachmentControllerBase::Private::slotAttachmentContentCreated( KJob *job )
{
    if ( !job->error() ) {
        const MessageComposer::AttachmentJob * const attachmentJob = dynamic_cast<MessageComposer::AttachmentJob*>( job );
        Q_ASSERT( attachmentJob );
        emit q->showAttachment( attachmentJob->content(), QByteArray() );
    } else {
        // TODO: show warning to the user
        kWarning() << "Error creating KMime::Content for attachment:" << job->errorText();
    }
}

void AttachmentControllerBase::editAttachment( AttachmentPart::Ptr part, bool openWith )
{
    KTemporaryFile *tempFile = dumpAttachmentToTempFile( part );
    if( !tempFile ) {
        KMessageBox::sorry( d->wParent,
                            i18n( "KMail was unable to write the attachment to a temporary file." ),
                            i18n( "Unable to edit attachment" ) );
        return;
    }

    MessageViewer::EditorWatcher *watcher = new MessageViewer::EditorWatcher(
                KUrl::fromPath( tempFile->fileName() ),
                QString::fromLatin1( part->mimeType() ), openWith,
                this, d->wParent );
    connect( watcher, SIGNAL(editDone(MessageViewer::EditorWatcher*)),
             this, SLOT(editDone(MessageViewer::EditorWatcher*)) );

    if( watcher->start() ) {
        // The attachment is being edited.
        // We will clean things up in editDone().
        d->editorPart[ watcher ] = part;
        d->editorTempFile[ watcher ] = tempFile;

        // Delete the temp file if the composer is closed (and this object is destroyed).
        tempFile->setParent( this ); // Manages lifetime.
    } else {
        kWarning() << "Could not start EditorWatcher.";
        delete watcher;
        delete tempFile;
    }
}

void AttachmentControllerBase::editAttachmentWith( AttachmentPart::Ptr part )
{
    editAttachment( part, true );
}

void AttachmentControllerBase::saveAttachmentAs( AttachmentPart::Ptr part )
{
    QString pname = part->name();
    if( pname.isEmpty() ) {
        pname = i18n( "unnamed" );
    }

    KUrl url = KFileDialog::getSaveUrl( pname,
                                        QString( /*filter*/ ), d->wParent,
                                        i18n( "Save Attachment As" ) );

    if( url.isEmpty() ) {
        kDebug() << "Save Attachment As dialog canceled.";
        return;
    }

    byteArrayToRemoteFile(part->data(), url);
}

void AttachmentControllerBase::byteArrayToRemoteFile(const QByteArray &aData, const KUrl &aURL, bool overwrite)
{
    KIO::StoredTransferJob *job = KIO::storedPut( aData, aURL, -1, overwrite ? KIO::Overwrite : KIO::DefaultFlags );
    connect( job, SIGNAL(result(KJob*)), SLOT(slotPutResult(KJob*)) );
}

void AttachmentControllerBase::slotPutResult(KJob *job)
{
    KIO::StoredTransferJob *_job = qobject_cast<KIO::StoredTransferJob *>( job );

    if (job->error())
    {
        if (job->error() == KIO::ERR_FILE_ALREADY_EXIST)
        {
            if (KMessageBox::warningContinueCancel(0,
                                                   i18n("File %1 exists.\nDo you want to replace it?", _job->url().toLocalFile()), i18n("Save to File"), KGuiItem(i18n("&Replace")))
                    == KMessageBox::Continue)
                byteArrayToRemoteFile(_job->data(), _job->url(), true);
        }
        else {
            KIO::JobUiDelegate *ui = static_cast<KIO::Job*>( job )->ui();
            ui->showErrorMessage();
        }
    }
}

void AttachmentControllerBase::attachmentProperties( AttachmentPart::Ptr part )
{
    QPointer<AttachmentPropertiesDialog> dialog = new AttachmentPropertiesDialog(
                part, false, d->wParent );

    dialog->setEncryptEnabled( d->encryptEnabled );
    dialog->setSignEnabled( d->signEnabled );

    if( dialog->exec() && dialog ) {
        d->model->updateAttachment( part );
    }
    delete dialog;
}

void AttachmentControllerBase::showAddAttachmentDialog()
{
#ifndef KDEPIM_MOBILE_UI
    QPointer<KEncodingFileDialog> dialog = new KEncodingFileDialog(
                QString( /*startDir*/ ), QString( /*encoding*/ ), QString( /*filter*/ ),
                i18n( "Attach File" ), KFileDialog::Other, d->wParent );

    dialog->okButton()->setGuiItem( KGuiItem( i18n("&Attach"), QLatin1String( "document-open" ) ) );
    dialog->setMode( KFile::Files|KFile::Directory );
    if( dialog->exec() == KDialog::Accepted && dialog ) {
        const KUrl::List files = dialog->selectedUrls();
        const QString encoding = MessageViewer::NodeHelper::fixEncoding( dialog->selectedEncoding() );
        const int numberOfFiles(files.count());
        for (int i=0; i<numberOfFiles; ++i) {
            const KUrl url = files.at( i );
            KUrl urlWithEncoding = url;
            urlWithEncoding.setFileEncoding( encoding );
            if ( KMimeType::findByUrl( urlWithEncoding )->name() == QLatin1String( "inode/directory" ) ) {
                const int rc = KMessageBox::warningYesNo( d->wParent,i18n("Do you really want to attach this directory \"%1\" ?", url.toLocalFile() ),i18n( "Attach directory" ) );
                if ( rc == KMessageBox::Yes ) {
                    addAttachment( urlWithEncoding );
                }
            } else {
                addAttachment( urlWithEncoding );
            }
        }
    }
    delete dialog;
#else
    // use native dialog, while being much simpler, it actually fits on the screen much better than our own monster dialog
    const QString fileName = KFileDialog::getOpenFileName( KUrl(), QString(), d->wParent, i18n("Attach File" ) );
    if ( !fileName.isEmpty() ) {
        addAttachment( KUrl::fromLocalFile( fileName ) );
    }
#endif
}

void AttachmentControllerBase::addAttachment( AttachmentPart::Ptr part )
{
    part->setEncrypted( d->model->isEncryptSelected() );
    part->setSigned( d->model->isSignSelected() );
    d->model->addAttachment( part );

    if( MessageComposer::MessageComposerSettings::self()->showMessagePartDialogOnAttach() ) {
        attachmentProperties( part );
    }
    emit fileAttached();
}

MessageCore::AttachmentFromUrlBaseJob * AttachmentControllerBase::createAttachmentJob(const KUrl &url)
{
    MessageCore::AttachmentFromUrlBaseJob *ajob = 0;
    if( KMimeType::findByUrl( url )->name() == QLatin1String( "inode/directory" ) ) {
        kDebug() << "Creating attachment from folder";
        ajob = new AttachmentFromFolderJob ( url, this );
    } else {
        ajob = new AttachmentFromUrlJob( url, this );
        kDebug() << "Creating attachment from file";
    }
    if( MessageComposer::MessageComposerSettings::maximumAttachmentSize() > 0 ) {
        ajob->setMaximumAllowedSize( MessageComposer::MessageComposerSettings::maximumAttachmentSize() );
    }
    return ajob;
}

void AttachmentControllerBase::addAttachmentUrlSync(const KUrl &url)
{
    MessageCore::AttachmentFromUrlBaseJob *ajob = createAttachmentJob(url);
    if(ajob->exec()) {
        AttachmentPart::Ptr part = ajob->attachmentPart();
        addAttachment( part );
    } else {
        if( ajob->error() ) {
            KMessageBox::sorry( d->wParent, ajob->errorString(), i18n( "Failed to attach file" ) );
        }
    }
}

void AttachmentControllerBase::addAttachment( const KUrl &url )
{
    MessageCore::AttachmentFromUrlBaseJob *ajob = createAttachmentJob(url);
    connect( ajob, SIGNAL(result(KJob*)), this, SLOT(loadJobResult(KJob*)) );
    ajob->start();
}

void AttachmentControllerBase::addAttachments( const KUrl::List &urls )
{
    foreach( const KUrl &url, urls ) {
        addAttachment( url );
    }
}

void AttachmentControllerBase::showAttachPublicKeyDialog()
{
    using Kleo::KeySelectionDialog;
    QPointer<KeySelectionDialog> dialog = new KeySelectionDialog(
                i18n( "Attach Public OpenPGP Key" ),
                i18n( "Select the public key which should be attached." ),
                std::vector<GpgME::Key>(),
                KeySelectionDialog::PublicKeys|KeySelectionDialog::OpenPGPKeys,
                false /* no multi selection */,
                false /* no remember choice box */,
                d->wParent, "attach public key selection dialog" );

    if( dialog->exec() == KDialog::Accepted && dialog ) {
        exportPublicKey( dialog->fingerprint() );
    }
    delete dialog;
}

void AttachmentControllerBase::enableAttachPublicKey( bool enable )
{
    d->attachPublicKeyAction->setEnabled( enable );
}

void AttachmentControllerBase::enableAttachMyPublicKey( bool enable )
{
    d->attachMyPublicKeyAction->setEnabled( enable );
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
