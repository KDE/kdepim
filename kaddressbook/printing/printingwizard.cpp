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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "printingwizard.h"

#include <QtGui/QPushButton>
#include <QtGui/QPrinter>

#include <kabc/addresseelist.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>

#include <akonadi/descendantsproxymodel.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/entityfilterproxymodel.h>

#include "globalcontactmodel.h"
#include "contactstreemodel.h"
#include <akonadi_next/entitytreeview.h>

// including the styles
#include "detailledstyle.h"
#include "mikesstyle.h"
#include "ringbinderstyle.h"

#include "printprogress.h"
#include "printstyle.h"
#include "printsortmode.h"

using namespace KABPrinting;

PrintingWizard::PrintingWizard( QPrinter *printer, Akonadi::EntityTreeView *itemView, QWidget *parent )
  : KAssistantDialog( parent ), mPrinter( printer ), mItemView(itemView), mStyle( 0 )
{
  mSelectionPage = new SelectionPage( this );
  mSelectionPage->setUseSelection( mItemView->selectionModel()->hasSelection() );
  KPageWidgetItem *mSelectionPageItem = new KPageWidgetItem( mSelectionPage, i18n("Choose Contacts to Print") );
  addPage( mSelectionPageItem );

  setAppropriate( mSelectionPageItem, true );


  mStylePage = new StylePage( this );
  connect( mStylePage, SIGNAL( styleChanged(int) ), SLOT( slotStyleSelected(int) ) );
  addPage( mStylePage, i18n("Choose Printing Style") );

  registerStyles();

  if ( mStyleFactories.count() > 0 )
    slotStyleSelected( 0 );
}

PrintingWizard::~PrintingWizard()
{
}

void PrintingWizard::accept()
{
  print();
  close();
}

void PrintingWizard::registerStyles()
{
  mStyleFactories.append( new DetailledPrintStyleFactory( this ) );
  mStyleFactories.append( new MikesStyleFactory( this ) );
  mStyleFactories.append( new RingBinderPrintStyleFactory( this ) );

  mStylePage->clearStyleNames();
  for ( int i = 0; i < mStyleFactories.count(); ++i )
    mStylePage->addStyleName( mStyleFactories.at( i )->description() );
}

void PrintingWizard::slotStyleSelected( int index )
{
  if ( index < 0 || index >= mStyleFactories.count() )
    return;

  //enableButton( KDialog::User1, false ); // finish button

  if ( mStyle )
    mStyle->hidePages();

  mStyle = mStyleList.value( index );
  if ( !mStyle ) {
    PrintStyleFactory *factory = mStyleFactories.at( index );
    kDebug(5720) <<"PrintingWizardImpl::slotStyleSelected:"
                  << "creating print style"
                  << factory->description();
    mStyle = factory->create();
    mStyleList.insert( index, mStyle );
  }

  mStyle->showPages();

  mStylePage->setPreview( mStyle->preview() );

  //setFinishEnabled( page( pageCount() - 1 ), true );

  if ( mStyle->preferredSortField() != 0 ) {
#warning "port me"
    mStylePage->setSortField( mStyle->preferredSortField() );
#if 0
    mStylePage->setSortAscending( mStyle->preferredSortType() );
#endif
  }
}

QPrinter* PrintingWizard::printer()
{
  return mPrinter;
}

void PrintingWizard::print()
{
  // create and show print progress widget:
  PrintProgress *progress = new PrintProgress( this );
  KPageWidgetItem *progressItem = new KPageWidgetItem( progress, i18n( "Print Progress" ) );
  addPage( progressItem );
  setCurrentPage( progressItem );
  kapp->processEvents();

  // prepare list of contacts to print:
  QAbstractItemModel* model = mItemView->model();
  Q_ASSERT(model);

  KABC::AddresseeList list;
  if ( mStyle != 0 ) {
    if ( mSelectionPage->useSelection() ) {
      foreach ( const QModelIndex &index, mItemView->selectionModel()->selectedRows() ) {
        const Akonadi::Item item = model->data( index, Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
        Q_ASSERT( item.isValid() );
        const KABC::Addressee adr = item.payload<KABC::Addressee>();
        list.append( adr );
      }
    } else if ( mSelectionPage->useFilters() ) {
      //TODO ? or remove it it's not necessary
    } else if ( mSelectionPage->useCategories() ) {
#if 0
      QStringList categories = mSelectionPage->categories();
      KABC::AddressBook::ConstIterator it;
      for ( it = addressBook()->constBegin(); it != addressBook()->constEnd(); ++it ) {
        const QStringList tmp( (*it).categories() );
        QStringList::ConstIterator tmpIt;
        for ( tmpIt = tmp.constBegin(); tmpIt != tmp.constEnd(); ++tmpIt )
          if ( categories.contains( *tmpIt ) ) {
            list.append( *it );
            break;
          }
      }
#else
      Q_ASSERT(false);
#endif
    } else {
      Akonadi::ContactsTreeModel *contactsModel = GlobalContactModel::instance()->model();

      Akonadi::DescendantsProxyModel *descendantTree = new Akonadi::DescendantsProxyModel( this );
      descendantTree->setSourceModel( contactsModel );

      Akonadi::EntityFilterProxyModel *allContacts = new Akonadi::EntityFilterProxyModel( this );
      allContacts->setSourceModel( descendantTree );
      allContacts->addMimeTypeInclusionFilter( KABC::Addressee::mimeType() );

      // create a string list of all entries:
      for ( int row = 0; row < allContacts->rowCount(); ++row ) {
        const QModelIndex index = allContacts->index( row, 0 );
        const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
        Q_ASSERT( item.isValid() );
        const KABC::Addressee contact = item.payload<KABC::Addressee>();
        list.append( contact );
      }
    }

    list.setReverseSorting( !mStylePage->sortAscending() );
    PrintSortMode sortMode( mStylePage->sortField() );
    list.sortByMode( &sortMode );
  }

  kDebug(5720) <<"PrintingWizardImpl::print: printing"
                << list.count() << "contacts.";
  // ... print:
  enableButton( KDialog::User3, false ); // back button
  enableButton( KDialog::Cancel, false );
  mStyle->print( list, progress );
}

#include "printingwizard.moc"
