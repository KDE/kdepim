/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <tqvbox.h>

#include <kglobalsettings.h>

#include "kwidgetlist.h"

class KWidgetList::Private
{
  public:
    Private()
      : mSelectedItem( 0 )
    {
    }

    TQValueList<KWidgetListItem*> mItems;
    KWidgetListItem *mSelectedItem;
    TQVBox *mBox;
};

KWidgetList::KWidgetList( TQWidget *parent, const char *name )
  : TQScrollView( parent, name ),
    d( new Private )
{
  d->mBox = new TQVBox( viewport() );
  addChild( d->mBox );

  setResizePolicy( AutoOneFit );
  setFocusPolicy( TQWidget::StrongFocus );

  viewport()->setFocus();
}

KWidgetList::~KWidgetList()
{
  clear();

  delete d;
  d = 0;
}

uint KWidgetList::count() const
{
  return d->mItems.count();
}

void KWidgetList::appendItem( KWidgetListItem *item )
{
  if ( !item )
    return;

  if ( !d->mItems.contains( item ) ) {
    d->mItems.append( item );
    item->reparent( d->mBox, 0, TQPoint( 0, 0 ), true );
    item->setSelected( false );
    item->installEventFilter( this );

    if ( d->mItems.count() == 1 ) {
      d->mSelectedItem = item;
    } else {
      if ( !d->mSelectedItem )
        setSelected( item );
      else
        d->mSelectedItem->setSelected( true );
    }
  }
}

void KWidgetList::removeItem( int index )
{
  if ( index < 0 || index >= (int)d->mItems.count() )
    return;

  KWidgetListItem *item = d->mItems[ index ];
  d->mItems.remove( item );

  if ( d->mSelectedItem == item ) {
    // TODO: smarter selection
    if ( !d->mItems.isEmpty() )
      setSelected( d->mItems.first() );
    else
      d->mSelectedItem = 0;
  }

  delete item;

  if ( d->mItems.count() == 1 )
    d->mItems.first()->setSelected( false );
}

void KWidgetList::takeItem( KWidgetListItem *item )
{
  d->mItems.remove( item );
  item->reparent( 0, 0, TQPoint( 0, 0 ) );
  item->removeEventFilter( this );
  item->hide();

  if ( d->mSelectedItem == item ) {
    // TODO: smarter selection
    if ( !d->mItems.isEmpty() )
      setSelected( d->mItems.first() );
    else
      d->mSelectedItem = 0;
  }
}

void KWidgetList::setSelected( KWidgetListItem *item )
{
  if ( !item )
    return;

  if ( d->mItems.contains( item ) == 0 )
    return;

  if ( d->mSelectedItem )
    d->mSelectedItem->setSelected( false );

  item->setSelected( true );
  d->mSelectedItem = item;
}

void KWidgetList::setSelected( int index )
{
  setSelected( item( index ) );
}

bool KWidgetList::isSelected( KWidgetListItem *item ) const
{
  return ( d->mSelectedItem == item );
}

bool KWidgetList::isSelected( int index ) const
{
  return isSelected( item( index ) );
}

KWidgetListItem *KWidgetList::selectedItem() const
{
  return d->mSelectedItem;
}

KWidgetListItem *KWidgetList::item( int index ) const
{
  if ( index < 0 || index >= (int)d->mItems.count() )
    return 0;
  else
    return d->mItems[ index ];
}

int KWidgetList::index( KWidgetListItem *item ) const
{
  return d->mItems.findIndex( item );
}

void KWidgetList::clear()
{
  TQValueList<KWidgetListItem*>::Iterator it;
  for ( it = d->mItems.begin(); it != d->mItems.end(); ++it )
    delete *it;

  d->mItems.clear();

  d->mSelectedItem = 0;
}

void KWidgetList::setFocus()
{
  viewport()->setFocus();
}

bool KWidgetList::eventFilter( TQObject *object, TQEvent *event )
{
  if ( event->type() == TQEvent::MouseButtonPress ) {
    TQMouseEvent *mouseEvent = static_cast<TQMouseEvent*>( event );
    if ( mouseEvent->button() & LeftButton ) {
      TQValueList<KWidgetListItem*>::Iterator it;
      for ( it = d->mItems.begin(); it != d->mItems.end(); ++it ) {
        if ( *it == object ) {
          if ( d->mItems.count() != 1 ) {
            setSelected( *it );
            emit selectionChanged( *it );
          }
          return true;
        }
      }
    }
  } else if ( event->type() == TQEvent::MouseButtonDblClick ) {
    TQValueList<KWidgetListItem*>::Iterator it;
    for ( it = d->mItems.begin(); it != d->mItems.end(); ++it ) {
      if ( *it == object ) {
        if ( d->mItems.count() != 1 ) {
          setSelected( *it );
          emit doubleClicked( *it );
        }
        return true;
      }
    }
  } else if ( event->type() == TQEvent::KeyPress ) {
    TQKeyEvent *keyEvent = static_cast<TQKeyEvent*>( event );
    if ( keyEvent->key() == Qt::Key_Up ) {
      if ( d->mSelectedItem == 0 ) {
        if ( !d->mItems.isEmpty() ) {
          setSelected( d->mItems.first() );
          return true;
        }
      }

      for ( int i = 0; i < (int)d->mItems.count(); ++i ) {
        if ( d->mItems[ i ] == d->mSelectedItem ) {
          if ( ( i - 1 ) >= 0 ) {
            setSelected( d->mItems[ i - 1 ] );
            return true;
          }
        }
      }
      return true;
    } else if ( keyEvent->key() == Qt::Key_Down ) {
      if ( d->mSelectedItem == 0 ) {
        if ( !d->mItems.isEmpty() ) {
          setSelected( d->mItems.last() );
          return true;
        }
      }

      for ( int i = 0; i < (int)d->mItems.count(); ++i )
        if ( d->mItems[ i ] == d->mSelectedItem ) {
          if ( ( i + 1 ) < (int)d->mItems.count() ) {
            setSelected( d->mItems[ i + 1 ] );
            return true;
          }
        }
      return true;
    }
  }

  return TQScrollView::eventFilter( object, event );
}

KWidgetListItem::KWidgetListItem( KWidgetList *parent, const char *name )
  : TQWidget( parent, name )
{
  mForegroundColor = KGlobalSettings::textColor();
  mBackgroundColor = KGlobalSettings::baseColor();
  mSelectionForegroundColor = KGlobalSettings::highlightedTextColor();
  mSelectionBackgroundColor = KGlobalSettings::highlightColor();

  setFocusPolicy( TQWidget::StrongFocus );
}

KWidgetListItem::~KWidgetListItem()
{
}

void KWidgetListItem::setSelected( bool select )
{
  if ( select ) {
    setPaletteForegroundColor( mSelectionForegroundColor );
    setPaletteBackgroundColor( mSelectionBackgroundColor );
  } else {
    setPaletteForegroundColor( mForegroundColor );
    setPaletteBackgroundColor( mBackgroundColor );
  }
}

void KWidgetListItem::setForegroundColor( const TQColor &color )
{
  mForegroundColor = color;
}

void KWidgetListItem::setBackgroundColor( const TQColor &color )
{
  mBackgroundColor = color;
}

void KWidgetListItem::setSelectionForegroundColor( const TQColor &color )
{
  mSelectionForegroundColor = color;
}

void KWidgetListItem::setSelectionBackgroundColor( const TQColor &color )
{
  mSelectionBackgroundColor = color;
}

#include "kwidgetlist.moc"
