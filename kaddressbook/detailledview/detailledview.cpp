/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mirko Boehm <mirko@kde.org>

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
#include <kdebug.h>

#include <qlabel.h>
#include <qlayout.h>

#include "detailledview.h"

using namespace KABC;

DetailListItem::DetailListItem( QListView *parent, const KABC::Addressee& a,
                                const KABC::Field::List &list, KABC::AddressBook *ab )
    : QListViewItem( parent ),
      mContact( a ),
      mFieldList( list ),
      mAddressBook( ab )
{
  refresh();
}

void DetailListItem::refresh()
{
  mContact = mAddressBook->findByUid( mContact.uid() );

  KABC::Field::List::Iterator it;
  int i;
  for ( i = 0, it = mFieldList.begin(); it != mFieldList.end(); ++it, ++i )
    setText( i, (*it)->value( mContact ) );
}

KABC::Addressee DetailListItem::addressee()
{
  return mContact;
}

void DetailListItem::setAddressee( const KABC::Addressee &addr )
{
  mContact = addr;
  refresh();
}

KAddressBookDetailedView::KAddressBookDetailedView(KABC::AddressBook *doc,
                                                     QWidget *parent,
                                                     const char *name)
    : KAddressBookView(doc, parent, name), showList(true), showDetails(true)
{
  initGUI();

  m_look = viewContainer->look();

  QValueList<int> sizes;
  sizes.append(1); sizes.append(1);
  splitter->setSizes(sizes);

  connect(pbHideList, SIGNAL(clicked()),
          SLOT(slotShowHideList()));
  connect(pbHideDetails, SIGNAL(clicked()),
          SLOT(slotShowHideDetails()));

  connect( viewTree, SIGNAL(doubleClicked(QListViewItem*)),
           this, SLOT(addresseeExecuted(QListViewItem*)) );
}

KAddressBookDetailedView::~KAddressBookDetailedView()
{
}

void KAddressBookDetailedView::writeConfig(KConfig *config)
{
  KAddressBookView::writeConfig(config);

  viewTree->saveLayout(config, config->group());
}

void KAddressBookDetailedView::readConfig(KConfig *config)
{
  KAddressBookView::readConfig(config);
  
  // Add the columns
  KABC::Field::List fieldList = fields();
  KABC::Field::List::ConstIterator it;

  for( it = fieldList.begin(); it != fieldList.end(); ++it )
      viewTree->addColumn( (*it)->label() );

  // Restore the layout of the listview
  viewTree->restoreLayout(config, config->group());
}

QStringList KAddressBookDetailedView::selectedUids()
{
  QStringList uids;

  DetailListItem *current= dynamic_cast<DetailListItem*>( viewTree->selectedItem() );

  if ( current )
    uids.append( current->addressee().uid() );

  return uids;
}

void KAddressBookDetailedView::incrementalSearch( const QString &value,
                                                  KABC::Field *field )
{
    if ( value.isEmpty() ) {
      viewTree->clearSelection();
      return;
    }

    KABC::Field::List fieldList = fields();
    KABC::Field::List::ConstIterator it;
    int column = 0;
    for( it = fieldList.begin(); it != fieldList.end(); ++it ) {
      if ( (*it)->equals( field ) ) break;
      ++column;
    }
    
    if ( it == fieldList.end() ) column = 0;

    // Now do the inc search
    bool block = viewTree->signalsBlocked();
    viewTree->blockSignals( true );
    viewTree->setCurrentItem( viewTree->firstChild() );
    viewTree->blockSignals( block );

    QListViewItem *item = viewTree->findItem(value, column, Qt::BeginsWith);
    if ( item ) {
        // We have a match. Deselect all the others and select this one
        viewTree->clearSelection();
        viewTree->setSelected(item, true);
        viewTree->ensureItemVisible(item);
    }
}

void KAddressBookDetailedView::refresh(QString uid)
{
  if (uid == QString::null) {
    // Clear the list view
    viewTree->clear();
    
    KABC::Addressee::List addresseeList = addressees();
    KABC::Addressee::List::Iterator it;
    for (it = addresseeList.begin(); it != addresseeList.end(); ++it ) {
      new DetailListItem( viewTree, *it, fields(), addressBook() );
    }

    // Sometimes the background pixmap gets messed up when we add lots
    // of items.
    viewTree->repaint();
  } else {
    // Only need to update on entry. Iterate through and try to find it
    DetailListItem *kabItem;
    QListViewItemIterator it( viewTree );
    while ( it.current() ) {
      kabItem = dynamic_cast<DetailListItem*>( it.current() );
      if ( kabItem && kabItem->addressee().uid() == uid ) {
        kabItem->refresh();
        return;
      }
      ++it;
    }

    refresh( QString::null );
  }
}


