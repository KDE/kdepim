/*
  Copyright (c) 1997 Markus Wuebben <markus.wuebben@kde.org>
  Copyright (C) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Andras Mantia <andras@kdab.net>
  Copyright (c) 2010 Torgny Nyblom <nyblom@kde.org>
  Copyright (C) 2011-2015 Laurent Montel <montel@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
//#define MESSAGEVIEWER_READER_HTML_DEBUG 1

#include "viewer_p.h"
#include "viewer.h"
#include "messageviewer_debug.h"
#include "viewer/objecttreeemptysource.h"
#include "viewer/objecttreeviewersource.h"
#include "messagedisplayformatattribute.h"
#include "grantleethememanager.h"
#include "globalsettings_grantleetheme.h"
#include "scamdetection/scamdetectionwarningwidget.h"
#include "scamdetection/scamattribute.h"
#include "adblock/adblockmanager.h"
#include "widgets/todoedit.h"
#include "widgets/eventedit.h"
#include "widgets/noteedit.h"
#include "viewer/mimeparttree/mimeparttreeview.h"
#include "widgets/openattachmentfolderwidget.h"
#include "PimCommon/SlideContainer"
#include "PimCommon/GravatarCache"
#include "job/attachmentencryptwithchiasmusjob.h"
#include "job/attachmenteditjob.h"
#include "job/modifymessagedisplayformatjob.h"
#ifdef MESSAGEVIEWER_READER_HTML_DEBUG
#include "htmlwriter/filehtmlwriter.h"
#include "htmlwriter/teehtmlwriter.h"
#endif
#include <unistd.h> // link()
#include <errno.h>
//KDE includes
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QAction>
#include <KActionCollection>
#include <KActionMenu>
#include <KCharsets>
#include <KGuiItem>
#include <QWebView>
#include <QWebPage>
#include <QWebFrame>
#include <QMenu>
#include <KMessageBox>
#include <KMimeTypeChooser>
#include <KMimeTypeTrader>
#include <KRun>
#include <KSelectAction>
#include <KSharedConfigPtr>
#include <KStandardGuiItem>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <KToggleAction>
#include <QPrintPreviewDialog>
#include <QIcon>
#include <kfileitemactions.h>
#include <KFileItemListProperties>
#include <KLocalizedString>
#include <QMimeData>
#include <KContacts/Addressee>
#include <KContacts/VCardConverter>
#include <KEmailAddress>
#include <AkonadiCore/ItemModifyJob>
#include <AkonadiCore/ItemCreateJob>

#include <libkleo/kleo/cryptobackendfactory.h>
#include <libkleo/kleo/cryptobackend.h>

#include <MailTransport/mailtransport/errorattribute.h>

//Qt includes
#include <QClipboard>
#include <QDesktopWidget>
#include <QFileInfo>
#include <QItemSelectionModel>
#include <QSignalMapper>
#include <QSplitter>
#include <QTreeView>
#include <QPrinter>
#include <QPrintDialog>
#include <QHeaderView>
#include <QMimeDatabase>

//libkdepim
#include "Libkdepim/BroadcastStatus"
#include <attachment/attachmentpropertiesdialog.h>

#include <AkonadiCore/collection.h>
#include <AkonadiCore/itemfetchjob.h>
#include <AkonadiCore/itemfetchscope.h>
#include <Akonadi/KMime/MessageStatus>
#include <Akonadi/KMime/SpecialMailCollections>
#include <AkonadiCore/attributefactory.h>
#include <Akonadi/KMime/MessageParts>

#include "utils/autoqpointer.h"

//own includes
#include "widgets/attachmentdialog.h"
#include "viewer/attachmentstrategy.h"
#include "csshelper.h"
#include "settings/messageviewersettings.h"
#include "header/headerstyle.h"
#include "header/headerstrategy.h"
#include "widgets/htmlstatusbar.h"
#include "htmlwriter/webkitparthtmlwriter.h"
#include "widgets/mailsourceviewer.h"
#include "viewer/mimeparttree/mimetreemodel.h"
#include "viewer/nodehelper.h"
#include "viewer/objecttreeparser.h"
#include "viewer/urlhandlermanager.h"
#include "utils/messageviewerutil.h"
#include "widgets/vcardviewer.h"
#include "viewer/mailwebview.h"
#include "findbar/findbarmailwebview.h"
#include "PimCommon/TranslatorWidget"
#include "job/createtodojob.h"
#include "job/createeventjob.h"
#include "job/createnotejob.h"

#include "interfaces/bodypart.h"
#include "interfaces/htmlwriter.h"

#include <utils/stringutil.h>

#include <helpers/nodehelper.h>
#include "settings/messagecoresettings.h"
#include <AkonadiCore/agentinstance.h>
#include <AkonadiCore/agentmanager.h>
#include <AkonadiCore/CollectionFetchJob>
#include <AkonadiCore/collectionfetchscope.h>

#include <boost/bind.hpp>
#include <KJobWidgets/KJobWidgets>
#include <QApplication>
#include <QStandardPaths>

using namespace boost;
using namespace MailTransport;
using namespace MessageViewer;
using namespace MessageCore;

const int ViewerPrivate::delay = 150;
const qreal ViewerPrivate::zoomBy = 20;

static QAtomicInt _k_attributeInitialized;

ViewerPrivate::ViewerPrivate(Viewer *aParent, QWidget *mainWindow,
                             KActionCollection *actionCollection)
    : QObject(aParent),
      mNodeHelper(new NodeHelper),
      mViewer(0),
      mFindBar(0),
      mTranslatorWidget(0),
      mAttachmentStrategy(0),
      mHeaderStrategy(0),
      mHeaderStyle(0),
      mUpdateReaderWinTimer(0),
      mResizeTimer(0),
      mOldGlobalOverrideEncoding(QStringLiteral("---")),   // init with dummy value
      mMsgDisplay(true),
      mCSSHelper(0),
      mMainWindow(mainWindow),
      mActionCollection(actionCollection),
      mCopyAction(0),
      mCopyURLAction(0),
      mUrlOpenAction(0),
      mSelectAllAction(0),
      mScrollUpAction(0),
      mScrollDownAction(0),
      mScrollUpMoreAction(0),
      mScrollDownMoreAction(0),
      mHeaderOnlyAttachmentsAction(0),
      mSelectEncodingAction(0),
      mToggleFixFontAction(0),
      mToggleDisplayModeAction(0),
      mZoomTextOnlyAction(0),
      mZoomInAction(0),
      mZoomOutAction(0),
      mZoomResetAction(0),
      mToggleMimePartTreeAction(0),
      mSpeakTextAction(0),
      mCreateNoteAction(0),
      mCanStartDrag(false),
      mHtmlWriter(0),
      mSavedRelativePosition(0),
      mDecrytMessageOverwrite(false),
      mShowSignatureDetails(false),
      mShowAttachmentQuicklist(true),
      mRecursionCountForDisplayMessage(0),
      mCurrentContent(0),
      mMessagePartNode(0),
      q(aParent),
      mShowFullToAddressList(true),
      mShowFullCcAddressList(true),
      mPreviouslyViewedItem(-1),
      mScamDetectionWarning(0),
      mOpenAttachmentFolderWidget(0),
      mZoomFactor(100),
      mSliderContainer(0),
      mShareServiceManager(Q_NULLPTR)
{
    mMimePartTree = 0;
    if (!mainWindow) {
        mMainWindow = aParent;
    }
    if (_k_attributeInitialized.testAndSetAcquire(0, 1)) {
        Akonadi::AttributeFactory::registerAttribute<MessageViewer::MessageDisplayFormatAttribute>();
        Akonadi::AttributeFactory::registerAttribute<MessageViewer::ScamAttribute>();
    }
    mShareServiceManager = new PimCommon::ShareServiceUrlManager(this);

    mThemeManager = new GrantleeTheme::GrantleeThemeManager(GrantleeTheme::GrantleeThemeManager::Mail, QStringLiteral("header.desktop"), mActionCollection, QStringLiteral("messageviewer/themes/"));
    mThemeManager->setDownloadNewStuffConfigFile(QStringLiteral("messageviewer_header_themes.knsrc"));
    connect(mThemeManager, SIGNAL(grantleeThemeSelected()), this, SLOT(slotGrantleeHeaders()));
    connect(mThemeManager, SIGNAL(updateThemes()), this, SLOT(slotGrantleeThemesUpdated()));

    mDisplayFormatMessageOverwrite = MessageViewer::Viewer::UseGlobalSetting;
    mHtmlLoadExtOverride = false;

    mHtmlLoadExternalGlobalSetting = false;
    mHtmlMailGlobalSetting = false;

    mZoomTextOnly = false;

    mUpdateReaderWinTimer.setObjectName(QStringLiteral("mUpdateReaderWinTimer"));
    mResizeTimer.setObjectName(QStringLiteral("mResizeTimer"));

    mExternalWindow  = false;
    mPrinting = false;

    createWidgets();
    createActions();
    initHtmlWidget();
    readConfig();

    mLevelQuote = GlobalSettings::self()->collapseQuoteLevelSpin() - 1;

    mResizeTimer.setSingleShot(true);
    connect(&mResizeTimer, SIGNAL(timeout()),
            this, SLOT(slotDelayedResize()));

    mUpdateReaderWinTimer.setSingleShot(true);
    connect(&mUpdateReaderWinTimer, SIGNAL(timeout()),
            this, SLOT(updateReaderWin()));

    connect(mColorBar, SIGNAL(clicked()),
            this, SLOT(slotToggleHtmlMode()));

    // FIXME: Don't use the full payload here when attachment loading on demand is used, just
    //        like in KMMainWidget::slotMessageActivated().
    Akonadi::ItemFetchScope fs;
    fs.fetchFullPayload();
    fs.fetchAttribute<MailTransport::ErrorAttribute>();
    fs.fetchAttribute<MessageViewer::MessageDisplayFormatAttribute>();
    fs.fetchAttribute<MessageViewer::ScamAttribute>();
    mMonitor.setItemFetchScope(fs);
    connect(&mMonitor, SIGNAL(itemChanged(Akonadi::Item,QSet<QByteArray>)),
            this, SLOT(slotItemChanged(Akonadi::Item,QSet<QByteArray>)));
    connect(&mMonitor, SIGNAL(itemRemoved(Akonadi::Item)),
            this, SLOT(slotClear()));
    connect(&mMonitor, SIGNAL(itemMoved(Akonadi::Item,Akonadi::Collection,Akonadi::Collection)),
            this, SLOT(slotItemMoved(Akonadi::Item,Akonadi::Collection,Akonadi::Collection)));
}

ViewerPrivate::~ViewerPrivate()
{
    GlobalSettings::self()->save();
    delete mHtmlWriter; mHtmlWriter = 0;
    delete mViewer; mViewer = 0;
    delete mCSSHelper;
    mNodeHelper->forceCleanTempFiles();
    delete mNodeHelper;
    delete mThemeManager;
}

//-----------------------------------------------------------------------------
KMime::Content *ViewerPrivate::nodeFromUrl(const QUrl &url)
{
    KMime::Content *node = 0;
    if (url.isEmpty()) {
        return mMessage.data();
    }
    if (!url.isLocalFile()) {
        QString path = url.adjusted(QUrl::StripTrailingSlash).path();
        if (path.contains(QLatin1Char(':'))) {
            //if the content was not found, it might be in an extra node. Get the index of the extra node (the first part of the url),
            //and use the remaining part as a ContentIndex to find the node inside the extra node
            int i = path.left(path.indexOf(QLatin1Char(':'))).toInt();
            path = path.mid(path.indexOf(QLatin1Char(':')) + 1);
            KMime::ContentIndex idx(path);
            QList<KMime::Content *> extras = mNodeHelper->extraContents(mMessage.data());
            if (i >= 0 && i < extras.size()) {
                KMime::Content *c = extras[i];
                node = c->content(idx);
            }
        } else {
            if (mMessage) {
                node = mMessage->content(KMime::ContentIndex(path));
            }
        }
    } else {
        const QString path = url.toLocalFile();
        const uint right = path.lastIndexOf(QLatin1Char('/'));
        const uint left = path.lastIndexOf(QLatin1Char('.'), right);

        KMime::ContentIndex index(path.mid(left + 1, right - left - 1));
        node = mMessage->content(index);
    }
    return node;
}

void ViewerPrivate::openAttachment(KMime::Content *node, const QString &name)
{
    if (!node) {
        return;
    }

    if (node->contentType(false)) {
        if (node->contentType()->mimeType() == "text/x-moz-deleted") {
            return;
        }
        if (node->contentType()->mimeType() == "message/external-body") {
            if (node->contentType()->hasParameter(QStringLiteral("url"))) {
                const QString url = node->contentType()->parameter(QStringLiteral("url"));
                KRun::runUrl(QUrl(url), QStringLiteral("text/html"), q);
                return;
            }
        }
    }

    const bool isEncapsulatedMessage = node->parent() && node->parent()->bodyIsMessage();
    if (isEncapsulatedMessage) {

        // the viewer/urlhandlermanager expects that the message (mMessage) it is passed is the root when doing index calculation
        // in urls. Simply passing the result of bodyAsMessage() does not cut it as the resulting pointer is a child in its tree.
        KMime::Message::Ptr m = KMime::Message::Ptr(new KMime::Message);
        m->setContent(node->parent()->bodyAsMessage()->encodedContent());
        m->parse();
        atmViewMsg(m);
        return;
    }
    // determine the MIME type of the attachment
    // prefer the value of the Content-Type header
    QMimeDatabase mimeDb;
    auto mimetype = mimeDb.mimeTypeForName(QString::fromLatin1(node->contentType()->mimeType().toLower()));
    if (mimetype.isValid() && mimetype.inherits(KContacts::Addressee::mimeType())) {
        showVCard(node);
        return;
    }

    // special case treatment on mac and windows
    QString atmName = name;
    if (name.isEmpty()) {
        atmName = mNodeHelper->tempFileUrlFromNode(node).toLocalFile();
    }
    if (Util::handleUrlWithQDesktopServices(atmName)) {
        return;
    }

    if (!mimetype.isValid() || mimetype.name() == QLatin1String("application/octet-stream")) {
        mimetype = Util::mimetype(name);
    }
    KService::Ptr offer =
        KMimeTypeTrader::self()->preferredService(mimetype.name(), QStringLiteral("Application"));

    const QString filenameText = NodeHelper::fileName(node);

    AttachmentDialog dialog(mMainWindow, filenameText, offer ? offer->name() : QString(),
                            QLatin1String("askSave_") + mimetype.name());
    const int choice = dialog.exec();

    if (choice == AttachmentDialog::Save) {
        QUrl currentUrl;
        if (Util::saveContents(mMainWindow, KMime::Content::List() << node, currentUrl)) {
            showOpenAttachmentFolderWidget(currentUrl);
        }
    } else if (choice == AttachmentDialog::Open) { // Open
        if (offer) {
            attachmentOpenWith(node, offer);
        } else {
            attachmentOpen(node);
        }
    } else if (choice == AttachmentDialog::OpenWith) {
        attachmentOpenWith(node);
    } else { // Cancel
        qCDebug(MESSAGEVIEWER_LOG) << "Canceled opening attachment";
    }

}

bool ViewerPrivate::deleteAttachment(KMime::Content *node, bool showWarning)
{
    if (!node) {
        return true;
    }
    KMime::Content *parent = node->parent();
    if (!parent) {
        return true;
    }

    QList<KMime::Content *> extraNodes = mNodeHelper->extraContents(mMessage.data());
    if (extraNodes.contains(node->topLevel())) {
        KMessageBox::error(mMainWindow,
                           i18n("Deleting an attachment from an encrypted or old-style mailman message is not supported."),
                           i18n("Delete Attachment"));
        return true; //cancelled
    }

    if (showWarning && KMessageBox::warningContinueCancel(mMainWindow,
            i18n("Deleting an attachment might invalidate any digital signature on this message."),
            i18n("Delete Attachment"), KStandardGuiItem::del(), KStandardGuiItem::cancel(),
            QStringLiteral("DeleteAttachmentSignatureWarning"))
            != KMessageBox::Continue) {
        return false; //cancelled
    }
    //don't confuse the model
#ifndef QT_NO_TREEVIEW
    mMimePartTree->clearModel();
#endif
    QString filename;
    QString name;
    QByteArray mimetype;
    if (node->contentDisposition(false)) {
        filename = node->contentDisposition()->filename();
    }

    if (node->contentType(false)) {
        name = node->contentType()->name();
        mimetype = node->contentType()->mimeType();
    }

    // text/plain part:
    KMime::Content *deletePart = new KMime::Content(parent);
    deletePart->contentType()->setMimeType("text/x-moz-deleted");
    deletePart->contentType()->setName(QStringLiteral("Deleted: %1").arg(name), "utf8");
    deletePart->contentDisposition()->setDisposition(KMime::Headers::CDattachment);
    deletePart->contentDisposition()->setFilename(QStringLiteral("Deleted: %1").arg(name));

    deletePart->contentType()->setCharset("utf-8");
    deletePart->contentTransferEncoding()->setEncoding(KMime::Headers::CE7Bit);
    QByteArray bodyMessage = QByteArray("\nYou deleted an attachment from this message. The original MIME headers for the attachment were:");
    bodyMessage += ("\nContent-Type: ") + mimetype;
    bodyMessage += ("\nname=\"") + name.toUtf8() + "\"";
    bodyMessage += ("\nfilename=\"") + filename.toUtf8() + "\"";
    deletePart->setBody(bodyMessage);
    parent->replaceContent(node, deletePart);

    parent->assemble();

    KMime::Message *modifiedMessage = mNodeHelper->messageWithExtraContent(mMessage.data());
#ifndef QT_NO_TREEVIEW
    mMimePartTree->mimePartModel()->setRoot(modifiedMessage);
#endif
    mMessageItem.setPayloadFromData(modifiedMessage->encodedContent());
    Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob(mMessageItem);
    job->disableRevisionCheck();
    connect(job, SIGNAL(result(KJob*)), SLOT(itemModifiedResult(KJob*)));
    return true;
}

void ViewerPrivate::itemModifiedResult(KJob *job)
{
    if (job->error()) {
        qCDebug(MESSAGEVIEWER_LOG) << "Item update failed:" << job->errorString();
    } else {
        setMessageItem(mMessageItem, MessageViewer::Viewer::Force);
    }
}

void ViewerPrivate::editAttachment(KMime::Content *node, bool showWarning)
{
    MessageViewer::AttachmentEditJob *job = new MessageViewer::AttachmentEditJob(this);
    connect(job, SIGNAL(refreshMessage(Akonadi::Item)), this, SLOT(slotRefreshMessage(Akonadi::Item)));
    job->setMainWindow(mMainWindow);
    job->setMessageItem(mMessageItem);
    job->setMessage(mMessage);
    job->addAttachment(node, showWarning);
    job->canDeleteJob();
}

void ViewerPrivate::createOpenWithMenu(QMenu *topMenu, const QString &contentTypeStr, bool fromCurrentContent)
{
    const KService::List offers = KFileItemActions::associatedApplications(QStringList() << contentTypeStr, QString());
    if (!offers.isEmpty()) {
        QMenu *menu = topMenu;
        QActionGroup *actionGroup = new QActionGroup(menu);

        if (fromCurrentContent) {
            connect(actionGroup, SIGNAL(triggered(QAction*)), this, SLOT(slotOpenWithActionCurrentContent(QAction*)));
        } else {
            connect(actionGroup, SIGNAL(triggered(QAction*)), this, SLOT(slotOpenWithAction(QAction*)));
        }

        if (offers.count() > 1) { // submenu 'open with'
            menu = new QMenu(i18nc("@title:menu", "&Open With"), topMenu);
            menu->menuAction()->setObjectName(QStringLiteral("openWith_submenu")); // for the unittest
            topMenu->addMenu(menu);
        }
        qCDebug(MESSAGEVIEWER_LOG) << offers.count() << "offers" << topMenu << menu;

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
        if (fromCurrentContent) {
            connect(openWithAct, &QAction::triggered, this, &ViewerPrivate::slotOpenWithDialogCurrentContent);
        } else {
            connect(openWithAct, &QAction::triggered, this, &ViewerPrivate::slotOpenWithDialog);
        }

        menu->addAction(openWithAct);
    } else { // no app offers -> Open With...
        QAction *act = new QAction(topMenu);
        act->setText(i18nc("@title:menu", "&Open With..."));
        if (fromCurrentContent) {
            connect(act, &QAction::triggered, this, &ViewerPrivate::slotOpenWithDialogCurrentContent);
        } else {
            connect(act, &QAction::triggered, this, &ViewerPrivate::slotOpenWithDialog);
        }
        topMenu->addAction(act);
    }
}

void ViewerPrivate::slotOpenWithDialogCurrentContent()
{
    if (!mCurrentContent) {
        return;
    }
    attachmentOpenWith(mCurrentContent);
}

void ViewerPrivate::slotOpenWithDialog()
{
    auto contents = selectedContents();
    if (contents.count() == 1) {
        attachmentOpenWith(contents.first());
    }
}

void ViewerPrivate::slotOpenWithActionCurrentContent(QAction *act)
{
    if (!mCurrentContent) {
        return;
    }
    KService::Ptr app = act->data().value<KService::Ptr>();
    attachmentOpenWith(mCurrentContent, app);
}

void ViewerPrivate::slotOpenWithAction(QAction *act)
{
    KService::Ptr app = act->data().value<KService::Ptr>();
    auto contents = selectedContents();
    if (contents.count() == 1) {
        attachmentOpenWith(contents.first(), app);
    }
}

void ViewerPrivate::showAttachmentPopup(KMime::Content *node, const QString &name, const QPoint &globalPos)
{
    prepareHandleAttachment(node, name);
    QMenu *menu = new QMenu();
    QAction *action;
    bool deletedAttachment = false;
    if (node->contentType(false)) {
        deletedAttachment = (node->contentType()->mimeType() == "text/x-moz-deleted");
    }
    const QString contentTypeStr = QLatin1String(node->contentType()->mimeType());

    QSignalMapper *attachmentMapper = new QSignalMapper(menu);
    connect(attachmentMapper, SIGNAL(mapped(int)),
            this, SLOT(slotHandleAttachment(int)));

    action = menu->addAction(QIcon::fromTheme(QStringLiteral("document-open")), i18nc("to open", "Open"));
    action->setEnabled(!deletedAttachment);
    connect(action, SIGNAL(triggered(bool)), attachmentMapper, SLOT(map()));
    attachmentMapper->setMapping(action, Viewer::Open);
    if (!deletedAttachment) {
        createOpenWithMenu(menu, contentTypeStr, true);
    }

    QMimeDatabase mimeDb;
    auto mimetype = mimeDb.mimeTypeForName(contentTypeStr);
    if (mimetype.isValid()) {
        const QStringList parentMimeType = mimetype.parentMimeTypes();
        if ((contentTypeStr == QLatin1String("text/plain")) ||
                (contentTypeStr == QLatin1String("image/png")) ||
                (contentTypeStr == QLatin1String("image/jpeg")) ||
                parentMimeType.contains(QStringLiteral("text/plain")) ||
                parentMimeType.contains(QStringLiteral("image/png")) ||
                parentMimeType.contains(QStringLiteral("image/jpeg"))
           ) {
            action = menu->addAction(i18nc("to view something", "View"));
            action->setEnabled(!deletedAttachment);
            connect(action, SIGNAL(triggered(bool)), attachmentMapper, SLOT(map()));
            attachmentMapper->setMapping(action, Viewer::View);
        }
    }

    const bool attachmentInHeader = mViewer->isAttachmentInjectionPoint(globalPos);
    const bool hasScrollbar = mViewer->hasVerticalScrollBar();
    if (attachmentInHeader && hasScrollbar) {
        action = menu->addAction(i18n("Scroll To"));
        connect(action, SIGNAL(triggered(bool)), attachmentMapper, SLOT(map()));
        attachmentMapper->setMapping(action, Viewer::ScrollTo);
    }

    action = menu->addAction(QIcon::fromTheme(QStringLiteral("document-save-as")), i18n("Save As..."));
    action->setEnabled(!deletedAttachment);
    connect(action, SIGNAL(triggered(bool)), attachmentMapper, SLOT(map()));
    attachmentMapper->setMapping(action, Viewer::Save);

    action = menu->addAction(QIcon::fromTheme(QStringLiteral("edit-copy")), i18n("Copy"));
    action->setEnabled(!deletedAttachment);
    connect(action, SIGNAL(triggered(bool)), attachmentMapper, SLOT(map()));
    attachmentMapper->setMapping(action, Viewer::Copy);

    const bool isEncapsulatedMessage = node->parent() && node->parent()->bodyIsMessage();
    const bool canChange = mMessageItem.isValid() && mMessageItem.parentCollection().isValid() &&
                           (mMessageItem.parentCollection().rights() != Akonadi::Collection::ReadOnly) &&
                           !isEncapsulatedMessage;

    if (GlobalSettings::self()->allowAttachmentEditing()) {
        action = menu->addAction(QIcon::fromTheme(QStringLiteral("document-properties")), i18n("Edit Attachment"));
        connect(action, SIGNAL(triggered()), attachmentMapper, SLOT(map()));
        attachmentMapper->setMapping(action, Viewer::Edit);
        action->setEnabled(canChange);
    }
    action = menu->addAction(QIcon::fromTheme(QStringLiteral("edit-delete")), i18n("Delete Attachment"));
    connect(action, SIGNAL(triggered()), attachmentMapper, SLOT(map()));
    attachmentMapper->setMapping(action, Viewer::Delete);
    action->setEnabled(canChange && !deletedAttachment);
    if (name.endsWith(QLatin1String(".xia"), Qt::CaseInsensitive)
            && Kleo::CryptoBackendFactory::instance()->protocol("Chiasmus")) {
        action = menu->addAction(i18n("Decrypt With Chiasmus..."));
        connect(action, SIGNAL(triggered(bool)), attachmentMapper, SLOT(map()));
        attachmentMapper->setMapping(action, Viewer::ChiasmusEncrypt);
    }
    action = menu->addAction(i18n("Properties"));
    connect(action, SIGNAL(triggered(bool)), attachmentMapper, SLOT(map()));
    attachmentMapper->setMapping(action, Viewer::Properties);
    menu->exec(globalPos);
    delete menu;
}

void ViewerPrivate::prepareHandleAttachment(KMime::Content *node, const QString &fileName)
{
    mCurrentContent = node;
    mCurrentFileName = fileName;
}

QString ViewerPrivate::createAtmFileLink(const QString &atmFileName) const
{
    QFileInfo atmFileInfo(atmFileName);

    // tempfile name is /TMP/attachmentsRANDOM/atmFileInfo.fileName()"
    const QString tmpPath = QDir::tempPath() + QLatin1Char('/') +  QLatin1String("attachments");
    QDir().mkpath(tmpPath);
    QTemporaryDir *linkDir = new QTemporaryDir(tmpPath);
    QString linkPath = linkDir->path() + atmFileInfo.fileName();
    QFile *linkFile = new QFile(linkPath);
    linkFile->open(QIODevice::ReadWrite);
    const QString linkName = linkFile->fileName();
    delete linkFile;
    delete linkDir;

    if (::link(QFile::encodeName(atmFileName), QFile::encodeName(linkName)) == 0) {
        return linkName; // success
    }
    return QString();
}

KService::Ptr ViewerPrivate::getServiceOffer(KMime::Content *content)
{
    const QString fileName = mNodeHelper->writeNodeToTempFile(content);

    const QString contentTypeStr = QLatin1String(content->contentType()->mimeType());

    // determine the MIME type of the attachment
    // prefer the value of the Content-Type header
    QMimeDatabase mimeDb;
    auto mimetype = mimeDb.mimeTypeForName(contentTypeStr);

    if (mimetype.isValid() && mimetype.inherits(KContacts::Addressee::mimeType())) {
        attachmentView(content);
        return KService::Ptr(0);
    }

    if (!mimetype.isValid() || mimetype.name() == QLatin1String("application/octet-stream")) {
        /*TODO(Andris) port when on-demand loading is done   && msgPart.isComplete() */
        mimetype = MessageViewer::Util::mimetype(fileName);
    }
    return KMimeTypeTrader::self()->preferredService(mimetype.name(), QStringLiteral("Application"));
}

