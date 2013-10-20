/*
  This file is part of KAddressBook.
  Copyright (c) 1996-2002 Mirko Boehm <mirko@kde.org>
                          Tobias Koenig <tokoe@kde.org>

  Copyright (c) 2009 Laurent Montel <montel@kde.org>
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "printingwizard.h"
#include "settings.h"

#include "contactselectionwidget.h"
#include "contactsorter.h"
#include "printprogress.h"
#include "printstyle.h"
#include "stylepage.h"

// including the styles
#include "detailled/detailledstyle.h"
#include "mike/mikesstyle.h"
#include "ringbinder/ringbinderstyle.h"
#include "compact/compactstyle.h"

#include <KApplication>
#include <KDebug>
#include <KDialog>
#include <KGlobal>
#include <KLocale>

#include <QPushButton>
#include <QPrinter>

using namespace KABPrinting;

PrintingWizard::PrintingWizard( QPrinter *printer, QItemSelectionModel *selectionModel,
                                QWidget *parent )
  : KAssistantDialog( parent ), mPrinter( printer ), mStyle( 0 )
{
  setCaption( i18n( "Print Contacts" ) );
  showButton( Help, false );

  mSelectionPage = new ContactSelectionWidget( selectionModel, this );
  mSelectionPage->setMessageText( i18n( "Which contacts do you want to print?" ) );

  KPageWidgetItem *mSelectionPageItem =
    new KPageWidgetItem( mSelectionPage, i18n( "Choose Contacts to Print" ) );
  addPage( mSelectionPageItem );
  setAppropriate( mSelectionPageItem, true );

  mStylePage = new StylePage( this );
  connect( mStylePage, SIGNAL(styleChanged(int)), SLOT(slotStyleSelected(int)) );
  addPage( mStylePage, i18n( "Choose Printing Style" ) );

  registerStyles();

  if ( mStyleFactories.count() > Settings::self()->printingStyle() ) {
    mStylePage->setPrintingStyle( Settings::self()->printingStyle() ); // should emit styleChanged
    slotStyleSelected( Settings::self()->printingStyle() );
  }

  mStylePage->setSortOrder( Settings::self()->sortOrder() == 0 ?
                              Qt::AscendingOrder :
                              Qt::DescendingOrder );
}

PrintingWizard::~PrintingWizard()
{
}

void PrintingWizard::setDefaultAddressBook( const Akonadi::Collection &addressBook )
{
  mSelectionPage->setDefaultAddressBook( addressBook );
}

void PrintingWizard::accept()
{
  print();
  close();
  setResult(QDialog::Accepted);
}

void PrintingWizard::registerStyles()
{
  mStyleFactories.append( new DetailledPrintStyleFactory( this ) );
  mStyleFactories.append( new MikesStyleFactory( this ) );
  mStyleFactories.append( new RingBinderPrintStyleFactory( this ) );
  mStyleFactories.append( new CompactStyleFactory( this ) );

  mStylePage->clearStyleNames();
  for ( int i = 0; i < mStyleFactories.count(); ++i ) {
    mStylePage->addStyleName( mStyleFactories.at( i )->description() );
  }
}

void PrintingWizard::slotStyleSelected( int index )
{
  if ( index < 0 || index >= mStyleFactories.count() ) {
    return;
  }

  if ( mStyle ) {
    mStyle->hidePages();
  }

  mStyle = mStyleList.value( index );
  if ( !mStyle ) {
    PrintStyleFactory *factory = mStyleFactories.at( index );
    kDebug() << "creating print style" << factory->description();

    mStyle = factory->create();
    mStyleList.insert( index, mStyle );
  }

  mStyle->showPages();

  mStylePage->setPreview( mStyle->preview() );

  mStylePage->setSortField( mStyle->preferredSortField() );
  mStylePage->setSortOrder( mStyle->preferredSortOrder() );
}

QPrinter *PrintingWizard::printer()
{
  return mPrinter;
}

int PrintingWizard::printingStyle() const
{
  return mStylePage->printingStyle();
}

int PrintingWizard::sortOrder() const
{
  return mStylePage->sortOrder();
}

void PrintingWizard::print()
{
  // create and show print progress widget:
  mProgress = new PrintProgress( this );
  KPageWidgetItem *progressItem = new KPageWidgetItem( mProgress, i18n( "Print Progress" ) );
  addPage( progressItem );
  setCurrentPage( progressItem );
  kapp->processEvents();

  KABC::Addressee::List contacts = mSelectionPage->selectedContacts();

  const ContactSorter sorter( mStylePage->sortField(), mStylePage->sortOrder() );
  sorter.sort( contacts );

  kDebug() <<"printing" << contacts.count() << "contacts.";
  // ... print:
  enableButton( KDialog::User3, false ); // back button
  enableButton( KDialog::Cancel, false );
  mStyle->print( contacts, mProgress );
}

#include "printingwizard.moc"
