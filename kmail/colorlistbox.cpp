/*
 *   kmail: KDE mail client
 *   This file: Copyright (C) 2000 Espen Sand, espen@kde.org
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <tqpainter.h>

#include <kcolordialog.h>
#include <kcolordrag.h>

#include "colorlistbox.h"

ColorListBox::ColorListBox( TQWidget *parent, const char *name, WFlags f )
  :KListBox( parent, name, f ), mCurrentOnDragEnter(-1)
{
  connect( this, TQT_SIGNAL(selected(int)), this, TQT_SLOT(newColor(int)) );
  setAcceptDrops( true);
}


void ColorListBox::setEnabled( bool state )
{
  if( state == isEnabled() )
  {
    return;
  }

  TQListBox::setEnabled( state );
  for( uint i=0; i<count(); i++ )
  {
    updateItem( i );
  }
}


void ColorListBox::setColor( uint index, const TQColor &color )
{
  if( index < count() )
  {
    ColorListItem *colorItem = (ColorListItem*)item(index);
    colorItem->setColor(color);
    updateItem( colorItem );
    emit changed();
  }
}


TQColor ColorListBox::color( uint index ) const
{
  if( index < count() )
  {
    ColorListItem *colorItem = (ColorListItem*)item(index);
    return( colorItem->color() );
  }
  else
  {
    return( black );
  }
}


void ColorListBox::newColor( int index )
{
  if( isEnabled() == false )
  {
    return;
  }

  if( (uint)index < count() )
  {
    TQColor c = color( index );
    if( KColorDialog::getColor( c, this ) != TQDialog::Rejected )
    {
      setColor( index, c );
    }
  }
}


void ColorListBox::dragEnterEvent( TQDragEnterEvent *e )
{
  if( KColorDrag::canDecode(e) && isEnabled() )
  {
    mCurrentOnDragEnter = currentItem();
    e->accept( true );
  }
  else
  {
    mCurrentOnDragEnter = -1;
    e->accept( false );
  }
}


void ColorListBox::dragLeaveEvent( TQDragLeaveEvent * )
{
  if( mCurrentOnDragEnter != -1 )
  {
    setCurrentItem( mCurrentOnDragEnter );
    mCurrentOnDragEnter = -1;
  }
}


void ColorListBox::dragMoveEvent( TQDragMoveEvent *e )
{
  if( KColorDrag::canDecode(e) && isEnabled() )
  {
    ColorListItem *item = (ColorListItem*)itemAt( e->pos() );
    if( item != 0 )
    {
      setCurrentItem ( item );
    }
  }
}


void ColorListBox::dropEvent( TQDropEvent *e )
{
  TQColor color;
  if( KColorDrag::decode( e, color ) )
  {
    int index = currentItem();
    if( index != -1 )
    {
      ColorListItem *colorItem = (ColorListItem*)item(index);
      colorItem->setColor(color);
      triggerUpdate( false ); // Redraw item
    }
    mCurrentOnDragEnter = -1;
  }
}



ColorListItem::ColorListItem( const TQString &text, const TQColor &color )
  : TQListBoxItem(), mColor( color ), mBoxWidth( 30 )
{
  setText( text );
}


const TQColor &ColorListItem::color( void )
{
  return( mColor );
}


void ColorListItem::setColor( const TQColor &color )
{
  mColor = color;
}


void ColorListItem::paint( TQPainter *p )
{
  TQFontMetrics fm = p->fontMetrics();
  int h = fm.height();

  p->drawText( mBoxWidth+3*2, fm.ascent() + fm.leading()/2, text() );

  p->setPen( Qt::black );
  p->drawRect( 3, 1, mBoxWidth, h-1 );
  p->fillRect( 4, 2, mBoxWidth-2, h-3, mColor );
}


int ColorListItem::height(const TQListBox *lb ) const
{
  return( lb->fontMetrics().lineSpacing()+1 );
}


int ColorListItem::width(const TQListBox *lb ) const
{
  return( mBoxWidth + lb->fontMetrics().width( text() ) + 6 );
}

#include "colorlistbox.moc"
