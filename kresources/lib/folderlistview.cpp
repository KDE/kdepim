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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "folderlistview.h"
#include "folderlister.h"

#include <k3listview.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmenu.h>
#include <kdebug.h>

#include <q3listview.h>
#include <q3header.h>
#include <QPainter>

static const int BoxSize = 16;


//BEGIN FolderListView
FolderListView::FolderListView( QWidget *parent, const QList<Property> &types )
    : K3ListView( parent )
{
  setEnabledTypes( types );

  connect( this, SIGNAL(mouseButtonPressed(int, Q3ListViewItem*, const QPoint&, int)),
           this, SLOT(slotMousePressed(int, Q3ListViewItem*, const QPoint&, int)) );
  connect( this, SIGNAL(spacePressed(Q3ListViewItem*)),
           this, SLOT(showPopupMenu(Q3ListViewItem*)) );
}

void FolderListView::setEnabledTypes( const QList<Property> &types )
{
kDebug() <<"FolderListView::setEnabledTypes";
  for ( int i = 0; i< columns(); ++i ) removeColumn( i );
  mTypes = types;
  if ( !mTypes.contains( FolderName ) ) mTypes.prepend( FolderName );
  mColumnMap[FolderName] = addColumn( i18n("Folder") );
  mTypeMap[mColumnMap[FolderName]] = FolderName;


  if ( mTypes.contains( Event ) ) {
    mColumnMap[Event] = addColumn( i18nc("Short column header meaning default for new events", "Events") );
    mTypeMap[mColumnMap[Event]] = Event;
  } else mColumnMap[Event] = -1;

  if ( mTypes.contains( Todo ) ) {
    mColumnMap[Todo] = addColumn( i18nc("Short column header meaning default for new to-dos", "Todos") );
    mTypeMap[mColumnMap[Todo]] = Todo;
  } else mColumnMap[Todo] = -1;

  if ( mTypes.contains( Journal ) ) {
    mColumnMap[Journal] = addColumn( i18nc("Short column header meaning default for new journals", "Journals") );
    mTypeMap[mColumnMap[Journal]] = Journal;
  } else mColumnMap[Journal] = -1;

  if ( mTypes.contains( Contact ) ) {
    mColumnMap[Contact] = addColumn( i18nc("Short column header meaning default for new contacts", "Contacts") );
    mTypeMap[mColumnMap[Contact]] = Contact;
  } else mColumnMap[Contact] = -1;

  if ( mTypes.contains( All ) ) {
    mColumnMap[All] = addColumn( i18nc("Short column header meaning default for all items", "All") );
    mTypeMap[mColumnMap[All]] = All;
  } else mColumnMap[All] = -1;

  if ( mTypes.contains( Unknown ) ) {
    mColumnMap[Unknown] = addColumn( i18nc("Short column header meaning default for unknown new items", "Unknown") );
    mTypeMap[mColumnMap[Unknown]] = Unknown;
  } else mColumnMap[Unknown] = -1;
}