KMime::Content::List ViewerPrivate::selectedContents()
{
    return mMimePartTree->selectedContents();
}

void ViewerPrivate::attachmentOpenWith(KMime::Content *node, const KService::Ptr &offer)
{
    QString name = mNodeHelper->writeNodeToTempFile(node);
    QString linkName = createAtmFileLink(name);
    QList<QUrl> lst;
    QUrl url;
    bool autoDelete = true;

    if (linkName.isEmpty()) {
        autoDelete = false;
        linkName = name;
    }

    const QFileDevice::Permissions perms = QFile::permissions(linkName);
    QFile::setPermissions(linkName, perms | QFileDevice::ReadUser | QFileDevice::WriteUser);

    url.setPath(linkName);
    lst.append(url);
    if (offer) {
        if ((!KRun::runService(*offer, lst, 0, autoDelete)) && autoDelete) {
            QFile::remove(url.toLocalFile());
        }
    } else {
        if ((! KRun::displayOpenWithDialog(lst, mMainWindow, autoDelete)) && autoDelete) {
            QFile::remove(url.toLocalFile());
        }
    }
}

void ViewerPrivate::attachmentOpen(KMime::Content *node)
{
    KService::Ptr offer = getServiceOffer(node);
    if (!offer) {
        qCDebug(MESSAGEVIEWER_LOG) << "got no offer";
        return;
    }
    attachmentOpenWith(node, offer);
}

CSSHelper *ViewerPrivate::cssHelper() const
{
    return mCSSHelper;
}

bool ViewerPrivate::decryptMessage() const
{
    if (!GlobalSettings::self()->alwaysDecrypt()) {
        return mDecrytMessageOverwrite;
    } else {
        return true;
    }
}

int ViewerPrivate::pointsToPixel(int pointSize) const
{
    return (pointSize * mViewer->logicalDpiY() + 36) / 72;
}

void ViewerPrivate::displaySplashPage(const QString &info)
{
    mMsgDisplay = false;
    adjustLayout();

    const QString location = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kmail2/about/main.html"));  //FIXME(Andras) copy to $KDEDIR/share/apps/messageviewer
    const QString stylesheet = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("/kf5/infopage/kde_infopage.css"));
    QString rtlStylesheet;
    if (QApplication::isRightToLeft()) {
        rtlStylesheet = QLatin1String("@import \"") + QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("/kf5/infopage/kde_infopage_rtl.css")) +  QLatin1String("\";");
    }
    QFile f(location);
    if (!f.open(QIODevice::ReadOnly)) {
        qCWarning(MESSAGEVIEWER_LOG) << "Failed to read splash page: " << f.errorString();
        return;
    }
    const QString content = QString::fromLocal8Bit(f.readAll()).arg(stylesheet, rtlStylesheet);
    f.close();

    const QString fontSize = QString::number(pointsToPixel(mCSSHelper->bodyFont().pointSize()));
    const QString catchPhrase; //not enough space for a catch phrase at default window size i18n("Part of the Kontact Suite");
    const QString quickDescription = i18n("The KDE email client.");

    mViewer->setHtml(content.arg(fontSize).arg(mAppName).arg(catchPhrase).arg(quickDescription).arg(info), QUrl::fromLocalFile(location));
    mViewer->show();
}

void ViewerPrivate::enableMessageDisplay()
{
    mMsgDisplay = true;
    adjustLayout();
}

void ViewerPrivate::displayMessage()
{
    showHideMimeTree();

    mNodeHelper->setOverrideCodec(mMessage.data(), overrideCodec());

    if (mMessageItem.hasAttribute<MessageViewer::MessageDisplayFormatAttribute>()) {
        const MessageViewer::MessageDisplayFormatAttribute *const attr = mMessageItem.attribute<MessageViewer::MessageDisplayFormatAttribute>();
        setHtmlLoadExtOverride(attr->remoteContent());
        setDisplayFormatMessageOverwrite(attr->messageFormat());
    }

    htmlWriter()->begin(QString());
    htmlWriter()->queue(mCSSHelper->htmlHead(mUseFixedFont));

    if (!mMainWindow) {
        q->setWindowTitle(mMessage->subject()->asUnicodeString());
    }

    // Don't update here, parseMsg() can overwrite the HTML mode, which would lead to flicker.
    // It is updated right after parseMsg() instead.
    mColorBar->setMode(Util::Normal, HtmlStatusBar::NoUpdate);

    if (mMessageItem.hasAttribute<ErrorAttribute>()) {
        //TODO: Insert link to clear error so that message might be resent
        const ErrorAttribute *const attr = mMessageItem.attribute<ErrorAttribute>();
        Q_ASSERT(attr);
        const QColor foreground = KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::NegativeText).color();
        const QColor background = KColorScheme(QPalette::Active, KColorScheme::View).background(KColorScheme::NegativeBackground).color();

        htmlWriter()->queue(QStringLiteral("<div style=\"background:%1;color:%2;border:1px solid %3\">%4</div>").arg(background.name(), foreground.name(), foreground.name(), attr->message().toHtmlEscaped()));
        htmlWriter()->queue(QStringLiteral("<p></p>"));
    }

    parseContent(mMessage.data());
#ifndef QT_NO_TREEVIEW
    mMimePartTree->setRoot(mNodeHelper->messageWithExtraContent(mMessage.data()));
