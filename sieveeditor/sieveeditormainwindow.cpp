/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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
#include "addsieveserverdialog.h"

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
    updateActions();
    readConfig();
    mMainWidget = new SieveEditorMainWidget;
    setCentralWidget(mMainWidget);
}

SieveEditorMainWindow::~SieveEditorMainWindow()
{
    KSharedConfig::Ptr config = KGlobal::config();

    KConfigGroup group = config->group( QLatin1String("SieveEditorMainWindow") );
    group.writeEntry( "Size", size() );
}

void SieveEditorMainWindow::readConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = KConfigGroup( config, "SieveEditorMainWindow" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(600,400) );
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
    //TODO
}

void SieveEditorMainWindow::updateActions()
{
    //TODO
}

void SieveEditorMainWindow::slotQuitApp()
{
    kapp->quit();
}

void SieveEditorMainWindow::slotConfigure()
{
    QPointer<SieveEditorConfigureDialog> dlg = new SieveEditorConfigureDialog(this);
    if (dlg->exec()) {
        //TODO
    }
    delete dlg;
}

void SieveEditorMainWindow::slotAddServerSieve()
{
    QPointer<AddSieveServerDialog> dlg = new AddSieveServerDialog(this);
    if (dlg->exec()) {
        //TODO
    }
    delete dlg;
}
