/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

  Parts based on KMail code by:
  Various authors.

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

#include "attachmentjob.h"

#include <boost/shared_ptr.hpp>

#include <KEncodingProber>

#include "contentjobbase_p.h"
#include "globalpart.h"
#include "singlepartjob.h"
#include "util.h"

#include <KDebug>

using namespace MessageComposer;
using namespace KPIM;

class MessageComposer::AttachmentJobPrivate : public ContentJobBasePrivate
{
  public:
    AttachmentJobPrivate( AttachmentJob *qq )
      : ContentJobBasePrivate( qq )
    {
    }

    QByteArray detectCharset( const QByteArray &data );

    AttachmentPart::Ptr part;

    Q_DECLARE_PUBLIC( AttachmentJob )
};

QByteArray AttachmentJobPrivate::detectCharset( const QByteArray &data )
{
  // NOTE: The old code could also could determine the charset from the url
  // of the file that was loaded as attachment (url.fileEncoding()).  If we
  // really want that, we can store the chareset in AttachmentPart and make
  // AttachmentFromUrlJob set it.

  KEncodingProber prober;
  prober.feed( data );
  kDebug() << "Autodetected charset" << prober.encoding() << "with confidence" << prober.confidence();

  // The prober detects binary attachments as UTF-16LE with confidence 99%, which
  // obviously is wrong, so work around this here (most mail clients don't understand
  // UTF-16LE).
  const QByteArray detectedEncoding = prober.encoding();
  if( prober.confidence() > 0.6 && !detectedEncoding.toLower().contains( "utf-16" ) ) {
    return detectedEncoding;
  } else {
    kWarning() << "Could not autodetect charset; using UTF-8.";
    return QByteArray( "utf-8" );
  }
}



AttachmentJob::AttachmentJob( AttachmentPart::Ptr part, QObject *parent )
  : ContentJobBase( *new AttachmentJobPrivate( this ), parent )
{
  Q_D( AttachmentJob );
  d->part = part;
}

AttachmentJob::~AttachmentJob()
{
}

AttachmentPart::Ptr AttachmentJob::attachmentPart() const
{
  Q_D( const AttachmentJob );
  return d->part;
}

void AttachmentJob::setAttachmentPart( AttachmentPart::Ptr part )
{
  Q_D( AttachmentJob );
  d->part = part;
}

void AttachmentJob::doStart()
{
  Q_D( AttachmentJob );
  Q_ASSERT( d->part );

  // Set up a subjob to generate the attachment content.
  SinglepartJob *sjob = new SinglepartJob( this );
  sjob->setData( d->part->data() );

  // Figure out a charset to encode parts of the headers with.
  const QString dataToEncode = d->part->name() + d->part->description() + d->part->fileName();
  const QByteArray charset = selectCharset( globalPart()->charsets( true ), dataToEncode );

  // Set up the headers.
  sjob->contentTransferEncoding()->setEncoding( d->part->encoding() );

  sjob->contentType()->setName( d->part->name(), charset );
  sjob->contentType()->setMimeType( d->part->mimeType() );
  if( sjob->contentType()->isText() ) {
    // If it is a text file, detect its charset.
    sjob->contentType()->setCharset( d->detectCharset( d->part->data() ) );
  }

  sjob->contentDescription()->fromUnicodeString( d->part->description(), charset );

  sjob->contentDisposition()->setFilename( d->part->fileName() );
  sjob->contentDisposition()->setRFC2047Charset( charset );
  if( d->part->isInline() ) {
    sjob->contentDisposition()->setDisposition( KMime::Headers::CDinline );
  } else {
    sjob->contentDisposition()->setDisposition( KMime::Headers::CDattachment );
  }

  ContentJobBase::doStart();
}

void AttachmentJob::process()
{
  Q_D( AttachmentJob );
  // The content has been created by our subjob.
  Q_ASSERT( d->subjobContents.count() == 1 );
  d->resultContent = d->subjobContents.first();
  emitResult();
}

#include "attachmentjob.moc"