#endif
    mColorBar->update();

    htmlWriter()->queue(QStringLiteral("</body></html>"));
    connect(mPartHtmlWriter, SIGNAL(finished()), this, SLOT(injectAttachments()), Qt::UniqueConnection);
    connect(mPartHtmlWriter, SIGNAL(finished()), this, SLOT(toggleFullAddressList()), Qt::UniqueConnection);
    connect(mPartHtmlWriter, SIGNAL(finished()), this, SLOT(slotMessageRendered()), Qt::UniqueConnection);
    htmlWriter()->flush();
}

void ViewerPrivate::collectionFetchedForStoringDecryptedMessage(KJob *job)
{
    if (job->error()) {
        return;
    }

    Akonadi::Collection col;
    Q_FOREACH (const Akonadi::Collection &c, static_cast<Akonadi::CollectionFetchJob *>(job)->collections()) {
        if (c == mMessageItem.parentCollection()) {
            col = c;
            break;
        }
    }

    if (!col.isValid()) {
        return;
    }
    Akonadi::AgentInstance::List instances = Akonadi::AgentManager::self()->instances();
    const QString itemResource = col.resource();
    Akonadi::AgentInstance resourceInstance;
    foreach (const Akonadi::AgentInstance &instance, instances) {
        if (instance.identifier() == itemResource) {
            resourceInstance = instance;
            break;
        }
    }
    bool isInOutbox = true;
    Akonadi::Collection outboxCollection = Akonadi::SpecialMailCollections::self()->collection(
            Akonadi::SpecialMailCollections::Outbox, resourceInstance);
    if (resourceInstance.isValid() && outboxCollection != col) {
        isInOutbox = false;
    }

    if (!isInOutbox) {
        KMime::Message::Ptr unencryptedMessage = mNodeHelper->unencryptedMessage(mMessage);
        if (unencryptedMessage) {
            mMessageItem.setPayload<KMime::Message::Ptr>(unencryptedMessage);
            Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob(mMessageItem);
            connect(job, SIGNAL(result(KJob*)), SLOT(itemModifiedResult(KJob*)));
        }
    }
}

void ViewerPrivate::postProcessMessage(ObjectTreeParser *otp, KMMsgEncryptionState encryptionState)
{
    if (GlobalSettings::self()->storeDisplayedMessagesUnencrypted()) {

        // Hack to make sure the S/MIME CryptPlugs follows the strict requirement
        // of german government:
        // --> All received encrypted messages *must* be stored in unencrypted form
        //     after they have been decrypted once the user has read them.
        //     ( "Aufhebung der Verschluesselung nach dem Lesen" )
        //
        // note: Since there is no configuration option for this, we do that for
        //       all kinds of encryption now - *not* just for S/MIME.
        //       This could be changed in the objectTreeToDecryptedMsg() function
        //       by deciding when (or when not, resp.) to set the 'dataNode' to
        //       something different than 'curNode'.

        const bool messageAtLeastPartiallyEncrypted = (KMMsgFullyEncrypted == encryptionState) ||
                (KMMsgPartiallyEncrypted == encryptionState);
        // only proceed if we were called the normal way - not by
        // double click on the message (==not running in a separate window)
        if (decryptMessage() && // only proceed if the message has actually been decrypted
                !otp->hasPendingAsyncJobs() && // only proceed if no pending async jobs are running:
                messageAtLeastPartiallyEncrypted) {
            //check if the message is in the outbox folder
            //FIXME: using root() is too much, but using mMessageItem.parentCollection() returns no collections in job->collections()
            //FIXME: this is done async, which means it is possible that the user selects another message while
            //       this job is running. In that case, collectionFetchedForStoringDecryptedMessage() will work
            //       on the wrong item!
            Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(),
                    Akonadi::CollectionFetchJob::Recursive);
            connect(job, SIGNAL(result(KJob*)),
                    this, SLOT(collectionFetchedForStoringDecryptedMessage(KJob*)));
        }
    }
}

void ViewerPrivate::parseContent(KMime::Content *content)
{
    assert(content != 0);

    // Check if any part of this message is a v-card
    // v-cards can be either text/x-vcard or text/directory, so we need to check
    // both.
    KMime::Content *vCardContent = findContentByType(content, "text/x-vcard");
    if (!vCardContent) {
        vCardContent = findContentByType(content, "text/directory");
    }
    bool hasVCard = false;
    if (vCardContent) {
        // ### FIXME: We should only do this if the vCard belongs to the sender,
        // ### i.e. if the sender's email address is contained in the vCard.
        const QByteArray vCard = vCardContent->decodedContent();
        KContacts::VCardConverter t;
        if (!t.parseVCards(vCard).isEmpty()) {
            hasVCard = true;
            mNodeHelper->writeNodeToTempFile(vCardContent);
        }
    }

    KMime::Message *message = dynamic_cast<KMime::Message *>(content);
    if (message) {
        htmlWriter()->queue(writeMsgHeader(message, hasVCard ? vCardContent : 0, true));
    }

    // Pass control to the OTP now, which does the real work
    mNodeHelper->removeTempFiles();
    mNodeHelper->setNodeUnprocessed(mMessage.data(), true);
    MailViewerSource otpSource(this);
    ObjectTreeParser otp(&otpSource, mNodeHelper, 0, mMessage.data() != content /* show only single node */);
    otp.setAllowAsync(!mPrinting);
    otp.setPrinting(mPrinting);
    otp.parseObjectTree(content);

    // TODO: Setting the signature state to nodehelper is not enough, it should actually
    // be added to the store, so that the message list correctly displays the signature state
    // of messages that were parsed at least once
    // store encrypted/signed status information in the KMMessage
    //  - this can only be done *after* calling parseObjectTree()
    KMMsgEncryptionState encryptionState = mNodeHelper->overallEncryptionState(content);
    KMMsgSignatureState  signatureState  = mNodeHelper->overallSignatureState(content);
    mNodeHelper->setEncryptionState(content, encryptionState);
    // Don't reset the signature state to "not signed" (e.g. if one canceled the
    // decryption of a signed messages which has already been decrypted before).
    if (signatureState != KMMsgNotSigned ||
            mNodeHelper->signatureState(content) == KMMsgSignatureStateUnknown) {
        mNodeHelper->setSignatureState(content, signatureState);
    }

    postProcessMessage(&otp, encryptionState);

    showHideMimeTree();
}

QString ViewerPrivate::writeMsgHeader(KMime::Message *aMsg, KMime::Content *vCardNode,
                                      bool topLevel)
{
    if (!headerStyle()) {
        qCCritical(MESSAGEVIEWER_LOG) << "trying to writeMsgHeader() without a header style set!";
    }
    if (!headerStrategy()) {
        qCCritical(MESSAGEVIEWER_LOG) << "trying to writeMsgHeader() without a header strategy set!";
    }
    QString href;
    if (vCardNode) {
        href = mNodeHelper->asHREF(vCardNode, QStringLiteral("body"));
    }

    headerStyle()->setHeaderStrategy(headerStrategy());
    headerStyle()->setVCardName(href);
    headerStyle()->setPrinting(mPrinting);
    headerStyle()->setTopLevel(topLevel);
    headerStyle()->setAllowAsync(true);
    headerStyle()->setSourceObject(this);
    headerStyle()->setNodeHelper(mNodeHelper);
    headerStyle()->setMessagePath(mMessagePath);

    if (mMessageItem.isValid()) {
        Akonadi::MessageStatus status;
        status.setStatusFromFlags(mMessageItem.flags());

        headerStyle()->setMessageStatus(status);
    }

    return headerStyle()->format(aMsg);
}

void ViewerPrivate::showVCard(KMime::Content *msgPart)
{
    const QByteArray vCard = msgPart->decodedContent();

    VCardViewer *vcv = new VCardViewer(mMainWindow, vCard);
    vcv->setAttribute(Qt::WA_DeleteOnClose);
    vcv->show();
}

void ViewerPrivate::initHtmlWidget()
{
    mViewer->setFocusPolicy(Qt::WheelFocus);
    mViewer->installEventFilter(this);

    if (!htmlWriter()) {
        mPartHtmlWriter = new WebKitPartHtmlWriter(mViewer, 0);
#ifdef MESSAGEVIEWER_READER_HTML_DEBUG
        mHtmlWriter = new TeeHtmlWriter(new FileHtmlWriter(QString()),
                                        mPartHtmlWriter);
#else
        mHtmlWriter = mPartHtmlWriter;
#endif
    }

    connect(mViewer, SIGNAL(linkHovered(QString,QString,QString)),
            this, SLOT(slotUrlOn(QString,QString,QString)));
    connect(mViewer, SIGNAL(linkClicked(QUrl)),
            this, SLOT(slotUrlOpen(QUrl)), Qt::QueuedConnection);
    connect(mViewer, SIGNAL(popupMenu(QUrl,QUrl,QPoint)),
            SLOT(slotUrlPopup(QUrl,QUrl,QPoint)));
    connect(mViewer, SIGNAL(messageMayBeAScam()), this, SLOT(slotMessageMayBeAScam()));
    connect(mScamDetectionWarning, SIGNAL(showDetails()), mViewer, SLOT(slotShowDetails()));
    connect(mScamDetectionWarning, SIGNAL(moveMessageToTrash()), this, SIGNAL(moveMessageToTrash()));
    connect(mScamDetectionWarning, SIGNAL(messageIsNotAScam()), this, SLOT(slotMessageIsNotAScam()));
    connect(mScamDetectionWarning, SIGNAL(addToWhiteList()), this, SLOT(slotAddToWhiteList()));
}

bool ViewerPrivate::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent *>(e);
        if (me->button() == Qt::LeftButton && (me->modifiers() & Qt::ShiftModifier)) {
            // special processing for shift+click
            URLHandlerManager::instance()->handleShiftClick(mHoveredUrl, this);
            return true;
        }
        if (me->button() == Qt::LeftButton) {
            mCanStartDrag = URLHandlerManager::instance()->willHandleDrag(mHoveredUrl, this);
            mLastClickPosition = me->pos();
        }
    } else if (e->type() ==  QEvent::MouseButtonRelease) {
        mCanStartDrag = false;
    } else if (e->type() == QEvent::MouseMove) {

        QMouseEvent *me = static_cast<QMouseEvent *>(e);

        // First, update the hovered URL
        mHoveredUrl = mViewer->linkOrImageUrlAt(me->globalPos());

        // If we are potentially handling a drag, deal with that.
        if (mCanStartDrag && me->buttons() & Qt::LeftButton) {

            if ((mLastClickPosition - me->pos()).manhattanLength() > QApplication::startDragDistance()) {
                if (URLHandlerManager::instance()->handleDrag(mHoveredUrl, this)) {

                    // If the URL handler manager started a drag, don't handle this in the future
                    mCanStartDrag = false;
                }
            }

            // Don't tell WebKit about this mouse move event, or it might start its own drag!
            return true;
        }
    }
    //Don't tell to Webkit to get zoom > 300 and < 100
    else if (e->type() == QEvent::Wheel) {
        QWheelEvent *me = static_cast<QWheelEvent *>(e);
        if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
            const int numDegrees = me->delta() / 8;
            const int numSteps = numDegrees / 15;
            const qreal factor = mZoomFactor + numSteps * 10;
            if (factor >= 10 && factor <= 300) {
                mZoomFactor = factor;
                setZoomFactor(factor / 100.0);
            }
            return true;
        }
    }

    // standard event processing
    return false;
}

void ViewerPrivate::readConfig()
{
    delete mCSSHelper;
    mCSSHelper = new CSSHelper(mViewer);

    mUseFixedFont = GlobalSettings::self()->useFixedFont();
    if (mToggleFixFontAction) {
        mToggleFixFontAction->setChecked(mUseFixedFont);
    }

    mHtmlMailGlobalSetting = GlobalSettings::self()->htmlMail();
    mHtmlLoadExternalGlobalSetting = GlobalSettings::self()->htmlLoadExternal();

    mZoomTextOnly = GlobalSettings::self()->zoomTextOnly();
    setZoomTextOnly(mZoomTextOnly);
    readGravatarConfig();

    if (headerStrategy()) {
        headerStrategy()->readConfig();
    }
    KToggleAction *raction = actionForHeaderStyle(headerStyle(), headerStrategy());
    if (raction) {
        raction->setChecked(true);
    }

    setAttachmentStrategy(AttachmentStrategy::create(GlobalSettings::self()->attachmentStrategy()));
    raction = actionForAttachmentStrategy(attachmentStrategy());
    if (raction) {
        raction->setChecked(true);
    }

    adjustLayout();

    readGlobalOverrideCodec();

    // Note that this call triggers an update, see this call has to be at the
    // bottom when all settings are already est.
    setHeaderStyleAndStrategy(HeaderStyle::create(GlobalSettings::self()->headerStyle()),
                              HeaderStrategy::create(GlobalSettings::self()->headerSetDisplayed()));
    initGrantleeThemeName();

#ifndef KDEPIM_NO_WEBKIT
    mViewer->settings()->setFontSize(QWebSettings::MinimumFontSize, GlobalSettings::self()->minimumFontSize());
    mViewer->settings()->setFontSize(QWebSettings::MinimumLogicalFontSize, GlobalSettings::self()->minimumFontSize());
    mViewer->settings()->setAttribute(QWebSettings::PrintElementBackgrounds, GlobalSettings::self()->printBackgroundColorImages());
#endif

    if (mMessage) {
        update();
    }
    mColorBar->update();
}

void ViewerPrivate::readGravatarConfig()
{
    PimCommon::GravatarCache::self()->setMaximumSize(GlobalSettings::self()->gravatarCacheSize());
    if (!GlobalSettings::self()->gravatarSupportEnabled()) {
        PimCommon::GravatarCache::self()->clear();
    }
}

void ViewerPrivate::slotGeneralFontChanged()
{
    delete mCSSHelper;
    mCSSHelper = new CSSHelper(mViewer);
    if (mMessage) {
        update();
    }
}

void ViewerPrivate::writeConfig(bool sync)
{
    GlobalSettings::self()->setUseFixedFont(mUseFixedFont);
    if (headerStyle()) {
        GlobalSettings::self()->setHeaderStyle(QLatin1String(headerStyle()->name()));
        GrantleeTheme::GrantleeSettings::self()->setGrantleeMailThemeName(headerStyle()->theme().dirName());
    }
    if (headerStrategy()) {
        GlobalSettings::self()->setHeaderSetDisplayed(QLatin1String(headerStrategy()->name()));
    }
    if (attachmentStrategy()) {
        GlobalSettings::self()->setAttachmentStrategy(QLatin1String(attachmentStrategy()->name()));
    }
    GlobalSettings::self()->setZoomTextOnly(mZoomTextOnly);

    saveSplitterSizes();
    if (sync) {
        Q_EMIT requestConfigSync();
    }
}

void ViewerPrivate::setHeaderStyleAndStrategy(HeaderStyle *style,
        HeaderStrategy *strategy , bool writeInConfigFile)
{

    if (mHeaderStyle == style && mHeaderStrategy == strategy) {
        return;
    }

    mHeaderStyle = style ? style : HeaderStyle::fancy();
    mHeaderStrategy = strategy ? strategy : HeaderStrategy::rich();
    if (mHeaderOnlyAttachmentsAction) {
        mHeaderOnlyAttachmentsAction->setEnabled(mHeaderStyle->hasAttachmentQuickList());
        if (!mHeaderStyle->hasAttachmentQuickList() &&
                mAttachmentStrategy->requiresAttachmentListInHeader()) {
            // Style changed to something without an attachment quick list, need to change attachment
            // strategy
            setAttachmentStrategy(AttachmentStrategy::smart());
            actionForAttachmentStrategy(mAttachmentStrategy)->setChecked(true);
        }
    }
    update(Viewer::Force);

    if (!mExternalWindow && writeInConfigFile) {
        writeConfig();
    }

}

void ViewerPrivate::setAttachmentStrategy(const AttachmentStrategy *strategy)
{
    if (mAttachmentStrategy == strategy) {
        return;
    }
    mAttachmentStrategy = strategy ? strategy : AttachmentStrategy::smart();
    update(Viewer::Force);
}

void ViewerPrivate::setOverrideEncoding(const QString &encoding)
{
    if (encoding == mOverrideEncoding) {
        return;
    }

    mOverrideEncoding = encoding;
    if (mSelectEncodingAction) {
        if (encoding.isEmpty()) {
            mSelectEncodingAction->setCurrentItem(0);
        } else {
            const QStringList encodings = mSelectEncodingAction->items();
            int i = 0;
            for (QStringList::const_iterator it = encodings.constBegin(), end = encodings.constEnd(); it != end; ++it, ++i) {
                if (NodeHelper::encodingForName(*it) == encoding) {
                    mSelectEncodingAction->setCurrentItem(i);
                    break;
                }
            }
            if (i == encodings.size()) {
                // the value of encoding is unknown => use Auto
                qCWarning(MESSAGEVIEWER_LOG) << "Unknown override character encoding" << encoding
                                             << ". Using Auto instead.";
                mSelectEncodingAction->setCurrentItem(0);
                mOverrideEncoding.clear();
            }
        }
    }
    update(Viewer::Force);
}

