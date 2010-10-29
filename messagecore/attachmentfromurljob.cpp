/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

  Parts based on KMail code by various authors.

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

#include "attachmentfromurljob.h"

#include <KDebug>
#include <KGlobal>
#include <KIO/Scheduler>
#include <KLocale>
#include <KMimeType>

#include <QtCore/QFileInfo>

#include <boost/shared_ptr.hpp>

using namespace MessageCore;

class MessageCore::AttachmentFromUrlJob::Private
{
  public:
    Private( AttachmentFromUrlJob *qq );

    void transferJobData( KIO::Job *job, const QByteArray &jobData );
    void transferJobResult( KJob *job );

    AttachmentFromUrlJob *const q;
    KUrl mUrl;
    qint64 mMaxSize;
    QByteArray mData;
};

AttachmentFromUrlJob::Private::Private( AttachmentFromUrlJob *qq )
  : q( qq ),
    mMaxSize( -1 )
{
}

void AttachmentFromUrlJob::Private::transferJobData( KIO::Job *job, const QByteArray &jobData )
{
  Q_UNUSED( job );
  mData += jobData;
}

void AttachmentFromUrlJob::Private::transferJobResult( KJob *job )
{
  if ( job->error() ) {
    // TODO this loses useful stuff from KIO, like detailed error descriptions, causes+solutions,
    // ... use UiDelegate somehow?
    q->setError( job->error() );
    q->setErrorText( job->errorString() );
    q->emitResult();
    return;
  }

  Q_ASSERT( dynamic_cast<KIO::TransferJob*>( job ) );
  KIO::TransferJob *transferJob = static_cast<KIO::TransferJob*>( job );

  // Determine the MIME type and filename of the attachment.
  const QString mimeType = transferJob->mimetype();
  kDebug() << "Mimetype is" << mimeType;

  QString fileName = mUrl.fileName();
  if ( fileName.isEmpty() ) {
    const KMimeType::Ptr mimeTypePtr = KMimeType::mimeType( mimeType, KMimeType::ResolveAliases );
    if ( mimeTypePtr ) {
      fileName = i18nc( "a file called 'unknown.ext'", "unknown%1",
                        mimeTypePtr->mainExtension() );
    } else {
      fileName = i18nc( "a filed called 'unknown'", "unknown" );
    }
  }

  // Create the AttachmentPart.
  Q_ASSERT( q->attachmentPart() == 0 ); // Not created before.

  AttachmentPart::Ptr part = AttachmentPart::Ptr( new AttachmentPart );
  part->setCharset( mUrl.fileEncoding().toLatin1() );
  part->setMimeType( mimeType.toLatin1() );
  part->setName( fileName );
  part->setFileName( fileName );
  part->setData( mData );
  q->setAttachmentPart( part );
  q->emitResult(); // Success.
}


AttachmentFromUrlJob::AttachmentFromUrlJob( const KUrl &url, QObject *parent )
  : AttachmentLoadJob( parent ),
    d( new Private( this ) )
{
  d->mUrl = url;
}

AttachmentFromUrlJob::~AttachmentFromUrlJob()
{
  delete d;
}

KUrl AttachmentFromUrlJob::url() const
{
  return d->mUrl;
}

void AttachmentFromUrlJob::setUrl( const KUrl &url )
{
  d->mUrl = url;
}

qint64 AttachmentFromUrlJob::maximumAllowedSize() const
{
  return d->mMaxSize;
}

void AttachmentFromUrlJob::setMaximumAllowedSize( qint64 size )
{
  d->mMaxSize = size;
}

void AttachmentFromUrlJob::doStart()
{
  if ( !d->mUrl.isValid() ) {
    setError( KJob::UserDefinedError );
    setErrorText( i18n( "\"%1\" not found. Please specify the full path.", d->mUrl.prettyUrl() ) );
    emitResult();
    return;
  }

  if ( d->mMaxSize != -1 && d->mUrl.isLocalFile() ) {
    const qint64 size = QFileInfo( d->mUrl.toLocalFile() ).size();
    if ( size > d->mMaxSize ) {
      setError( KJob::UserDefinedError );
      setErrorText( i18n( "You may not attach files bigger than %1.",
                          KGlobal::locale()->formatByteSize( d->mMaxSize ) ) );
      emitResult();
      return;
    }
  }

  Q_ASSERT( d->mData.isEmpty() ); // Not started twice.

  KIO::TransferJob *job = KIO::get( d->mUrl, KIO::NoReload,
      ( uiDelegate() ? KIO::DefaultFlags : KIO::HideProgressInfo ) );
  QObject::connect( job, SIGNAL( result( KJob* ) ),
                    this, SLOT( transferJobResult( KJob* ) ) );
  QObject::connect( job, SIGNAL( data( KIO::Job*, QByteArray ) ),
                    this, SLOT( transferJobData( KIO::Job*, QByteArray ) ) );
}

#include "attachmentfromurljob.moc"
