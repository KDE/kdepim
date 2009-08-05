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

#include "attachmentfrommimecontentjob.h"

#include "attachmentpart.h"
#include "jobbase_p.h"

#include <QTimer>

#include <KDebug>

#include "kmime/kmime_content.h"

using namespace MessageComposer;
using KMime::Content;

class MessageComposer::AttachmentFromMimeContentJobPrivate : public JobBasePrivate
{
  public:
    AttachmentFromMimeContentJobPrivate( AttachmentFromMimeContentJob *qq );

    void doStart(); // slot

    const Content *mimeContent;
    QByteArray data;
    AttachmentPart *part;

    Q_DECLARE_PUBLIC( AttachmentFromMimeContentJob )
};

AttachmentFromMimeContentJobPrivate::AttachmentFromMimeContentJobPrivate( AttachmentFromMimeContentJob *qq )
  : JobBasePrivate( qq )
  , part( 0 )
{
}

void AttachmentFromMimeContentJobPrivate::doStart()
{
  Q_Q( AttachmentFromMimeContentJob );

  // Create the AttachmentPart.
  Q_ASSERT( part == 0 );
  part = new AttachmentPart;
  Content *content = const_cast<Content*>( mimeContent );
  part->setData( content->decodedContent() );

  // Get the details from the MIME headers.
  if( content->contentType( false ) ) {
    part->setMimeType( content->contentType()->mimeType() );
    part->setName( content->contentType()->name() );
  }
  if( content->contentTransferEncoding( false ) ) {
    part->setEncoding( content->contentTransferEncoding()->encoding() );
  }
  if( content->contentDisposition( false ) ) {
    //part->setFileName( content->contentDisposition()->filename() ); TODO support filename
    part->setInline( content->contentDisposition()->disposition() == KMime::Headers::CDinline );
  }
  if( content->contentDescription( false ) ) {
    part->setDescription( content->contentDescription()->asUnicodeString() );
  }

  q->emitResult(); // Success.
}



AttachmentFromMimeContentJob::AttachmentFromMimeContentJob( const Content *content, QObject *parent )
  : JobBase( *new AttachmentFromMimeContentJobPrivate( this ), parent )
{
  Q_D( AttachmentFromMimeContentJob );
  d->mimeContent = content;
}

AttachmentFromMimeContentJob::~AttachmentFromMimeContentJob()
{
}

void AttachmentFromMimeContentJob::start()
{
  QTimer::singleShot( 0, this, SLOT(doStart()) );
}

const Content *AttachmentFromMimeContentJob::mimeContent() const
{
  Q_D( const AttachmentFromMimeContentJob );
  return d->mimeContent;
}

void AttachmentFromMimeContentJob::setMimeContent( const Content *content )
{
  Q_D( AttachmentFromMimeContentJob );
  d->mimeContent = content;
}

AttachmentPart *AttachmentFromMimeContentJob::attachmentPart() const
{
  Q_D( const AttachmentFromMimeContentJob );
  return d->part;
}

#include "attachmentfrommimecontentjob.moc"