void ViewerPrivate::printMessage(const Akonadi::Item &message)
{
    disconnect(mPartHtmlWriter, SIGNAL(finished()), this, SLOT(slotPrintMsg()));
    connect(mPartHtmlWriter, SIGNAL(finished()), this, SLOT(slotPrintMsg()));
    setMessageItem(message, Viewer::Force);
}

void ViewerPrivate::printPreviewMessage(const Akonadi::Item &message)
{
    disconnect(mPartHtmlWriter, SIGNAL(finished()), this, SLOT(slotPrintPreview()));
    connect(mPartHtmlWriter, SIGNAL(finished()), this, SLOT(slotPrintPreview()));
    setMessageItem(message, Viewer::Force);
}

void ViewerPrivate::resetStateForNewMessage()
{
    mClickedUrl.clear();
    mImageUrl.clear();
    enableMessageDisplay(); // just to make sure it's on
    mMessage.reset();
    mNodeHelper->clear();
    mMessagePartNode = 0;
#ifndef QT_NO_TREEVIEW
    mMimePartTree->clearModel();
#endif
    mSavedRelativePosition = 0;
    setShowSignatureDetails(false);
    mFindBar->closeBar();
    mTranslatorWidget->slotCloseWidget();
    mCreateTodo->slotCloseWidget();
    mCreateEvent->slotCloseWidget();
    mScamDetectionWarning->setVisible(false);
    mOpenAttachmentFolderWidget->setVisible(false);

    if (mPrinting) {
        if (MessageViewer::GlobalSettings::self()->respectExpandCollapseSettings()) {
            if (MessageViewer::GlobalSettings::self()->showExpandQuotesMark()) {
                mLevelQuote = MessageViewer::GlobalSettings::self()->collapseQuoteLevelSpin() - 1;
            } else {
                mLevelQuote = -1;
            }
        } else {
            mLevelQuote = -1;
        }
    }
}

void ViewerPrivate::setMessageInternal(const KMime::Message::Ptr message,
                                       Viewer::UpdateMode updateMode)
{
    if (mCreateNoteAction) {
        QString createNoteText;
        if (relatedNoteRelation().isValid()) {
            createNoteText = i18nc("edit a note on this message", "Edit Note");
        } else {
            createNoteText = i18nc("create a new note out of this message", "Create Note");
        }

        mCreateNoteAction->setText(createNoteText);
        mCreateNoteAction->setIconText(createNoteText);
    }

    mMessage = message;
    if (message) {
        mNodeHelper->setOverrideCodec(mMessage.data(), overrideCodec());
    }

#ifndef QT_NO_TREEVIEW
    mMimePartTree->setRoot(mNodeHelper->messageWithExtraContent(message.data()));
    update(updateMode);
#endif

}

void ViewerPrivate::setMessageItem(const Akonadi::Item &item, Viewer::UpdateMode updateMode)
{
    resetStateForNewMessage();
    foreach (const Akonadi::Entity::Id monitoredId, mMonitor.itemsMonitoredEx()) {
        mMonitor.setItemMonitored(Akonadi::Item(monitoredId), false);
    }
    Q_ASSERT(mMonitor.itemsMonitoredEx().isEmpty());

    mMessageItem = item;
    if (mMessageItem.isValid()) {
        mMonitor.setItemMonitored(mMessageItem, true);
    }

    if (!mMessageItem.hasPayload<KMime::Message::Ptr>()) {
        if (mMessageItem.isValid()) {
            qCWarning(MESSAGEVIEWER_LOG) << "Payload is not a MessagePtr!";
        }
        return;
    }

    setMessageInternal(mMessageItem.payload<KMime::Message::Ptr>(), updateMode);
}

void ViewerPrivate::setMessage(const KMime::Message::Ptr &aMsg, Viewer::UpdateMode updateMode)
{
    resetStateForNewMessage();

    Akonadi::Item item;
    item.setMimeType(KMime::Message::mimeType());
    item.setPayload(aMsg);
    mMessageItem = item;

    setMessageInternal(aMsg, updateMode);
}

void ViewerPrivate::setMessagePart(KMime::Content *node)
{
    // Cancel scheduled updates of the reader window, as that would stop the
    // timer of the HTML writer, which would make viewing attachment not work
    // anymore as not all HTML is written to the HTML part.
    // We're updating the reader window here ourselves anyway.
    mUpdateReaderWinTimer.stop();

    if (node) {
        mMessagePartNode = node;
        if (node->bodyIsMessage()) {
            mMainWindow->setWindowTitle(node->bodyAsMessage()->subject()->asUnicodeString());
        } else {
            QString windowTitle = NodeHelper::fileName(node);
            if (windowTitle.isEmpty()) {
                windowTitle = node->contentDescription()->asUnicodeString();
            }
            if (!windowTitle.isEmpty()) {
                mMainWindow->setWindowTitle(i18n("View Attachment: %1", windowTitle));
            }
        }

        htmlWriter()->begin(QString());
        htmlWriter()->queue(mCSSHelper->htmlHead(mUseFixedFont));

        parseContent(node);

        htmlWriter()->queue(QStringLiteral("</body></html>"));
        htmlWriter()->flush();
    }
}

void ViewerPrivate::showHideMimeTree()
{
#ifndef QT_NO_TREEVIEW
    if (mimePartTreeIsEmpty()) {
        mMimePartTree->hide();
        return;
    }
    bool showMimeTree = false;
    if (GlobalSettings::self()->mimeTreeMode() == GlobalSettings::EnumMimeTreeMode::Always) {
        mMimePartTree->show();
        showMimeTree = true;
    } else {
        // don't rely on QSplitter maintaining sizes for hidden widgets:
        saveSplitterSizes();
        mMimePartTree->hide();
        showMimeTree = false;
    }
    if (mToggleMimePartTreeAction && (mToggleMimePartTreeAction->isChecked() != showMimeTree)) {
        mToggleMimePartTreeAction->setChecked(showMimeTree);
    }
#endif
}

void ViewerPrivate::atmViewMsg(KMime::Message::Ptr message)
{
    Q_ASSERT(message);
    Q_EMIT showMessage(message, overrideEncoding());
}

void ViewerPrivate::adjustLayout()
{
#ifndef QT_NO_TREEVIEW
    const int mimeH = GlobalSettings::self()->mimePaneHeight();
    const int messageH = GlobalSettings::self()->messagePaneHeight();
    QList<int> splitterSizes;
    splitterSizes << messageH << mimeH;

    mSplitter->addWidget(mMimePartTree);
    mSplitter->setSizes(splitterSizes);

    if (GlobalSettings::self()->mimeTreeMode() == GlobalSettings::EnumMimeTreeMode::Always &&
            mMsgDisplay) {
        mMimePartTree->show();
    } else {
        mMimePartTree->hide();
    }
#endif

    if (GlobalSettings::self()->showColorBar() && mMsgDisplay) {
        mColorBar->show();
    } else {
        mColorBar->hide();
    }
}

void ViewerPrivate::saveSplitterSizes() const
{
#ifndef QT_NO_TREEVIEW
    if (!mSplitter || !mMimePartTree) {
        return;
    }
    if (mMimePartTree->isHidden()) {
        return;    // don't rely on QSplitter maintaining sizes for hidden widgets.
    }
    GlobalSettings::self()->setMimePaneHeight(mSplitter->sizes()[1]);
    GlobalSettings::self()->setMessagePaneHeight(mSplitter->sizes()[0]);
#endif
}

void ViewerPrivate::createWidgets()
{
    //TODO: Make a MDN bar similar to Mozillas password bar and show MDNs here as soon as a
    //      MDN enabled message is shown.
    QVBoxLayout *vlay = new QVBoxLayout(q);
    vlay->setMargin(0);
    mSplitter = new QSplitter(Qt::Vertical, q);
    connect(mSplitter, SIGNAL(splitterMoved(int,int)), this, SLOT(saveSplitterSizes()));
    mSplitter->setObjectName(QStringLiteral("mSplitter"));
    mSplitter->setChildrenCollapsible(false);
    vlay->addWidget(mSplitter);
#ifndef QT_NO_TREEVIEW
    mMimePartTree = new MimePartTreeView(mSplitter);
    connect(mMimePartTree, &QAbstractItemView::activated, this, &ViewerPrivate::slotMimePartSelected);
    connect(mMimePartTree, &QWidget::customContextMenuRequested, this, &ViewerPrivate::slotMimeTreeContextMenuRequested);
#endif

    mBox = new QWidget(mSplitter);
    QHBoxLayout *mBoxHBoxLayout = new QHBoxLayout(mBox);
    mBoxHBoxLayout->setMargin(0);

    mColorBar = new HtmlStatusBar(mBox);
    mBoxHBoxLayout->addWidget(mColorBar);
    QWidget *readerBox = new QWidget(mBox);
    QVBoxLayout *readerBoxVBoxLayout = new QVBoxLayout(readerBox);
    readerBoxVBoxLayout->setMargin(0);
    mBoxHBoxLayout->addWidget(readerBox);

    mColorBar->setObjectName(QStringLiteral("mColorBar"));
    mColorBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    mScamDetectionWarning = new ScamDetectionWarningWidget(readerBox);
    mScamDetectionWarning->setObjectName(QStringLiteral("scandetectionwarning"));
    readerBoxVBoxLayout->addWidget(mScamDetectionWarning);

    mOpenAttachmentFolderWidget = new OpenAttachmentFolderWidget(readerBox);
    mOpenAttachmentFolderWidget->setObjectName(QStringLiteral("openattachementfolderwidget"));
    readerBoxVBoxLayout->addWidget(mOpenAttachmentFolderWidget);

    mViewer = new MailWebView(mActionCollection, readerBox);
    readerBoxVBoxLayout->addWidget(mViewer);
    mViewer->setObjectName(QStringLiteral("mViewer"));

    mCreateTodo = new MessageViewer::TodoEdit(readerBox);
    readerBoxVBoxLayout->addWidget(mCreateTodo);
    connect(mCreateTodo, &TodoEdit::createTodo, this, &ViewerPrivate::slotCreateTodo);
    mCreateTodo->setObjectName(QStringLiteral("createtodowidget"));
    mCreateTodo->hide();

    mCreateEvent = new MessageViewer::EventEdit(readerBox);
    readerBoxVBoxLayout->addWidget(mCreateEvent);
    connect(mCreateEvent, &EventEdit::createEvent, this, &ViewerPrivate::slotCreateEvent);
    mCreateEvent->setObjectName(QStringLiteral("createeventwidget"));
    mCreateEvent->hide();

    mCreateNote = new MessageViewer::NoteEdit(readerBox);
    readerBoxVBoxLayout->addWidget(mCreateNote);
    connect(mCreateNote, &NoteEdit::createNote, this, &ViewerPrivate::slotCreateNote);
    mCreateNote->setObjectName(QStringLiteral("createnotewidget"));
    mCreateNote->hide();

    mSliderContainer = new PimCommon::SlideContainer(readerBox);
    mSliderContainer->setObjectName(QStringLiteral("slidercontainer"));
    readerBoxVBoxLayout->addWidget(mSliderContainer);
    mFindBar = new FindBarMailWebView(mViewer, q);
    connect(mFindBar, &FindBarBase::hideFindBar, mSliderContainer, &PimCommon::SlideContainer::slideOut);
    mSliderContainer->setContent(mFindBar);

    mTranslatorWidget = new PimCommon::TranslatorWidget(readerBox);
    mTranslatorWidget->setObjectName(QStringLiteral("translatorwidget"));
    readerBoxVBoxLayout->addWidget(mTranslatorWidget);
#ifndef QT_NO_TREEVIEW
    mSplitter->setStretchFactor(mSplitter->indexOf(mMimePartTree), 0);
#endif
}

