/*
    This file is part of KAddressBook.
    Copyright (C) 2003 Helge Deller <deller@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

/*
 *  - ldifvcardthumbnail -
 *
 *  This kioslave generates tumbnails for vCard and LDIF files. 
 *  The thumbnails are used e.g. by Konqueror or in the file selection
 *  dialog.
 *
 */

#include <qdatetime.h>
#include <qfile.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qtextstream.h>

#include <kabc/ldifconverter.h>
#include <kabc/vcardtool.h>
#include <kpixmapsplitter.h>
#include <kstandarddirs.h>

#include "ldifvcardcreator.h"

extern "C"
{
  ThumbCreator *new_creator()
  {
    return new VCard_LDIFCreator;
  }
};

VCard_LDIFCreator::VCard_LDIFCreator()
  : mSplitter( 0 )
{
}

VCard_LDIFCreator::~VCard_LDIFCreator()
{
  delete mSplitter;
}

bool VCard_LDIFCreator::create(const QString &path, int width, int height, QImage &img)
{
  if ( !mSplitter ) {
    mSplitter = new KPixmapSplitter;
    QString pixmap = locate( "data", "konqueror/pics/thumbnailfont_7x4.png" );
    if ( !pixmap.isEmpty() ) {
      // FIXME: make font/glyphsize configurable...
      mSplitter->setPixmap( QPixmap( pixmap ) );
      mSplitter->setItemSize( QSize( 4, 7 ) );
    }
  }

  // determine some sizes...
  // example: width: 60, height: 64
  QSize pixmapSize( width, height );
  if (height * 3 > width * 4)
    pixmapSize.setHeight( width * 4 / 3 );
  else
    pixmapSize.setWidth( height * 3 / 4 );

  if ( pixmapSize != mPixmap.size() )
    mPixmap.resize( pixmapSize );
    
  // one pixel for the rectangle, the rest. whitespace
  int xborder = 1 + pixmapSize.width()/16;  // minimum x-border
  int yborder = 1 + pixmapSize.height()/16; // minimum y-border

  QSize chSize = mSplitter->itemSize(); // the size of one char
  int xOffset = chSize.width();
  int yOffset = chSize.height();

  // calculate a better border so that the text is centered
  int canvasWidth = pixmapSize.width() - 2 * xborder;
  int canvasHeight = pixmapSize.height() -  2 * yborder;
  int numCharsPerLine = (int) (canvasWidth / chSize.width());
  int numLines = (int) (canvasHeight / chSize.height());

  // create text-preview
  QFile file( path );
  if ( !file.open( IO_ReadOnly ) )
    return false;

  // read the file
  QTextStream t( &file );
  t.setEncoding( QTextStream::Latin1 );
  QString contents = t.read();
  file.close();

  // convert the file contents to a KABC::Addressee address
  bool ok = true;
  KABC::AddresseeList addrList;
  KABC::Addressee addr;
  KABC::VCardTool tool;

  addrList = tool.parseVCards( contents );
  if ( addrList.count() == 0 ) {
    ok = KABC::LDIFConverter::LDIFToAddressee( contents, addrList );
    if ( ok )
      addr = addrList[ 0 ];
    else
      return false;
  } else
    addr = addrList[ 0 ];

  // prepare the text to show
  QString text, info;

  info = addr.formattedName().simplifyWhiteSpace();
  if ( !info.isEmpty() )
    text += info + "\n";
  else
    text += QString( addr.givenName() + " " + addr.familyName() ).stripWhiteSpace() + "\n";

  info = addr.organization().simplifyWhiteSpace();
  if ( !info.isEmpty() )
    text += info + "\n";

  QString pn = addr.phoneNumber( KABC::PhoneNumber::Pref ).number();
  if ( !pn.isEmpty() )
    text += pn + "\n";

  pn = addr.phoneNumber( KABC::PhoneNumber::Work ).number();
  if ( !pn.isEmpty() )
    text += pn + "\n";

  pn = addr.phoneNumber( KABC::PhoneNumber::Home ).number();
  if ( !pn.isEmpty() )
    text += pn + "\n";

  // render the information
  mPixmap.fill( QColor( 245, 245, 245 ) ); // light-grey background
  QRect rect;
  int rest = mPixmap.width() - (numCharsPerLine * chSize.width());
  xborder = QMAX( xborder, rest / 2 ); // center horizontally
  rest = mPixmap.height() - (numLines * chSize.height());
  yborder = QMAX( yborder, rest / 2 ); // center vertically
  // end centering

  int x = xborder, y = yborder; // where to paint the characters
  int posNewLine  = mPixmap.width() - (chSize.width() + xborder);
  int posLastLine = mPixmap.height() - (chSize.height() + yborder);
  bool newLine = false;
  Q_ASSERT( posNewLine > 0 );
  const QPixmap *fontPixmap = &(mSplitter->pixmap());

  for ( uint i = 0; i < text.length(); i++ ) {
    if ( x > posNewLine || newLine ) {  // start a new line?
      x = xborder;
      y += yOffset;

      if ( y > posLastLine ) // more text than space
        break;

      // after starting a new line, we also jump to the next
      // physical newline in the file if we don't come from one
      if ( !newLine ) {
        int pos = text.find( '\n', i );
        if ( pos > (int) i )
          i = pos +1;
      }

      newLine = false;
    }

    // check for newlines in the text (unix,dos)
    QChar ch = text.at( i );
    if ( ch == '\n' ) {
      newLine = true;
      continue;
    } else if ( ch == '\r' && text.at(i+1) == '\n' ) {
      newLine = true;
      i++; // skip the next character (\n) as well
      continue;
    }

    rect = mSplitter->coordinates( ch );
    if ( !rect.isEmpty() )
      bitBlt( &mPixmap, QPoint(x,y), fontPixmap, rect, Qt::CopyROP );

    x += xOffset; // next character
  }

  if ( ok )
    img = mPixmap.convertToImage();

  return ok;
}

ThumbCreator::Flags VCard_LDIFCreator::flags() const
{
  return (Flags)(DrawFrame | BlendIcon);
}
