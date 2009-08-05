/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

  Parts based on KMail code by:
  Copyright 2009 Thomas McGuire <mcguire@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "util.h"

#include <QTextCodec>

#include <KCharsets>
#include <KDebug>

#include <kmime/kmime_charfreq.h>
#include <kmime/kmime_content.h>
#include <kmime/kmime_util.h>

using namespace KMime;
using namespace MessageComposer;

QByteArray MessageComposer::selectCharset( const QList<QByteArray> &charsets, const QString &text )
{
  foreach( const QByteArray &name, charsets ) {
    // We use KCharsets::codecForName() instead of QTextCodec::codecForName() here, because
    // the former knows us-ascii is latin1.
    QTextCodec *codec = KGlobal::charsets()->codecForName( QString::fromLatin1( name ) );
    if( !codec ) {
      kWarning() << "Could not get text codec for charset" << name;
      continue;
    }
    if( codec->canEncode( text ) ) {
      // Special check for us-ascii (needed because us-ascii is not exactly latin1).
      if( name == "us-ascii" && !isUsAscii( text ) ) {
        continue;
      }
      kDebug() << "Chosen charset" << name;
      return name;
    }
  }
  kDebug() << "No appropriate charset found.";
  return QByteArray();
}

QList<Headers::contentEncoding> MessageComposer::encodingsForData( const QByteArray &data )
{
  QList<Headers::contentEncoding> allowed;
  CharFreq cf( data );

  switch ( cf.type() ) {
    case CharFreq::SevenBitText:
      allowed << Headers::CE7Bit;
    case CharFreq::EightBitText:
      allowed << Headers::CE8Bit;
    case CharFreq::SevenBitData:
      if ( cf.printableRatio() > 5.0/6.0 ) {
        // let n the length of data and p the number of printable chars.
        // Then base64 \approx 4n/3; qp \approx p + 3(n-p)
        // => qp < base64 iff p > 5n/6.
        allowed << Headers::CEquPr;
        allowed << Headers::CEbase64;
      } else {
        allowed << Headers::CEbase64;
        allowed << Headers::CEquPr;
      }
      break;
    case CharFreq::EightBitData:
      allowed << Headers::CEbase64;
      break;
    case CharFreq::None:
    default:
      Q_ASSERT( false );
  }

  return allowed;
}

qint64 MessageComposer::sizeWithEncoding( const QByteArray &data,
                                          Headers::contentEncoding encoding )
{
  Content *c = new Content;
  c->setBody( data );
  c->contentTransferEncoding()->setEncoding( encoding );
  int size = c->size();
  delete c;
  return size;
}

