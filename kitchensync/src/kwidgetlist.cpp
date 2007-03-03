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

#include <qvbox.h>

#include <kglobalsettings.h>

#include "kwidgetlist.h"

class KWidgetList::Private
{
  public:
    Private()
      : mSelectedItem( 0 )
    {
    }

    QValueList<KWidgetListItem*> mItems;
    KWidgetListItem *mSelectedItem;
    QVBox *mBox;
};

KWidgetList::KWidgetList( QWidget *parent, const char *name )
  : QScrollView( parent, name ),
    d( new Private )
{
  d->mBox = new QVBox( viewport() );
  addChild( d->mBox );

  setResizePolicy( AutoOneFit );
  setFocusPolicy( QWidget::StrongFocus );

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
    item->reparent( d->mBox, 0, QPoint( 0, 0 ), true );
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
  item->reparent( 0, 0, QPoint( 0, 0 ) );
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
  QValueList<KWidgetListItem*>::Iterator it;
  for ( it = d->mItems.begin(); it != d->mItems.end(); ++it )
    delete *it;

  d->mItems.clear();

  d->mSelectedItem = 0;
}

void KWidgetList::setFocus()
{
  viewport()->setFocus();
}

bool KWidgetList::eventFilter( QObject *object, QEvent *event )
{
  if ( event->type() == QEvent::MouseButtonPress ) {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>( event );
    if ( mouseEvent->button() & LeftButton ) {
      QValueList<KWidgetListItem*>::Iterator it;
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
  } else if ( event->type() == QEvent::MouseButtonDblClick ) {
    QValueList<KWidgetListItem*>::Iterator it;
    for ( it = d->mItems.begin(); it != d->mItems.end(); ++it ) {
      if ( *it == object ) {
        if ( d->mItems.count() != 1 ) {
          setSelected( *it );
          emit doubleClicked( *it );
        }
        return true;
      }
    }
  } else if ( event->type() == QEvent::KeyPress ) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent*>( event );
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

  return QScrollView::eventFilter( object, event );
}

KWidgetListItem::KWidgetListItem( KWidgetList *parent, const char *name )
  : QWidget( parent, name )
{
  mForegroundColor = KGlobalSettings::textColor();
  mBackgroundColor = KGlobalSettings::baseColor();
  mSelectionForegroundColor = KGlobalSettings::highlightedTextColor();
  mSelectionBackgroundColor = KGlobalSettings::highlightColor();

  setFocusPolicy( QWidget::StrongFocus );
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

void KWidgetListItem::setForegroundColor( const QColor &color )
{
  mForegroundColor = color;
}

void KWidgetListItem::setBackgroundColor( const QColor &color )
{
  mBackgroundColor = color;
}

void KWidgetListItem::setSelectionForegroundColor( const QColor &color )
{
  mSelectionForegroundColor = color;
}

void KWidgetListItem::setSelectionBackgroundColor( const QColor &color )
{
  mSelectionBackgroundColor = color;
}

#include "kwidgetlist.moc"
