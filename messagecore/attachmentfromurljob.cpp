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

#include <boost/shared_ptr.hpp>

#include <QFileInfo>

#include <KDebug>
#include <KGlobal>
#include <KIO/Scheduler>
#include <KLocale>
#include <KMimeType>

using namespace KPIM;

class KPIM::AttachmentFromUrlJob::Private
{
  public:
    Private( AttachmentFromUrlJob *qq );

    void transferJobData( KIO::Job *job, const QByteArray &jobData );
    void transferJobResult( KJob *job );

    AttachmentFromUrlJob *const q;
    KUrl url;
    qint64 maxSize;
    QByteArray data;
};

AttachmentFromUrlJob::Private::Private( AttachmentFromUrlJob *qq )
  : q( qq )
  , maxSize( -1 )
{
}

void AttachmentFromUrlJob::Private::transferJobData( KIO::Job *job, const QByteArray &jobData )
{
  Q_UNUSED( job );
  data += jobData;
  // TODO original code used a QBuffer; why?
}

void AttachmentFromUrlJob::Private::transferJobResult( KJob *job )
{
  if( job->error() ) {
    // TODO this loses useful stuff from KIO, like detailed error descriptions, causes+solutions,
    // ... use UiDelegate somehow?
    q->setError( job->error() );
    q->setErrorText( job->errorString() );
    q->emitResult();
    return;
  }

  Q_ASSERT( dynamic_cast<KIO::TransferJob*>( job ) );
  KIO::TransferJob *tjob = static_cast<KIO::TransferJob*>( job );

  // Determine the MIME type and filename of the attachment.
  QString mimeType = tjob->mimetype();
  kDebug() << "Mimetype is" << tjob->mimetype();
  QString filename = url.fileName();
  if( filename.isEmpty() ) {
    KMimeType::Ptr mtype = KMimeType::mimeType( mimeType, KMimeType::ResolveAliases );
    if ( mtype ) {
      filename = i18nc( "a file called 'unknown.ext'", "unknown%1",
                        mtype->mainExtension() );
    } else {
      filename = i18nc( "a filed called 'unknown'", "unknown" );
    }
  }

  // Create the AttachmentPart.
  Q_ASSERT( q->attachmentPart() == 0 ); // Not created before.
  AttachmentPart::Ptr part = AttachmentPart::Ptr( new AttachmentPart );
  part->setCharset( url.fileEncoding().toLatin1() );
  part->setMimeType( mimeType.toLatin1() );
  part->setName( filename );
  part->setFileName( filename );
  part->setData( data );
  q->setAttachmentPart( part );
  q->emitResult(); // Success.
}



AttachmentFromUrlJob::AttachmentFromUrlJob( const KUrl &url, QObject *parent )
  : AttachmentLoadJob( parent )
  , d( new Private( this ) )
{
  d->url = url;
}

AttachmentFromUrlJob::~AttachmentFromUrlJob()
{
  delete d;
}

KUrl AttachmentFromUrlJob::url() const
{
  return d->url;
}

void AttachmentFromUrlJob::setUrl( const KUrl &url )
{
  d->url = url;
}

qint64 AttachmentFromUrlJob::maximumAllowedSize() const
{
  return d->maxSize;
}

void AttachmentFromUrlJob::setMaximumAllowedSize( qint64 size )
{
  d->maxSize = size;
}

void AttachmentFromUrlJob::doStart()
{
  if( !d->url.isValid() ) {
    setError( KJob::UserDefinedError );
    setErrorText( i18n( "\"%1\" not found. Please specify the full path.", d->url.prettyUrl() ) );
    emitResult();
    return;
  }

  if( d->maxSize != -1 && d->url.isLocalFile() ) {
    const qint64 size = QFileInfo( d->url.toLocalFile() ).size();
    if( size > d->maxSize ) {
      setError( KJob::UserDefinedError );
      setErrorText( i18n( "You may not attach files bigger than %1.",
                          KGlobal::locale()->formatByteSize( d->maxSize ) ) );
      emitResult();
      return;
    }
  }

  Q_ASSERT( d->data.isEmpty() ); // Not started twice.
  KIO::TransferJob *tjob = KIO::get( d->url, KIO::NoReload,
      ( uiDelegate() ? KIO::DefaultFlags : KIO::HideProgressInfo ) );
  KIO::Scheduler::scheduleJob( tjob );
  QObject::connect( tjob, SIGNAL(result(KJob*)), this, SLOT(transferJobResult(KJob*)) );
  QObject::connect( tjob, SIGNAL(data(KIO::Job*,QByteArray)),
                    this, SLOT(transferJobData(KIO::Job*,QByteArray)) );
}

#include "attachmentfromurljob.moc"
