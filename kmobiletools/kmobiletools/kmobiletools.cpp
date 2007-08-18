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

#include <KShortcutsDialog>
#include <KEditToolBar>
#include <KStatusBar>
#include <KLocale>
#include <KActionCollection>
#include <KStandardAction>
#include <KLibLoader>
#include <KMessageBox>
#include <KApplication>
#include <KToolBar>

#include <libkmobiletools/engineslist.h>

kmobiletools::kmobiletools()
    : KParts::MainWindow()
{
    setObjectName( QString("kmobiletools") );

    setupActions();
    // set the shell's ui resource file
    setXMLFile( "kmobiletoolsui.rc" );
    // then, setup our actions
    createGUI( 0 );

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

            // setup tool bar for kmobiletools core services
            connect( m_part, SIGNAL(showServiceToolBar(bool)),
                    this, SLOT(showServiceToolBar(bool)) );
            connect( m_part, SIGNAL(showDeviceToolBar(bool)),
                    this, SLOT(showDeviceToolBar(bool)) );
        }
    }
    else
    {
        // if we couldn't find our Part, we exit since the Shell by
        // itself can't do anything useful
        /// @TODO replace with ErrorHandler
        KMessageBox::error(this, i18n("Could not find our part."));
        kapp->quit();
        // we return here, cause kapp->quit() only means "exit the
        // next time we enter the event loop...
        return;
    }

    statusBar()->show();

    createGUI(m_part);
    resize(740,700);
    setAutoSaveSettings();
}

kmobiletools::~kmobiletools()
{
    delete m_part;
    kDebug() <<"kmobiletools::~kmobiletools()";
}

void kmobiletools::setupActions()
{
    KStandardAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
    KStandardAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
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

void kmobiletools::showServiceToolBar( bool showToolBar )
{
    toolBar( "serviceToolBar" )->setVisible( showToolBar );
}

void kmobiletools::showDeviceToolBar( bool showToolBar )
{
    toolBar( "deviceToolBar" )->setVisible( showToolBar );
}

void kmobiletools::applyNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config()->group("MainWindow"));
}

bool kmobiletools::queryClose()
{
    if( KMobileTools::EnginesList::instance()->closing() )
    {
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
