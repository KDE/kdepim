/* This file is part of the KDE libraries

   Copyright (C) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>
   
   Taken in large parts from the kate highlighting list view kateschema.cpp:
   Copyright (C) 2001-2003 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002, 2003 Anders Lund <anders.lund@lund.tdcadsl.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "folderlistview.h"
#include "folderlister.h"

#include <klistview.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kdebug.h>

#include <qlistview.h>
#include <qheader.h>
#include <qpainter.h>

static const int BoxSize = 16;


//BEGIN FolderListView
FolderListView::FolderListView( QWidget *parent, const QValueList<Property> &types )
    : KListView( parent )
{
  setEnabledTypes( types );
  
  connect( this, SIGNAL(mouseButtonPressed(int, QListViewItem*, const QPoint&, int)),
           this, SLOT(slotMousePressed(int, QListViewItem*, const QPoint&, int)) );
  connect( this, SIGNAL(spacePressed(QListViewItem*)),
           this, SLOT(showPopupMenu(QListViewItem*)) );
}

void FolderListView::setEnabledTypes( const QValueList<Property> &types )
{
kdDebug() << "FolderListView::setEnabledTypes" << endl;
  for ( int i = 0; i< columns(); ++i ) removeColumn( i );
  mTypes = types;
  if ( !mTypes.contains( FolderName ) ) mTypes.prepend( FolderName );
  mColumnMap[FolderName] = addColumn( i18n("Folder") );
  mTypeMap[mColumnMap[FolderName]] = FolderName;
  

  if ( mTypes.contains( Event ) ) {
    mColumnMap[Event] = addColumn( i18n("Short column header meaning default for new events", "Events") );
    mTypeMap[mColumnMap[Event]] = Event;
  } else mColumnMap[Event] = -1;

  if ( mTypes.contains( Todo ) ) {
    mColumnMap[Todo] = addColumn( i18n("Short column header meaning default for new to-dos", "Todos") );
    mTypeMap[mColumnMap[Todo]] = Todo;
  } else mColumnMap[Todo] = -1;

  if ( mTypes.contains( Journal ) ) {
    mColumnMap[Journal] = addColumn( i18n("Short column header meaning default for new journals", "Journals") );
    mTypeMap[mColumnMap[Journal]] = Journal;
  } else mColumnMap[Journal] = -1;

  if ( mTypes.contains( Contact ) ) {
    mColumnMap[Contact] = addColumn( i18n("Short column header meaning default for new contacts", "Contacts") );
    mTypeMap[mColumnMap[Contact]] = Contact;
  } else mColumnMap[Contact] = -1;

  if ( mTypes.contains( All ) ) {
    mColumnMap[All] = addColumn( i18n("Short column header meaning default for all items", "All") );
    mTypeMap[mColumnMap[All]] = All;
  } else mColumnMap[All] = -1;

  if ( mTypes.contains( Unknown ) ) {
    mColumnMap[Unknown] = addColumn( i18n("Short column header meaning default for unknown new items", "Unknown") );
    mTypeMap[mColumnMap[Unknown]] = Unknown;
  } else mColumnMap[Unknown] = -1;
}

void FolderListView::showPopupMenu( FolderListItem *i, const QPoint &globalPos )
{
  if ( !i ) return;
  KPIM::FolderLister::Entry folder( i->folder() );

  KPopupMenu m( this );
  int id;

  m.insertTitle( folder.name, 9999 );
  id = m.insertItem( i18n("&Enabled"), this, SLOT(slotPopupHandler(int)), 0, FolderName );
  m.setItemChecked( id, i->isOn() );
  m.insertSeparator();
  
  if ( ( folder.type & KPIM::FolderLister::Event ) && (mTypes.contains( Event ) ) ) {
    id = m.insertItem( i18n("Default for new &events"), this, SLOT(slotPopupHandler(int)), 0, Event );
    m.setItemChecked( id, i->isDefault( Event ) );
  }
  if ( ( folder.type & KPIM::FolderLister::Todo ) && (mTypes.contains( Todo ) ) ) {
    id = m.insertItem( i18n("Default for new &todos"), this, SLOT(slotPopupHandler(int)), 0, Todo );
    m.setItemChecked( id, i->isDefault( Todo ) );
  }
  if ( ( folder.type & KPIM::FolderLister::Journal ) && (mTypes.contains( Journal ) ) ) {
    id = m.insertItem( i18n("Default for new &journals"), this, SLOT(slotPopupHandler(int)), 0, Journal );
    m.setItemChecked( id, i->isDefault( Journal ) );
  }
  if ( ( folder.type & KPIM::FolderLister::Contact ) && (mTypes.contains( Contact ) ) ) {
    id = m.insertItem( i18n("Default for new &contacts"), this, SLOT(slotPopupHandler(int)), 0, Contact );
    m.setItemChecked( id, i->isDefault( Contact ) );
  }
  if ( ( folder.type == KPIM::FolderLister::All ) && (mTypes.contains( All ) ) ) {
    id = m.insertItem( i18n("Default for all new &items"), this, SLOT(slotPopupHandler(int)), 0, All );
    m.setItemChecked( id, i->isDefault( All ) );
  }
  if ( ( folder.type == KPIM::FolderLister::Unknown ) && (mTypes.contains( Unknown ) ) ) {
    id = m.insertItem( i18n("Default for &unknown new &items"), this, SLOT(slotPopupHandler(int)), 0, Unknown );
    m.setItemChecked( id, i->isDefault( Unknown ) );
  }
  
  m.exec( globalPos );
}

void FolderListView::showPopupMenu( QListViewItem *i )
{
  if ( dynamic_cast<FolderListItem*>(i) )
    showPopupMenu( (FolderListItem*)i, viewport()->mapToGlobal(itemRect(i).topLeft()) );
}

void FolderListView::slotPopupHandler( int z )
{
  ((FolderListItem*)currentItem())->changeProperty( (Property)z );
}

// Because QListViewItem::activatePos() is going to become deprecated,
// and also because this attempt offers more control, I connect mousePressed to this.
void FolderListView::slotMousePressed(int btn, QListViewItem* i, const QPoint& pos, int c)
{
  if ( dynamic_cast<FolderListItem*>(i) ) {
    if ( btn == Qt::RightButton ) {
      showPopupMenu( (FolderListItem*)i, /*mapToGlobal(*/pos/*)*/ );
    }
    else if ( btn == Qt::LeftButton && c > 0 ) {
      // map pos to item/column and call FolderListItem::activate(col, pos)
      ((FolderListItem*)i)->activate( c, viewport()->mapFromGlobal( pos ) - QPoint( 0, itemRect(i).top() ) );
//     } else {
//       KListView::slotMousePressed( btn, i, pos, c );
    }
  }
}

