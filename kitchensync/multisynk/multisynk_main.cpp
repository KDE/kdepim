/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#include <klocale.h>
#include <kstdaction.h>

#include "mainwidget.h"

#include "multisynk_main.h"

MainWindow::MainWindow()
  : KMainWindow( 0 )
{
  setWFlags( getWFlags() | WGroupLeader );

  setCaption( i18n( "PIM Synchronization" ) );

  mWidget = new MainWidget( this, this, "MainWidget" );

  setCentralWidget( mWidget );

  initActions();

  createGUI( "multisynkui.rc", false );
  createGUI( 0 );

  resize( 400, 300 ); // initial size
  setAutoSaveSettings();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initActions()
{
  KStdAction::quit( this, SLOT( close() ), actionCollection() );
}

#include "multisynk_main.moc"
