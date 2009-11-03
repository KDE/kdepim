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

#include "cryptojob.h"

#include "contentjobbase_p.h"
#include "encryptjob.h"
#include "kleo/cryptobackendfactory.h"
#include "kleo/cryptobackend.h"
#include "kleo/enum.h"
#include "kleo/signjob.h"
#include "kleo/signencryptjob.h"
#include "signjob.h"
#include "util.h"

#include <kdebug.h>
#include <kmime/kmime_message.h>
#include <kmime/kmime_content.h>
#include <QBuffer>

#include <gpgme++/global.h>
#include <gpgme++/signingresult.h>
#include <gpgme++/encryptionresult.h>
#include <sstream>

using namespace Message;

class Message::CryptoJobPrivate : public ContentJobBasePrivate
{
  public:
    CryptoJobPrivate( CryptoJob *qq )
      : ContentJobBasePrivate( qq )
      , content( 0 )
      , sign( false )
      , encrypt( false )
    {
    }

    KMime::Content* content;
    bool sign, encrypt;
    QStringList recipients;
    Kleo::CryptoMessageFormat format;
  
    Q_DECLARE_PUBLIC( CryptoJob )
};


CryptoJob::CryptoJob( QObject *parent )
  : ContentJobBase( *new CryptoJobPrivate( this ), parent )
{
}

CryptoJob::~CryptoJob()
{
}

void CryptoJob::setContent( KMime::Content* content )
{
  Q_D( CryptoJob );

  d->content = content;
}

void CryptoJob::setSignEncrypt( bool sign, bool encrypt )
{
  Q_D( CryptoJob );

  d->sign = sign;
  d->encrypt = encrypt;
}

void CryptoJob::setRecipients( QStringList recipients )
{
  Q_D( CryptoJob );

  d->recipients = recipients;
}

QStringList CryptoJob::recipients()
{
  Q_D( CryptoJob );

  return d->recipients;
}

void CryptoJob::setCryptoMessageFormat( Kleo::CryptoMessageFormat format)
{
  Q_D( CryptoJob );

  d->format = format;
}

void CryptoJob::doStart()
{
  Q_D( CryptoJob );
// TODO do what here again?
  ContentJobBase::doStart();

}

void CryptoJob::process()
{
  Q_D( CryptoJob );
  Q_ASSERT( d->resultContent == 0 ); // Not processed before.
  d->resultContent = new KMime::Content;
  // ok we have the signed, encrypted, or both, message computed by the subjob/s
  // now compose the new message
  kDebug() << "format is:" << d->format;
  if( d->encrypt )  {
    Q_ASSERT( d->subjobContents.count() == 1 );
    // if we encrypt, we always encrypt last, so just use the body of the subjob
    QByteArray body = d->subjobContents.first()->body();

    d->resultContent = Message::Util::composeHeadersAndBody( d->content, body, d->format, false /* not signing */ );
  } else if( d->sign ) {
    // just signing, we need cleartext + sig
    QByteArray body = d->subjobContents.first()->body();
    d->resultContent = d->content;
   // d->resultContent = Message::Util::composeHeadersAndBody( d->content, body, d->format, d->sign );
  }
  emitResult();
}

#include "cryptojob.moc"
