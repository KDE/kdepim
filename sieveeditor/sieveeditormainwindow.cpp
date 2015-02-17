/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include <KStandardGuiItem>
#include <KSharedConfig>

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
    connect(mMainWidget->sieveEditorMainWidget(), SIGNAL(undoAvailable(bool)), SLOT(slotUndoAvailable(bool)));
    connect(mMainWidget->sieveEditorMainWidget(), SIGNAL(redoAvailable(bool)), SLOT(slotRedoAvailable(bool)));
    connect(mMainWidget->sieveEditorMainWidget(), SIGNAL(copyAvailable(bool)), SLOT(slotCopyAvailable(bool)));
    slotRefreshList();
}

SieveEditorMainWindow::~SieveEditorMainWindow()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    KConfigGroup group = config->group(QLatin1String("SieveEditorMainWindow"));
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
    mSaveScript = KStandardAction::save(this, SLOT(slotSaveScript()), ac);
    mSaveScript->setEnabled(false);

    QAction *act = ac->addAction(QLatin1String("add_server_sieve"), this, SLOT(slotAddServerSieve()));
    act->setText(i18n("Add Server Sieve..."));

    mDeleteScript = ac->addAction(QLatin1String("delete_script"), this, SLOT(slotDeleteScript()));
    mDeleteScript->setText(i18n("Delete Script"));
    ac->setDefaultShortcut(mDeleteScript, QKeySequence(Qt::Key_Delete));
    mDeleteScript->setEnabled(false);

    mNewScript = ac->addAction(QLatin1String("create_new_script"), this, SLOT(slotCreateNewScript()));
    ac->setDefaultShortcut(mNewScript, QKeySequence(Qt::CTRL + Qt::Key_N));
    mNewScript->setText(i18n("Create New Script..."));
    mNewScript->setEnabled(false);

    mEditScript = ac->addAction(QLatin1String("edit_script"), this, SLOT(slotEditScript()));
    mEditScript->setText(i18n("Edit Script"));
    mEditScript->setEnabled(false);

    mDesactivateScript = ac->addAction(QLatin1String("desactivate_script"), this, SLOT(slotDesactivateScript()));
    mDesactivateScript->setText(i18n("Deactivate Script"));
    mDesactivateScript->setEnabled(false);

    mRefreshList = ac->addAction(QLatin1String("refresh_list"), this, SLOT(slotRefreshList()));
    mRefreshList->setText(i18n("Refresh List"));
    mRefreshList->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
    ac->setDefaultShortcut(mRefreshList, QKeySequence(Qt::Key_F5));

    mGoToLine = ac->addAction(QLatin1String("gotoline"), mMainWidget->sieveEditorMainWidget(), SLOT(slotGoToLine()));
    mGoToLine->setText(i18n("Go to Line"));
    mGoToLine->setIcon(QIcon::fromTheme(QLatin1String("go-jump")));
    ac->setDefaultShortcut(mGoToLine, QKeySequence(Qt::CTRL + Qt::Key_G));
    mGoToLine->setEnabled(false);

    mFindAction = KStandardAction::find(mMainWidget->sieveEditorMainWidget(), SLOT(slotFind()), ac);
    mReplaceAction = KStandardAction::replace(mMainWidget->sieveEditorMainWidget(), SLOT(slotReplace()), ac);
    mUndoAction = KStandardAction::undo(mMainWidget->sieveEditorMainWidget(), SLOT(slotUndo()), ac);
    mRedoAction = KStandardAction::redo(mMainWidget->sieveEditorMainWidget(), SLOT(slotRedo()), ac);
    mCopyAction = KStandardAction::copy(mMainWidget->sieveEditorMainWidget(), SLOT(slotCopy()), ac);
    mPasteAction = KStandardAction::paste(mMainWidget->sieveEditorMainWidget(), SLOT(slotPaste()), ac);
    mCutAction = KStandardAction::cut(mMainWidget->sieveEditorMainWidget(), SLOT(slotCut()), ac);
}

void SieveEditorMainWindow::slotRefreshList()
{
    if (mNetworkConfigurationManager->isOnline()) {
        mMainWidget->sieveEditorMainWidget()->refreshList();
    }
}

void SieveEditorMainWindow::slotSaveScript()
{
    mMainWidget->sieveEditorMainWidget()->saveScript();
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
    mSaveScript->setEnabled(hasPage);
    const bool editActionEnabled = (hasPage && mMainWidget->sieveEditorMainWidget()->pageMode() == KSieveUi::SieveEditorWidget::TextMode);
    mGoToLine->setEnabled(editActionEnabled);
    mFindAction->setEnabled(editActionEnabled);
    mReplaceAction->setEnabled(editActionEnabled);
    mUndoAction->setEnabled(editActionEnabled && mMainWidget->sieveEditorMainWidget()->isUndoAvailable());
    mRedoAction->setEnabled(editActionEnabled && mMainWidget->sieveEditorMainWidget()->isRedoAvailable());

    mCopyAction->setEnabled(editActionEnabled);
    mPasteAction->setEnabled(editActionEnabled); //FIXME
    mCutAction->setEnabled(editActionEnabled);

    mSaveScript->setEnabled(hasPage && !mNetworkIsDown);
    mRefreshList->setEnabled(!mNetworkIsDown);
}

void SieveEditorMainWindow::slotUndoAvailable(bool b)
{
    const bool hasPage = (mMainWidget->sieveEditorMainWidget()->tabWidget()->count()>0);
    const bool editActionEnabled = (hasPage && mMainWidget->sieveEditorMainWidget()->pageMode() == KSieveUi::SieveEditorWidget::TextMode);
    mUndoAction->setEnabled(editActionEnabled && b);
}

void SieveEditorMainWindow::slotRedoAvailable(bool b)
{
    const bool hasPage = (mMainWidget->sieveEditorMainWidget()->tabWidget()->count()>0);
    const bool editActionEnabled = (hasPage && mMainWidget->sieveEditorMainWidget()->pageMode() == KSieveUi::SieveEditorWidget::TextMode);
    mRedoAction->setEnabled(editActionEnabled && b);
}

void SieveEditorMainWindow::slotCopyAvailable(bool b)
{
    const bool hasPage = (mMainWidget->sieveEditorMainWidget()->tabWidget()->count()>0);
    const bool editActionEnabled = (hasPage && mMainWidget->sieveEditorMainWidget()->pageMode() == KSieveUi::SieveEditorWidget::TextMode);
    mCopyAction->setEnabled(editActionEnabled && b);
}
