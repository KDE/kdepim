/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlayout.h>
#include <qvbox.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>

#include "viewconfigurefieldspage.h"
#include "viewconfigurefilterpage.h"
#include "viewmanager.h"

#include "viewconfigurewidget.h"

ViewConfigureWidget::ViewConfigureWidget( KABC::AddressBook *ab, QWidget *parent,
                                          const char *name )
  : KAB::ConfigureWidget( ab, parent, name )
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );

  mMainWidget = new KJanusWidget( this, "JanusWidget", KJanusWidget::IconList );
  topLayout->addWidget( mMainWidget );

  // Add the first page, the attributes
  QVBox *page = addPage( i18n( "Fields" ), QString::null,
                         KGlobal::iconLoader()->loadIcon( "view_detailed",
                         KIcon::Panel ) );

  // Add the select fields page
  mFieldsPage = new ViewConfigureFieldsPage( addressBook(), page );

  // Add the second page, the filter selection
  page = addPage( i18n( "Default Filter" ), QString::null,
                  KGlobal::iconLoader()->loadIcon( "filter",
                  KIcon::Panel ) );

  mFilterPage = new ViewConfigureFilterPage( page );
}

ViewConfigureWidget::~ViewConfigureWidget()
{
}

void ViewConfigureWidget::restoreSettings( KConfig *config )
{
  mFieldsPage->restoreSettings( config );
  mFilterPage->restoreSettings( config );
}

void ViewConfigureWidget::saveSettings( KConfig *config )
{
  mFieldsPage->saveSettings( config );
  mFilterPage->saveSettings( config );
}

QVBox *ViewConfigureWidget::addPage( const QString &item, const QString &header,
                                   const QPixmap &pixmap )
{
  return mMainWidget->addVBoxPage( item, header, pixmap );
}

ViewConfigureDialog::ViewConfigureDialog( ViewConfigureWidget *wdg, const QString &viewName,
                                          QWidget *parent, const char *name )
  : KDialogBase( Swallow, i18n( "Modify View: " ) + viewName, Help | Ok | Cancel,
                 Ok, parent, name, true, true ), mConfigWidget( wdg )
{
  setMainWidget( mConfigWidget );

  resize( 600, 300 );
}

ViewConfigureDialog::~ViewConfigureDialog()
{
}

void ViewConfigureDialog::restoreSettings( KConfig *config )
{
  mConfigWidget->restoreSettings( config );
}

void ViewConfigureDialog::saveSettings( KConfig *config )
{
  mConfigWidget->saveSettings( config );
}

void ViewConfigureDialog::slotHelp()
{
  kapp->invokeHelp( "using-views" );
}

#include "viewconfigurewidget.moc"