void ViewerPrivate::createActions()
{
    KActionCollection *ac = mActionCollection;
    if (!ac) {
        return;
    }

    // header style
    KActionMenu *headerMenu = new KActionMenu(i18nc("View->", "&Headers"), this);
    ac->addAction(QStringLiteral("view_headers"), headerMenu);
    addHelpTextAction(headerMenu, i18n("Choose display style of message headers"));

    QActionGroup *group = new QActionGroup(this);
    KToggleAction *raction = new KToggleAction(i18nc("View->headers->", "&Enterprise Headers"), this);
    ac->addAction(QStringLiteral("view_headers_enterprise"), raction);
    connect(raction, &QAction::triggered, this, &ViewerPrivate::slotEnterpriseHeaders);
    addHelpTextAction(raction, i18n("Show the list of headers in Enterprise style"));
    group->addAction(raction);
    headerMenu->addAction(raction);

    raction  = new KToggleAction(i18nc("View->headers->", "&Fancy Headers"), this);
    ac->addAction(QStringLiteral("view_headers_fancy"), raction);
    connect(raction, &QAction::triggered, this, &ViewerPrivate::slotFancyHeaders);
    addHelpTextAction(raction, i18n("Show the list of headers in a fancy format"));
    group->addAction(raction);
    headerMenu->addAction(raction);

    raction  = new KToggleAction(i18nc("View->headers->", "&Brief Headers"), this);
    ac->addAction(QStringLiteral("view_headers_brief"), raction);
    connect(raction, &QAction::triggered, this, &ViewerPrivate::slotBriefHeaders);
    addHelpTextAction(raction, i18n("Show brief list of message headers"));
    group->addAction(raction);
    headerMenu->addAction(raction);

    raction  = new KToggleAction(i18nc("View->headers->", "&Standard Headers"), this);
    ac->addAction(QStringLiteral("view_headers_standard"), raction);
    connect(raction, &QAction::triggered, this, &ViewerPrivate::slotStandardHeaders);
    addHelpTextAction(raction, i18n("Show standard list of message headers"));
    group->addAction(raction);
    headerMenu->addAction(raction);

    raction  = new KToggleAction(i18nc("View->headers->", "&Long Headers"), this);
    ac->addAction(QStringLiteral("view_headers_long"), raction);
    connect(raction, &QAction::triggered, this, &ViewerPrivate::slotLongHeaders);
    addHelpTextAction(raction, i18n("Show long list of message headers"));
    group->addAction(raction);
    headerMenu->addAction(raction);

    raction  = new KToggleAction(i18nc("View->headers->", "&All Headers"), this);
    ac->addAction(QStringLiteral("view_headers_all"), raction);
    connect(raction, &QAction::triggered, this, &ViewerPrivate::slotAllHeaders);
    addHelpTextAction(raction, i18n("Show all message headers"));
    group->addAction(raction);
    headerMenu->addAction(raction);

    raction  = new KToggleAction(i18nc("View->headers->", "&Custom Headers"), this);
    ac->addAction(QStringLiteral("view_custom_headers"), raction);
    connect(raction, &QAction::triggered, this, &ViewerPrivate::slotCustomHeaders);
    addHelpTextAction(raction, i18n("Show custom headers"));
    group->addAction(raction);
    headerMenu->addAction(raction);
    //Same action group
    mThemeManager->setActionGroup(group);
    mThemeManager->setThemeMenu(headerMenu);

    // attachment style
    KActionMenu *attachmentMenu  = new KActionMenu(i18nc("View->", "&Attachments"), this);
    ac->addAction(QStringLiteral("view_attachments"), attachmentMenu);
    addHelpTextAction(attachmentMenu, i18n("Choose display style of attachments"));

    group = new QActionGroup(this);
    raction  = new KToggleAction(i18nc("View->attachments->", "&As Icons"), this);
    ac->addAction(QStringLiteral("view_attachments_as_icons"), raction);
    connect(raction, &QAction::triggered, this, &ViewerPrivate::slotIconicAttachments);
    addHelpTextAction(raction, i18n("Show all attachments as icons. Click to see them."));
    group->addAction(raction);
    attachmentMenu->addAction(raction);

    raction  = new KToggleAction(i18nc("View->attachments->", "&Smart"), this);
    ac->addAction(QStringLiteral("view_attachments_smart"), raction);
    connect(raction, &QAction::triggered, this, &ViewerPrivate::slotSmartAttachments);
    addHelpTextAction(raction, i18n("Show attachments as suggested by sender."));
    group->addAction(raction);
    attachmentMenu->addAction(raction);

    raction  = new KToggleAction(i18nc("View->attachments->", "&Inline"), this);
    ac->addAction(QStringLiteral("view_attachments_inline"), raction);
    connect(raction, &QAction::triggered, this, &ViewerPrivate::slotInlineAttachments);
    addHelpTextAction(raction, i18n("Show all attachments inline (if possible)"));
    group->addAction(raction);
    attachmentMenu->addAction(raction);

    raction  = new KToggleAction(i18nc("View->attachments->", "&Hide"), this);
    ac->addAction(QStringLiteral("view_attachments_hide"), raction);
    connect(raction, &QAction::triggered, this, &ViewerPrivate::slotHideAttachments);
    addHelpTextAction(raction, i18n("Do not show attachments in the message viewer"));
    group->addAction(raction);
    attachmentMenu->addAction(raction);

    mHeaderOnlyAttachmentsAction = new KToggleAction(i18nc("View->attachments->", "In Header Only"), this);
    ac->addAction(QStringLiteral("view_attachments_headeronly"), mHeaderOnlyAttachmentsAction);
    connect(mHeaderOnlyAttachmentsAction, &QAction::triggered,
            this, &ViewerPrivate::slotHeaderOnlyAttachments);
    addHelpTextAction(mHeaderOnlyAttachmentsAction, i18n("Show Attachments only in the header of the mail"));
    group->addAction(mHeaderOnlyAttachmentsAction);
    attachmentMenu->addAction(mHeaderOnlyAttachmentsAction);

    // Set Encoding submenu
    mSelectEncodingAction  = new KSelectAction(QIcon::fromTheme(QStringLiteral("character-set")), i18n("&Set Encoding"), this);
    mSelectEncodingAction->setToolBarMode(KSelectAction::MenuMode);
    ac->addAction(QStringLiteral("encoding"), mSelectEncodingAction);
    connect(mSelectEncodingAction, SIGNAL(triggered(int)),
            SLOT(slotSetEncoding()));
    QStringList encodings = NodeHelper::supportedEncodings(false);
    encodings.prepend(i18n("Auto"));
    mSelectEncodingAction->setItems(encodings);
    mSelectEncodingAction->setCurrentItem(0);

    //
    // Message Menu
    //

    // copy selected text to clipboard
    mCopyAction = ac->addAction(KStandardAction::Copy, QStringLiteral("kmail_copy"), this,
                                SLOT(slotCopySelectedText()));

    connect(mViewer, SIGNAL(selectionChanged()),
            this, SLOT(viewerSelectionChanged()));
    viewerSelectionChanged();

    // copy all text to clipboard
    mSelectAllAction  = new QAction(i18n("Select All Text"), this);
    ac->addAction(QStringLiteral("mark_all_text"), mSelectAllAction);
    connect(mSelectAllAction, SIGNAL(triggered(bool)), SLOT(selectAll()));
    ac->setDefaultShortcut(mSelectAllAction, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_A));

    // copy Email address to clipboard
    mCopyURLAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-copy")),
                                 i18n("Copy Link Address"), this);
    ac->addAction(QStringLiteral("copy_url"), mCopyURLAction);
    connect(mCopyURLAction, &QAction::triggered, this, &ViewerPrivate::slotUrlCopy);

    // open URL
    mUrlOpenAction = new QAction(QIcon::fromTheme(QStringLiteral("document-open")), i18n("Open URL"), this);
    ac->addAction(QStringLiteral("open_url"), mUrlOpenAction);
    connect(mUrlOpenAction, SIGNAL(triggered(bool)), this, SLOT(slotUrlOpen()));

    // use fixed font
    mToggleFixFontAction = new KToggleAction(i18n("Use Fi&xed Font"), this);
    ac->addAction(QStringLiteral("toggle_fixedfont"), mToggleFixFontAction);
    connect(mToggleFixFontAction, &QAction::triggered, this, &ViewerPrivate::slotToggleFixedFont);
    ac->setDefaultShortcut(mToggleFixFontAction, QKeySequence(Qt::Key_X));

    // Zoom actions
    mZoomTextOnlyAction = new KToggleAction(i18n("Zoom Text Only"), this);
    ac->addAction(QStringLiteral("toggle_zoomtextonly"), mZoomTextOnlyAction);
    connect(mZoomTextOnlyAction, &QAction::triggered, this, &ViewerPrivate::slotZoomTextOnly);
    mZoomInAction = new QAction(QIcon::fromTheme(QStringLiteral("zoom-in")), i18n("&Zoom In"), this);
    ac->addAction(QStringLiteral("zoom_in"), mZoomInAction);
    connect(mZoomInAction, &QAction::triggered, this, &ViewerPrivate::slotZoomIn);
    ac->setDefaultShortcut(mZoomInAction, QKeySequence(Qt::CTRL | Qt::Key_Plus));

    mZoomOutAction = new QAction(QIcon::fromTheme(QStringLiteral("zoom-out")), i18n("Zoom &Out"), this);
    ac->addAction(QStringLiteral("zoom_out"), mZoomOutAction);
    connect(mZoomOutAction, &QAction::triggered, this, &ViewerPrivate::slotZoomOut);
    ac->setDefaultShortcut(mZoomOutAction, QKeySequence(Qt::CTRL | Qt::Key_Minus));

    mZoomResetAction = new QAction(i18n("Reset"), this);
    ac->addAction(QStringLiteral("zoom_reset"), mZoomResetAction);
    connect(mZoomResetAction, &QAction::triggered, this, &ViewerPrivate::slotZoomReset);
    ac->setDefaultShortcut(mZoomResetAction, QKeySequence(Qt::CTRL | Qt::Key_0));

    // Show message structure viewer
    mToggleMimePartTreeAction = new KToggleAction(i18n("Show Message Structure"), this);
    ac->addAction(QStringLiteral("toggle_mimeparttree"), mToggleMimePartTreeAction);
    connect(mToggleMimePartTreeAction, &QAction::toggled,
            this, &ViewerPrivate::slotToggleMimePartTree);

    mViewSourceAction  = new QAction(i18n("&View Source"), this);
    ac->addAction(QStringLiteral("view_source"), mViewSourceAction);
    connect(mViewSourceAction, SIGNAL(triggered(bool)), SLOT(slotShowMessageSource()));
    ac->setDefaultShortcut(mViewSourceAction, QKeySequence(Qt::Key_V));

    mSaveMessageAction = new QAction(QIcon::fromTheme(QStringLiteral("document-save-as")), i18n("&Save message..."), this);
    ac->addAction(QStringLiteral("save_message"), mSaveMessageAction);
    connect(mSaveMessageAction, SIGNAL(triggered(bool)), SLOT(slotSaveMessage()));
    //Laurent: conflict with kmail shortcut
    //mSaveMessageAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));

    mSaveMessageDisplayFormat = new QAction(i18n("&Save Display Format"), this);
    ac->addAction(QStringLiteral("save_message_display_format"), mSaveMessageDisplayFormat);
    connect(mSaveMessageDisplayFormat, SIGNAL(triggered(bool)), SLOT(slotSaveMessageDisplayFormat()));

    mResetMessageDisplayFormat = new QAction(i18n("&Reset Display Format"), this);
    ac->addAction(QStringLiteral("reset_message_display_format"), mResetMessageDisplayFormat);
    connect(mResetMessageDisplayFormat, SIGNAL(triggered(bool)), SLOT(slotResetMessageDisplayFormat()));

    //
    // Scroll actions
    //
    mScrollUpAction = new QAction(i18n("Scroll Message Up"), this);
    ac->setDefaultShortcut(mScrollUpAction, QKeySequence(Qt::Key_Up));
    ac->addAction(QStringLiteral("scroll_up"), mScrollUpAction);
    connect(mScrollUpAction, &QAction::triggered,
            q, &Viewer::slotScrollUp);

    mScrollDownAction = new QAction(i18n("Scroll Message Down"), this);
    ac->setDefaultShortcut(mScrollDownAction, QKeySequence(Qt::Key_Down));
    ac->addAction(QStringLiteral("scroll_down"), mScrollDownAction);
    connect(mScrollDownAction, &QAction::triggered,
            q, &Viewer::slotScrollDown);

    mScrollUpMoreAction = new QAction(i18n("Scroll Message Up (More)"), this);
    ac->setDefaultShortcut(mScrollUpMoreAction, QKeySequence(Qt::Key_PageUp));
    ac->addAction(QStringLiteral("scroll_up_more"), mScrollUpMoreAction);
    connect(mScrollUpMoreAction, &QAction::triggered,
            q, &Viewer::slotScrollPrior);

    mScrollDownMoreAction = new QAction(i18n("Scroll Message Down (More)"), this);
    ac->setDefaultShortcut(mScrollDownMoreAction, QKeySequence(Qt::Key_PageDown));
    ac->addAction(QStringLiteral("scroll_down_more"), mScrollDownMoreAction);
    connect(mScrollDownMoreAction, &QAction::triggered,
            q, &Viewer::slotScrollNext);

    //
    // Actions not in menu
    //

    // Toggle HTML display mode.
    mToggleDisplayModeAction = new KToggleAction(i18n("Toggle HTML Display Mode"), this);
    ac->addAction(QStringLiteral("toggle_html_display_mode"), mToggleDisplayModeAction);
    ac->setDefaultShortcut(mToggleDisplayModeAction, QKeySequence(Qt::SHIFT + Qt::Key_H));
    connect(mToggleDisplayModeAction, &QAction::triggered,
            this, &ViewerPrivate::slotToggleHtmlMode);
    addHelpTextAction(mToggleDisplayModeAction, i18n("Toggle display mode between HTML and plain text"));

    // Load external reference
    QAction *loadExternalReferenceAction = new QAction(i18n("Load external references"), this);
    ac->addAction(QStringLiteral("load_external_reference"), loadExternalReferenceAction);
    ac->setDefaultShortcut(loadExternalReferenceAction, QKeySequence(Qt::SHIFT + Qt::CTRL + Qt::Key_R));
    connect(loadExternalReferenceAction, SIGNAL(triggered(bool)),
            SLOT(slotLoadExternalReference()));
    addHelpTextAction(loadExternalReferenceAction, i18n("Load external references from the Internet for this message."));

    mSpeakTextAction = new QAction(i18n("Speak Text"), this);
    mSpeakTextAction->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-text-to-speech")));
    ac->addAction(QStringLiteral("speak_text"), mSpeakTextAction);
    connect(mSpeakTextAction, &QAction::triggered,
            this, &ViewerPrivate::slotSpeakText);

    mCopyImageLocation = new QAction(i18n("Copy Image Location"), this);
    mCopyImageLocation->setIcon(QIcon::fromTheme(QStringLiteral("view-media-visualization")));
    ac->addAction(QStringLiteral("copy_image_location"), mCopyImageLocation);
    ac->setShortcutsConfigurable(mCopyImageLocation, false);
    connect(mCopyImageLocation, &QAction::triggered,
            this, &ViewerPrivate::slotCopyImageLocation);

    mTranslateAction = new QAction(i18n("Translate..."), this);
    ac->setDefaultShortcut(mTranslateAction, QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_T));
    mTranslateAction->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-locale")));
    ac->addAction(QStringLiteral("translate_text"), mTranslateAction);
    connect(mTranslateAction, &QAction::triggered,
            this, &ViewerPrivate::slotTranslate);

    mFindInMessageAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-find")), i18n("&Find in Message..."), this);
    ac->addAction(QStringLiteral("find_in_messages"), mFindInMessageAction);
    connect(mFindInMessageAction, &QAction::triggered, this, &ViewerPrivate::slotFind);
    ac->setDefaultShortcut(mFindInMessageAction, KStandardShortcut::find().first());

#ifndef KDEPIM_NO_WEBKIT
    mCaretBrowsing = new KToggleAction(i18n("Toggle Caret Browsing"), this);
    ac->setDefaultShortcut(mCaretBrowsing, Qt::Key_F7);
    ac->addAction(QStringLiteral("toggle_caret_browsing"), mCaretBrowsing);
    connect(mCaretBrowsing, &QAction::triggered, this, &ViewerPrivate::slotToggleCaretBrowsing);
    mCaretBrowsing->setChecked(false);
#endif
    mBlockImage = new QAction(i18n("Block image"), this);
    ac->addAction(QStringLiteral("adblock_image"), mBlockImage);
    ac->setShortcutsConfigurable(mBlockImage, false);
    connect(mBlockImage, &QAction::triggered, this, &ViewerPrivate::slotBlockImage);

    mBlockableItems = new QAction(i18n("Open Blockable Items..."), this);
    ac->addAction(QStringLiteral("adblock_blockable_items"), mBlockableItems);
    connect(mBlockableItems, &QAction::triggered, this, &ViewerPrivate::slotOpenBlockableItems);

    mExpandUrlAction = new QAction(i18n("Expand Short URL"), this);
    ac->addAction(QStringLiteral("expand_short_url"), mExpandUrlAction);
    ac->setShortcutsConfigurable(mExpandUrlAction, false);
    connect(mExpandUrlAction, &QAction::triggered, this, &ViewerPrivate::slotExpandShortUrl);

    mCreateTodoAction = new QAction(QIcon::fromTheme(QStringLiteral("task-new")), i18n("Create Todo"), this);
    mCreateTodoAction->setIconText(i18n("Create To-do"));
    addHelpTextAction(mCreateTodoAction, i18n("Allows you to create a calendar to-do or reminder from this message"));
    mCreateTodoAction->setWhatsThis(i18n("This option starts the KOrganizer to-do editor with initial values taken from the currently selected message. Then you can edit the to-do to your liking before saving it to your calendar."));
    ac->addAction(QStringLiteral("create_todo"), mCreateTodoAction);
    ac->setDefaultShortcut(mCreateTodoAction, QKeySequence(Qt::CTRL + Qt::Key_T));
    connect(mCreateTodoAction, &QAction::triggered, this, &ViewerPrivate::slotShowCreateTodoWidget);

    mCreateNoteAction = new QAction(QIcon::fromTheme(QStringLiteral("view-pim-notes")), i18nc("create a new note out of this message", "Create Note"), this);
    mCreateNoteAction->setIconText(i18nc("create a new note out of this message", "Create Note"));
    addHelpTextAction(mCreateNoteAction, i18n("Allows you to create a note from this message"));
    mCreateNoteAction->setWhatsThis(i18n("This option starts an editor to create a note. Then you can edit the note to your liking before saving it."));
    ac->addAction(QStringLiteral("create_note"), mCreateNoteAction);
    connect(mCreateNoteAction, &QAction::triggered, this, &ViewerPrivate::slotShowCreateNoteWidget);

    mCreateEventAction = new QAction(QIcon::fromTheme(QStringLiteral("appointment-new")), i18n("Create Event..."), this);
    mCreateEventAction->setIconText(i18n("Create Event"));
    addHelpTextAction(mCreateEventAction, i18n("Allows you to create a calendar Event"));
    ac->addAction(QStringLiteral("create_event"), mCreateEventAction);
    ac->setDefaultShortcut(mCreateEventAction, QKeySequence(Qt::CTRL + Qt::Key_E));
    connect(mCreateEventAction, &QAction::triggered, this, &ViewerPrivate::slotShowCreateEventWidget);

    mShareServiceUrlMenu = mShareServiceManager->menu();
    ac->addAction(QStringLiteral("shareservice_menu"), mShareServiceUrlMenu);
    connect(mShareServiceManager, &PimCommon::ShareServiceUrlManager::serviceUrlSelected, this, &ViewerPrivate::slotServiceUrlSelected);
}

void ViewerPrivate::showContextMenu(KMime::Content *content, const QPoint &pos)
{
#ifndef QT_NO_TREEVIEW
    if (!content) {
        return;
    }

    if (content->contentType(false)) {
        if (content->contentType()->mimeType() == "text/x-moz-deleted") {
            return;
        }
    }
    const bool isAttachment = !content->contentType()->isMultipart() && !content->isTopLevel();
    const bool isRoot = (content == mMessage.data());
    const auto hasAttachments = KMime::hasAttachment(mMessage.data());

    QMenu popup;

    if (!isRoot) {
        popup.addAction(QIcon::fromTheme(QStringLiteral("document-save-as")), i18n("Save &As..."),
                        this, SLOT(slotAttachmentSaveAs()));

        if (isAttachment) {
            popup.addAction(QIcon::fromTheme(QStringLiteral("document-open")), i18nc("to open", "Open"),
                            this, SLOT(slotAttachmentOpen()));

            if (selectedContents().count() == 1) {
                createOpenWithMenu(&popup, QLatin1String(content->contentType()->mimeType()), false);
            } else {
                popup.addAction(i18n("Open With..."), this, SLOT(slotAttachmentOpenWith()));
            }
            popup.addAction(i18nc("to view something", "View"), this, SLOT(slotAttachmentView()));
        }
    }

    if (hasAttachments) {
        popup.addAction(i18n("Save All Attachments..."), this,
                        SLOT(slotAttachmentSaveAll()));
    }

    // edit + delete only for attachments
    if (!isRoot) {
        if (isAttachment) {
            popup.addAction(QIcon::fromTheme(QStringLiteral("edit-copy")), i18n("Copy"),
                            this, SLOT(slotAttachmentCopy()));
#if 0  //FIXME Laurent Comment for the moment it crash see Bug 287177
            popup.addAction(QIcon::fromTheme("edit-delete"), i18n("Delete Attachment"),
                            this, SLOT(slotAttachmentDelete()));
#endif
            if (GlobalSettings::self()->allowAttachmentEditing())
                popup.addAction(QIcon::fromTheme(QStringLiteral("document-properties")), i18n("Edit Attachment"),
                                this, SLOT(slotAttachmentEdit()));
        }

        if (!content->isTopLevel()) {
            popup.addAction(i18n("Properties"), this, SLOT(slotAttachmentProperties()));
        }
    }
    popup.exec(mMimePartTree->viewport()->mapToGlobal(pos));
#endif

}

