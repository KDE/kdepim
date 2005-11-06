/*
    This file is part of KAddressBook.
    Copyright (C) 2003 Helge Deller <deller@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    version 2 License as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

/*
 *  - ldifvcardthumbnail -
 *
 *  kioslave which generates tumbnails for vCard and LDIF files.
 *  The thumbnails are used e.g. by Konqueror or in the file selection
 *  dialog.
 *
 */

#include <qdatetime.h>
#include <qfile.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qpainter.h>
#include <qtextstream.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kabc/ldifconverter.h>
#include <kabc/vcardconverter.h>
#include <kpixmapsplitter.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>

#include "ldifvcardcreator.h"

extern "C"
{
  ThumbCreator *new_creator()
  {
    KGlobal::locale()->insertCatalog( "kaddressbook" );
    return new VCard_LDIFCreator;
  }
}

VCard_LDIFCreator::VCard_LDIFCreator()
  : mSplitter( 0 )
{
}

VCard_LDIFCreator::~VCard_LDIFCreator()
{
  delete mSplitter;
}


bool VCard_LDIFCreator::readContents( const QString &path )
{
  // read file contents
  QFile file( path );
  if ( !file.open( QIODevice::ReadOnly ) )
    return false;

  QString info;
  text.truncate(0);

  // read the file
  QTextStream t( &file );
  t.setEncoding( QTextStream::UnicodeUTF8 );
  QString contents = t.read();
  file.close();

  // convert the file contents to a KABC::Addressee address
  KABC::Addressee::List addrList;
  KABC::Addressee addr;
  KABC::VCardConverter converter;

  addrList = converter.parseVCards( contents );
  if ( addrList.count() == 0 ) {
    KABC::AddresseeList l; // FIXME porting
    if ( !KABC::LDIFConverter::LDIFToAddressee( contents, l ) )
	return false;
    // FIXME porting
    KABC::AddresseeList::ConstIterator it( l.constBegin() );
    for ( ; it != l.constEnd(); ++ it ) {
        addrList.append( *it );
    }
  }
  if ( addrList.count()>1 ) {
    // create an overview (list of all names)
    name = i18n("One contact found:", "%n contacts found:", addrList.count());
    int no, linenr;
    for (linenr=no=0; linenr<30 && no<addrList.count(); ++no) {
       addr = addrList[no];
       info = addr.formattedName().simplified();
       if (info.isEmpty())
          info = addr.givenName() + " " + addr.familyName();
       info = info.simplified();
       if (info.isEmpty())
         continue;
       text.append(info);
       text.append("\n");
       ++linenr;
    }
    return true;
  }

  // create card for _one_ contact
  addr = addrList[ 0 ];

  // prepare the text
  name = addr.formattedName().simplified();
  if ( name.isEmpty() )
    name = addr.givenName() + " " + addr.familyName();
  name = name.simplified();


  KABC::PhoneNumber::List pnList = addr.phoneNumbers();
  QStringList phoneNumbers;
  for (int no=0; no<pnList.count(); ++no) {
    QString pn = pnList[no].number().simplified();
    if (!pn.isEmpty() && !phoneNumbers.contains(pn))
      phoneNumbers.append(pn);
  }
  if ( !phoneNumbers.isEmpty() )
      text += phoneNumbers.join("\n") + "\n";

  info = addr.organization().simplified();
  if ( !info.isEmpty() )
    text += info + "\n";

  // get an address
  KABC::Address address = addr.address(KABC::Address::Work);
  if (address.isEmpty())
    address = addr.address(KABC::Address::Home);
  if (address.isEmpty())
    address = addr.address(KABC::Address::Pref);
  info = address.formattedAddress();
  if ( !info.isEmpty() )
    text += info + "\n";

  return true;
}


