/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>
  
  Based on KMail code by:
  Copyright (c) 1997 Markus Wuebben <markus.wuebben@kde.org>

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

#include "attachmentpart.h"
#include "globalpart.h"
#include "jobbase_p.h"

#include <QFileInfo>
#include <QTimer>

#include <KDebug>
#include <KGlobal>
#include <KIO/Scheduler>
#include <KLocale>
#include <KMimeType>

using namespace MessageComposer;

class MessageComposer::AttachmentFromUrlJobPrivate : public JobBasePrivate
{
  public:
    AttachmentFromUrlJobPrivate( AttachmentFromUrlJob *qq );

    void doStart(); // slot
    void transferJobData( KIO::Job *job, const QByteArray &jobData );
    void transferJobResult( KJob *job );

    KUrl url;
    qint64 maxSize;
    QByteArray data;
    AttachmentPart *part;

    Q_DECLARE_PUBLIC( AttachmentFromUrlJob )
};

AttachmentFromUrlJobPrivate::AttachmentFromUrlJobPrivate( AttachmentFromUrlJob *qq )
  : JobBasePrivate( qq )
  , maxSize( -1 )
  , part( 0 )
{
}

void AttachmentFromUrlJobPrivate::doStart()
{
  Q_Q( AttachmentFromUrlJob );

  if( !url.isValid() ) {
    q->setError( JobBase::UserError );
    q->setErrorText( i18n( "\"%1\" not found. Please specify the full path.", url.prettyUrl() ) );
    q->emitResult();
    return;
  }

  if( maxSize != -1 && url.isLocalFile() ) {
    const qint64 size = QFileInfo( url.toLocalFile() ).size();
    if( size > maxSize ) {
      q->setError( JobBase::UserError );
      q->setErrorText( i18n( "You may not attach files bigger than %1.",
                             KGlobal::locale()->formatByteSize( maxSize ) ) );
      q->emitResult();
      return;
    }
  }

  Q_ASSERT( data.isEmpty() ); // Not started twice.
  KIO::TransferJob *tjob = KIO::get( url, KIO::NoReload,
      ( q->globalPart()->isGuiEnabled() ? KIO::DefaultFlags : KIO::HideProgressInfo ) );
  KIO::Scheduler::scheduleJob( tjob );
  QObject::connect( tjob, SIGNAL(result(KJob*)), q, SLOT(transferJobResult(KJob*)) );
  QObject::connect( tjob, SIGNAL(data(KIO::Job*,QByteArray)),
                    q, SLOT(transferJobData(KIO::Job*,QByteArray)) );
}

void AttachmentFromUrlJobPrivate::transferJobData( KIO::Job *job, const QByteArray &jobData )
{
  Q_UNUSED( job );
  data += jobData;
  // TODO original code used a QBuffer; why?
}

void AttachmentFromUrlJobPrivate::transferJobResult( KJob *job )
{
  Q_Q( AttachmentFromUrlJob );

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
    filename = i18nc( "a file called 'unknown.ext'", "unknown%1",
                  KMimeType::mimeType( mimeType )->mainExtension() );
  }

  // Create the AttachmentPart.
  Q_ASSERT( part == 0 );
  part = new AttachmentPart;
  part->setMimeType( mimeType.toLatin1() );
  part->setName( filename );
  part->setFileName( filename );
  part->setData( data );
  q->emitResult(); // Success.
}



AttachmentFromUrlJob::AttachmentFromUrlJob( const KUrl &url, QObject *parent )
  : JobBase( *new AttachmentFromUrlJobPrivate( this ), parent )
{
  Q_D( AttachmentFromUrlJob );
  d->url = url;
}

AttachmentFromUrlJob::~AttachmentFromUrlJob()
{
}

void AttachmentFromUrlJob::start()
{
  QTimer::singleShot( 0, this, SLOT(doStart()) );
}

KUrl AttachmentFromUrlJob::url() const
{
  Q_D( const AttachmentFromUrlJob );
  return d->url;
}

void AttachmentFromUrlJob::setUrl( const KUrl &url )
{
  Q_D( AttachmentFromUrlJob );
  d->url = url;
}

qint64 AttachmentFromUrlJob::maximumAllowedSize() const
{
  Q_D( const AttachmentFromUrlJob );
  return d->maxSize;
}

void AttachmentFromUrlJob::setMaximumAllowedSize( qint64 size )
{
  Q_D( AttachmentFromUrlJob );
  d->maxSize = size;
}

AttachmentPart *AttachmentFromUrlJob::attachmentPart() const
{
  Q_D( const AttachmentFromUrlJob );
  return d->part;
}

#include "attachmentfromurljob.moc"
