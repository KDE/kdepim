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

#include "attachmentfrompublickeyjob.h"

#include "attachmentpart.h"
#include "globalpart.h"
#include "jobbase_p.h"

#include <QTimer>

#include <KDebug>
#include <KLocalizedString>

#include <kleo/cryptobackendfactory.h>
#include <kleo/exportjob.h>
#include <kleo/ui/progressdialog.h>

using namespace MessageComposer;

class MessageComposer::AttachmentFromPublicKeyJobPrivate : public JobBasePrivate
{
  public:
    AttachmentFromPublicKeyJobPrivate( AttachmentFromPublicKeyJob *qq );

    void doStart(); // slot
    void exportResult( const GpgME::Error &error, const QByteArray &keyData ); // slot
    void emitGpgError( const GpgME::Error &error );

    QString fingerprint;
    QByteArray data;
    AttachmentPart *part;

    Q_DECLARE_PUBLIC( AttachmentFromPublicKeyJob )
};

AttachmentFromPublicKeyJobPrivate::AttachmentFromPublicKeyJobPrivate( AttachmentFromPublicKeyJob *qq )
  : JobBasePrivate( qq )
  , part( 0 )
{
}

void AttachmentFromPublicKeyJobPrivate::doStart()
{
  Q_Q( AttachmentFromPublicKeyJob );

  Kleo::ExportJob *job = Kleo::CryptoBackendFactory::instance()->openpgp()->publicKeyExportJob( true );
  Q_ASSERT( job );
  QObject::connect( job, SIGNAL(result(GpgME::Error,QByteArray)),
      q, SLOT(exportResult(GpgME::Error,QByteArray)) );

  const GpgME::Error error = job->start( QStringList( fingerprint ) );
  if( error ) {
    emitGpgError( error );
    // TODO check autodeletion policy of Kleo::Jobs...
    return;
  } else if( q->globalPart()->isGuiEnabled() ) {
    (void)new Kleo::ProgressDialog( job, i18n( "Exporting key..." ),
                                    q->globalPart()->parentWidgetForGui() );
  }
}

void AttachmentFromPublicKeyJobPrivate::exportResult( const GpgME::Error &error, const QByteArray &keyData )
{
  Q_Q( AttachmentFromPublicKeyJob );

  if( error ) {
    emitGpgError( error );
    return;
  }

  // Create the AttachmentPart.
  Q_ASSERT( part == 0 );
  part = new AttachmentPart;
  part->setName( i18n( "OpenPGP key 0x%1", fingerprint ) );
  part->setFileName( QString::fromLatin1( "0x" + fingerprint.toLatin1() + ".asc" ) );
  part->setMimeType( "application/pgp-keys" );
  part->setData( keyData );

  q->emitResult(); // Success.
}

void AttachmentFromPublicKeyJobPrivate::emitGpgError( const GpgME::Error &error )
{
  Q_Q( AttachmentFromPublicKeyJob );

  Q_ASSERT( error );
  const QString msg = i18n( "<p>An error occurred while trying to export "
         "the key from the backend:</p>"
         "<p><b>%1</b></p>",
         QString::fromLocal8Bit( error.asString() ) );
  q->setError( JobBase::UserError );
  q->setErrorText( msg );
  q->emitResult();
}



AttachmentFromPublicKeyJob::AttachmentFromPublicKeyJob( const QString &fingerprint, QObject *parent )
  : JobBase( *new AttachmentFromPublicKeyJobPrivate( this ), parent )
{
  Q_D( AttachmentFromPublicKeyJob );
  d->fingerprint = fingerprint;
}

AttachmentFromPublicKeyJob::~AttachmentFromPublicKeyJob()
{
}

void AttachmentFromPublicKeyJob::start()
{
  QTimer::singleShot( 0, this, SLOT(doStart()) );
}

QString AttachmentFromPublicKeyJob::fingerprint() const
{
  Q_D( const AttachmentFromPublicKeyJob );
  return d->fingerprint;
}

void AttachmentFromPublicKeyJob::setFingerprint( const QString &fingerprint )
{
  Q_D( AttachmentFromPublicKeyJob );
  d->fingerprint = fingerprint;
}

AttachmentPart *AttachmentFromPublicKeyJob::attachmentPart() const
{
  Q_D( const AttachmentFromPublicKeyJob );
  return d->part;
}

#include "attachmentfrompublickeyjob.moc"
