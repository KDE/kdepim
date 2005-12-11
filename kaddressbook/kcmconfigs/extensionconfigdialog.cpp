/*
    This file is part of KAddressBook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlayout.h>
//Added by qt3to4:
#include <QFrame>
#include <QGridLayout>

#include <klocale.h>

#include "configurewidget.h"
#include "extensionconfigdialog.h"
#include "extensionwidget.h"

ExtensionConfigDialog::ExtensionConfigDialog( KAB::ExtensionFactory *factory, KConfig *config,
                                              QWidget *parent, const char *name )
  : KDialogBase( Plain, i18n( "Extension Settings" ), Ok | Cancel, Ok, parent,
                 name, true, true ), mWidget( 0 ), mConfig( config )
{
  QFrame *page = plainPage();
  QGridLayout *layout = new QGridLayout( page, 1, 1, marginHint(), spacingHint() );

  mWidget = factory->configureWidget( page, "ExtensionConfigWidget" );
  layout->addWidget( mWidget, 0, 0 );

  mWidget->restoreSettings( mConfig );
}

ExtensionConfigDialog::~ExtensionConfigDialog()
{
}

void ExtensionConfigDialog::slotOk()
{
  mWidget->saveSettings( mConfig );

  KDialogBase::slotOk();
}

#include "extensionconfigdialog.moc"