KToggleAction *ViewerPrivate::actionForHeaderStyle(const HeaderStyle *style, const HeaderStrategy *strategy)
{
    if (!mActionCollection) {
        return 0;
    }
    QString actionName;
    if (style == HeaderStyle::enterprise()) {
        actionName = QStringLiteral("view_headers_enterprise");
    } else if (style == HeaderStyle::fancy()) {
        actionName = QStringLiteral("view_headers_fancy");
    } else if (style == HeaderStyle::brief()) {
        actionName = QStringLiteral("view_headers_brief");
    } else if (style == HeaderStyle::plain()) {
        if (strategy == HeaderStrategy::standard()) {
            actionName = QStringLiteral("view_headers_standard");
        } else if (strategy == HeaderStrategy::rich()) {
            actionName = QStringLiteral("view_headers_long");
        } else if (strategy == HeaderStrategy::all()) {
            actionName = QStringLiteral("view_headers_all");
        }
    } else if (style == HeaderStyle::custom()) {
        actionName = QStringLiteral("view_custom_headers");
    } else if (style == HeaderStyle::grantlee()) {
        return mThemeManager->actionForTheme();
    }

    if (actionName.isEmpty()) {
        return 0;
    } else {
        return static_cast<KToggleAction *>(mActionCollection->action(actionName));
    }
}

KToggleAction *ViewerPrivate::actionForAttachmentStrategy(const AttachmentStrategy *as)
{
    if (!mActionCollection) {
        return 0;
    }
    QString actionName ;
    if (as == AttachmentStrategy::iconic()) {
        actionName = QStringLiteral("view_attachments_as_icons");
    } else if (as == AttachmentStrategy::smart()) {
        actionName = QStringLiteral("view_attachments_smart");
    } else if (as == AttachmentStrategy::inlined()) {
        actionName = QStringLiteral("view_attachments_inline");
    } else if (as == AttachmentStrategy::hidden()) {
        actionName = QStringLiteral("view_attachments_hide");
    } else if (as == AttachmentStrategy::headerOnly()) {
        actionName = QStringLiteral("view_attachments_headeronly");
    }

    if (actionName.isEmpty()) {
        return 0;
    } else {
        return static_cast<KToggleAction *>(mActionCollection->action(actionName));
    }
}

void ViewerPrivate::readGlobalOverrideCodec()
{
    // if the global character encoding wasn't changed then there's nothing to do
    if (MessageCore::GlobalSettings::self()->overrideCharacterEncoding() == mOldGlobalOverrideEncoding) {
        return;
    }

    setOverrideEncoding(MessageCore::GlobalSettings::self()->overrideCharacterEncoding());
    mOldGlobalOverrideEncoding = MessageCore::GlobalSettings::self()->overrideCharacterEncoding();
}

const QTextCodec *ViewerPrivate::overrideCodec() const
{
    if (mOverrideEncoding.isEmpty() || mOverrideEncoding == QLatin1String("Auto")) { // Auto
        return 0;
    } else {
        return ViewerPrivate::codecForName(mOverrideEncoding.toLatin1());
    }
}

static QColor nextColor(const QColor &c)
{
    int h, s, v;
    c.getHsv(&h, &s, &v);
    return QColor::fromHsv((h + 50) % 360, qMax(s, 64), v);
}

QString ViewerPrivate::renderAttachments(KMime::Content *node, const QColor &bgColor) const
{

    if (!node) {
        return QString();
    }

    QString html;
    KMime::Content *child = MessageCore::NodeHelper::firstChild(node);

    if (child) {
        QString subHtml = renderAttachments(child, nextColor(bgColor));
        if (!subHtml.isEmpty()) {

            QString visibility;
            if (!mShowAttachmentQuicklist) {
                visibility.append(QLatin1String("display:none;"));
            }

            QString margin;
            if (node != mMessage.data() || headerStyle() != HeaderStyle::enterprise()) {
                margin = QStringLiteral("padding:2px; margin:2px; ");
            }
            QString align = QStringLiteral("left");
            if (headerStyle() == HeaderStyle::enterprise()) {
                align = QStringLiteral("right");
            }
            const bool result = (node->contentType()->mediaType().toLower() == "message" || node->contentType()->mediaType().toLower() == "multipart" || node == mMessage.data());
            if (result)
                html += QStringLiteral("<div style=\"background:%1; %2"
                                       "vertical-align:middle; float:%3; %4\">").arg(bgColor.name()).arg(margin)
                        .arg(align).arg(visibility);
            html += subHtml;
            if (result) {
                html += QLatin1String("</div>");
            }
        }
    } else {
        NodeHelper::AttachmentDisplayInfo info = NodeHelper::attachmentDisplayInfo(node);
        if (info.displayInHeader) {
            html += QLatin1String("<div style=\"float:left;\">");
            html += QStringLiteral("<span style=\"white-space:nowrap; border-width: 0px; border-left-width: 5px; border-color: %1; 2px; border-left-style: solid;\">").arg(bgColor.name());
            mNodeHelper->writeNodeToTempFile(node);
            const QString href = mNodeHelper->asHREF(node, QStringLiteral("header"));
            html += QLatin1String("<a href=\"") + href +
                    QLatin1String("\">");
            QString imageMaxSize;
            if (!info.icon.isEmpty()) {
                QImage tmpImg(info.icon);
                if (tmpImg.width() > 48 || tmpImg.height() > 48) {
                    imageMaxSize = QStringLiteral("width=\"48\" height=\"48\"");
                }
            }
            html += QStringLiteral("<img %1 style=\"vertical-align:middle;\" src=\"").arg(imageMaxSize) + info.icon + QLatin1String("\"/>&nbsp;");
            if (headerStyle() == HeaderStyle::enterprise()) {
                QFont bodyFont = mCSSHelper->bodyFont(mUseFixedFont);
                QFontMetrics fm(bodyFont);
                html += fm.elidedText(info.label, Qt::ElideRight, 180);
            } else if (headerStyle() == HeaderStyle::fancy()) {
                QFont bodyFont = mCSSHelper->bodyFont(mUseFixedFont);
                QFontMetrics fm(bodyFont);
                html += fm.elidedText(info.label, Qt::ElideRight, 1000);
            } else {
                html += info.label;
            }
            html += QLatin1String("</a></span></div> ");
        }
    }

    KMime::Content *next  = MessageCore::NodeHelper::nextSibling(node);
    if (next) {
        html += renderAttachments(next, nextColor(bgColor));
    }

    return html;
}

KMime::Content *ViewerPrivate::findContentByType(KMime::Content *content, const QByteArray &type)
{
    auto list = content->contents();
    Q_FOREACH (KMime::Content *c, list) {
        if (c->contentType()->mimeType() ==  type) {
            return c;
        }
    }
    return 0;

}

//-----------------------------------------------------------------------------
const QTextCodec *ViewerPrivate::codecForName(const QByteArray &_str)
{
    if (_str.isEmpty()) {
        return 0;
    }
    QByteArray codec = _str.toLower();
    return KCharsets::charsets()->codecForName(QLatin1String(codec));
}

void ViewerPrivate::update(MessageViewer::Viewer::UpdateMode updateMode)
{
    // Avoid flicker, somewhat of a cludge
    if (updateMode == Viewer::Force) {
        // stop the timer to avoid calling updateReaderWin twice
        mUpdateReaderWinTimer.stop();
        saveRelativePosition();
        updateReaderWin();
    } else if (mUpdateReaderWinTimer.isActive()) {
        mUpdateReaderWinTimer.setInterval(delay);
    } else {
        mUpdateReaderWinTimer.start(0);
    }
}

void ViewerPrivate::slotUrlOpen(const QUrl &url)
{
    if (!url.isEmpty()) {
        mClickedUrl = url;
    }

    // First, let's see if the URL handler manager can handle the URL. If not, try KRun for some
    // known URLs, otherwise fallback to emitting a signal.
    // That signal is caught by KMail, and in case of mailto URLs, a composer is shown.

    if (URLHandlerManager::instance()->handleClick(mClickedUrl, this)) {
        return;
    }

    Q_EMIT urlClicked(mMessageItem, mClickedUrl);
}

void ViewerPrivate::slotUrlOn(const QString &link, const QString &title, const QString &textContent)
{
    Q_UNUSED(title)
    Q_UNUSED(textContent)

    // The "link" we get here is not URL-encoded, and therefore there is no way QUrl could
    // parse it correctly. To workaround that, we use QWebFrame::hitTestContent() on the mouse position
    // to get the URL before WebKit managed to mangle it.
    QUrl url(mViewer->linkOrImageUrlAt(QCursor::pos()));
    const QString protocol = url.scheme();
    if (protocol == QLatin1String("kmail") ||
            protocol == QLatin1String("x-kmail") ||
            protocol == QLatin1String("attachment") ||
            (protocol.isEmpty() && url.path().isEmpty())) {
        mViewer->setAcceptDrops(false);
    } else {
        mViewer->setAcceptDrops(true);
    }

    if (link.trimmed().isEmpty()) {
        KPIM::BroadcastStatus::instance()->reset();
        Q_EMIT showStatusBarMessage(QString());
        return;
    }

    QString msg = URLHandlerManager::instance()->statusBarMessage(url, this);
    if (msg.isEmpty()) {
        msg = link;
    }

    KPIM::BroadcastStatus::instance()->setTransientStatusMsg(msg);
    Q_EMIT showStatusBarMessage(msg);
}

void ViewerPrivate::slotUrlPopup(const QUrl &aUrl, const QUrl &imageUrl, const QPoint &aPos)
{
    if (!mMsgDisplay) {
        return;
    }
    mClickedUrl = aUrl;
    mImageUrl = imageUrl;

    if (URLHandlerManager::instance()->handleContextMenuRequest(aUrl, aPos, this)) {
        return;
    }

    if (!mActionCollection) {
        return;
    }

    if (aUrl.scheme() == QLatin1String("mailto")) {
        mCopyURLAction->setText(i18n("Copy Email Address"));
    } else {
        mCopyURLAction->setText(i18n("Copy Link Address"));
    }

    Q_EMIT popupMenu(mMessageItem, aUrl, imageUrl, aPos);
}

void ViewerPrivate::slotLoadExternalReference()
{
    if (mColorBar->isNormal() || htmlLoadExtOverride()) {
        return;
    }
    setHtmlLoadExtOverride(true);
    update(Viewer::Force);
}

void ViewerPrivate::slotToggleHtmlMode()
{
    if (mColorBar->isNormal()) {
        return;
    }
    mScamDetectionWarning->setVisible(false);
    const bool useHtml  = !htmlMail();
    setDisplayFormatMessageOverwrite(useHtml ? MessageViewer::Viewer::Html : MessageViewer::Viewer::Text);
    update(Viewer::Force);
}

void ViewerPrivate::slotFind()
{
#ifndef KDEPIM_NO_WEBKIT
    if (mViewer->hasSelection()) {
        mFindBar->setText(mViewer->selectedText());
    }
#endif
    mSliderContainer->slideIn();
    mFindBar->focusAndSetCursor();
}

void ViewerPrivate::slotTranslate()
{
    const QString text = mViewer->selectedText();
    mTranslatorWidget->show();
    if (!text.isEmpty()) {
        mTranslatorWidget->setTextToTranslate(text);
    }
}

void ViewerPrivate::slotToggleFixedFont()
{
    mUseFixedFont = !mUseFixedFont;
    update(Viewer::Force);
}

void ViewerPrivate::slotToggleMimePartTree()
{
    if (mToggleMimePartTreeAction->isChecked()) {
        GlobalSettings::self()->setMimeTreeMode(GlobalSettings::EnumMimeTreeMode::Always);
    } else {
        GlobalSettings::self()->setMimeTreeMode(GlobalSettings::EnumMimeTreeMode::Never);
    }
    showHideMimeTree();
}

void ViewerPrivate::slotShowMessageSource()
{
    if (!mMessage) {
        return;
    }
    mNodeHelper->messageWithExtraContent(mMessage.data());
    const QString rawMessage = QString::fromLatin1(mMessage->encodedContent());
    const QString htmlSource = mViewer->htmlSource();

    MailSourceViewer *viewer = new MailSourceViewer(); // deletes itself upon close
    viewer->setWindowTitle(i18n("Message as Plain Text"));
    viewer->setRawSource(rawMessage);
    viewer->setDisplayedSource(htmlSource);
    if (mUseFixedFont) {
        viewer->setFixedFont();
    }

    // Well, there is no widget to be seen here, so we have to use QCursor::pos()
    // Update: (GS) I'm not going to make this code behave according to Xinerama
    //         configuration because this is quite the hack.
    if (QApplication::desktop()->isVirtualDesktop()) {
#ifndef QT_NO_CURSOR
        int scnum = QApplication::desktop()->screenNumber(QCursor::pos());
#else
        int scnum = 0;
#endif
        viewer->resize(QApplication::desktop()->screenGeometry(scnum).width() / 2,
                       2 * QApplication::desktop()->screenGeometry(scnum).height() / 3);
    } else {
        viewer->resize(QApplication::desktop()->geometry().width() / 2,
                       2 * QApplication::desktop()->geometry().height() / 3);
    }
    viewer->show();
}

void ViewerPrivate::updateReaderWin()
{
    if (!mMsgDisplay) {
        return;
    }

    if (mRecursionCountForDisplayMessage + 1 > 1) {
        // This recursion here can happen because the ObjectTreeParser in parseMsg() can exec() an
        // eventloop.
        // This happens in two cases:
        //   1) The ContactSearchJob started by FancyHeaderStyle::format
        //   2) Various modal passphrase dialogs for decryption of a message (bug 96498)
        //
        // While the exec() eventloop is running, it is possible that a timer calls updateReaderWin(),
        // and not aborting here would confuse the state terribly.
        qCWarning(MESSAGEVIEWER_LOG) << "Danger, recursion while displaying a message!";
        return;
    }
    mRecursionCountForDisplayMessage++;

    mViewer->setAllowExternalContent(htmlLoadExternal());

    htmlWriter()->reset();
    //TODO: if the item doesn't have the payload fetched, try to fetch it? Maybe not here, but in setMessageItem.
    if (mMessage) {
        if (GlobalSettings::self()->showColorBar()) {
            mColorBar->show();
        } else {
            mColorBar->hide();
        }
        displayMessage();
    } else if (mMessagePartNode) {
        setMessagePart(mMessagePartNode);
    } else {
        mColorBar->hide();
#ifndef QT_NO_TREEVIEW
        mMimePartTree->hide();
#endif
        htmlWriter()->begin(QString());
        htmlWriter()->write(mCSSHelper->htmlHead(mUseFixedFont) + QLatin1String("</body></html>"));
        htmlWriter()->end();
    }

    if (mSavedRelativePosition) {
        mViewer->scrollToRelativePosition(mSavedRelativePosition);
        mSavedRelativePosition = 0;
    }
    mRecursionCountForDisplayMessage--;
}

void ViewerPrivate::slotMimePartSelected(const QModelIndex &index)
{
#ifndef QT_NO_TREEVIEW
    KMime::Content *content = static_cast<KMime::Content *>(index.internalPointer());
    if (!mMimePartTree->mimePartModel()->parent(index).isValid() && index.row() == 0) {
        update(Viewer::Force);
    } else {
        setMessagePart(content);
    }
#endif
}

void ViewerPrivate::slotBriefHeaders()
{
    setHeaderStyleAndStrategy(HeaderStyle::brief(),
                              HeaderStrategy::brief(), true);
}

void ViewerPrivate::slotFancyHeaders()
{

    setHeaderStyleAndStrategy(HeaderStyle::fancy(),
                              HeaderStrategy::rich(), true);
}

void ViewerPrivate::slotEnterpriseHeaders()
{
    setHeaderStyleAndStrategy(HeaderStyle::enterprise(),
                              HeaderStrategy::rich(), true);
}

void ViewerPrivate::slotStandardHeaders()
{
    setHeaderStyleAndStrategy(HeaderStyle::plain(),
                              HeaderStrategy::standard(), true);
}

