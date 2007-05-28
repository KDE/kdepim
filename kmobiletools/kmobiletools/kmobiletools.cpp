/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#include "kmobiletools.h"

#include <kfiledialog.h>
#include <kconfig.h>
#include <kurl.h>
#include <kshortcutsdialog.h>

#include <kedittoolbar.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <ktoggleaction.h>

#include <klibloader.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kparts/partmanager.h>
#include <ktoolbar.h>

#include <libkmobiletools/engineslist.h>

kmobiletools::kmobiletools()
    : KParts::MainWindow()
{
    setObjectName(QLatin1String("kmobiletools"));

    setupActions();
    // set the shell's ui resource file
    setXMLFile("kmobiletoolsui.rc");
    // then, setup our actions
    createGUI(0);
    // and a status bar

    // this routine will find and load our Part.  it finds the Part by
    // name which is a bad idea usually.. but it's alright in this
    // case since our Part is made for this Shell
    KLibFactory *factory = KLibLoader::self()->factory("libkmobiletoolsmainpart");
    if (factory)
    {
        // now that the Part is loaded, we cast it to a Part to get
        // our hands on it
        m_part = static_cast<KParts::Part *>(factory->create(this, "KParts::Part"));

        if (m_part)
        {
            // tell the KParts::MainWindow that this is indeed the main widget
            setCentralWidget(m_part->widget());

            // and integrate the part's GUI with the shell's
        }
    }
    else
    {
        // if we couldn't find our Part, we exit since the Shell by
        // itself can't do anything useful
        KMessageBox::error(this, i18n("Could not find our part."));
        kapp->quit();
        // we return here, cause kapp->quit() only means "exit the
        // next time we enter the event loop...
        return;
    }

    // apply the saved mainwindow settings, if any, and ask the mainwindow
    // to automatically save settings if changed: window size, toolbar
    // position, icon size, etc.
    statusBar()->show();

//     optionsShowToolbar();
//     setupGUI();
    createGUI(m_part);
    resize(740,700);
    setAutoSaveSettings();
//     connect((static_cast<kmobiletoolsMainPart *>(m_part))->partmanager(), SIGNAL(activePartChanged(KParts::Part *)), this, SLOT(slotUpdateToolbars(KParts::Part *) ));
//     connect((static_cast<kmobiletoolsMainPart *>(m_part))->partmanager(), SIGNAL(activePartChanged(KParts::Part *)), this, SLOT(slotUpdateToolbars() ));
}

kmobiletools::~kmobiletools()
{
    kDebug() << "kmobiletools::~kmobiletools()\n";
}

void kmobiletools::slotUpdateToolbars(KParts::Part *)
{
//     KParts::MainWindow::m_activePart=0;
//     createGUI(new_part);
//     createGUI(m_part);
//     insertChildClient(m_part);

}

void kmobiletools::slotQuit()
{
    delete m_part;
    kapp->quit();
}

void kmobiletools::setupActions()
{
//     KStandardAction::quit(kapp, SLOT(quit()), actionCollection());

//     m_toolbarAction = KStandardAction::showToolbar(this, SLOT(optionsShowToolbar()), actionCollection());
//     m_statusbarAction = KStandardAction::showStatusbar(this, SLOT(optionsShowStatusbar()), actionCollection());

    KStandardAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
    KStandardAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
}

void kmobiletools::saveProperties(KConfig* /*config*/)
{
    // the 'config' object points to the session managed
    // config file.  anything you write here will be available
    // later when this app is restored
}

void kmobiletools::readProperties(KConfig* /*config*/)
{
    // the 'config' object points to the session managed
    // config file.  this function is automatically called whenever
    // the app is being restored.  read in here whatever you wrote
    // in 'saveProperties'
}

void kmobiletools::optionsShowToolbar()
{
    // this is all very cut and paste code for showing/hiding the
    // toolbar
    if (m_toolbarAction->isChecked())
        toolBar()->show();
    else
        toolBar()->hide();
}

void kmobiletools::optionsShowStatusbar()
{
    // this is all very cut and paste code for showing/hiding the
    // statusbar
    if (m_statusbarAction->isChecked())
        statusBar()->show();
    else
        statusBar()->hide();
}

void kmobiletools::optionsConfigureKeys()
{
    KShortcutsDialog::configure(actionCollection());
}

void kmobiletools::optionsConfigureToolbars()
{
    saveMainWindowSettings(KGlobal::config()->group("MainWindow"));

    // use the standard toolbar editor
    KEditToolBar dlg(factory());
    connect(&dlg, SIGNAL(newToolBarConfig()),
             this, SLOT(applyNewToolbarConfig()));
    dlg.exec();
}

void kmobiletools::applyNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config()->group("MainWindow"));
}

bool kmobiletools::queryClose()
{
    if( KMobileTools::EnginesList::instance()->closing() )
    {
//         delete d_gsm;
        return true;
    }
    hide();
    // Message modified from kopete...
    KMessageBox::information( this,
      i18n( "<qt>Closing the main window will keep KMobileTools running in the "
      "system tray. Use 'Quit' from the system tray menu to quit the application.</qt>" ),
      i18n( "Docking in System Tray" ), "hideOnCloseInfo");

    return false;
}

#include "kmobiletools.moc"
