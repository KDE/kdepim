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

#include <qdebug.h>
#include <kmime/kmime_content.h>

#include <boost/shared_ptr.hpp>

using namespace MessageCore;
using KMime::Content;

class MessageCore::AttachmentFromMimeContentJob::Private
{
public:
    const Content *mMimeContent;
};


AttachmentFromMimeContentJob::AttachmentFromMimeContentJob( const Content *content, QObject *parent )
    : AttachmentLoadJob( parent ),
      d( new Private )
{
    d->mMimeContent = content;
}

AttachmentFromMimeContentJob::~AttachmentFromMimeContentJob()
{
    delete d;
}

const Content *AttachmentFromMimeContentJob::mimeContent() const
{
    return d->mMimeContent;
}

void AttachmentFromMimeContentJob::setMimeContent( const Content *content )
{
    d->mMimeContent = content;
}

void AttachmentFromMimeContentJob::doStart()
{
    // Create the AttachmentPart.
    Q_ASSERT( attachmentPart() == 0 );

    AttachmentPart::Ptr part = AttachmentPart::Ptr( new AttachmentPart );
    Content *content = const_cast<Content*>( d->mMimeContent );
    part->setData( content->decodedContent() );

    // Get the details from the MIME headers.
    if ( content->contentType( false ) ) {
        part->setMimeType( content->contentType()->mimeType() );
        part->setName( content->contentType()->name() );
    }

    if ( content->contentTransferEncoding( false ) ) {
        part->setEncoding( content->contentTransferEncoding()->encoding() );
    }

    if ( content->contentDisposition( false ) ) {
        part->setFileName( content->contentDisposition()->filename() );
        part->setInline( content->contentDisposition()->disposition() == KMime::Headers::CDinline );
    }

    if ( content->contentDescription( false ) ) {
        part->setDescription( content->contentDescription()->asUnicodeString() );
    }

    setAttachmentPart( part );
    emitResult(); // Success.
}

