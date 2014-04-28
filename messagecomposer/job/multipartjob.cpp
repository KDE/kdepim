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

#include "multipartjob.h"
#include "contentjobbase_p.h"

#include <KDebug>

#include <KMime/kmime_content.h>

using namespace MessageComposer;

class MessageComposer::MultipartJobPrivate : public ContentJobBasePrivate
{
public:
    MultipartJobPrivate( MultipartJob *qq )
        : ContentJobBasePrivate( qq )
    {
    }

    QByteArray subtype;

};

MultipartJob::MultipartJob( QObject *parent )
    : ContentJobBase( *new MultipartJobPrivate( this ), parent )
{
}

MultipartJob::~MultipartJob()
{
}

QByteArray MultipartJob::multipartSubtype() const
{
    Q_D( const MultipartJob );
    return d->subtype;
}

void MultipartJob::setMultipartSubtype( const QByteArray &subtype )
{
    Q_D( MultipartJob );
    d->subtype = subtype;
}

void MultipartJob::process()
{
    Q_D( MultipartJob );
    Q_ASSERT( d->resultContent == 0 ); // Not processed before.
    Q_ASSERT( !d->subtype.isEmpty() );
    d->resultContent = new KMime::Content;
    d->resultContent->contentType( true )->setMimeType( "multipart/" + d->subtype );
    d->resultContent->contentType()->setBoundary( KMime::multiPartBoundary() );
    d->resultContent->contentTransferEncoding()->setEncoding( KMime::Headers::CE7Bit );
    d->resultContent->setPreamble( "This is a multi-part message in MIME format.\n" );
    foreach( KMime::Content *c, d->subjobContents ) {
        d->resultContent->addContent( c );
        if( c->contentTransferEncoding()->encoding() == KMime::Headers::CE8Bit ) {
            d->resultContent->contentTransferEncoding()->setEncoding( KMime::Headers::CE8Bit );
            break;
        }
    }
    kDebug() << "Created" << d->resultContent->contentType()->mimeType() << "content with"
             << d->resultContent->contents().count() << "subjobContents.";
    emitResult();
}

