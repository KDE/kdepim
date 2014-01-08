/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include <KSharedConfig>
#include <KGlobal>
#include <KLocalizedString>
#include <KConfigGroup>
#include <KStandardAction>
#include <KApplication>
#include <KActionCollection>
#include <KAction>

#include <QPointer>

SieveEditorMainWindow::SieveEditorMainWindow()
    : KXmlGuiWindow()
{    
    setupActions();
    setupGUI();
    readConfig();
    mMainWidget = new SieveEditorMainWidget;
    connect(mMainWidget, SIGNAL(updateButtons(bool,bool,bool,bool)), this, SLOT(slotUpdateButtons(bool,bool,bool,bool)));
    setCentralWidget(mMainWidget);
}

SieveEditorMainWindow::~SieveEditorMainWindow()
{
    KSharedConfig::Ptr config = KGlobal::config();

    KConfigGroup group = config->group( QLatin1String("SieveEditorMainWindow") );
    group.writeEntry( "Size", size() );
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
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = KConfigGroup( config, "SieveEditorMainWindow" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(800,600) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
}

void SieveEditorMainWindow::setupActions()
{
    KActionCollection* ac=actionCollection();

    KStandardAction::quit(this, SLOT(slotQuitApp()), ac );
    KStandardAction::preferences( this, SLOT(slotConfigure()), ac );

    KAction *act = ac->addAction(QLatin1String("add_server_sieve"), this, SLOT(slotAddServerSieve()));
    act->setText(i18n("Add Server Sieve..."));

    mDeleteScript = ac->addAction(QLatin1String("delete_script"), this, SLOT(slotDeleteScript()));
    mDeleteScript->setText(i18n("Delete Script"));
    mDeleteScript->setEnabled(false);

    mNewScript = ac->addAction(QLatin1String("create_new_script"), this, SLOT(slotCreateNewScript()));
    mNewScript->setText(i18n("Create New Script..."));
    mNewScript->setEnabled(false);

    mEditScript = ac->addAction(QLatin1String("edit_script"), this, SLOT(slotEditScript()));
    mEditScript->setText(i18n("Edit Script"));
    mEditScript->setEnabled(false);

    mDesactivateScript = ac->addAction(QLatin1String("desactivate_script"), this, SLOT(slotDesactivateScript()));
    mDesactivateScript->setText(i18n("Desactivate Script"));
    mDesactivateScript->setEnabled(false);

    mRefreshList = ac->addAction(QLatin1String("refresh_list"), this, SLOT(slotRefreshList()));
    mRefreshList->setText(i18n("Refresh List"));
    mRefreshList->setShortcut(QKeySequence( Qt::Key_F5 ));
}

void SieveEditorMainWindow::slotRefreshList()
{
    mMainWidget->refreshList();
}

void SieveEditorMainWindow::slotDesactivateScript()
{
    mMainWidget->desactivateScript();
}

void SieveEditorMainWindow::slotEditScript()
{
    mMainWidget->editScript();
}

void SieveEditorMainWindow::slotCreateNewScript()
{
    mMainWidget->createNewScript();
}

void SieveEditorMainWindow::slotDeleteScript()
{
    mMainWidget->deleteScript();
}

void SieveEditorMainWindow::slotQuitApp()
{
    close();
}

void SieveEditorMainWindow::slotConfigure()
{
    QPointer<SieveEditorConfigureDialog> dlg = new SieveEditorConfigureDialog(this);
    if (dlg->exec()) {
        dlg->saveServerSieveConfig();
        mMainWidget->updateServerList();
    }
    delete dlg;
}

void SieveEditorMainWindow::slotAddServerSieve()
{
    QPointer<ServerSieveSettingsDialog> dlg = new ServerSieveSettingsDialog(this);
    if (dlg->exec()) {
        mMainWidget->updateServerList();
    }
    delete dlg;
}