void FolderListView::showPopupMenu( FolderListItem *i, const QPoint &globalPos )
{
  if ( !i ) return;
  KPIM::FolderLister::Entry folder( i->folder() );

  KMenu m( this );

  m.setTitle( folder.name);
  QAction *action = m.addAction( i18n("&Enabled"), this, SLOT(slotPopupHandler(QAction*)) );
  action->setData( FolderName );
  action->setCheckable( true );
  action->setChecked( i->isOn() );
  m.addSeparator();

  if ( ( folder.type & KPIM::FolderLister::Event ) && (mTypes.contains( Event ) ) ) {
    action = m.addAction( i18n("Default for New &Events"), this, SLOT(slotPopupHandler(QAction*)) );
    action->setData( Event );
    action->setCheckable( true );
    action->setChecked( i->isDefault( Event ) );
  }
  if ( ( folder.type & KPIM::FolderLister::Todo ) && (mTypes.contains( Todo ) ) ) {
    action = m.addAction( i18n("Default for New &Todos"), this, SLOT(slotPopupHandler(QAction*)) );
    action->setData( Todo );
    action->setCheckable( true );
    action->setChecked( i->isDefault( Todo ) );
  }
  if ( ( folder.type & KPIM::FolderLister::Journal ) && (mTypes.contains( Journal ) ) ) {
    action = m.addAction( i18n("Default for New &Journals"), this, SLOT(slotPopupHandler(QAction*)) );
    action->setData( Journal );
    action->setCheckable( true );
    action->setChecked( i->isDefault( Journal ) );
  }
  if ( ( folder.type & KPIM::FolderLister::Contact ) && (mTypes.contains( Contact ) ) ) {
    action = m.addAction( i18n("Default for New &Contacts"), this, SLOT(slotPopupHandler(QAction*)) );
    action->setData( Contact );
    action->setCheckable( true );
    action->setChecked( i->isDefault( Contact ) );
  }
  if ( ( folder.type == KPIM::FolderLister::All ) && (mTypes.contains( All ) ) ) {
    action = m.addAction( i18n("Default for All New &Items"), this, SLOT(slotPopupHandler(QAction*)) );
    action->setData( All );
    action->setCheckable( true );
    action->setChecked( i->isDefault( All ) );
  }
  if ( ( folder.type == KPIM::FolderLister::Unknown ) && (mTypes.contains( Unknown ) ) ) {
    action = m.addAction( i18n("Default for &Unknown New Items"), this, SLOT(slotPopupHandler(QAction*)) );
    action->setData( Unknown );
    action->setCheckable( true );
    action->setChecked( i->isDefault( Unknown ) );
  }

  m.exec( globalPos );
}

void FolderListView::showPopupMenu( Q3ListViewItem *i )
{
  if ( dynamic_cast<FolderListItem*>(i) )
    showPopupMenu( (FolderListItem*)i, viewport()->mapToGlobal(itemRect(i).topLeft()) );
}

void FolderListView::slotPopupHandler( QAction *action )
{
  ((FolderListItem*)currentItem())->changeProperty( (Property)action->data().toInt() );
}

// Because QListViewItem::activatePos() is going to become deprecated,
// and also because this attempt offers more control, I connect mousePressed to this.
void FolderListView::slotMousePressed(int btn, Q3ListViewItem* i, const QPoint& pos, int c)
{
  if ( dynamic_cast<FolderListItem*>(i) ) {
    if ( btn == Qt::RightButton ) {
      showPopupMenu( (FolderListItem*)i, /*mapToGlobal(*/pos/*)*/ );
    }
    else if ( btn == Qt::LeftButton && c > 0 ) {
      // map pos to item/column and call FolderListItem::activate(col, pos)
      ((FolderListItem*)i)->activate( c, viewport()->mapFromGlobal( pos ) - QPoint( 0, itemRect(i).top() ) );
//     } else {
//       K3ListView::slotMousePressed( btn, i, pos, c );
    }
  }
}

//END

//BEGIN FolderListItem

void FolderListItem::activate( int column, const QPoint &localPos )
{
  if ( !mFolderListView ) return;
  Q3ListView *lv = listView();
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
kDebug() <<"FolderListItem::changeProperty(" << p <<")";
  if ( p == FolderListView::FolderName ) {
kDebug() <<"it's folderName";
    setOn( !isOn() );
  } else if ( typeSupported( p ) ) {
    Q3ListViewItemIterator it( listView() );
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

  Q3ListView *lv = listView();
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
    p->fillRect( 0, 0, width, height(), cg.brush( QPalette::Base ) );
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
        p->setPen( QPen( cg.color( QPalette::Text ), 1 ) );
      else
        p->setPen( QPen( lv->palette().color( QPalette::Disabled, QPalette::Text ), 1 ) );

      p->drawEllipse( x+marg, y+2, BoxSize-4, BoxSize-4 );

      if ( isDefault( prop ) ) {
        if ( isEnabled() )
          p->setBrush( cg.brush( QPalette::Text ) );
        else
          p->setBrush( lv->palette().color( QPalette::Disabled, QPalette::Text ) );
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
