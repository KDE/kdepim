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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <QLayout>
#include <kvbox.h>
//Added by qt3to4:
#include <QPixmap>
#include <QVBoxLayout>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpagewidget.h>
#include <ktoolinvocation.h>

#include "viewconfigurefieldspage.h"
#include "viewconfigurefilterpage.h"
#include "viewmanager.h"

#include "viewconfigurewidget.h"

ViewConfigureWidget::ViewConfigureWidget( KABC::AddressBook *ab, QWidget *parent )
  : KAB::ConfigureWidget( ab, parent )
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );

  mMainWidget = new KPageWidget( this );
  mMainWidget->setFaceType( KPageView::List );
  topLayout->addWidget( mMainWidget );

  // Add the first page, the attributes
  KVBox *page = addPage( i18n( "Fields" ), QString(),
                         kapp->iconLoader()->loadIcon( "view_detailed",
                         K3Icon::Panel ) );

  // Add the select fields page
  mFieldsPage = new ViewConfigureFieldsPage( addressBook(), page );

  // Add the second page, the filter selection
  page = addPage( i18n( "Default Filter" ), QString(),
                  kapp->iconLoader()->loadIcon( "filter",
                  K3Icon::Panel ) );

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

KVBox *ViewConfigureWidget::addPage( const QString &item, const QString &header,
                                   const QPixmap &pixmap )
{
  KVBox *page = new KVBox( mMainWidget );
#warning What about the page icon and page header?
  mMainWidget->addPage( page, item );
  return page;
}

ViewConfigureDialog::ViewConfigureDialog( ViewConfigureWidget *wdg, const QString &viewName,
                                          QWidget *parent, const char *name )
  : KDialog( parent ), mConfigWidget( wdg )
{
  setCaption( i18n( "Modify View: " ) + viewName );
  setButtons( Help | Ok | Cancel );
  setDefaultButton( Ok );
  showButtonSeparator( true );

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
  KToolInvocation::invokeHelp( "using-views" );
}

#include "viewconfigurewidget.moc"
