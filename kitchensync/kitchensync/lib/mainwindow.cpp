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
#include <konnector.h>

#include "syncconfig.h"
#include "configuredialog.h"
#include "partbar.h"
#include "profiledialog.h"

#include "konnectorbar.h"
#include "kitchensync.h"
#include "actionmanager.h"

#include "mainwindow.h"

using namespace KSync;

MainWindow::MainWindow( QWidget *widget, const char *name )
  : KParts::MainWindow( widget, name )
{
  mActionManager = new ActionManager( actionCollection() );

  mView = new KitchenSync( mActionManager, this );
  setCentralWidget( mView );

  mActionManager->setView( mView );
  mActionManager->initActions();
  KStdAction::quit( this, SLOT( close() ), actionCollection() );
  setXMLFile( "ksyncgui.rc" );
  createGUI( 0 );

  mView->initProfiles();
  mActionManager->readConfig();
  mView->activateProfile();

  m_konBar = new KonnectorBar( statusBar() );
  connect( m_konBar, SIGNAL( toggled( bool ) ),
           mView, SLOT( slotKonnectorBar( bool ) ) );
  statusBar()->addWidget( m_konBar, 0, true );
  statusBar()->show();

  setAutoSaveSettings();
}

MainWindow::~MainWindow()
{
  delete mActionManager;
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