//END

//BEGIN FolderListItem

void FolderListItem::activate( int column, const QPoint &localPos )
{
  if ( !mFolderListView ) return;
  QListView *lv = listView();
  int x = 0;
  for( int c = 0; c < column-1; c++ )
    x += lv->columnWidth( c );
  int w;
  FolderListView::Property prop( mFolderListView->typeForColumn(column) );
  switch( prop ) {
    case FolderListView::Event:
    case FolderListView::Todo:
    case FolderListView::Journal:
    case FolderListView::Contact:
    case FolderListView::All:
    case FolderListView::Unknown:
      w = BoxSize;
      break;
    default:
      return;
  }
  if ( !QRect( x, 0, w, BoxSize ).contains( localPos ) )
    changeProperty( prop );
}

void FolderListItem::changeProperty( FolderListView::Property p )
{
kdDebug() << "FolderListItem::changeProperty( " << p << ")" << endl;
  if ( p == FolderListView::FolderName ) {
kdDebug() << "it's folderName" << endl;
    setOn( !isOn() );
  } else if ( typeSupported( p ) ) {
    QListViewItemIterator it( listView() );
    while ( it.current() ) {
      FolderListItem *item = dynamic_cast<FolderListItem*>( it.current() );
      if ( item ) {
        item->setDefault( p, item==this );
      }
      ++it;
    }
  }
  listView()->triggerUpdate();
  ((FolderListView*)listView())->emitChanged();
}


bool FolderListItem::typeSupported( FolderListView::Property prop )
{
  return ( (prop==FolderListView::Event)  && (mFolder.type & KPIM::FolderLister::Event) ) ||
         ( (prop==FolderListView::Todo)   && (mFolder.type & KPIM::FolderLister::Todo) ) ||
         ( (prop==FolderListView::Journal)&& (mFolder.type & KPIM::FolderLister::Journal) ) ||
         ( (prop==FolderListView::Contact)&& (mFolder.type & KPIM::FolderLister::Contact) ) ||
         ( (prop==FolderListView::All)    && (mFolder.type == KPIM::FolderLister::All) ) ||
         ( (prop==FolderListView::Unknown)&& (mFolder.type == KPIM::FolderLister::Unknown) );
}

bool FolderListItem::isDefault( FolderListView::Property prop )
{
  if ( !typeSupported( prop ) || prop<0 || prop>=FolderListView::PROP_MAX ) return false;
  else return mIsDefault[ prop ];
}

void FolderListItem::setDefault( FolderListView::Property prop, bool def )
{
  if ( prop<0 || prop>=FolderListView::PROP_MAX ) return;
  else mIsDefault[ prop ] = def;

}


void FolderListItem::paintCell( QPainter *p, const QColorGroup &cg, int col, int width, int align )
{
  if ( !p ) return;

  QListView *lv = listView();
  Q_ASSERT( lv ); //###
  if ( !lv ) return;

  // use a private color group and set the text/highlighted text colors
//   QColorGroup mcg = lv->viewport()->colorGroup();
  FolderListView::Property prop( mFolderListView->typeForColumn(col) );

  if ( prop == FolderListView::FolderName ) {
    // col 0 is drawn by the superclass method
    super::paintCell( p, cg, col, width, align );
    return;
  } else {
    p->fillRect( 0, 0, width, height(), QBrush( cg.base() ) );
  }

  int marg = lv->itemMargin();
  QColor c;

  switch ( prop )
  {
    case FolderListView::Event:
    case FolderListView::Todo:
    case FolderListView::Journal:
    case FolderListView::Contact:
    case FolderListView::All:
    case FolderListView::Unknown:
    {
      if ( !typeSupported( prop ) ) break;
      
      int x = 0;
      int y = (height() - BoxSize) / 2;

      if ( isEnabled() )
        p->setPen( QPen( cg.text(), 1 ) );
      else
        p->setPen( QPen( lv->palette().color( QPalette::Disabled, QColorGroup::Text ), 1 ) );

      p->drawEllipse( x+marg, y+2, BoxSize-4, BoxSize-4 );
      
      if ( isDefault( prop ) ) {
        if ( isEnabled() )
          p->setBrush( cg.text() );
        else
          p->setBrush( lv->palette().color( QPalette::Disabled, QColorGroup::Text ) );
        p->drawEllipse( x + marg + 3, y + 5, BoxSize-10, BoxSize-10 );
      }
      break;
    }
    default:
      break;
  }
}
//END

#include "folderlistview.moc"
// kate: space-indent on; indent-width 2; replace-tabs on;
