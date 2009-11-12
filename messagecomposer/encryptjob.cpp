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

#include "encryptjob.h"

#include "contentjobbase_p.h"
#include "kleo/cryptobackendfactory.h"
#include "kleo/cryptobackend.h"
#include "kleo/encryptjob.h"
#include "kleo/enum.h"
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

class Message::EncryptJobPrivate : public ContentJobBasePrivate
{
  public:
    EncryptJobPrivate( EncryptJob *qq )
      : ContentJobBasePrivate( qq )
      , content( 0 )
    {
    }

    KMime::Content* content;
    std::vector<GpgME::Key> keys;
    Kleo::CryptoMessageFormat format;
    QStringList recipients;

    // copied from messagecomposer.cpp
    bool binaryHint( Kleo::CryptoMessageFormat f )
    {
      switch ( f ) {
      case Kleo::SMIMEFormat:
      case Kleo::SMIMEOpaqueFormat:
        return true;
      default:
      case Kleo::OpenPGPMIMEFormat:
      case Kleo::InlineOpenPGPFormat:
        return false;
      }
    }


    GpgME::SignatureMode signingMode( Kleo::CryptoMessageFormat f )
    {
      switch ( f ) {
      case Kleo::SMIMEOpaqueFormat:
        return GpgME::NormalSignatureMode;
      case Kleo::InlineOpenPGPFormat:
        return GpgME::Clearsigned;
      default:
      case Kleo::SMIMEFormat:
      case Kleo::OpenPGPMIMEFormat:
        return GpgME::Detached;
      }
    }


    Q_DECLARE_PUBLIC( EncryptJob )
};

EncryptJob::EncryptJob( QObject *parent )
  : ContentJobBase( *new EncryptJobPrivate( this ), parent )
{
}

EncryptJob::~EncryptJob()
{
}


void EncryptJob::setContent( KMime::Content* content )
{
  Q_D( EncryptJob );

  d->content = content;
  d->content->assemble();
}

void EncryptJob::setCryptoMessageFormat( Kleo::CryptoMessageFormat format)
{
  Q_D( EncryptJob );

  d->format = format;
}

void EncryptJob::setEncryptionKeys( std::vector<GpgME::Key> keys )
{
  Q_D( EncryptJob );

  d->keys = keys;
}

void EncryptJob::setRecipients( QStringList recipients ) {
  Q_D( EncryptJob );

  d->recipients = recipients;
}

QStringList EncryptJob::recipients() {
  Q_D( EncryptJob );

  return d->recipients;
}

std::vector<GpgME::Key> EncryptJob::encryptionKeys() {
  Q_D( EncryptJob );

  return d->keys;
}

void EncryptJob::process()
{
  Q_D( EncryptJob );
  Q_ASSERT( d->resultContent == 0 ); // Not processed before.

  kDebug() << "starting encryption job";
  if( d->keys.size() == 0 ) { // not good
    kDebug() << "HELP! Encrypt job but have no keys to encrypt with.";
    // TODO handle gracefully
    return;
  }
  
  // if setContent hasn't been called, we assume that a subjob was added
  // and we want to use that
  if( !d->content || !d->content->hasContent() ) {
    Q_ASSERT( d->subjobContents.size() == 1 );
    d->content = d->subjobContents.first();
  }

  d->resultContent = new KMime::Content;

  const Kleo::CryptoBackend::Protocol *proto = 0;
  if( d->format & Kleo::AnyOpenPGP ) {
    proto = Kleo::CryptoBackendFactory::instance()->openpgp();
  } else if( d->format & Kleo::AnySMIME ) {
    proto = Kleo::CryptoBackendFactory::instance()->smime();
  }

  Q_ASSERT( proto );

  kDebug() << "got backend, starting job";
  Kleo::EncryptJob* seJob = proto->encryptJob( !d->binaryHint( d->format ), d->format == Kleo::InlineOpenPGPFormat );

  // for now just do the main recipients
  QByteArray encryptedBody;
  const GpgME::EncryptionResult res = seJob->exec( d->keys,
                                                   d->content->encodedContent(),
                                                   false,
                                                   encryptedBody );

  if ( res.error() ) {
    kDebug() << "encrypt failed:" << res.error().asString();
    //        job->showErrorDialog( globalPart()->parentWidgetForGui() );
  }
  d->resultContent = Message::Util::composeHeadersAndBody( d->content, encryptedBody, d->format, false );

  emitResult();
  return;

}

#include "encryptjob.moc"
