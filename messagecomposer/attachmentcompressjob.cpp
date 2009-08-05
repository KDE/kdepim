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

#include "attachmentcompressjob.h"

#include "attachmentpart.h"
#include "globalpart.h"
#include "jobbase_p.h"

#include <QBuffer>
#include <QTimer>

#include <KDebug>
#include <KLocalizedString>
#include <KMessageBox>
#include <KZip>

using namespace MessageComposer;

class MessageComposer::AttachmentCompressJobPrivate : public JobBasePrivate
{
  public:
    AttachmentCompressJobPrivate( AttachmentCompressJob *qq );

    void doStart(); // slot

    const AttachmentPart *originalPart;
    AttachmentPart *compressedPart;
    bool warnCompressedSizeLarger;

    Q_DECLARE_PUBLIC( AttachmentCompressJob )
};

AttachmentCompressJobPrivate::AttachmentCompressJobPrivate( AttachmentCompressJob *qq )
  : JobBasePrivate( qq )
  , originalPart( 0 )
  , compressedPart( 0 )
  , warnCompressedSizeLarger( true )
{
}

void AttachmentCompressJobPrivate::doStart()
{
  Q_Q( AttachmentCompressJob );

  Q_ASSERT( originalPart );
  QByteArray decoded = originalPart->data();

  QByteArray array;
  QBuffer dev( &array );
  KZip zip( &dev );
  if( !zip.open( QIODevice::WriteOnly ) ) {
    q->setError( JobBase::BugError );
    q->setErrorText( i18n( "Could not initiate attachment compression." ) );
    q->emitResult();
    return;
  }

  // Compress.
  zip.setCompression( KZip::DeflateCompression );
  if( !zip.writeFile( originalPart->name(), QString( /*user*/ ), QString( /*group*/ ),
                      decoded.data(), decoded.size() ) ) {
    q->setError( JobBase::BugError );
    q->setErrorText( i18n( "Could not compress the attachment." ) );
    q->emitResult();
    return;
  }
  zip.close();

  // Check size.
  if( warnCompressedSizeLarger && array.size() >= decoded.size() ) {
    if( !q->globalPart()->isGuiEnabled() ) {
      q->setError( JobBase::UserError );
      q->setErrorText( i18n( "The compressed attachment is larger than the original." ) );
      q->emitResult();
      return;
    } else {
      int result = KMessageBox::questionYesNo( q->globalPart()->parentWidgetForGui(),
            i18n( "The compressed attachment is larger than the original. "
                  "Do you want to keep the original one?" ),
            QString( /*caption*/ ),
            KGuiItem( i18nc( "Do not compress", "Keep" ) ),
            KGuiItem( i18n( "Compress" ) ) );
      if( result == KMessageBox::Yes ) {
        q->setError( JobBase::UserCancelledError );
        q->setErrorText( i18n( "The user chose to keep the uncompressed file." ) );
        q->emitResult();
        return;
      }
    }
  }

  // Create new part.
  Q_ASSERT( compressedPart == 0 );
  compressedPart = new AttachmentPart;
  compressedPart->setName( originalPart->name() + QString::fromLatin1( ".zip" ) );
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



AttachmentCompressJob::AttachmentCompressJob( const AttachmentPart *part, QObject *parent )
  : JobBase( *new AttachmentCompressJobPrivate( this ), parent )
{
  Q_D( AttachmentCompressJob );
  d->originalPart = part;
}

AttachmentCompressJob::~AttachmentCompressJob()
{
}

void AttachmentCompressJob::start()
{
  QTimer::singleShot( 0, this, SLOT(doStart()) );
}

const AttachmentPart *AttachmentCompressJob::originalPart() const
{
  Q_D( const AttachmentCompressJob );
  return d->originalPart;
}

void AttachmentCompressJob::setOriginalPart( const AttachmentPart *part )
{
  Q_D( AttachmentCompressJob );
  d->originalPart = part;
}

AttachmentPart *AttachmentCompressJob::compressedPart() const
{
  Q_D( const AttachmentCompressJob );
  return d->compressedPart;
}

bool AttachmentCompressJob::warnCompressedSizeLarger() const
{
  Q_D( const AttachmentCompressJob );
  return d->warnCompressedSizeLarger;
}

void AttachmentCompressJob::setWarnCompressedSizeLarger( bool warn )
{
  Q_D( AttachmentCompressJob );
  d->warnCompressedSizeLarger = warn;
}

#include "attachmentcompressjob.moc"
