/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "contentjob.h"

#include "composer.h"
#include "job_p.h"
#include "util.h"

#include <KDebug>
#include <KLocalizedString>

#include <kmime/kmime_charfreq.h>
#include <kmime/kmime_content.h>
#include <kmime/kmime_headers.h>

using namespace MessageComposer;
using namespace KMime;

class MessageComposer::ContentJobPrivate : public JobPrivate
{
  public:
    ContentJobPrivate( ContentJob *qq )
      : JobPrivate( qq )
      , contentDisposition( 0 )
      , contentTransferEncoding( 0 )
      , contentType( 0 )
    {
    }

    bool chooseCTE();

    QByteArray data;
    Headers::ContentDisposition *contentDisposition;
    Headers::ContentTransferEncoding *contentTransferEncoding;
    Headers::ContentType *contentType;

    Q_DECLARE_PUBLIC( ContentJob )
};

bool ContentJobPrivate::chooseCTE()
{
  // Based on KMail code by:
  // Copyright 2009 Thomas McGuire <mcguire@kde.org>

  Q_Q( ContentJob );
  Q_ASSERT( composer );
  QList<Headers::contentEncoding> allowed;
  CharFreq cf( data );

  switch ( cf.type() ) {
    case CharFreq::SevenBitText:
      allowed << Headers::CE7Bit;
    case CharFreq::EightBitText:
      if ( composer->behaviour().isActionEnabled( Behaviour::EightBitTransport ) )
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

#if 0 //TODO signing
  // In the following cases only QP and Base64 are allowed:
  // - the buffer will be OpenPGP/MIME signed and it contains trailing
  //   whitespace (cf. RFC 3156)
  // - a line starts with "From "
  if ( ( willBeSigned && cf.hasTrailingWhitespace() ) ||
         cf.hasLeadingFrom() ) {
    ret.removeAll( DwMime::kCte8bit );
    ret.removeAll( DwMime::kCte7bit );
  }
#endif

  if( contentTransferEncoding ) {
    // Specific CTE set.  Check that our data fits in it.
    if( !allowed.contains( contentTransferEncoding->encoding() ) ) {
      q->setError( Job::BugError );
      q->setErrorText( i18n( "%1 Content-Transfer-Encoding cannot correctly encode this message.",
          nameForEncoding( contentTransferEncoding->encoding() ) ) );
      return false;
    }
  } else {
    // No specific CTE set.  Choose the best one.
    Q_ASSERT( !allowed.isEmpty() );
    contentTransferEncoding = new Headers::ContentTransferEncoding;
    contentTransferEncoding->setEncoding( allowed.first() );
  }
  kDebug() << "Settled on encoding" << nameForEncoding( contentTransferEncoding->encoding() );
  return true;
}

ContentJob::ContentJob( QObject *parent )
  : Job( *new ContentJobPrivate( this ), parent )
{
}

ContentJob::~ContentJob()
{
}

QByteArray ContentJob::data() const
{
  Q_D( const ContentJob );
  return d->data;
}

void ContentJob::setData( const QByteArray &data )
{
  Q_D( ContentJob );
  d->data = data;
}

Headers::ContentDisposition *ContentJob::contentDisposition()
{
  Q_D( ContentJob );
  if( !d->contentDisposition ) {
    d->contentDisposition = new Headers::ContentDisposition;
  }
  return d->contentDisposition;
}

Headers::ContentTransferEncoding *ContentJob::contentTransferEncoding()
{
  Q_D( ContentJob );
  if( !d->contentTransferEncoding ) {
    d->contentTransferEncoding = new Headers::ContentTransferEncoding;
  }
  return d->contentTransferEncoding;
}

Headers::ContentType *ContentJob::contentType()
{
  Q_D( ContentJob );
  if( !d->contentType ) {
    d->contentType = new Headers::ContentType;
  }
  return d->contentType;
}

void ContentJob::process()
{
  Q_D( ContentJob );
  Q_ASSERT( d->resultContent == 0 ); // Not processed before.
  d->resultContent = new Content;

  if( !d->chooseCTE() ) {
    Q_ASSERT( error() );
    emitResult();
    return;
  }
  
  // Set headers.
  if( d->contentDisposition ) {
    d->resultContent->setHeader( d->contentDisposition );
    d->contentDisposition->setParent( d->resultContent );
  }
  Q_ASSERT( d->contentTransferEncoding ); // chooseCTE() created it if it didn't exist.
  {
    d->resultContent->setHeader( d->contentTransferEncoding );
    d->contentTransferEncoding->setParent( d->resultContent );

    kDebug() << "decoded" << d->contentTransferEncoding->decoded()
      << "needToEncode" << d->contentTransferEncoding->needToEncode();
  }
  if( d->contentType ) {
    d->resultContent->setHeader( d->contentType );
    d->contentType->setParent( d->resultContent );
  }
  
  // Set data.
  d->resultContent->setBody( d->data );

  kDebug() << "encoded content" << d->resultContent->encodedContent();

  emitResult();
}

#include "contentjob.moc"
