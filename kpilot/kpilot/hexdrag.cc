/*
 *   khexedit - Versatile hex editor
 *   Copyright (C) 1999  Espen Sand, espensa@online.no
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
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */


#include "hexdrag.h"
static const char *mediaString = "application/octet-stream";


CHexDrag::CHexDrag( const QByteArray &data, QWidget *dragSource,
		    const char *name )
  :QDragObject(dragSource,name)
{
  setData( data );
  prepPixmap();
}


CHexDrag::CHexDrag( QWidget *dragSource, const char *name )
  :QDragObject(dragSource,name)
{
  prepPixmap();
}


void CHexDrag::setData( const QByteArray &data )
{
  mData = data;
}



void CHexDrag::prepPixmap(void)
{
  //
  // Wont use it yet,
  //
  /*
  KIconLoader &loader = *KGlobal::iconLoader();
  QPixmap pix = loader.loadIcon( "binary.xpm" );

  QPoint hotspot( pix.width()-20,pix.height()/2 );
  setPixmap( pix, hotspot ); 
  */
}


const char *CHexDrag::format( int i ) const
{
  if( i == 0 )
  {
    return( mediaString );
  }
  else
  {
    return( 0 );
  }
  return( i == 0 ? mediaString : 0 );
}


QByteArray CHexDrag::encodedData( const char *fmt ) const
{
  if( fmt != 0 )
  {
    if( strcmp( fmt, mediaString) == 0 )
    {
      return( mData );
    }
  }

  QByteArray buf;
  return( buf );
}


bool CHexDrag::canDecode( const QMimeSource *e )
{
  return( e->provides(mediaString) );
}


bool CHexDrag::decode( const QMimeSource *e, QByteArray &dest )
{
  dest = e->encodedData(mediaString);
  return( dest.size() == 0 ? false : true );

  //
  // I get an 
  // "X Error: BadAtom (invalid Atom parameter) 5
  //   Major opcode:  17"
  //
  // if I try to use the code below on a source that has been 
  // collected from QClipboard. It is the e->provides(mediaString)
  // that fail (Qt-2.0). Sometimes it works :(
  //
  // printf("0: %s\n", e->format(0) ); No problem. 
  // printf("1: %s\n", e->format(1) ); Crash. 
  //
  #if 0
  if( e->provides(mediaString) == true )
  {
    dest = e->encodedData(mediaString);
    return( true );
  }
  else
  {
    return( false );
  }
  #endif
}


#include "hexdrag.moc"
