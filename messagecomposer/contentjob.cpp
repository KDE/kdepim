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
#include "job_p.h"

#include <KDebug>

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

    QByteArray data;
    Headers::ContentDisposition *contentDisposition;
    Headers::ContentTransferEncoding *contentTransferEncoding;
    Headers::ContentType *contentType;
};

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
  
  // Headers.
  if( d->contentDisposition ) {
    d->resultContent->setHeader( d->contentDisposition );
    d->contentDisposition->setParent( d->resultContent );
  }
  if( d->contentTransferEncoding ) {
    d->resultContent->setHeader( d->contentTransferEncoding );
    d->contentTransferEncoding->setParent( d->resultContent );

    kDebug() << "decoded" << d->contentTransferEncoding->decoded()
      << "needToEncode" << d->contentTransferEncoding->needToEncode();
  }
  if( d->contentType ) {
    d->resultContent->setHeader( d->contentType );
    d->contentType->setParent( d->resultContent );
  }
  
  // Data.
  d->resultContent->setBody( d->data );

  kDebug() << "encoded content" << d->resultContent->encodedContent();

  emitResult();
}

#include "contentjob.moc"
