/*                                                                      
    This file is part of KAddressBook.
    Copyright (c) 1996-2002 Mirko Boehm <mirko@kde.org>
                                                                        
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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      


#include <qpaintdevicemetrics.h>
#include <qpainter.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <knotifyclient.h>
#include <kprinter.h>
#include <kurl.h>

#include "kabentrypainter.h"

KABEntryPainter::KABEntryPainter()
  : mShowAddresses( true ), mShowEmails( true ), mShowPhones( true ),
    mShowURLs( true )
{
}

KABEntryPainter::~KABEntryPainter()
{
  mEmailRects.clear();
  mPhoneRects.clear();
  mURLRects.clear();
  mTalkRects.clear();
}

void KABEntryPainter::setForegroundColor( const QColor &color )
{
  mForegroundColor = color;
}

void KABEntryPainter::setBackgroundColor( const QColor &color )
{
  mBackgroundColor = color;
}

void KABEntryPainter::setHeaderColor( const QColor &color )
{
  mHeaderColor = color;
}

void KABEntryPainter::setHeaderFont( const QFont &font )
{
  mHeaderFont = font;
}

void KABEntryPainter::setHeadLineFont( const QFont &font )
{
  mHeadLineFont = font;
}

void KABEntryPainter::setBodyFont( const QFont &font )
{
  mBodyFont = font;
}

void KABEntryPainter::setFixedFont( const QFont &font )
{
  mFixedFont = font;
}

void KABEntryPainter::setCommentFont( const QFont &font )
{
  mCommentFont = font;
}

void KABEntryPainter::setUseHeaderColor( bool value )
{
  mUseHeaderColor = value;
}

void KABEntryPainter::setShowAddresses( bool value )
{
  mShowAddresses = value;
}

void KABEntryPainter::setShowEmails( bool value )
{
  mShowEmails = value;
}

void KABEntryPainter::setShowPhones( bool value )
{
  mShowPhones = value;
}

void KABEntryPainter::setShowURLs( bool value )
{
  mShowURLs = value;
}

int KABEntryPainter::hitsEmail( const QPoint &p )
{
  return hits( mEmailRects, p );
}

int KABEntryPainter::hitsURL( const QPoint &p )
{
  return hits( mURLRects, p );
}

int KABEntryPainter::hitsPhone( const QPoint &p )
{
  return hits( mPhoneRects, p );
}

int KABEntryPainter::hitsTalk( const QPoint &p )
{
  return hits( mTalkRects, p );
}

int KABEntryPainter::hits( const QRectList& list, const QPoint &p )
{
  QRectList::const_iterator pos;
  int count = 0;

  for ( pos = list.begin(); pos != list.end(); ++pos ) {
    if ( (*pos).contains( p ) )
      return count;

    ++count;
  }

  return -1;
}

bool KABEntryPainter::printAddressee( const KABC::Addressee &addr,
                                      const QRect &window, QPainter *painter,
                                      int top, bool fake, QRect *brect )
{
  // TODO: custom fields, custom (?) for Entry
  const int Width = window.width();
  const int Height = window.height();
  const int Ruler1 = Width/32;
  const int Ruler2 = 2 * Ruler1;
  const int Ruler3 = 3 * Ruler1;
  QString text, line1, line2, line3, line4;
  QRect rect;

  // settings derived from the options:
  QFontMetrics fmHeader( mHeaderFont );
  QFontMetrics fmHeadLine( mHeadLineFont );
  QFontMetrics fmBody( mBodyFont );
  QFontMetrics fmFixed( mFixedFont );
  QFontMetrics fmComment( mCommentFont );

  int y = top;
  KABC::Address address;

  // this is used to prepare some fields for printing and decide about
  // the layout later:
  QValueList<QStringList> parts;
  QValueList<QRectList*> contents;

  mEmailRects.clear();
  mPhoneRects.clear();
  mURLRects.clear();

  // set window the painter works on:
  painter->setWindow( window );

  // first draw a black rectangle on top, containing the entries name, centered:
  painter->setFont( mHeaderFont );
  painter->setBrush( QBrush( mBackgroundColor ) );
  painter->setPen( mBackgroundColor );
  text = addr.realName();

  // replacement for: api->addressbook()->literalName(entry, text);
  rect = painter->boundingRect( Ruler1, y, Width, Height,
                                Qt::AlignVCenter | Qt::AlignLeft, text );
  rect.setHeight( (int)( 1.25 * rect.height() ) );

  if ( !fake && mUseHeaderColor )
    painter->drawRect( 0, y, Width, rect.height() );

  painter->setPen( mUseHeaderColor ? mHeaderColor : mForegroundColor );
  if ( !fake ) {
    // create a little (1/8) space on top of the letters:
    float ypos = y + ( (float)rect.height() ) * 0.125;
    painter->drawText( Ruler1, (int)ypos, Width, rect.height(),
                       Qt::AlignVCenter | Qt::AlignLeft, text );
  }

  // paint the birthday to the right:
  QDateTime dt = addr.birthday();
  if ( dt.isValid() ) {
    line1 = KGlobal::locale()->formatDate( dt.date(), true );
    if ( !fake ) {
      // create a little (1/8) space on top of the letters:
      float ypos = y + ( (float)rect.height() ) * 0.125;
      painter->drawText( 0, (int)ypos, Width-Ruler1, rect.height(),
                         Qt::AlignVCenter | Qt::AlignRight, line1 );
    }
  }

  y += rect.height();

  // now draw the data according to the person:
  painter->setFont( mBodyFont );
  y += fmBody.lineSpacing() / 2;

  painter->setPen( mForegroundColor );
  if ( !addr.prefix().isEmpty() ) {
    line1 = addr.prefix().stripWhiteSpace();

    if ( fake ) {
      rect = painter->boundingRect( Ruler1, y, Width-Ruler1, Height,
                                    Qt::AlignTop | Qt::AlignLeft, line1 );
    } else {
      painter->drawText( Ruler1, y, Width-Ruler1, Height, Qt::AlignTop | Qt::AlignLeft,
                         line1, -1, &rect );
    }

    y += rect.height();
  }

  if ( !( addr.prefix().isEmpty() ) )
    y += fmBody.lineSpacing() / 2;

  // fill the parts stringlist, it contains "parts" (printable areas)
  // that will be combined to fill the page as effectively as possible:
  // Email addresses:
  if ( !addr.emails().isEmpty() && mShowEmails ) {
    contents.push_back( &mEmailRects );
    QStringList list;

    list.append( addr.emails().count() == 1 ? i18n( "Email address:" )
                 : i18n( "Email addresses:" ) );
    list += addr.emails();
    parts.push_back( list );
  }

  // Telephones:
  const KABC::PhoneNumber::List phoneNumbers( addr.phoneNumbers() );
  if ( !phoneNumbers.isEmpty() && mShowPhones ) {
    contents.push_back( &mPhoneRects );
    QStringList list;
    QString line;

    list.append( phoneNumbers.count() == 1 ? i18n( "Telephone:" )
                 : i18n( "Telephones:" ) );

    KABC::PhoneNumber::List::ConstIterator it;
    for ( it = phoneNumbers.begin(); it != phoneNumbers.end(); ++it ) {
      line = (*it).typeLabel();
      line += ": " + (*it).number();
      list.append( line.stripWhiteSpace() );
    }

    parts.push_back( list );
  }

  // Web pages/URLs:
  if ( !addr.url().isEmpty() && addr.url().isValid() && mShowURLs ) {
    contents.push_back( &mURLRects );
    QStringList list;

    list.append( i18n( "Web page:" ) );
    list += addr.url().prettyURL();
    parts.push_back( list );
  }

  /*
  // Talk addresses:
  if ( !addr.talk.isEmpty() ) {
    contents.push_back( &mTalkRects );
    QStringList list;

    list.append( addr.talk.count() == 1 ? i18n( "Talk address:" )
                 : i18n( "Talk addresses:" ) );
    list += addr.talk;
    parts.push_back( list );
  }
  */

  QRect limits[] = { QRect( 0, y, Width / 2, Height ),
                     QRect( Width / 2, y, Width / 2, Height ),
                     QRect( 0, y, Width / 2, Height ),
                     QRect( Width / 2, y, Width / 2, Height ) };
  int heights[ 4 ]= { 0, 0, 0, 0 };

  QValueList<QStringList>::iterator pos = parts.begin();
  QValueList<QRectList*>::iterator rpos = contents.begin();

  for ( uint counter = 0; counter < parts.count(); ++counter ) {
    const int Offset = counter > 1 ? QMAX( heights[ 0 ], heights[ 1 ] ) : 0;
    QStringList list = *pos;

    painter->setFont( mHeadLineFont );
    if ( fake ) {
      rect = painter->boundingRect( limits[ counter ].left(),
                                    limits[ counter ].top() + heights[counter]
                                    + Offset, limits[ counter ].width(),
                                    limits[ counter ].height(),
                                    Qt::AlignTop | Qt::AlignLeft, *list.at( 0 ) );
    } else {
      painter->drawText( limits[ counter ].left(), limits[ counter ].top() + 
                         heights[ counter ] + Offset, limits[ counter ].width(),
                         limits[ counter ].height(), Qt::AlignTop | Qt::AlignLeft,
                         *list.at( 0 ), -1, &rect );
    }

    heights[ counter ] += rect.height();

    // paint the other elements at Ruler1:
    painter->setFont( mFixedFont );
    for ( uint c2 = 1; c2 < list.count(); ++c2 ) {
      // TODO: implement proper line breaking!
      if ( fake ) {
        rect = painter->boundingRect ( limits[ counter ].left() + Ruler1,
                                       limits[ counter ].top() + heights[ counter ]
                                       + Offset, limits[ counter ].width() - Ruler1,
                                       limits[ counter ].height(), Qt::AlignTop | Qt::AlignLeft,
                                       *list.at( c2 ) );
      } else {
        painter->drawText( limits[ counter ].left() + Ruler1, limits[ counter ].top()
                           + heights[ counter ] + Offset, limits[ counter ].width()
                           - Ruler1, limits[ counter ].height(), Qt::AlignTop | Qt::AlignLeft,
                           *list.at( c2 ), -1, &rect );
      }
      (*rpos)->push_back( rect );
      heights[ counter ] += rect.height();
    }

    ++pos;
    ++rpos;
  }

  y = y + QMAX( heights[ 0 ], heights[ 1 ] ) + QMAX( heights[ 2 ], heights[ 3 ] );
  // ^^^^^ done with emails, telephone, URLs and talk addresses

  // now print the addresses:
  KABC::Address::List addresses = addr.addresses();
  if ( addresses.count() > 0 && mShowAddresses ) {
    y += fmBody.lineSpacing() / 2;
    painter->setFont( mHeadLineFont );
    if ( fake ) {
      rect = painter->boundingRect( 0, y, Width, Height, Qt::AlignTop | Qt::AlignLeft,
                                    addresses.count() == 1 ? i18n( "Address:" )
                                    : i18n( "Addresses:" ) );
    } else {
      painter->drawText( 0, y, Width, Height, Qt::AlignTop | Qt::AlignLeft,
                         addresses.count() == 1 ? i18n( "Address:" )
                         : i18n( "Addresses:" ), -1, &rect );
    }

    y += rect.height();
    y += fmBody.lineSpacing() / 4;
    painter->setFont( mBodyFont );

    KABC::Address::List::ConstIterator it;
    for ( it = addresses.begin(); it != addresses.end(); ++it ) {
      address = *it;
      switch ( address.type() ) {
        case KABC::Address::Dom:
          line1 = i18n( "Domestic Address" );
          break;
        case KABC::Address::Intl:
          line1 = i18n( "International Address" );
          break;
        case KABC::Address::Postal:
          line1 = i18n( "Postal Address" );
          break;
        case KABC::Address::Parcel:
          line1 = i18n( "Parcel Address" );
          break;
        case KABC::Address::Home:
          line1 = i18n( "Home Address" );
          break;
        case KABC::Address::Work:
          line1 = i18n( "Work Address" );
          break;
        case KABC::Address::Pref:
        default:
          line1 = i18n( "Preferred Address" );
      }

      line1 += QString::fromLatin1( ":" );
      text = QString::null;

      if ( !address.extended().isEmpty() )
        text = address.extended().stripWhiteSpace();

      if ( !text.isEmpty() ) {
        line1 = line1 + QString::fromLatin1( " (" ) + text +
        QString::fromLatin1( ")" );
      }

      line1 = line1.stripWhiteSpace();
      line2 = address.street();
      if ( !address.postOfficeBox().isEmpty() )
        line2 += QString::fromLatin1( " - " ) + address.postOfficeBox();

      // print address in american style, this will need localisation:
      line3 = address.locality() + ( address.region().isEmpty() ?
              QString::fromLatin1( "" ) : QString::fromLatin1( ", " ) + 
              address.region() ) + ( address.postalCode().isEmpty()
              ? QString::fromLatin1( "" ) : QString::fromLatin1( " " )
              + address.postalCode() );
      line4 = address.country();

      if ( fake ) {
        rect = painter->boundingRect( Ruler1, y, Width - Ruler1, Height,
                                      Qt::AlignTop | Qt::AlignLeft, line1 );
      } else {
        painter->drawText( Ruler1, y, Width - Ruler1, Height,
                           Qt::AlignTop | Qt::AlignLeft, line1, -1, &rect );
      }

      y += rect.height();
      if ( !line2.isEmpty() ) {
        if ( fake ) {
          rect = painter->boundingRect( Ruler2, y, Width - Ruler2, Height,
                                        Qt::AlignTop | Qt::AlignLeft, line2 );
        } else {
          painter->drawText( Ruler2, y, Width - Ruler2, Height,
                             Qt::AlignTop | Qt::AlignLeft, line2, -1, &rect );
        }
        y += rect.height();
      }

      if ( !line3.isEmpty() ) {
        if ( fake ) {
          rect = painter->boundingRect( Ruler2, y, Width - Ruler2, Height,
                                        Qt::AlignTop | Qt::AlignLeft, line3 );
        } else {
          painter->drawText( Ruler2, y, Width - Ruler2, Height,
                             Qt::AlignTop | Qt::AlignLeft, line3, -1, &rect );
        }
        y += rect.height();
      }

      if ( !line4.isEmpty() ) {
        if ( fake ) {
          rect = painter->boundingRect( Ruler2, y, Width - Ruler2, Height,
                                        Qt::AlignTop | Qt::AlignLeft, line4 );
        } else {
          painter->drawText( Ruler2, y, Width - Ruler2, Height,
                             Qt::AlignTop | Qt::AlignLeft, line4, -1, &rect );
        }
        y += rect.height();
      }

      y += fmBody.lineSpacing() / 4;
      if ( !address.label().isEmpty() ) {
        if ( fake ) {
          rect = painter->boundingRect( Ruler2, y, Width - Ruler2, Height,
                                        Qt::AlignTop | Qt::AlignLeft,
                                        i18n( "(Deliver to:)" ) );
        } else {
          painter->drawText( Ruler2, y, Width - Ruler2, Height,
                             Qt::AlignTop | Qt::AlignLeft,
                             i18n( "(Deliver to:)" ), -1, &rect );
        }

        y += rect.height();
        y += fmBody.lineSpacing() / 4;
        if ( fake ) {
          rect = painter->boundingRect( Ruler3, y, Width - Ruler3, Height,
                                        Qt::AlignTop | Qt::AlignLeft, address.label() );
        } else {
          painter->drawText( Ruler3, y, Width - Ruler3, Height,
                             Qt::AlignTop | Qt::AlignLeft, address.label(), -1, &rect );
        }

        y += rect.height();
        y += fmBody.lineSpacing() / 2;
      }
    }
  }

  if ( !addr.note().isEmpty() ) {
    painter->setFont( mCommentFont );
    y += fmBody.lineSpacing() / 2;
    if ( fake ) {
      rect = painter->boundingRect( 0, y, Width, Height,
                                    Qt::AlignTop | Qt::AlignLeft | Qt::WordBreak,
                                    addr.note() );
    } else {
      painter->drawText( 0, y, Width, Height,
                         Qt::AlignTop | Qt::AlignLeft | Qt::WordBreak,
                         addr.note(), -1, &rect );
    }

    y += rect.height();
  }

  y += fmBody.lineSpacing() / 2;

  if ( brect != 0 )
    *brect = QRect( 0, top, Width, y - top );

  if ( y < Height )
    return true;
  else
    return false;
}
