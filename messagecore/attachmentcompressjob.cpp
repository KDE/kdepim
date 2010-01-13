/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>
  
  Based on KMail code by various authors.

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

#include "attachmentcompressjob.h"

#include <boost/shared_ptr.hpp>

#include <QBuffer>
#include <QTimer>

#include <KDebug>
#include <KLocalizedString>
#include <KZip>

using namespace KPIM;

class KPIM::AttachmentCompressJob::Private
{
  public:
    Private( AttachmentCompressJob *qq );

    void doStart(); // slot

    AttachmentCompressJob *const q;
    AttachmentPart::Ptr originalPart;
    AttachmentPart::Ptr compressedPart;
    bool compressedPartLarger;
};

AttachmentCompressJob::Private::Private( AttachmentCompressJob *qq )
  : q( qq )
  , compressedPartLarger( false )
{
}

void AttachmentCompressJob::Private::doStart()
{
  Q_ASSERT( originalPart );
  QByteArray decoded = originalPart->data();

  QByteArray array;
  QBuffer dev( &array );
  KZip zip( &dev );
  if( !zip.open( QIODevice::WriteOnly ) ) {
    q->setError( KJob::UserDefinedError );
    q->setErrorText( i18n( "Could not initiate attachment compression." ) );
    q->emitResult();
    return;
  }

  // Compress.
  zip.setCompression( KZip::DeflateCompression );
  if( !zip.writeFile( originalPart->name(), QString( /*user*/ ), QString( /*group*/ ),
                      decoded.data(), decoded.size() ) ) {
    q->setError( KJob::UserDefinedError );
    q->setErrorText( i18n( "Could not compress the attachment." ) );
    q->emitResult();
    return;
  }
  zip.close();
  compressedPartLarger = ( array.size() >= decoded.size() );

  // Create new part.
  Q_ASSERT( compressedPart == 0 );
  compressedPart = AttachmentPart::Ptr( new AttachmentPart );
  compressedPart->setName( originalPart->name() + QString::fromLatin1( ".zip" ) ); // TODO not sure name should be .zipped too
  compressedPart->setFileName( originalPart->fileName() + QString::fromLatin1( ".zip" ) );
  compressedPart->setDescription( originalPart->description() );
  compressedPart->setInline( originalPart->isInline() );
  compressedPart->setMimeType( "application/zip" );
  compressedPart->setCompressed( true );
  compressedPart->setEncrypted( originalPart->isEncrypted() );
  compressedPart->setSigned( originalPart->isSigned() );
  compressedPart->setData( array );
  q->emitResult(); // Success.

  // TODO consider adding a copy constructor to AttachmentPart.
}



AttachmentCompressJob::AttachmentCompressJob( const AttachmentPart::Ptr &part, QObject *parent )
  : KJob( parent )
  , d( new Private( this ) )
{
  d->originalPart = part;
}

AttachmentCompressJob::~AttachmentCompressJob()
{
  delete d;
}

void AttachmentCompressJob::start()
{
  QTimer::singleShot( 0, this, SLOT(doStart()) );
}

const AttachmentPart::Ptr AttachmentCompressJob::originalPart() const
{
  return d->originalPart;
}

void AttachmentCompressJob::setOriginalPart( const AttachmentPart::Ptr part )
{
  d->originalPart = part;
}

AttachmentPart::Ptr AttachmentCompressJob::compressedPart() const
{
  return d->compressedPart;
}

bool AttachmentCompressJob::isCompressedPartLarger() const
{
  return d->compressedPartLarger;
}

#include "attachmentcompressjob.moc"