void ViewerPrivate::slotLongHeaders()
{
    setHeaderStyleAndStrategy(HeaderStyle::plain(),
                              HeaderStrategy::rich(), true);
}

void ViewerPrivate::slotAllHeaders()
{
    setHeaderStyleAndStrategy(HeaderStyle::plain(),
                              HeaderStrategy::all(), true);
}

void ViewerPrivate::slotCustomHeaders()
{
    setHeaderStyleAndStrategy(HeaderStyle::custom(),
                              HeaderStrategy::custom(), true);
}

void ViewerPrivate::slotGrantleeHeaders()
{
    setHeaderStyleAndStrategy(HeaderStyle::grantlee(),
                              HeaderStrategy::grantlee(), true);
    initGrantleeThemeName();
    update(Viewer::Force);
}

void ViewerPrivate::initGrantleeThemeName()
{
    const QString themeName = GrantleeTheme::GrantleeSettings::self()->grantleeMailThemeName();
    headerStyle()->setTheme(mThemeManager->theme(themeName));
}

void ViewerPrivate::slotIconicAttachments()
{
    setAttachmentStrategy(AttachmentStrategy::iconic());
}

void ViewerPrivate::slotSmartAttachments()
{
    setAttachmentStrategy(AttachmentStrategy::smart());
}

void ViewerPrivate::slotInlineAttachments()
{
    setAttachmentStrategy(AttachmentStrategy::inlined());
}

void ViewerPrivate::slotHideAttachments()
{
    setAttachmentStrategy(AttachmentStrategy::hidden());
}

void ViewerPrivate::slotHeaderOnlyAttachments()
{
    setAttachmentStrategy(AttachmentStrategy::headerOnly());
}

void ViewerPrivate::attachmentView(KMime::Content *atmNode)
{
    if (atmNode) {

        const bool isEncapsulatedMessage = atmNode->parent() && atmNode->parent()->bodyIsMessage();
        if (isEncapsulatedMessage) {
            atmViewMsg(atmNode->parent()->bodyAsMessage());
        } else if ((qstricmp(atmNode->contentType()->mediaType(), "text") == 0) &&
                   ((qstricmp(atmNode->contentType()->subType(), "x-vcard") == 0) ||
                    (qstricmp(atmNode->contentType()->subType(), "directory") == 0))) {
            setMessagePart(atmNode);
        } else {
            Q_EMIT showReader(atmNode, htmlMail(), overrideEncoding());
        }
    }
}

void ViewerPrivate::slotDelayedResize()
{
    mSplitter->setGeometry(0, 0, q->width(), q->height());
}

void ViewerPrivate::slotPrintPreview()
{
    disconnect(mPartHtmlWriter, SIGNAL(finished()), this, SLOT(slotPrintPreview()));
    if (!mMessage) {
        return;
    }
    QPrintPreviewDialog previewdlg(mViewer);
    connect(&previewdlg, &QPrintPreviewDialog::paintRequested, this, [this](QPrinter * printer) {
        mViewer->print(printer);
    });
    previewdlg.exec();
}

void ViewerPrivate::slotPrintMsg()
{
    disconnect(mPartHtmlWriter, SIGNAL(finished()), this, SLOT(slotPrintMsg()));

    if (!mMessage) {
        return;
    }
    QPrinter printer;

    AutoQPointer<QPrintDialog> dlg(new QPrintDialog(&printer));

    if (dlg && dlg->exec() == QDialog::Accepted) {
        mViewer->print(&printer);
    }
}

void ViewerPrivate::slotSetEncoding()
{
    if (mSelectEncodingAction->currentItem() == 0) { // Auto
        mOverrideEncoding.clear();
    } else {
        mOverrideEncoding = NodeHelper::encodingForName(mSelectEncodingAction->currentText());
    }
    update(Viewer::Force);
}

QString ViewerPrivate::picsPath()
{
    if (mPicsPath.isEmpty()) {
        mPicsPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("/libmessageviewer/pics/"), QStandardPaths::LocateDirectory);
    }
    return mPicsPath;
}

QString ViewerPrivate::attachmentInjectionHtml()
{
    QString imgpath(picsPath());
    QString urlHandle;
    QString imgSrc;
    if (!mShowAttachmentQuicklist) {
        urlHandle.append(QStringLiteral("kmail:showAttachmentQuicklist"));
        imgSrc.append(QStringLiteral("quicklistClosed.png"));
    } else {
        urlHandle.append(QStringLiteral("kmail:hideAttachmentQuicklist"));
        imgSrc.append(QStringLiteral("quicklistOpened.png"));
    }

    QColor background = KColorScheme(QPalette::Active, KColorScheme::View).background().color();
    QString html = renderAttachments(mMessage.data(), background);
    Q_FOREACH (KMime::Content *node, mNodeHelper->extraContents(mMessage.data())) {
        html += renderAttachments(node, background);
    }
    if (html.isEmpty()) {
        return QString();
    }

    QString link;
    if (headerStyle() == HeaderStyle::fancy()) {
        link += QLatin1String("<div style=\"text-align: left;\"><a href=\"") + urlHandle + QLatin1String("\"><img src=\"file:///") + imgpath + imgSrc + QLatin1String("\"/></a></div>");
        html.prepend(link);
        html.prepend(QStringLiteral("<div style=\"float:left;\">%1&nbsp;</div>").arg(i18n("Attachments:")));
    } else {
        link += QLatin1String("<div style=\"text-align: right;\"><a href=\"") + urlHandle + QLatin1String("\"><img src=\"file:///") + imgpath + imgSrc + QLatin1String("\"/></a></div>");
        html.prepend(link);
    }
    return html;
}

void ViewerPrivate::injectAttachments()
{
    disconnect(mPartHtmlWriter, SIGNAL(finished()), this, SLOT(injectAttachments()));
    // inject attachments in header view
    // we have to do that after the otp has run so we also see encrypted parts

    mViewer->injectAttachments(bind(&ViewerPrivate::attachmentInjectionHtml, this));
}

void ViewerPrivate::slotSettingsChanged()
{
    update(Viewer::Force);
}

void ViewerPrivate::slotMimeTreeContextMenuRequested(const QPoint &pos)
{
#ifndef QT_NO_TREEVIEW
    QModelIndex index = mMimePartTree->indexAt(pos);
    if (index.isValid()) {
        KMime::Content *content = static_cast<KMime::Content *>(index.internalPointer());
        showContextMenu(content, pos);
    }
#endif
}

void ViewerPrivate::slotAttachmentOpenWith()
{
#ifndef QT_NO_TREEVIEW
    QItemSelectionModel *selectionModel = mMimePartTree->selectionModel();
    QModelIndexList selectedRows = selectionModel->selectedRows();

    Q_FOREACH (const QModelIndex &index, selectedRows) {
        KMime::Content *content = static_cast<KMime::Content *>(index.internalPointer());
        attachmentOpenWith(content);
    }
#endif
}

void ViewerPrivate::slotAttachmentOpen()
{
#ifndef QT_NO_TREEVIEW
    QItemSelectionModel *selectionModel = mMimePartTree->selectionModel();
    QModelIndexList selectedRows = selectionModel->selectedRows();

    Q_FOREACH (const QModelIndex &index, selectedRows) {
        KMime::Content *content = static_cast<KMime::Content *>(index.internalPointer());
        attachmentOpen(content);
    }
#endif
}

void ViewerPrivate::showOpenAttachmentFolderWidget(const QUrl &url)
{
    mOpenAttachmentFolderWidget->setFolder(url);
    mOpenAttachmentFolderWidget->slotShowWarning();
}

bool ViewerPrivate::mimePartTreeIsEmpty() const
{
#ifndef QT_NO_TREEVIEW
    return (mMimePartTree->model()->rowCount() == 0);
#else
    return false;
#endif
}

void ViewerPrivate::slotAttachmentSaveAs()
{
    const auto contents = selectedContents();
    QUrl currentUrl;
    if (Util::saveAttachments(contents, mMainWindow, currentUrl)) {
        showOpenAttachmentFolderWidget(currentUrl);
    }
}

void ViewerPrivate::slotAttachmentSaveAll()
{
    const auto contents = mMessage->attachments();
    QUrl currentUrl;
    if (Util::saveAttachments(contents, mMainWindow, currentUrl)) {
        showOpenAttachmentFolderWidget(currentUrl);
    }
}

void ViewerPrivate::slotAttachmentView()
{
    auto contents = selectedContents();

    Q_FOREACH (KMime::Content *content, contents) {
        attachmentView(content);
    }

}

void ViewerPrivate::slotAttachmentProperties()
{
    auto contents = selectedContents();

    if (contents.isEmpty()) {
        return;
    }

    Q_FOREACH (KMime::Content *content, contents) {
        attachmentProperties(content);
    }
}

