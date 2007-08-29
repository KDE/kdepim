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

#include <qvbox.h>

#include <kglobal.h>
#include <kiconloader.h>
#include <kjanuswidget.h>
#include <klocale.h>

#include "delegatepage.h"
#include "otheruserpage.h"
#include "outofofficepage.h"
#include "passwordpage.h"

#include "mainwindow.h"

MainWindow::MainWindow()
  : KMainWindow( 0 )
{
  KJanusWidget *wdg = new KJanusWidget( this, "", KJanusWidget::IconList );

  QPixmap icon = KGlobal::iconLoader()->loadIcon( "folder_yellow", KIcon::Desktop );
  QVBox *page = wdg->addVBoxPage( i18n( "Other Accounts" ), i18n( "Register other accounts" ), icon );
  new OtherUserPage( page );

  icon = KGlobal::iconLoader()->loadIcon( "edu_languages", KIcon::Desktop );
  page = wdg->addVBoxPage( i18n( "Delegates" ), i18n( "Setup delegates for my account" ), icon );
  new DelegatePage( page );

  icon = KGlobal::iconLoader()->loadIcon( "kontact_summary_green", KIcon::Desktop );
  page = wdg->addVBoxPage( i18n( "Out of Office..." ), i18n( "Setup Out of Office Message" ), icon );
  new OutOfOfficePage( page );

  icon = KGlobal::iconLoader()->loadIcon( "password", KIcon::Desktop );
  page = wdg->addVBoxPage( i18n( "Password" ), i18n( "Change the password" ), icon );
  new PasswordPage( page );

  setCentralWidget( wdg );

  resize( 540, 450 );
}