void KAddressBookDetailedView::setSelected(QString uid, bool selected)
{
  if ( uid == QString::null )
    return;

  QListViewItemIterator it( viewTree );
  while ( it.current() ) {
    DetailListItem* item = dynamic_cast<DetailListItem*>( it.current() );
    if ( item && item->addressee().uid() == uid )
      viewTree->setSelected( item, selected );
  }
}

void KAddressBookDetailedView::slotAddresseeSelected(const QString& uid)
{
    emit( selected( uid ) );
}

void KAddressBookDetailedView::slotModified()
{
    emit( modified() );
}

void KAddressBookDetailedView::init(KConfig* config)
{
    m_look->configure(config);
}

void KAddressBookDetailedView::slotContactSelected( QListViewItem *i )
{
  DetailListItem *item = dynamic_cast<DetailListItem*>( i );
  if ( !item )
	  return;

  KABC::Addressee addressee = item->addressee();
  m_look->setEntry( addressee );

  emit( selected( addressee.uid() ) );
}

void KAddressBookDetailedView::slotShowHideList()
{
    showList=!showList;
    if(showList)
    {
        frmListView->show();
        pbHideList->setText(i18n("Hide List"));
    } else {
        frmListView->hide();
        pbHideList->setText(i18n("Show List"));
        if(!showDetails)
        {
            slotShowHideDetails();
        }
    }
}

void KAddressBookDetailedView::slotShowHideDetails()
{
    showDetails = !showDetails;

    if( showDetails ) {
      frmDetails->show();
      pbHideDetails->setText(i18n("Hide Details"));
    } else {
      frmDetails->hide();
      pbHideDetails->setText(i18n("Show Details"));
      if( !showList )
        slotShowHideList();
    }
}

void KAddressBookDetailedView::addresseeExecuted( QListViewItem *item )
{
  if ( item ) {
    DetailListItem *kabItem = dynamic_cast<DetailListItem*>( item );
    if ( kabItem )
      emit executed( kabItem->addressee().uid() );
  } else
    emit executed( QString::null );
}

void KAddressBookDetailedView::initGUI()
{
  setCaption( i18n( "Detailled View" ) );
  QGridLayout* mainLayout = new QGridLayout( viewWidget(), 1, 1, 3, 3 ); 

  splitter = new QSplitter( viewWidget() );
  splitter->setOrientation( QSplitter::Horizontal );

  frmListView = new QFrame( splitter );
  frmListView->setFrameShape( QFrame::NoFrame );
  frmListView->setFrameShadow( QFrame::Plain );
  QVBoxLayout *frmListViewLayout = new QVBoxLayout( frmListView, 3, 3 ); 

  viewTree = new KListView( frmListView );
  viewTree->setAllColumnsShowFocus( true );
  frmListViewLayout->addWidget( viewTree );

  frmDetails = new QFrame( splitter );
  frmDetails->setFrameShape( QFrame::NoFrame );
  frmDetails->setFrameShadow( QFrame::Plain );
  QVBoxLayout *frmDetailsLayout = new QVBoxLayout( frmDetails, 3, 3 ); 

  viewContainer = new ViewContainer( frmDetails );
  frmDetailsLayout->addWidget( viewContainer );

  mainLayout->addMultiCellWidget( splitter, 1, 1, 0, 1 );

  pbHideDetails = new QPushButton( viewWidget() );
  pbHideDetails->setText( i18n( "Hide Details" ) );

  mainLayout->addWidget( pbHideDetails, 0, 1 );

  pbHideList = new QPushButton( viewWidget() );
  pbHideList->setText( i18n( "Hide List" ) );

  mainLayout->addWidget( pbHideList, 0, 0 );

  // signals and slots connections
  connect( viewTree, SIGNAL( selectionChanged(QListViewItem*) ), this, SLOT( slotContactSelected(QListViewItem*) ) );
}

#include "detailledview.moc"