void ViewerPrivate::attachmentProperties(KMime::Content *content)
{
    MessageCore::AttachmentPropertiesDialog *dialog = new MessageCore::AttachmentPropertiesDialog(content, mMainWindow);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void ViewerPrivate::slotAttachmentCopy()
{
#ifndef QT_NO_CLIPBOARD
    attachmentCopy(selectedContents());
#endif
}

void ViewerPrivate::attachmentCopy(const KMime::Content::List &contents)
{
#ifndef QT_NO_CLIPBOARD
    if (contents.isEmpty()) {
        return;
    }

    QList<QUrl> urls;
    Q_FOREACH (KMime::Content *content, contents) {
        auto url = QUrl::fromLocalFile(mNodeHelper->writeNodeToTempFile(content));
        if (!url.isValid()) {
            continue;
        }
        urls.append(url);
    }

    if (urls.isEmpty()) {
        return;
    }

    QMimeData *mimeData = new QMimeData;
    mimeData->setUrls(urls);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
#endif
}

void ViewerPrivate::slotAttachmentDelete()
{
    auto contents = selectedContents();
    if (contents.isEmpty()) {
        return;
    }

    bool showWarning = true;
    Q_FOREACH (KMime::Content *content, contents) {
        if (!deleteAttachment(content, showWarning)) {
            return;
        }
        showWarning = false;
    }
    update();
}

void ViewerPrivate::slotAttachmentEdit()
{
    auto contents = selectedContents();
    if (contents.isEmpty()) {
        return;
    }

    MessageViewer::AttachmentEditJob *job = new MessageViewer::AttachmentEditJob(this);
    connect(job, SIGNAL(refreshMessage(Akonadi::Item)), this, SLOT(slotRefreshMessage(Akonadi::Item)));
    job->setMainWindow(mMainWindow);
    job->setMessageItem(mMessageItem);
    job->setMessage(mMessage);

    bool showWarning = true;
    Q_FOREACH (KMime::Content *content, contents) {
        if (!job->addAttachment(content, showWarning)) {
            break;
        }
        showWarning = false;
    }
    job->canDeleteJob();
}

void ViewerPrivate::slotLevelQuote(int l)
{
    if (mLevelQuote != l) {
        mLevelQuote = l;
        update(Viewer::Force);
    }
}

void ViewerPrivate::slotHandleAttachment(int choice)
{
    if (!mCurrentContent) {
        return;
    }
    switch (choice) {
    case Viewer::Delete:
        deleteAttachment(mCurrentContent);
        break;
    case Viewer::Edit:
        editAttachment(mCurrentContent);
        break;
    case Viewer::Properties:
        attachmentProperties(mCurrentContent);
        break;
    case Viewer::Save: {
        QUrl currentUrl;
        if (Util::saveContents(mMainWindow, KMime::Content::List() << mCurrentContent, currentUrl)) {
            showOpenAttachmentFolderWidget(currentUrl);
        }
        break;
    }
    case Viewer::OpenWith:
        attachmentOpenWith(mCurrentContent);
        break;
    case Viewer::Open:
        attachmentOpen(mCurrentContent);
        break;
    case Viewer::View:
        attachmentView(mCurrentContent);
        break;
    case Viewer::ChiasmusEncrypt:
        attachmentEncryptWithChiasmus(mCurrentContent);
        break;
    case Viewer::Copy:
        attachmentCopy(KMime::Content::List() << mCurrentContent);
        break;
    case Viewer::ScrollTo:
        scrollToAttachment(mCurrentContent);
        break;
    }
}

void ViewerPrivate::slotSpeakText()
{
    const QString text = mViewer->selectedText();
    //TODO add texttospeech widget ?
    MessageViewer::Util::speakSelectedText(text);
}

void ViewerPrivate::slotCopyImageLocation()
{
#ifndef QT_NO_CLIPBOARD
    QApplication::clipboard()->setText(mImageUrl.url());
#endif
}

void ViewerPrivate::slotCopySelectedText()
{
#ifndef QT_NO_CLIPBOARD
    QString selection = mViewer->selectedText();
    selection.replace(QChar::Nbsp, QLatin1Char(' '));
    QApplication::clipboard()->setText(selection);
#endif
}

void ViewerPrivate::viewerSelectionChanged()
{
    mActionCollection->action(QStringLiteral("kmail_copy"))->setEnabled(!mViewer->selectedText().isEmpty());
}

void ViewerPrivate::selectAll()
{
    mViewer->selectAll();
}

void ViewerPrivate::clearSelection()
{
    mViewer->clearSelection();
}

void ViewerPrivate::slotUrlCopy()
{
#ifndef QT_NO_CLIPBOARD
    QClipboard *clip = QApplication::clipboard();
    if (mClickedUrl.scheme() == QLatin1String("mailto")) {
        // put the url into the mouse selection and the clipboard
        const QString address = KEmailAddress::decodeMailtoUrl(mClickedUrl);
        clip->setText(address, QClipboard::Clipboard);
        clip->setText(address, QClipboard::Selection);
        KPIM::BroadcastStatus::instance()->setStatusMsg(i18n("Address copied to clipboard."));
    } else {
        // put the url into the mouse selection and the clipboard
        clip->setText(mClickedUrl.url(), QClipboard::Clipboard);
        clip->setText(mClickedUrl.url(), QClipboard::Selection);
        KPIM::BroadcastStatus::instance()->setStatusMsg(i18n("URL copied to clipboard."));
    }
#endif
}

void ViewerPrivate::slotSaveMessage()
{
    if (!mMessageItem.hasPayload<KMime::Message::Ptr>()) {
        if (mMessageItem.isValid()) {
            qCWarning(MESSAGEVIEWER_LOG) << "Payload is not a MessagePtr!";
        }
        return;
    }

    Util::saveMessageInMbox(Akonadi::Item::List() << mMessageItem, mMainWindow);
}

void ViewerPrivate::saveRelativePosition()
{
    mSavedRelativePosition = mViewer->relativePosition();
}

//TODO(Andras) inline them
bool ViewerPrivate::htmlMail() const
{
    if (mDisplayFormatMessageOverwrite == Viewer::UseGlobalSetting) {
        return mHtmlMailGlobalSetting;
    } else {
        return (mDisplayFormatMessageOverwrite == Viewer::Html);
    }
}

bool ViewerPrivate::htmlLoadExternal() const
{
    return ((mHtmlLoadExternalGlobalSetting && !mHtmlLoadExtOverride) ||
            (!mHtmlLoadExternalGlobalSetting && mHtmlLoadExtOverride));
}

void ViewerPrivate::setDisplayFormatMessageOverwrite(Viewer::DisplayFormatMessage format)
{
    mDisplayFormatMessageOverwrite = format;
    // keep toggle display mode action state in sync.
    if (mToggleDisplayModeAction) {
        mToggleDisplayModeAction->setChecked(htmlMail());
    }
}

Viewer::DisplayFormatMessage ViewerPrivate::displayFormatMessageOverwrite() const
{
    return mDisplayFormatMessageOverwrite;
}

void ViewerPrivate::setHtmlLoadExtOverride(bool override)
{
    mHtmlLoadExtOverride = override;
}

bool ViewerPrivate::htmlLoadExtOverride() const
{
    return mHtmlLoadExtOverride;
}

void ViewerPrivate::setDecryptMessageOverwrite(bool overwrite)
{
    mDecrytMessageOverwrite = overwrite;
}

bool ViewerPrivate::showSignatureDetails() const
{
    return mShowSignatureDetails;
}

void ViewerPrivate::setShowSignatureDetails(bool showDetails)
{
    mShowSignatureDetails = showDetails;
}

bool ViewerPrivate::showAttachmentQuicklist() const
{
    return mShowAttachmentQuicklist;
}

void ViewerPrivate::setShowAttachmentQuicklist(bool showAttachmentQuicklist)
{
    mShowAttachmentQuicklist = showAttachmentQuicklist;
}

void ViewerPrivate::setExternalWindow(bool b)
{
    mExternalWindow = b;
}

void ViewerPrivate::scrollToAttachment(KMime::Content *node)
{
    const QString indexStr = node->index().toString();
    // The anchors for this are created in ObjectTreeParser::parseObjectTree()
    mViewer->scrollToAnchor(QLatin1String("att") + indexStr);

    // Remove any old color markings which might be there
    const KMime::Content *root = node->topLevel();
    const int totalChildCount = Util::allContents(root).size();
    for (int i = 0 ; i < totalChildCount + 1 ; ++i) {
        mViewer->removeAttachmentMarking(QStringLiteral("attachmentDiv%1").arg(i + 1));
    }

    // Don't mark hidden nodes, that would just produce a strange yellow line
    if (mNodeHelper->isNodeDisplayedHidden(node)) {
        return;
    }

    // Now, color the div of the attachment in yellow, so that the user sees what happened.
    // We created a special marked div for this in writeAttachmentMarkHeader() in ObjectTreeParser,
    // find and modify that now.
    mViewer->markAttachment(QLatin1String("attachmentDiv") + indexStr, QStringLiteral("border:2px solid %1").arg(cssHelper()->pgpWarnColor().name()));
}

void ViewerPrivate::setUseFixedFont(bool useFixedFont)
{
    mUseFixedFont = useFixedFont;
    if (mToggleFixFontAction) {
        mToggleFixFontAction->setChecked(mUseFixedFont);
    }
}

void ViewerPrivate::attachmentEncryptWithChiasmus(KMime::Content *content)
{
    MessageViewer::AttachmentEncryptWithChiasmusJob *job = new MessageViewer::AttachmentEncryptWithChiasmusJob(this);
    job->setContent(content);
    job->setCurrentFileName(mCurrentFileName);
    job->setMainWindow(mMainWindow);
    job->start();
}

bool ViewerPrivate::showFullToAddressList() const
{
    return mShowFullToAddressList;
}

void ViewerPrivate::setShowFullToAddressList(bool showFullToAddressList)
{
    mShowFullToAddressList = showFullToAddressList;
}

bool ViewerPrivate::showFullCcAddressList() const
{
    return mShowFullCcAddressList;
}

void ViewerPrivate::setShowFullCcAddressList(bool showFullCcAddressList)
{
    mShowFullCcAddressList = showFullCcAddressList;
}

void ViewerPrivate::toggleFullAddressList()
{
    toggleFullAddressList(QStringLiteral("To"));
    toggleFullAddressList(QStringLiteral("Cc"));
}

QString ViewerPrivate::recipientsQuickListLinkHtml(bool doShow, const QString &field)
{
    QString imgpath(picsPath());
    QString urlHandle;
    QString imgSrc;
    QString altText;
    if (doShow) {
        urlHandle.append(QLatin1String("kmail:hideFull") + field + QLatin1String("AddressList"));
        imgSrc.append(QLatin1String("quicklistOpened.png"));
        altText = i18n("Hide full address list");
    } else {
        urlHandle.append(QLatin1String("kmail:showFull") + field + QLatin1String("AddressList"));
        imgSrc.append(QLatin1String("quicklistClosed.png"));
        altText = i18n("Show full address list");
    }

    return QStringLiteral("<span style=\"text-align: right;\"><a href=\"") + urlHandle + QLatin1String("\"><img src=\"file:///") + imgpath + imgSrc + QLatin1String("\""
            "alt=\"") + altText + QLatin1String("\" /></a></span>");
}

void ViewerPrivate::toggleFullAddressList(const QString &field)
{
    const bool doShow = (field == QLatin1String("To") && showFullToAddressList()) || (field == QLatin1String("Cc") && showFullCcAddressList());
    // First inject the correct icon
    if (mViewer->replaceInnerHtml(QLatin1String("iconFull") + field + QLatin1String("AddressList"),
                                  bind(&ViewerPrivate::recipientsQuickListLinkHtml, this, doShow, field))) {
        // Then show/hide the full address list
        mViewer->setElementByIdVisible(QLatin1String("dotsFull")   + field + QLatin1String("AddressList"), !doShow);
        mViewer->setElementByIdVisible(QLatin1String("hiddenFull") + field + QLatin1String("AddressList"),  doShow);
    }
}

void ViewerPrivate::itemFetchResult(KJob *job)
{
    if (job->error()) {
        displaySplashPage(i18n("Message loading failed: %1.", job->errorText()));
    } else {
        Akonadi::ItemFetchJob *fetch = qobject_cast<Akonadi::ItemFetchJob *>(job);
        Q_ASSERT(fetch);
        if (fetch->items().isEmpty()) {
            displaySplashPage(i18n("Message not found."));
        } else {
            setMessageItem(fetch->items().first());
        }
    }
}

void ViewerPrivate::slotItemChanged(const Akonadi::Item &item, const QSet<QByteArray> &parts)
{
    if (item.id() != messageItem().id()) {
        qCDebug(MESSAGEVIEWER_LOG) << "Update for an already forgotten item. Weird.";
        return;
    }
    if (parts.contains("PLD:RFC822")) {
        setMessageItem(item, Viewer::Force);
    }
}

void ViewerPrivate::slotItemMoved(const Akonadi::Item &item, const Akonadi::Collection &,
                                  const Akonadi::Collection &)
{
    // clear the view after the current item has been moved somewhere else (e.g. to trash)
    if (item.id() == messageItem().id()) {
        slotClear();
    }
}

void ViewerPrivate::slotClear()
{
    q->clear(Viewer::Force);
    Q_EMIT itemRemoved();
}

void ViewerPrivate::slotMessageRendered()
{
    if (!mMessageItem.isValid()) {
        return;
    }

    /**
    * This slot might be called multiple times for the same message if
    * some asynchronous mementos are involved in rendering. Therefor we
    * have to make sure we execute the MessageLoadedHandlers only once.
    */
    if (mMessageItem.id() == mPreviouslyViewedItem) {
        return;
    }

    mPreviouslyViewedItem = mMessageItem.id();

    foreach (AbstractMessageLoadedHandler *handler, mMessageLoadedHandlers) {
        handler->setItem(mMessageItem);
    }
}

void ViewerPrivate::setZoomFactor(qreal zoomFactor)
{
#ifndef KDEPIM_NO_WEBKIT
    mViewer->setZoomFactor(zoomFactor);
#endif
}

void ViewerPrivate::slotZoomIn()
{
#ifndef KDEPIM_NO_WEBKIT
    if (mZoomFactor >= 300) {
        return;
    }
    mZoomFactor += zoomBy;
    if (mZoomFactor > 300) {
        mZoomFactor = 300;
    }
    mViewer->setZoomFactor(mZoomFactor / 100.0);
#endif
}

void ViewerPrivate::slotZoomOut()
{
#ifndef KDEPIM_NO_WEBKIT
    if (mZoomFactor <= 10) {
        return;
    }
    mZoomFactor -= zoomBy;
    if (mZoomFactor < 10) {
        mZoomFactor = 10;
    }
    mViewer->setZoomFactor(mZoomFactor / 100.0);
#endif
}

void ViewerPrivate::setZoomTextOnly(bool textOnly)
{
    mZoomTextOnly = textOnly;
    if (mZoomTextOnlyAction) {
        mZoomTextOnlyAction->setChecked(mZoomTextOnly);
    }
#ifndef KDEPIM_NO_WEBKIT
    mViewer->settings()->setAttribute(QWebSettings::ZoomTextOnly, mZoomTextOnly);
#endif
}

void ViewerPrivate::slotZoomTextOnly()
{
    setZoomTextOnly(!mZoomTextOnly);
}

void ViewerPrivate::slotZoomReset()
{
#ifndef KDEPIM_NO_WEBKIT
    mZoomFactor = 100;
    mViewer->setZoomFactor(1.0);
#endif
}

void ViewerPrivate::goOnline()
{
    Q_EMIT makeResourceOnline(Viewer::AllResources);
}

void ViewerPrivate::goResourceOnline()
{
    Q_EMIT makeResourceOnline(Viewer::SelectedResource);
}

void ViewerPrivate::slotToggleCaretBrowsing(bool toggle)
{
#ifndef KDEPIM_NO_WEBKIT
    if (toggle) {
        KMessageBox::information(mMainWindow,
                                 i18n("Caret Browsing will be activated. Switch off with F7 shortcut."),
                                 i18n("Activate Caret Browsing"));
    }
    mViewer->settings()->setAttribute(QWebSettings::CaretBrowsingEnabled, toggle);
#endif
    Q_UNUSED(toggle);
}

void ViewerPrivate::slotSaveMessageDisplayFormat()
{
    if (mMessageItem.isValid()) {
        MessageViewer::ModifyMessageDisplayFormatJob *job = new MessageViewer::ModifyMessageDisplayFormatJob(this);
        job->setMessageFormat(displayFormatMessageOverwrite());
        job->setMessageItem(mMessageItem);
        job->setRemoteContent(htmlLoadExtOverride());
        job->start();
    }
}

void ViewerPrivate::slotResetMessageDisplayFormat()
{
    if (mMessageItem.isValid()) {
        if (mMessageItem.hasAttribute<MessageViewer::MessageDisplayFormatAttribute>()) {
            MessageViewer::ModifyMessageDisplayFormatJob *job = new MessageViewer::ModifyMessageDisplayFormatJob(this);
            job->setMessageItem(mMessageItem);
            job->setResetFormat(true);
            job->start();
        }
    }
}

void ViewerPrivate::slotMessageMayBeAScam()
{
    if (mMessageItem.isValid()) {
        if (mMessageItem.hasAttribute<MessageViewer::ScamAttribute>()) {
            const MessageViewer::ScamAttribute *const attr = mMessageItem.attribute<MessageViewer::ScamAttribute>();
            if (attr && !attr->isAScam()) {
                return;
            }
        }
        if (mMessageItem.hasPayload<KMime::Message::Ptr>()) {
            KMime::Message::Ptr message = mMessageItem.payload<KMime::Message::Ptr>();
            const QString email = QLatin1String(KEmailAddress::firstEmailAddress(message->from()->as7BitString(false)));
            const QStringList lst = MessageViewer::GlobalSettings::self()->scamDetectionWhiteList();
            if (lst.contains(email)) {
                return;
            }
        }
    }
    mScamDetectionWarning->slotShowWarning();
}

void ViewerPrivate::slotMessageIsNotAScam()
{
    if (mMessageItem.isValid()) {
        MessageViewer::ScamAttribute *attr  = mMessageItem.attribute<MessageViewer::ScamAttribute>(Akonadi::Entity::AddIfMissing);
        attr->setIsAScam(false);
        Akonadi::ItemModifyJob *modify = new Akonadi::ItemModifyJob(mMessageItem);
        modify->setIgnorePayload(true);
        modify->disableRevisionCheck();
        connect(modify, SIGNAL(result(KJob*)), this, SLOT(slotModifyItemDone(KJob*)));
    }
}

void ViewerPrivate::slotModifyItemDone(KJob *job)
{
    if (job && job->error()) {
        qCWarning(MESSAGEVIEWER_LOG) << " Error trying to change attribute:" << job->errorText();
    }
}

void ViewerPrivate::slotGrantleeThemesUpdated()
{
    update(Viewer::Force);
}

void ViewerPrivate::saveMainFrameScreenshotInFile(const QString &filename)
{
#ifndef KDEPIM_NO_WEBKIT
    mViewer->saveMainFrameScreenshotInFile(filename);
#endif
}

void ViewerPrivate::slotAddToWhiteList()
{
    if (mMessageItem.isValid()) {
        if (mMessageItem.hasPayload<KMime::Message::Ptr>()) {
            KMime::Message::Ptr message = mMessageItem.payload<KMime::Message::Ptr>();
            const QString email = QLatin1String(KEmailAddress::firstEmailAddress(message->from()->as7BitString(false)));
            QStringList lst = MessageViewer::GlobalSettings::self()->scamDetectionWhiteList();
            if (lst.contains(email)) {
                return;
            }
            lst << email;
            MessageViewer::GlobalSettings::self()->setScamDetectionWhiteList(lst);
            MessageViewer::GlobalSettings::self()->save();
        }
    }
}

void ViewerPrivate::slotBlockImage()
{
    if (mImageUrl.isEmpty()) {
        return;
    }
#ifndef KDEPIM_NO_WEBKIT
    MessageViewer::AdBlockManager::self()->addCustomRule(mImageUrl.url(), true);
#endif
}

void ViewerPrivate::slotOpenBlockableItems()
{
#ifndef KDEPIM_NO_WEBKIT
    mViewer->openBlockableItemsDialog();
#endif
}

bool ViewerPrivate::isAShortUrl(const QUrl &url) const
{
    return mViewer->isAShortUrl(url);
}

void ViewerPrivate::slotExpandShortUrl()
{
    if (mClickedUrl.isValid()) {
        mViewer->expandUrl(mClickedUrl);
    }
}

void ViewerPrivate::slotShowCreateTodoWidget()
{
    if (mMessage) {
        mCreateTodo->setMessage(mMessage);
        mCreateTodo->showToDoWidget();
    } else {
        qCDebug(MESSAGEVIEWER_LOG) << " There is not valid message";
    }
}

void ViewerPrivate::slotCreateTodo(const KCalCore::Todo::Ptr &todoPtr, const Akonadi::Collection &collection)
{
    CreateTodoJob *createJob = new CreateTodoJob(todoPtr, collection, mMessageItem, this);
    createJob->start();
}

Akonadi::Relation ViewerPrivate::relatedNoteRelation() const
{
    Akonadi::Relation relation;
    foreach (const Akonadi::Relation &r, mMessageItem.relations()) {
        // assuming that GENERIC relations to emails are notes is a pretty horirific hack imo - aseigo
        if (r.type() == Akonadi::Relation::GENERIC && r.right().mimeType() == Akonadi::NoteUtils::noteMimeType()) {
            relation = r;
            break;
        }
    }
    return relation;
}

void ViewerPrivate::slotShowCreateNoteWidget()
{
    if (!mMessageItem.relations().isEmpty())  {
        Akonadi::Relation relation = relatedNoteRelation();
        if (relation.isValid()) {
            Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(relation.right());
            job->fetchScope().fetchFullPayload(true);
            connect(job, &Akonadi::ItemFetchJob::result, this, &ViewerPrivate::slotNoteItemFetched);
            return;
        }
    }

    showCreateNewNoteWidget();
}

void ViewerPrivate::showCreateNewNoteWidget()
{
    if (mMessage) {
        mCreateNote->setMessage(mMessage);
        mCreateNote->showNoteEdit();
    } else {
        qCDebug(MESSAGEVIEWER_LOG) << "There is not valid message";
    }
}

void ViewerPrivate::slotNoteItemFetched(KJob *job)
{
    if (job->error()) {
        qCDebug(MESSAGEVIEWER_LOG) << "There is not valid note:" << job->errorString();
        showCreateNewNoteWidget();
    } else {
        Akonadi::ItemFetchJob *fetch = qobject_cast<Akonadi::ItemFetchJob *>(job);
        Q_ASSERT(fetch);
        if (fetch->items().isEmpty() || !fetch->items().first().hasPayload<KMime::Message::Ptr>()) {
            showCreateNewNoteWidget();
        } else {
            Akonadi::NoteUtils::NoteMessageWrapper note(fetch->items().first().payload<KMime::Message::Ptr>());
            mCreateNote->setMessage(note.message());
            mCreateNote->showNoteEdit();
        }
    }
}

void ViewerPrivate::slotCreateNote(const KMime::Message::Ptr &notePtr, const Akonadi::Collection &collection)
{
    CreateNoteJob *createJob = new CreateNoteJob(notePtr, collection, mMessageItem, this);
    createJob->start();
}

void ViewerPrivate::slotShowCreateEventWidget()
{
    if (mMessage) {
        mCreateEvent->setMessage(mMessage);
        mCreateEvent->showEventEdit();
    } else {
        qCDebug(MESSAGEVIEWER_LOG) << " There is not valid message";
    }
}

void ViewerPrivate::slotCreateEvent(const KCalCore::Event::Ptr &eventPtr, const Akonadi::Collection &collection)
{
    CreateEventJob *createJob = new CreateEventJob(eventPtr, collection, mMessageItem, this);
    createJob->start();
}

void ViewerPrivate::addHelpTextAction(QAction *act, const QString &text)
{
    act->setStatusTip(text);
    act->setToolTip(text);
    if (act->whatsThis().isEmpty()) {
        act->setWhatsThis(text);
    }
}

void ViewerPrivate::slotRefreshMessage(const Akonadi::Item &item)
{
    if (item.id() == mMessageItem.id()) {
        setMessageItem(item, MessageViewer::Viewer::Force);
    }
}

void ViewerPrivate::slotServiceUrlSelected(PimCommon::ShareServiceUrlManager::ServiceType serviceType)
{
    const QUrl url = mShareServiceManager->generateServiceUrl(mClickedUrl.toString(), QString(), serviceType);
    mShareServiceManager->openUrl(url);
}
