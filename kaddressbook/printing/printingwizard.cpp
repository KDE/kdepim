/*                                                                      
    This file is part of KAddressBook.
    Copyright (c) 1996-2002 Mirko Boehm <mirko@kde.org>
                            Tobias Koenig <tokoe@kde.org>
                                                                        
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include <qradiobutton.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qheader.h>

#include <kdebug.h>
#include <kprinter.h>
#include <klocale.h>
#include <kdialog.h>
#include <kdialogbase.h>
#include <kapplication.h>
#include <kabc/addresseelist.h>

// including the styles
#include "detailledstyle.h"
#include "mikesstyle.h"

#include "printingwizard.h"
#include "printstyle.h"
#include "printprogress.h"
#include "../kabprefs.h"

using namespace KABPrinting;

PrintingWizardImpl::PrintingWizardImpl( KPrinter *printer, KABC::AddressBook* doc,
                                        const QStringList& selection,
                                        QWidget *parent, const char* name )
  : PrintingWizard( printer, doc, selection, parent, name ), mStyle( 0 )
{
  mSelectionPage = new SelectionPage( this );
  mSelectionPage->setUseSelection( !selection.isEmpty() );
  insertPage( mSelectionPage, i18n("Chose Contacts to Print"), -1 );

  mFilters = Filter::restore( kapp->config(), "Filter" );
  QStringList filters;
  for ( Filter::List::iterator it = mFilters.begin(); it != mFilters.end(); ++it )
    filters.append( (*it).name() );

  mSelectionPage->setFilters( filters );

  mSelectionPage->setCategories( KABPrefs::instance()->mCustomCategories );

  setAppropriate( mSelectionPage, true );


  mStylePage = new StylePage( doc, this );
  connect( mStylePage, SIGNAL( styleChanged(int) ), SLOT( slotStyleSelected(int) ) );
  insertPage( mStylePage, i18n("Chose Printing Style"), -1 );

  registerStyles();

  if ( mStyleFactories.count() > 0 )
    slotStyleSelected( 0 );
}

PrintingWizardImpl::~PrintingWizardImpl()
{
}

void PrintingWizardImpl::accept()
{
  print();
  close();
}

void PrintingWizardImpl::registerStyles()
{
  mStyleFactories.append( new DetailledPrintStyleFactory( this ) );
  mStyleFactories.append( new MikesStyleFactory( this ) );

  mStylePage->clearStyleNames();
  for ( uint i = 0; i < mStyleFactories.count(); ++i )
    mStylePage->addStyleName( mStyleFactories.at( i )->description() );
}

void PrintingWizardImpl::slotStyleSelected( int index )
{
  if ( index < 0 || (uint)index >= mStyleFactories.count() )
    return;

  setFinishEnabled( mStylePage, false );

  if ( mStyle )
    mStyle->hidePages();

  if ( mStyleList.at( index ) != 0 )
    mStyle = mStyleList.at( index );
  else {
    PrintStyleFactory *factory = mStyleFactories.at( index );
    kdDebug(5720) << "PrintingWizardImpl::slotStyleSelected: "
                  << "creating print style "
                  << factory->description() << endl;
    mStyle = factory->create();
    mStyleList.insert( index, mStyle );
  }

  mStyle->showPages();

  mStylePage->setPreview( mStyle->preview() );

  setFinishEnabled( page( pageCount() - 1 ), true );

  if ( mStyle->preferredSortField() != 0 ) {
    mStylePage->setSortField( mStyle->preferredSortField() );
    mStylePage->setSortAscending( mStyle->preferredSortType() );
  }
}

KABC::AddressBook* PrintingWizardImpl::document()
{
  return mDocument;
}

KPrinter* PrintingWizardImpl::printer()
{
  return mPrinter;
}

void PrintingWizardImpl::print()
{
  // create and show print progress widget:
  PrintProgress *progress = new PrintProgress( this );
  insertPage( progress, i18n( "Print Progress" ), -1 );
  showPage( progress );
  kapp->processEvents();

  // prepare list of contacts to print:

  KABC::AddresseeList list;
  if ( mStyle != 0 ) {
    if ( mSelectionPage->useSelection() ) {
      QStringList::Iterator it;
      for ( it = mSelection.begin(); it != mSelection.end(); ++it ) {
        KABC::Addressee addr = document()->findByUid( *it );
        if ( !addr.isEmpty() )
          list.append( addr );
      }
    } else if ( mSelectionPage->useFilters() ) {
      // find contacts that can pass selected filter
      Filter::List::Iterator filterIt;
      for ( filterIt = mFilters.begin(); filterIt != mFilters.end(); ++filterIt )
        if ( (*filterIt).name() == mSelectionPage->filter() )
          break;

      KABC::AddressBook::Iterator it;
      for ( it = document()->begin(); it != document()->end(); ++it ) {
        if ( (*filterIt).filterAddressee( *it ) )
          list.append( *it );
      }
                
    } else if ( mSelectionPage->useCategories() ) {
      QStringList categories = mSelectionPage->categories();
      KABC::AddressBook::Iterator it;
      for ( it = document()->begin(); it != document()->end(); ++it ) {
        QStringList tmp( (*it).categories() );
        QStringList::Iterator tmpIt;
        for ( tmpIt = tmp.begin(); tmpIt != tmp.end(); ++tmpIt )
          if ( categories.contains( *tmpIt ) ) {
            list.append( *it );
            break;
          }
      }
    } else {
      // create a string list of all entries:
      KABC::AddressBook::Iterator it;
      for( it = document()->begin(); it != document()->end(); ++it )
        list.append( *it );
    }

    list.setReverseSorting( !mStylePage->sortAscending() );
    list.sortByField( mStylePage->sortField() );
  }

  kdDebug(5720) << "PrintingWizardImpl::print: printing "
                << list.count() << " contacts." << endl;

  // ... print:
  setBackEnabled( progress, false );
  cancelButton()->setEnabled( false );
  mStyle->print( list, progress );
}

#include "printingwizard.moc"
