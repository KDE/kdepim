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

#include <KDebug>
#include <KLocalizedString>
#include <KZip>

#include <QtCore/QBuffer>
#include <QtCore/QTimer>

#include <boost/shared_ptr.hpp>

using namespace MessageCore;
static const mode_t archivePerms = S_IFREG | 0644;

class MessageCore::AttachmentCompressJob::Private
{
  public:
    Private( AttachmentCompressJob *qq );

    void doStart(); // slot

    AttachmentCompressJob *const q;
    AttachmentPart::Ptr mOriginalPart;
    AttachmentPart::Ptr mCompressedPart;
    bool mCompressedPartLarger;
};

AttachmentCompressJob::Private::Private( AttachmentCompressJob *qq )
  : q( qq ),
    mCompressedPartLarger( false )
{
}

void AttachmentCompressJob::Private::doStart()
{
  Q_ASSERT( mOriginalPart );
  const QByteArray decoded = mOriginalPart->data();

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
  time_t zipTime = QDateTime::currentDateTime().toTime_t();
  if( !zip.writeFile( mOriginalPart->name(), QString( /*user*/ ), QString( /*group*/ ),
                      decoded.data(), decoded.size(), archivePerms, zipTime, zipTime, zipTime ) ) {
    q->setError( KJob::UserDefinedError );
    q->setErrorText( i18n( "Could not compress the attachment." ) );
    q->emitResult();
    return;
  }
  zip.close();
  mCompressedPartLarger = (array.size() >= decoded.size());

  // Create new part.
  Q_ASSERT( mCompressedPart == 0 );
  mCompressedPart = AttachmentPart::Ptr( new AttachmentPart );
  mCompressedPart->setName( mOriginalPart->name() + QString::fromLatin1( ".zip" ) ); // TODO not sure name should be .zipped too
  mCompressedPart->setFileName( mOriginalPart->fileName() + QString::fromLatin1( ".zip" ) );
  mCompressedPart->setDescription( mOriginalPart->description() );
  mCompressedPart->setInline( mOriginalPart->isInline() );
  mCompressedPart->setMimeType( "application/zip" );
  mCompressedPart->setCompressed( true );
  mCompressedPart->setEncrypted( mOriginalPart->isEncrypted() );
  mCompressedPart->setSigned( mOriginalPart->isSigned() );
  mCompressedPart->setData( array );
  q->emitResult(); // Success.

  // TODO consider adding a copy constructor to AttachmentPart.
}


AttachmentCompressJob::AttachmentCompressJob( const AttachmentPart::Ptr &part, QObject *parent )
  : KJob( parent ),
    d( new Private( this ) )
{
  d->mOriginalPart = part;
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
  return d->mOriginalPart;
}

void AttachmentCompressJob::setOriginalPart( const AttachmentPart::Ptr &part )
{
  d->mOriginalPart = part;
}

AttachmentPart::Ptr AttachmentCompressJob::compressedPart() const
{
  return d->mCompressedPart;
}

bool AttachmentCompressJob::isCompressedPartLarger() const
{
  return d->mCompressedPartLarger;
}

#include "attachmentcompressjob.moc"
