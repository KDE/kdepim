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

#include <QtCore/QEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QVBoxLayout>

#include <kcolorscheme.h>

#include "kwidgetlist.h"

class KWidgetList::Private
{
  public:
    Private()
      : mSelectedItem( 0 )
    {
    }

    QList<KWidgetListItem*> mItems;
    KWidgetListItem *mSelectedItem;
    QWidget *mBox;
    QVBoxLayout *mBoxLayout;
};

KWidgetList::KWidgetList( QWidget *parent )
  : QScrollArea( parent ),
    d( new Private )
{
  d->mBox = new QWidget();
  d->mBoxLayout = new QVBoxLayout( d->mBox );
  d->mBoxLayout->setMargin( 0 );
  d->mBoxLayout->setSpacing( 0 );

  setWidget( d->mBox );
  setWidgetResizable( true );
  setFocusPolicy( Qt::StrongFocus );

  setFocusProxy( d->mBox );
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
  if ( !item ) {
    return;
  }

  if ( !d->mItems.contains( item ) ) {
    d->mItems.append( item );
    item->setParent( d->mBox );
    item->setSelected( false );
    item->installEventFilter( this );
    d->mBoxLayout->addWidget( item );

    if ( d->mItems.count() == 1 ) {
      d->mSelectedItem = item;
    } else {
      if ( !d->mSelectedItem ) {
        setSelected( item );
      } else {
        d->mSelectedItem->setSelected( true );
      }
    }
  }
}

void KWidgetList::removeItem( int index )
{
  if ( index < 0 || index >= (int)d->mItems.count() ) {
    return;
  }

  KWidgetListItem *item = d->mItems[ index ];
  d->mItems.removeAll( item );

  if ( d->mSelectedItem == item ) {
    // TODO: smarter selection
    if ( !d->mItems.isEmpty() ) {
      setSelected( d->mItems.first() );
    } else {
      d->mSelectedItem = 0;
    }
  }

  delete item;

  if ( d->mItems.count() == 1 ) {
    d->mItems.first()->setSelected( false );
  }
}

void KWidgetList::takeItem( KWidgetListItem *item )
{
  d->mItems.removeAll( item );
  item->setParent( 0 );
  item->removeEventFilter( this );
  item->hide();

  if ( d->mSelectedItem == item ) {
    // TODO: smarter selection
    if ( !d->mItems.isEmpty() ) {
      setSelected( d->mItems.first() );
    } else {
      d->mSelectedItem = 0;
    }
  }
}

void KWidgetList::setSelected( KWidgetListItem *item )
{
  if ( !item ) {
    return;
  }

  if ( d->mItems.contains( item ) == 0 ) {
    return;
  }

  if ( d->mSelectedItem ) {
    d->mSelectedItem->setSelected( false );
  }

  item->setSelected( true );
  d->mSelectedItem = item;
}

void KWidgetList::setSelected( int index )
{
  setSelected( item( index ) );
}

bool KWidgetList::isSelected( KWidgetListItem *item ) const
{
  return d->mSelectedItem == item;
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
  if ( index < 0 || index >= (int)d->mItems.count() ) {
    return 0;
  } else {
    return d->mItems[ index ];
  }
}

int KWidgetList::index( KWidgetListItem *item ) const
{
  return d->mItems.indexOf( item );
}

void KWidgetList::clear()
{
  for ( int i = 0; i < d->mItems.count(); ++i ) {
    delete d->mItems[ i ];
  }

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
    if ( mouseEvent->button() & Qt::LeftButton ) {
      for ( int i = 0; i < d->mItems.count(); ++i ) {
        KWidgetListItem *item = d->mItems[ i ];
        if ( item == object ) {
          if ( d->mItems.count() != 1 ) {
            setSelected( item );
            emit selectionChanged( item );
          }
          return true;
        }
      }
    }
  } else if ( event->type() == QEvent::MouseButtonDblClick ) {
    for ( int i = 0; i < d->mItems.count(); ++i ) {
      KWidgetListItem *item = d->mItems[ i ];
      if ( item == object ) {
        if ( d->mItems.count() != 1 ) {
          setSelected( item );
          emit doubleClicked( item );
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
          ensureVisible( d->mItems.first()->x(),
                         d->mItems.first()->y(), 50, 150 );
          return true;
        }
      }

      for ( int i = 0; i < d->mItems.count(); ++i ) {
        if ( d->mItems[ i ] == d->mSelectedItem ) {
          if ( ( i - 1 ) >= 0 ) {
            setSelected( d->mItems[ i - 1 ] );
            ensureVisible( d->mItems[ i - 1 ]->x(),
                           d->mItems[ i - 1 ]->y(), 50, 150 );
            return true;
          }
        }
      }
      return true;
    } else if ( keyEvent->key() == Qt::Key_Down ) {
      if ( d->mSelectedItem == 0 ) {
        if ( !d->mItems.isEmpty() ) {
          setSelected( d->mItems.last() );
          ensureVisible( d->mItems.last()->x(),
                         d->mItems.last()->y(), 50, 150 );
          return true;
        }
      }

      for ( int i = 0; i < d->mItems.count(); ++i ) {
        if ( d->mItems[ i ] == d->mSelectedItem ) {
          if ( ( i + 1 ) < d->mItems.count() ) {
            setSelected( d->mItems[ i + 1 ] );
            ensureVisible( d->mItems[ i + 1 ]->x(),
                           d->mItems[ i + 1 ]->y(), 50, 150 );
            return true;
          }
        }
      }
      return true;
    }
  }

  return QScrollArea::eventFilter( object, event );
}

KWidgetListItem::KWidgetListItem( KWidgetList *parent )
  : QWidget( parent )
{
  mForegroundColor = KColorScheme( KColorScheme::View ).foreground().color();
  mBackgroundColor = KColorScheme( KColorScheme::View ).background().color();
  mSelectionForegroundColor = KColorScheme( KColorScheme::Selection ).foreground().color();
  mSelectionBackgroundColor = KColorScheme( KColorScheme::Selection ).background().color();

  setFocusPolicy( Qt::StrongFocus );

  setAutoFillBackground( true );
}

KWidgetListItem::~KWidgetListItem()
{
}

void KWidgetListItem::setSelected( bool select )
{
  QPalette pal;
  if ( select ) {
    pal.setColor( foregroundRole(), mSelectionForegroundColor );
    pal.setColor( backgroundRole(), mSelectionBackgroundColor );
  } else {
    pal.setColor( foregroundRole(), mForegroundColor );
    pal.setColor( backgroundRole(), mBackgroundColor );
  }

  setPalette( pal );
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
