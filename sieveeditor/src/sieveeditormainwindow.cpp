/*
  Copyright (c) 2013-2016 Montel Laurent <montel@kde.org>

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

#include "sieveeditormainwindow.h"
#include "sieveeditormainwidget.h"
#include "sieveeditorconfiguredialog.h"
#include "serversievesettingsdialog.h"
#include "sieveserversettings.h"
#include "sieveeditorcentralwidget.h"
#include "sieveeditorglobalconfig.h"
#include "sieveeditorbookmarks.h"
#include "PimCommon/KActionMenuChangeCase"

#include <KStandardGuiItem>
#include <KSharedConfig>
#include <KIconEngine>
#include <KIconLoader>

#include <KLocalizedString>
#include <KConfigGroup>
#include <KStandardAction>
#include <KActionCollection>
#include <QAction>
#include <QStatusBar>
#include <QIcon>

#include <QPointer>
#include <QLabel>
#include <QCloseEvent>
#include <QNetworkConfigurationManager>

SieveEditorMainWindow::SieveEditorMainWindow()
    : KXmlGuiWindow()
{
    mMainWidget = new SieveEditorCentralWidget(this);
    connect(mMainWidget, &SieveEditorCentralWidget::configureClicked, this, &SieveEditorMainWindow::slotConfigure);
    connect(mMainWidget->sieveEditorMainWidget(), &SieveEditorMainWidget::updateButtons, this, &SieveEditorMainWindow::slotUpdateButtons);
    setCentralWidget(mMainWidget);
    setupActions();
    setupGUI();
    readConfig();
    initStatusBar();
    mNetworkConfigurationManager = new QNetworkConfigurationManager();
    connect(mNetworkConfigurationManager, &QNetworkConfigurationManager::onlineStateChanged, this, &SieveEditorMainWindow::slotSystemNetworkOnlineStateChanged);

    connect(mMainWidget->sieveEditorMainWidget()->tabWidget(), &QTabWidget::currentChanged, this, &SieveEditorMainWindow::slotUpdateActions);
    connect(mMainWidget->sieveEditorMainWidget(), &SieveEditorMainWidget::modeEditorChanged, this, &SieveEditorMainWindow::slotUpdateActions);
    slotSystemNetworkOnlineStateChanged(mNetworkConfigurationManager->isOnline());
    connect(mMainWidget->sieveEditorMainWidget(), &SieveEditorMainWidget::undoAvailable, this, &SieveEditorMainWindow::slotUndoAvailable);
    connect(mMainWidget->sieveEditorMainWidget(), &SieveEditorMainWidget::redoAvailable, this, &SieveEditorMainWindow::slotRedoAvailable);
    connect(mMainWidget->sieveEditorMainWidget(), &SieveEditorMainWidget::copyAvailable, this, &SieveEditorMainWindow::slotCopyAvailable);
    connect(mMainWidget->sieveEditorMainWidget(), &SieveEditorMainWidget::sieveEditorTabCurrentChanged, this, &SieveEditorMainWindow::slotUpdateActions);
    mMainWidget->sieveEditorMainWidget()->refreshList();
}

SieveEditorMainWindow::~SieveEditorMainWindow()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group = config->group(QStringLiteral("SieveEditorMainWindow"));
    group.writeEntry("Size", size());
    if (SieveEditorGlobalConfig::self()->closeWallet()) {
        SieveServerSettings::self()->closeWallet();
    }
}

void SieveEditorMainWindow::initStatusBar()
{
    mStatusBarInfo = new QLabel;
    statusBar()->insertWidget(0, mStatusBarInfo, 4);
}

void SieveEditorMainWindow::slotSystemNetworkOnlineStateChanged(bool state)
{
    if (state) {
        mStatusBarInfo->setText(i18n("Network is Up."));
    } else {
        mStatusBarInfo->setText(i18n("Network is Down."));
    }
    mMainWidget->sieveEditorMainWidget()->setEnabled(state);
    slotUpdateActions();
}

void SieveEditorMainWindow::slotUpdateButtons(bool newScriptAction, bool editScriptAction, bool deleteScriptAction, bool desactivateScriptAction)
{
    mDeleteScript->setEnabled(deleteScriptAction);
    mNewScript->setEnabled(newScriptAction);
    mEditScript->setEnabled(editScriptAction);
    mDesactivateScript->setEnabled(desactivateScriptAction);
}

void SieveEditorMainWindow::readConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group = KConfigGroup(config, "SieveEditorMainWindow");
    const QSize sizeDialog = group.readEntry("Size", QSize(800, 600));
    if (sizeDialog.isValid()) {
        resize(sizeDialog);
    }
}

void SieveEditorMainWindow::setupActions()
{
    KActionCollection *ac = actionCollection();

    KStandardAction::quit(this, SLOT(close()), ac);
    KStandardAction::preferences(this, SLOT(slotConfigure()), ac);

    mUploadScript = KStandardAction::save(this, SLOT(slotUploadScript()), ac);
    mUploadScript->setText(i18n("Upload"));
    mUploadScript->setEnabled(false);

    QAction *act = ac->addAction(QStringLiteral("add_server_sieve"), this, SLOT(slotAddServerSieve()));
    act->setText(i18n("Add Sieve Server..."));

    mDeleteScript = ac->addAction(QStringLiteral("delete_script"), this, SLOT(slotDeleteScript()));
    mDeleteScript->setText(i18n("Delete Script"));
    ac->setDefaultShortcut(mDeleteScript, QKeySequence(Qt::Key_Delete));
    mDeleteScript->setEnabled(false);

    mNewScript = ac->addAction(QStringLiteral("create_new_script"), this, SLOT(slotCreateNewScript()));
    ac->setDefaultShortcut(mNewScript, QKeySequence(Qt::CTRL + Qt::Key_N));
    mNewScript->setText(i18n("Create New Script..."));
    mNewScript->setEnabled(false);

    mEditScript = ac->addAction(QStringLiteral("edit_script"), this, SLOT(slotEditScript()));
    mEditScript->setText(i18n("Edit Script..."));
    mEditScript->setEnabled(false);

    mDesactivateScript = ac->addAction(QStringLiteral("desactivate_script"), this, SLOT(slotDesactivateScript()));
    mDesactivateScript->setText(i18n("Deactivate Script"));
    mDesactivateScript->setEnabled(false);

    mRefreshList = ac->addAction(QStringLiteral("refresh_list"), this, SLOT(slotRefreshList()));
    mRefreshList->setText(i18n("Refresh List"));
    mRefreshList->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
    ac->setDefaultShortcut(mRefreshList, QKeySequence(Qt::Key_F5));

    mGoToLine = ac->addAction(QStringLiteral("gotoline"), mMainWidget->sieveEditorMainWidget(), SLOT(slotGoToLine()));
    mGoToLine->setText(i18n("Go to Line..."));
    mGoToLine->setIcon(QIcon::fromTheme(QStringLiteral("go-jump")));
    ac->setDefaultShortcut(mGoToLine, QKeySequence(Qt::CTRL + Qt::Key_G));
    mGoToLine->setEnabled(false);

    mFindAction = KStandardAction::find(mMainWidget->sieveEditorMainWidget(), SLOT(slotFind()), ac);
    mReplaceAction = KStandardAction::replace(mMainWidget->sieveEditorMainWidget(), SLOT(slotReplace()), ac);
    mUndoAction = KStandardAction::undo(mMainWidget->sieveEditorMainWidget(), SLOT(slotUndo()), ac);
    mRedoAction = KStandardAction::redo(mMainWidget->sieveEditorMainWidget(), SLOT(slotRedo()), ac);
    mCopyAction = KStandardAction::copy(mMainWidget->sieveEditorMainWidget(), SLOT(slotCopy()), ac);
    mPasteAction = KStandardAction::paste(mMainWidget->sieveEditorMainWidget(), SLOT(slotPaste()), ac);
    mCutAction = KStandardAction::cut(mMainWidget->sieveEditorMainWidget(), SLOT(slotCut()), ac);
    mSelectAllAction = KStandardAction::selectAll(mMainWidget->sieveEditorMainWidget(), SLOT(slotSelectAll()), ac);
    mSaveAsAction = KStandardAction::saveAs(mMainWidget->sieveEditorMainWidget(), SLOT(slotSaveAs()), ac);

    mImportAction = ac->addAction(QStringLiteral("import_script"), mMainWidget->sieveEditorMainWidget(), SLOT(slotImport()));
    mImportAction->setText(i18n("Import..."));
    mImportAction->setEnabled(false);

    mShareAction = ac->addAction(QStringLiteral("share_script"), mMainWidget->sieveEditorMainWidget(), SLOT(slotShareScript()));
    mShareAction->setText(i18n("Share..."));
    const QStringList overlays = QStringList() << QStringLiteral("list-add");
    mShareAction->setIcon(QIcon(new KIconEngine(QStringLiteral("get-hot-new-stuff"), KIconLoader::global(), overlays)));
    mShareAction->setEnabled(false);

    mSpellCheckAction = ac->addAction(QStringLiteral("check_spelling"), mMainWidget->sieveEditorMainWidget(), SLOT(slotCheckSpelling()));
    mSpellCheckAction->setIcon(QIcon::fromTheme(QStringLiteral("tools-check-spelling")));
    mSpellCheckAction->setText(i18n("Check Spelling..."));
    mSpellCheckAction->setEnabled(false);

    mCheckSyntaxAction = ac->addAction(QStringLiteral("check_syntax"), mMainWidget->sieveEditorMainWidget(), SLOT(slotCheckSyntax()));
    mCheckSyntaxAction->setText(i18n("Check Syntax"));
    mCheckSyntaxAction->setEnabled(false);

    mAutoGenerateScriptAction = ac->addAction(QStringLiteral("autogenerate_script"), mMainWidget->sieveEditorMainWidget(), SLOT(slotAutoGenerateScript()));
    mAutoGenerateScriptAction->setText(i18n("Autogenerate Script..."));
    mAutoGenerateScriptAction->setEnabled(false);

    mCommentAction = ac->addAction(QStringLiteral("comment_code"), mMainWidget->sieveEditorMainWidget(), SLOT(slotComment()));
    ac->setDefaultShortcut(mCommentAction, Qt::CTRL + Qt::Key_D);
    mCommentAction->setText(i18n("Comment"));

    mUncommentAction = ac->addAction(QStringLiteral("uncomment_code"), mMainWidget->sieveEditorMainWidget(), SLOT(slotUncomment()));
    ac->setDefaultShortcut(mUncommentAction, Qt::CTRL + Qt::SHIFT + Qt::Key_D);
    mUncommentAction->setText(i18n("Uncomment"));

    mZoomInAction = new QAction(QIcon::fromTheme(QStringLiteral("zoom-in")), i18n("&Zoom In"), this);
    ac->addAction(QStringLiteral("zoom_in"), mZoomInAction);
    connect(mZoomInAction, &QAction::triggered, mMainWidget->sieveEditorMainWidget(), &SieveEditorMainWidget::slotZoomIn);
    ac->setDefaultShortcut(mZoomInAction, QKeySequence(Qt::CTRL + Qt::Key_Plus));

    mZoomOutAction = new QAction(QIcon::fromTheme(QStringLiteral("zoom-out")), i18n("Zoom &Out"), this);
    ac->addAction(QStringLiteral("zoom_out"), mZoomOutAction);
    connect(mZoomOutAction, &QAction::triggered, mMainWidget->sieveEditorMainWidget(), &SieveEditorMainWidget::slotZoomOut);
    ac->setDefaultShortcut(mZoomOutAction, QKeySequence(Qt::CTRL + Qt::Key_Minus));

    mZoomResetAction = ac->addAction(QStringLiteral("zoom_reset"), mMainWidget->sieveEditorMainWidget(), SLOT(slotZoomReset()));
    ac->setDefaultShortcut(mZoomResetAction, QKeySequence(Qt::CTRL + Qt::Key_0));
    mZoomResetAction->setText(i18nc("Reset the zoom", "Reset"));

    mMenuChangeCaseAction = new PimCommon::KActionMenuChangeCase(this);
    ac->addAction(QStringLiteral("change_case_menu"), mMenuChangeCaseAction);
    mMenuChangeCaseAction->appendInActionCollection(ac);
    connect(mMenuChangeCaseAction, &PimCommon::KActionMenuChangeCase::upperCase, mMainWidget->sieveEditorMainWidget(), &SieveEditorMainWidget::slotUpperCase);
    connect(mMenuChangeCaseAction, &PimCommon::KActionMenuChangeCase::lowerCase, mMainWidget->sieveEditorMainWidget(), &SieveEditorMainWidget::slotLowerCase);
    connect(mMenuChangeCaseAction, &PimCommon::KActionMenuChangeCase::sentenceCase, mMainWidget->sieveEditorMainWidget(), &SieveEditorMainWidget::slotSentenceCase);
    connect(mMenuChangeCaseAction, &PimCommon::KActionMenuChangeCase::reverseCase, mMainWidget->sieveEditorMainWidget(), &SieveEditorMainWidget::slotReverseCase);

    mBookmarkMenu = new KActionMenu(i18nc("@title:menu", "&Bookmarks"), ac);
    mSieveEditorBookmarks = new SieveEditorBookmarks(this, ac, mBookmarkMenu->menu(), this);
    ac->addAction(QStringLiteral("bookmark"), mBookmarkMenu);
    connect(mSieveEditorBookmarks, &SieveEditorBookmarks::openUrl, this, &SieveEditorMainWindow::slotOpenBookmarkUrl);

    mDebugSieveScriptAction = ac->addAction(QStringLiteral("debug_sieve"), mMainWidget->sieveEditorMainWidget(), SLOT(slotDebugSieveScript()));
    mDebugSieveScriptAction->setText(i18n("Debug Sieve Script..."));
    ac->setDefaultShortcut(mDebugSieveScriptAction, QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_D));

    mWrapTextAction = new QAction(i18n("Wordwarp"), this);
    mWrapTextAction->setCheckable(true);
    ac->addAction(QStringLiteral("wordwrap"), mWrapTextAction);
    connect(mWrapTextAction, &QAction::triggered, mMainWidget->sieveEditorMainWidget(), &SieveEditorMainWidget::slotWordWrap);

    mPrintAction = KStandardAction::print(mMainWidget->sieveEditorMainWidget(), SLOT(slotPrint()), ac);

    mPrintPreviewAction = KStandardAction::printPreview(mMainWidget->sieveEditorMainWidget(), SLOT(slotPrintPreview()), ac);
}

void SieveEditorMainWindow::slotRefreshList()
{
    if (mNetworkConfigurationManager->isOnline()) {
        mMainWidget->sieveEditorMainWidget()->refreshList();
    }
}

void SieveEditorMainWindow::slotUploadScript()
{
    mMainWidget->sieveEditorMainWidget()->uploadScript();
}

void SieveEditorMainWindow::slotDesactivateScript()
{
    mMainWidget->sieveEditorMainWidget()->desactivateScript();
}

void SieveEditorMainWindow::slotEditScript()
{
    mMainWidget->sieveEditorMainWidget()->editScript();
}

void SieveEditorMainWindow::slotCreateNewScript()
{
    mMainWidget->sieveEditorMainWidget()->createNewScript();
}

void SieveEditorMainWindow::slotDeleteScript()
{
    mMainWidget->sieveEditorMainWidget()->deleteScript();
}

void SieveEditorMainWindow::closeEvent(QCloseEvent *e)
{
    if (mMainWidget->sieveEditorMainWidget()->needToSaveScript()) {
        e->ignore();
        return;
    } else {
        e->accept();
    }
}

void SieveEditorMainWindow::slotConfigure()
{
    QPointer<SieveEditorConfigureDialog> dlg = new SieveEditorConfigureDialog(this);
    if (dlg->exec()) {
        dlg->saveServerSieveConfig();
        mMainWidget->sieveEditorMainWidget()->updateServerList();
    }
    delete dlg;
}

void SieveEditorMainWindow::slotAddServerSieve()
{
    QPointer<ServerSieveSettingsDialog> dlg = new ServerSieveSettingsDialog(this);
    if (dlg->exec()) {
        const SieveEditorUtil::SieveServerConfig conf = dlg->serverSieveConfig();
        SieveEditorUtil::addServerSieveConfig(conf);
        mMainWidget->sieveEditorMainWidget()->updateServerList();
    }
    delete dlg;
}

void SieveEditorMainWindow::slotUpdateActions()
{
    const bool hasPage = (mMainWidget->sieveEditorMainWidget()->tabWidget()->count() > 0);
    mUploadScript->setEnabled(hasPage);
    const bool editActionEnabled = (hasPage && mMainWidget->sieveEditorMainWidget()->pageMode() == KSieveUi::SieveEditorWidget::TextMode);
    mGoToLine->setEnabled(editActionEnabled);
    mFindAction->setEnabled(editActionEnabled);
    mReplaceAction->setEnabled(editActionEnabled);
    mUndoAction->setEnabled(editActionEnabled && mMainWidget->sieveEditorMainWidget()->isUndoAvailable());
    mRedoAction->setEnabled(editActionEnabled && mMainWidget->sieveEditorMainWidget()->isRedoAvailable());

    mCopyAction->setEnabled(editActionEnabled && mMainWidget->sieveEditorMainWidget()->hasSelection());
    mPasteAction->setEnabled(editActionEnabled);
    mCutAction->setEnabled(editActionEnabled && mMainWidget->sieveEditorMainWidget()->hasSelection());

    mSelectAllAction->setEnabled(editActionEnabled);

    mUploadScript->setEnabled(hasPage && !mNetworkIsDown);
    mRefreshList->setEnabled(!mNetworkIsDown);
    mSaveAsAction->setEnabled(hasPage);
    mImportAction->setEnabled(hasPage);
    mShareAction->setEnabled(hasPage && !mNetworkIsDown);
    mSpellCheckAction->setEnabled(editActionEnabled);
    mCheckSyntaxAction->setEnabled(editActionEnabled && !mNetworkIsDown);
    mAutoGenerateScriptAction->setEnabled(editActionEnabled);
    mCommentAction->setEnabled(editActionEnabled);
    mUncommentAction->setEnabled(editActionEnabled);
    mMenuChangeCaseAction->setEnabled(editActionEnabled);
    mZoomInAction->setEnabled(editActionEnabled);
    mZoomOutAction->setEnabled(editActionEnabled);
    mZoomResetAction->setEnabled(editActionEnabled);

    mBookmarkMenu->setEnabled(editActionEnabled);
    mDebugSieveScriptAction->setEnabled(editActionEnabled);
    mWrapTextAction->setEnabled(editActionEnabled);
    mWrapTextAction->setChecked(mMainWidget->sieveEditorMainWidget()->isWordWrap());

    mPrintAction->setEnabled(editActionEnabled && mMainWidget->sieveEditorMainWidget()->printSupportEnabled());
    mPrintPreviewAction->setEnabled(editActionEnabled && mMainWidget->sieveEditorMainWidget()->printSupportEnabled());
}

void SieveEditorMainWindow::slotUndoAvailable(bool b)
{
    const bool hasPage = (mMainWidget->sieveEditorMainWidget()->tabWidget()->count() > 0);
    const bool editActionEnabled = (hasPage && mMainWidget->sieveEditorMainWidget()->pageMode() == KSieveUi::SieveEditorWidget::TextMode);
    mUndoAction->setEnabled(editActionEnabled && b);
}

void SieveEditorMainWindow::slotRedoAvailable(bool b)
{
    const bool hasPage = (mMainWidget->sieveEditorMainWidget()->tabWidget()->count() > 0);
    const bool editActionEnabled = (hasPage && mMainWidget->sieveEditorMainWidget()->pageMode() == KSieveUi::SieveEditorWidget::TextMode);
    mRedoAction->setEnabled(editActionEnabled && b);
}

void SieveEditorMainWindow::slotCopyAvailable(bool b)
{
    const bool hasPage = (mMainWidget->sieveEditorMainWidget()->tabWidget()->count() > 0);
    const bool editActionEnabled = (hasPage && mMainWidget->sieveEditorMainWidget()->pageMode() == KSieveUi::SieveEditorWidget::TextMode);
    mCopyAction->setEnabled(editActionEnabled && b);
    mCutAction->setEnabled(editActionEnabled && b);
}

void SieveEditorMainWindow::slotOpenBookmarkUrl(const QUrl &url)
{
    mMainWidget->sieveEditorMainWidget()->openBookmarkUrl(url);
}

QString SieveEditorMainWindow::currentHelpTitle() const
{
    return mMainWidget->sieveEditorMainWidget()->currentHelpTitle();
}

QUrl SieveEditorMainWindow::currentHelpUrl() const
{
    return mMainWidget->sieveEditorMainWidget()->currentHelpUrl();
}
