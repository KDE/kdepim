/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "groupconfigdialog.h"

#include "groupconfig.h"

#include <klocale.h>

#include <tqlayout.h>

GroupConfigDialog::GroupConfigDialog( TQWidget *parent, SyncProcess *process )
  : KDialogBase( parent, 0, true, i18n("Configure Synchronization Group"),
     Ok | User1 | User2, Ok )
{
  TQFrame *topFrame = makeMainWidget();

  TQBoxLayout *topLayout = new TQVBoxLayout( topFrame );

  mConfigWidget = new GroupConfig( topFrame );
  topLayout->addWidget( mConfigWidget );

  mConfigWidget->setSyncProcess( process );

  setInitialSize( configDialogSize( "size_groupconfigdialog" ) );

  enableButton( User1, false );
  setButtonText( User1, i18n( "Remove Member" ) );

  connect( mConfigWidget, TQT_SIGNAL( memberSelected( bool ) ), TQT_SLOT( memberSelected( bool ) ) );

  setButtonText( User2, i18n("Add Member...") );
}

GroupConfigDialog::~GroupConfigDialog()
{
  saveDialogSize( "size_groupconfigdialog" );
}

void GroupConfigDialog::slotOk()
{
  mConfigWidget->saveConfig();

  accept();
}

void GroupConfigDialog::slotUser1()
{
  mConfigWidget->removeMember();
}

void GroupConfigDialog::slotUser2()
{
  mConfigWidget->addMember();
}

void GroupConfigDialog::memberSelected( bool selected )
{
  enableButton( User1, selected );
}

#include "groupconfigdialog.moc"
