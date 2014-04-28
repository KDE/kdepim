/*
  Copyright (C) 2009 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Leo Franchi <lfranchi@kde.org>

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

#include "transparentjob.h"

#include "contentjobbase_p.h"

#include <qdebug.h>
#include <KMime/kmime_message.h>
#include <KMime/kmime_content.h>
using namespace MessageComposer;

class MessageComposer::TransparentJobPrivate : public MessageComposer::ContentJobBasePrivate
{
public:
    TransparentJobPrivate( TransparentJob *qq )
        : ContentJobBasePrivate( qq )
        , content( 0 )
    {
    }

    KMime::Content* content;
    
    Q_DECLARE_PUBLIC( TransparentJob )
};

TransparentJob::TransparentJob( QObject *parent )
    : MessageComposer::ContentJobBase( *new TransparentJobPrivate( this ), parent )
{
}

TransparentJob::~TransparentJob()
{
}


void TransparentJob::setContent( KMime::Content* content )
{
    Q_D( TransparentJob );

    d->content = content;
}


void TransparentJob::process()
{
    Q_D( TransparentJob );
    d->resultContent = d->content;
    emitResult();
}

