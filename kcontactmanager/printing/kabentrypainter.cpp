/*
    This file is part of KContactManager.
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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "kabentrypainter.h"

#include <QtGui/QPainter>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kurl.h>

KABEntryPainter::KABEntryPainter()
  : mShowAddresses( true ), mShowEmails( true ), mShowPhones( true ),
    mShowURLs( true ), mPrintHeadLines( true )
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

void KABEntryPainter::setPrintHeadLines( bool value )
{
  mPrintHeadLines = value;
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

static QString addressTypeToString(const KABC::Address& address)
{
  QString line1;
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
  return line1 + ':'; // TODO merge into the i18n calls?
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
  QRect rect;

  // settings derived from the options:
  QFontMetrics fmHeader( mHeaderFont );
  QFontMetrics fmHeadLine( mHeadLineFont );
  QFontMetrics fmBody( mBodyFont );
  QFontMetrics fmFixed( mFixedFont );
  QFontMetrics fmComment( mCommentFont );

  int y = top;

  // this is used to prepare some fields for printing and decide about
  // the layout later:
  QList<QStringList> parts;
  QList<QRectList*> contents;

  mEmailRects.clear();
  mPhoneRects.clear();
  mURLRects.clear();

  // set window the painter works on:
  painter->setWindow( window );

  // first draw a black rectangle on top, containing the entries name, centered:
  painter->setFont( mHeaderFont );
  painter->setBrush( QBrush( mBackgroundColor ) );
  painter->setPen( mBackgroundColor );
  const QString text = addr.realName();

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
    const QString line1 = KGlobal::locale()->formatDate( dt.date(), KLocale::ShortDate );
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
  // This used to be addr.prefix(), but that makes no sense, it's part of the formattedName already.
  // The role, OTOH, can be useful. E.g. formattedName="Dr Foo Bar", role="Dentist"
  if ( !addr.role().isEmpty() ) {
    const QString line1 = addr.role().trimmed();

    if ( fake ) {
      rect = painter->boundingRect( Ruler1, y, Width-Ruler1, Height,
                                    Qt::AlignTop | Qt::AlignLeft, line1 );
    } else {
      painter->drawText( Ruler1, y, Width-Ruler1, Height, Qt::AlignTop | Qt::AlignLeft,
                         line1, &rect );
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
      list.append( line.trimmed() );
    }

    parts.push_back( list );
  }

  // Web pages/URLs:
  if ( !addr.url().isEmpty() && addr.url().isValid() && mShowURLs ) {
    contents.push_back( &mURLRects );
    QStringList list;

    list.append( i18n( "Web page:" ) );
    list += addr.url().prettyUrl();
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

  QList<QStringList>::iterator pos = parts.begin();
  QList<QRectList*>::iterator rpos = contents.begin();

  for ( int counter = 0; counter < parts.count(); ++counter ) {
    const int Offset = counter > 1 ? qMax( heights[ 0 ], heights[ 1 ] ) : 0;
    QStringList list = *pos;

    if ( mPrintHeadLines ) {
      painter->setFont( mHeadLineFont );
      if ( fake ) {
        rect = painter->boundingRect( limits[ counter ].left(),
                                      limits[ counter ].top() + heights[counter]
                                      + Offset, limits[ counter ].width(),
                                      limits[ counter ].height(),
                                      Qt::AlignTop | Qt::AlignLeft, list.at( 0 ) );
      } else {
        painter->drawText( limits[ counter ].left(), limits[ counter ].top() +
                           heights[ counter ] + Offset, limits[ counter ].width(),
                           limits[ counter ].height(), Qt::AlignTop | Qt::AlignLeft,
                           list.at( 0 ), &rect );
      }

      heights[ counter ] += rect.height();
    }

    // paint the other elements at Ruler1:
    painter->setFont( mFixedFont );
    for ( int c2 = 1; c2 < list.count(); ++c2 ) {
      // TODO: implement proper line breaking!
      if ( fake ) {
        rect = painter->boundingRect ( limits[ counter ].left() + Ruler1,
                                       limits[ counter ].top() + heights[ counter ]
                                       + Offset, limits[ counter ].width() - Ruler1,
                                       limits[ counter ].height(), Qt::AlignTop | Qt::AlignLeft,
                                       list.at( c2 ) );
      } else {
        painter->drawText( limits[ counter ].left() + Ruler1, limits[ counter ].top()
                           + heights[ counter ] + Offset, limits[ counter ].width()
                           - Ruler1, limits[ counter ].height(), Qt::AlignTop | Qt::AlignLeft,
                           list.at( c2 ), &rect );
      }
      (*rpos)->push_back( rect );
      heights[ counter ] += rect.height();
    }

    ++pos;
    ++rpos;
  }

  y += qMax( heights[ 0 ], heights[ 1 ] ) + qMax( heights[ 2 ], heights[ 3 ] );
  // ^^^^^ done with emails, telephone, URLs and talk addresses

  // now print the addresses:
  const KABC::Address::List addresses = addr.addresses();
  if ( !addresses.isEmpty() && mShowAddresses ) {
    //y += fmBody.lineSpacing() / 2;
    KABC::Address::List::ConstIterator it;
    for ( it = addresses.constBegin(); it != addresses.constEnd(); ++it ) {
      const KABC::Address address = *it;

      if ( mPrintHeadLines ) {
        // First line: address type
        painter->setFont( mHeadLineFont );
        QString headerText = addressTypeToString(*it);
        if ( fake ) {
          rect = painter->boundingRect( 0, y, Width, Height, Qt::AlignTop | Qt::AlignLeft,
                                        headerText );
        } else {
          painter->drawText( 0, y, Width, Height, Qt::AlignTop | Qt::AlignLeft,
                             headerText, &rect );
        }
        y += rect.height();
      }

      y += fmBody.lineSpacing() / 4;
      painter->setFont( mBodyFont );

      const QString formattedAddress = (*it).formattedAddress();
      const QStringList laddr = formattedAddress.split( QChar( '\n' ), QString::SkipEmptyParts );
      Q_FOREACH(const QString& line, laddr) {
        if ( fake ) {
          rect = painter->boundingRect( Ruler1, y, Width - Ruler1, Height,
                                        Qt::AlignTop | Qt::AlignLeft, line );
        } else {
          painter->drawText( Ruler1, y, Width - Ruler1, Height,
                             Qt::AlignTop | Qt::AlignLeft, line, &rect );
        }
        y += rect.height();
      }

      y += fmBody.lineSpacing() / 4;
      if ( !address.label().isEmpty() ) {
        if ( fake ) {
          rect = painter->boundingRect( Ruler1, y, Width - Ruler1, Height,
                                        Qt::AlignTop | Qt::AlignLeft,
                                        i18n( "(Deliver to:)" ) );
        } else {
          painter->drawText( Ruler1, y, Width - Ruler1, Height,
                             Qt::AlignTop | Qt::AlignLeft,
                             i18n( "(Deliver to:)" ), &rect );
        }

        y += rect.height();
        y += fmBody.lineSpacing() / 4;
        if ( fake ) {
          rect = painter->boundingRect( Ruler2, y, Width - Ruler2, Height,
                                        Qt::AlignTop | Qt::AlignLeft, address.label() );
        } else {
          painter->drawText( Ruler2, y, Width - Ruler2, Height,
                             Qt::AlignTop | Qt::AlignLeft, address.label(), &rect );
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
                                    Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap,
                                    addr.note() );
    } else {
      painter->drawText( 0, y, Width, Height,
                         Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap,
                         addr.note(), &rect );
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
