/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <zecke@handhelds.org>
† † Copyright (c) 2002,2003 Maximilian Reiﬂ <harlekin@handhelds.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qwidgetstack.h>

#include <klocale.h>
#include <kstatusbar.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kcmultidialog.h>

#include <kparts/componentfactory.h>
#include <kparts/mainwindow.h>
#include <kpopupmenu.h>

#include <syncer.h>
#include <syncuikde.h>

#include <konnectormanager.h>
#include <konnectorplugin.h>
#include <error.h>
#include <progress.h>

#include "syncconfig.h"
#include "configuredialog.h"
#include "partbar.h"
#include "profiledialog.h"

#include "konnectorbar.h"
#include "konnectordialog.h"
#include "kitchensync.h"
#include "syncalgo.h"

#include "mainwindow.h"

using namespace KSync;

MainWindow::MainWindow( QWidget *widget, const char *name )
  : KParts::MainWindow( widget, name )
{
  mView = new KitchenSync( this );
  setCentralWidget( mView );

  initActions();
  setXMLFile("ksyncgui.rc");
  createGUI( 0 );

  mView->initProfiles();
  mView->slotProfile();

  //statusBar()->insertItem(i18n("Not Connected"), 10, 0, true );
  m_konBar = new KonnectorBar( statusBar() );
  m_konBar->setName(i18n("No Konnector") );
  connect( m_konBar, SIGNAL( toggled( bool ) ),
           mView, SLOT( slotKonnectorBar( bool ) ) );
  statusBar()->addWidget( m_konBar, 0, true );
  statusBar()->show();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initActions()
{
  (void)new KAction( i18n("Synchronize" ), "reload", 0,
                     mView, SLOT( slotSync() ),
		     actionCollection(), "sync" );

  (void)new KAction( i18n("Quit"), "exit", 0,
                     mView, SLOT(slotQuit()),
		     actionCollection(), "quit" );

  (void)new KAction( i18n("Configure Profiles..."),  "configure", 0,
                     mView, SLOT(slotConfigProf() ),
                     actionCollection(), "config_profile" );

  (void)new KAction( i18n("Configure Current Profile..."),  "configure", 0,
                     mView, SLOT(slotConfigCur() ),
                     actionCollection(), "config_current" );

  m_profAct = new KSelectAction( i18n("Profile"), KShortcut(), mView,
                                 SLOT(slotProfile() ),
                                 actionCollection(), "select_prof");

  KStdAction::preferences( mView, SLOT( slotPreferences() ),
                           actionCollection() );
}

int MainWindow::currentProfile()
{
  return m_profAct->currentItem();
}

void MainWindow::setProfiles( const QStringList &profiles )
{
  m_profAct->setItems( profiles );
}

#include "mainwindow.moc"