bool VCard_LDIFCreator::createImageSmall()
{
  text = name + "\n" + text;

  if ( !mSplitter ) {
    mSplitter = new KPixmapSplitter;
    QString pixmap = locate( "data", "konqueror/pics/thumbnailfont_7x4.png" );
    if ( pixmap.isEmpty() ) {
      kdWarning() << "VCard_LDIFCreator: Font image \"thumbnailfont_7x4.png\" not found!\n";
      return false;
    }
    mSplitter->setPixmap( QPixmap( pixmap ) );
    mSplitter->setItemSize( QSize( 4, 7 ) );
  }

  QSize chSize = mSplitter->itemSize(); // the size of one char
  int xOffset = chSize.width();
  int yOffset = chSize.height();

  // calculate a better border so that the text is centered
  int canvasWidth = pixmapSize.width() - 2 * xborder;
  int canvasHeight = pixmapSize.height() -  2 * yborder;
  int numCharsPerLine = (int) (canvasWidth / chSize.width());
  int numLines = (int) (canvasHeight / chSize.height());

  // render the information
  QRect rect;
  int rest = mPixmap.width() - (numCharsPerLine * chSize.width());
  xborder = qMax( xborder, rest / 2 ); // center horizontally
  rest = mPixmap.height() - (numLines * chSize.height());
  yborder = qMax( yborder, rest / 2 ); // center vertically
  // end centering

  int x = xborder, y = yborder; // where to paint the characters
  int posNewLine  = mPixmap.width() - (chSize.width() + xborder);
  int posLastLine = mPixmap.height() - (chSize.height() + yborder);
  bool newLine = false;
  Q_ASSERT( posNewLine > 0 );
  const QPixmap *fontPixmap = &(mSplitter->pixmap());

  for ( int i = 0; i < text.length(); i++ ) {
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
    /* FIXME porting
    if ( !rect.isEmpty() )
      bitBlt( &mPixmap, QPoint(x,y), fontPixmap, rect, Qt::CopyROP );
     */

    x += xOffset; // next character
  }

  return true;
}

bool VCard_LDIFCreator::createImageBig()
{
  QFont normalFont( KGlobalSettings::generalFont() );
  QFont titleFont( normalFont );
  titleFont.setBold(true);
  // titleFont.setUnderline(true);
  titleFont.setItalic(true);

  QPainter painter(&mPixmap);
  painter.setFont(titleFont);
  QFontMetrics fm(painter.fontMetrics());

  // draw contact name
  painter.setClipRect(2, 2, pixmapSize.width()-4, pixmapSize.height()-4);
  QPoint p(5, fm.height()+2);
  painter.drawText(p, name);
  p.setY( 3*p.y()/2 );

  // draw contact information
  painter.setFont(normalFont);
  fm = painter.fontMetrics();

  const QStringList list( QStringList::split('\n', text) );
  for ( QStringList::ConstIterator it = list.begin();
             p.y()<=pixmapSize.height() && it != list.end(); ++it ) {
     p.setY( p.y() + fm.height() );
     painter.drawText(p, *it);
  }

  return true;
}

bool VCard_LDIFCreator::create(const QString &path, int width, int height, QImage &img)
{
  if ( !readContents(path) )
    return false;

  // resize the image if necessary
  pixmapSize = QSize( width, height );
  if (height * 3 > width * 4)
    pixmapSize.setHeight( width * 4 / 3 );
  else
    pixmapSize.setWidth( height * 3 / 4 );

  if ( pixmapSize != mPixmap.size() )
    mPixmap.resize( pixmapSize );

  mPixmap.fill( QColor( 245, 245, 245 ) ); // light-grey background

  // one pixel for the rectangle, the rest. whitespace
  xborder = 1 + pixmapSize.width()/16;  // minimum x-border
  yborder = 1 + pixmapSize.height()/16; // minimum y-border

  bool ok;
  if ( width >= 150 /*pixel*/ )
    ok = createImageBig();
  else
    ok = createImageSmall();
  if (!ok)
    return false;

  img = mPixmap.convertToImage();
  return true;
}

ThumbCreator::Flags VCard_LDIFCreator::flags() const
{
  return (Flags)(DrawFrame | BlendIcon);
}
