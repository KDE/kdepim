/*
 *   This file is part of ScalixAdmin.
 *
 *   Copyright (C) 2007 Trolltech ASA. All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <kicon.h>
#include <klocale.h>
#include <kpagewidget.h>

#include "delegatepage.h"
#include "otheruserpage.h"
#include "outofofficepage.h"
#include "passwordpage.h"

#include "mainwindow.h"

MainWindow::MainWindow()
  : KMainWindow( 0 )
{
  KPageWidget *wdg = new KPageWidget( this );
  wdg->setFaceType( KPageWidget::List );

  KPageWidgetItem *item = new KPageWidgetItem( new OtherUserPage( wdg ), i18n( "Other Accounts" ) );
  item->setHeader( i18n( "Register other accounts" ) );
  item->setIcon( KIcon( "folder-yellow" ) );
  wdg->addPage( item );

  item = new KPageWidgetItem( new DelegatePage( wdg ), i18n( "Delegates" ) );
  item->setHeader( i18n( "Setup delegates for my account" ) );
  item->setIcon( KIcon( "contact" ) );
  wdg->addPage( item );

  item = new KPageWidgetItem( new OutOfOfficePage( wdg ), i18n( "Out of Office..." ) );
  item->setHeader( i18n( "Setup Out of Office Message" ) );
  item->setIcon( KIcon( "applications-office" ) );
  wdg->addPage( item );

  item = new KPageWidgetItem( new PasswordPage( wdg ), i18n( "Password" ) );
  item->setHeader( i18n( "Change the password" ) );
  item->setIcon( KIcon( "dialog-password" ) );
  wdg->addPage( item );

  setCentralWidget( wdg );

  resize( 540, 450 );
}
