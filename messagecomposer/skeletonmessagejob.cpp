/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "skeletonmessagejob.h"

#include "infopart.h"
#include "jobbase_p.h"

#include <QTextCodec>
#include <QTimer>

#include <KCharsets>
#include <KDebug>
#include <KGlobal>

#include <kmime/kmime_message.h>

using namespace MessageComposer;
using namespace KMime;

class MessageComposer::SkeletonMessageJobPrivate : public JobBasePrivate
{
  public:
    SkeletonMessageJobPrivate( SkeletonMessageJob *qq )
      : JobBasePrivate( qq )
      , infoPart( 0 )
      , message( 0 )
    {
    }

    void doStart(); // slot

    InfoPart *infoPart;
    Message *message;

    Q_DECLARE_PUBLIC( SkeletonMessageJob )
};

void SkeletonMessageJobPrivate::doStart()
{
  Q_Q( SkeletonMessageJob );

  Q_ASSERT( infoPart );
  Q_ASSERT( message == 0 );
  message = new Message;

  // From:
  {
    Headers::From *from = new Headers::From( message );
    Types::Mailbox address;
    address.fromUnicodeString( infoPart->from() );
    from->addAddress( address );
    message->setHeader( from );
  }
  
  // To:
  {
    Headers::To *to = new Headers::To( message );
    foreach( const QString &a, infoPart->to() ) {
      Types::Mailbox address;
      address.fromUnicodeString( a );
      to->addAddress( address );
    }
    message->setHeader( to );
  }

  // Cc:
  {
    Headers::Cc *cc = new Headers::Cc( message );
    foreach( const QString &a, infoPart->cc() ) {
      Types::Mailbox address;
      address.fromUnicodeString( a );
      cc->addAddress( address );
    }
    message->setHeader( cc );
  }

  // Bcc:
  {
    Headers::Bcc *bcc = new Headers::Bcc( message );
    foreach( const QString &a, infoPart->bcc() ) {
      Types::Mailbox address;
      address.fromUnicodeString( a );
      bcc->addAddress( address );
    }
    message->setHeader( bcc );
  }

  // Subject:
  {
    Headers::Subject *subject = new Headers::Subject( message );
    subject->fromUnicodeString( infoPart->subject(), "utf-8" );
    // TODO should we be more specific about the charset?
    message->setHeader( subject );
  }

  q->emitResult(); // Success.
}


SkeletonMessageJob::SkeletonMessageJob( InfoPart *infoPart, QObject *parent )
  : JobBase( *new SkeletonMessageJobPrivate( this ), parent )
{
  Q_D( SkeletonMessageJob );
  d->infoPart = infoPart;
}

SkeletonMessageJob::~SkeletonMessageJob()
{
}

InfoPart *SkeletonMessageJob::infoPart() const
{
  Q_D( const SkeletonMessageJob );
  return d->infoPart;
}

void SkeletonMessageJob::setInfoPart( InfoPart *part )
{
  Q_D( SkeletonMessageJob );
  d->infoPart = part;
}

Message *SkeletonMessageJob::message() const
{
  Q_D( const SkeletonMessageJob );
  return d->message;
}

void SkeletonMessageJob::start()
{
  QTimer::singleShot( 0, this, SLOT(doStart()) );
}

#include "skeletonmessagejob.moc"
