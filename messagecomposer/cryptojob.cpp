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


    KMime::Content* composeHeadersAndBody( KMime::Content* orig, QByteArray encodedBody,  Kleo::CryptoMessageFormat format, bool sign  );
    void makeToplevelContentType( KMime::Content* content, Kleo::CryptoMessageFormat format, bool sign );
    void setNestedContentType( KMime::Content* content, Kleo::CryptoMessageFormat format, bool sign );
    void setNestedContentDisposition( KMime::Content* content, Kleo::CryptoMessageFormat format, bool sign );
    bool makeMultiMime( Kleo::CryptoMessageFormat f, bool sign );
    
    KMime::Content* content;
    bool sign, encrypt;
    QStringList encRecipients;
    std::vector<GpgME::Key> encKeys;
    std::vector<GpgME::Key> signers;
    Kleo::CryptoMessageFormat format;
  
    Q_DECLARE_PUBLIC( CryptoJob )
};

KMime::Content* CryptoJobPrivate::composeHeadersAndBody( KMime::Content* orig, QByteArray encodedBody, Kleo::CryptoMessageFormat format, bool sign )
{
  Q_Q( CryptoJob );

  KMime::Content* result = new KMime::Content;

  if( !( format & Kleo::InlineOpenPGPFormat ) ) { // make a MIME message
    // make headers and CE+CTE
    kDebug() << "making MIME message";
    makeToplevelContentType( result, format, sign );
    const QByteArray boundary = KMime::multiPartBoundary();

    if( makeMultiMime( format, sign ) ) {
      result->contentType()->setBoundary( boundary );
    }

    if( format & Kleo::SMIMEOpaqueFormat ) {
      result->contentDisposition()->setDisposition( KMime::Headers::CDattachment );
      result->contentDisposition()->setFilename( QString::fromAscii( "smime.p7m" ) );
    }

    if( !makeMultiMime( format, sign ) && format & Kleo::AnySMIME ) {
      result->contentTransferEncoding()->setEncoding( KMime::Headers::CEbase64 );
    } else {
      result->contentTransferEncoding()->setEncoding( orig->contentTransferEncoding()->encoding() );

    }

    result->assemble();
    kDebug() << "processed header:" << result->head();
    // now make the body
    if( !makeMultiMime( format, sign ) ) {
      // set body to be body + sig
      result->setBody( orig->body() + "\n" + encodedBody );
    } else {
      // Build the encapsulated MIME parts.
      // Build a MIME part holding the version information
      // taking the body contents returned in
      // structuring.data.bodyTextVersion.
      KMime::Content* vers = 0;
      if( !sign && format == Kleo::OpenPGPMIMEFormat ) {
        vers = new KMime::Content;
        vers->contentType()->setMimeType( "application/php-encrypted" );
        vers->contentDisposition()->setDisposition( KMime::Headers::CDattachment );
        vers->setBody( "Version: 1" );

      }

      // Build a MIME part holding the code information
      // taking the body contents returned in ciphertext.
      KMime::Content* code = new KMime::Content;
      setNestedContentType( code, format, sign );
      setNestedContentDisposition( code, format, sign );

      if( format & Kleo::AnySMIME ) {
        code->contentTransferEncoding()->setEncoding( KMime::Headers::CEbase64 );
        code->contentTransferEncoding()->needToEncode();
      }
      code->setBody( encodedBody );

      // compose the multi-part message
      KMime::Content* main =  new KMime::Content;
      if( sign && makeMultiMime( format, true ) )
        result->addContent( orig );
      if( vers )
        result->addContent( vers );
      result->addContent( code );
    }
  } else { // not MIME, just plain message
    result->setHead( orig->head() );

    QByteArray resultingBody;
    if( sign && makeMultiMime( format, true ) )
      resultingBody += orig->body();
    if( !encodedBody.isEmpty() )
      resultingBody += encodedBody;
    else {
      kDebug() << "Got no encoded payload trying to save as plaintext inline pgp!";
      // TODO handle gracefully
    }

    result->setBody( resultingBody );
  }
  result->assemble();
  return result;

}

// set the correct top-level ContentType on the message
void CryptoJobPrivate::makeToplevelContentType( KMime::Content* content, Kleo::CryptoMessageFormat format, bool sign )
{
  Q_Q( CryptoJob );
  switch ( format ) {
    default:
    case Kleo::InlineOpenPGPFormat:
    case Kleo::OpenPGPMIMEFormat:
      if( sign ) {
        content->contentType()->setMimeType( QByteArray( "multipart/signed" ) );
        content->contentType()->setParameter( QString::fromAscii( "protocol" ), QString::fromAscii( "application/pgp-signature" ) );
        content->contentType()->setParameter( QString::fromAscii( "micalg" ), QString::fromAscii( "pgp-sha1" ) );

      } else {
        content->contentType()->setMimeType( QByteArray( "multipart/encryted" ) );
        content->contentType()->setParameter( QString::fromAscii( "protocol" ), QString::fromAscii( "application/pgp-signature" ) );
      }
      return;
    case Kleo::SMIMEFormat:
      if ( sign ) {
        kDebug() << "setting headers for SMIME";
        content->contentType()->setMimeType( QByteArray( "multipart/signed" ) );
        content->contentType()->setParameter( QString::fromAscii( "protocol" ), QString::fromAscii( "application/pkcs7-signature" ) );
        content->contentType()->setParameter( QString::fromAscii( "micalg" ), QString::fromAscii( "sha1" ) );// FIXME: obtain this parameter from gpgme!
        return;
      }
      // fall through (for encryption, there's no difference between
      // SMIME and SMIMEOpaque, since there is no mp/encrypted for
      // S/MIME)
    case Kleo::SMIMEOpaqueFormat:

        kDebug() << "setting headers for SMIME/opaque";
      content->contentType()->setMimeType( QByteArray( "application/pkcs7-mime" ) );
      content->contentType()->setParameter( QString::fromAscii( "name" ), QString::fromAscii( "smime.p7m" ) );

      if( sign ) {
        content->contentType()->setParameter( QString::fromAscii( "smime-type" ), QString::fromAscii( "signed-data" ) );
      } else {
        content->contentType()->setParameter( QString::fromAscii( "smime-type" ), QString::fromAscii( "enveloped-data" ) );
      }
  }
}


void CryptoJobPrivate::setNestedContentType( KMime::Content* content, Kleo::CryptoMessageFormat format, bool sign )
{
  Q_Q( CryptoJob );
  switch( format ){
    case Kleo::OpenPGPMIMEFormat:
      if( sign ) {
        content->contentType()->setMimeType( QByteArray( "application/pgp-signature" ) );
        content->contentType()->setParameter( QString::fromAscii( "name" ), QString::fromAscii( "signature.asc" ) );
        content->contentDescription()->from7BitString( "This is a digitally signed message part." );
      } else {
        content->contentType()->setMimeType( QByteArray( "application/octet-stream" ) );
      }
      return;
    case Kleo::SMIMEFormat:
      if ( sign ) {
        content->contentType()->setMimeType( QByteArray( "application/pkcs7-signature" ) );
        content->contentType()->setParameter( QString::fromAscii( "name" ), QString::fromAscii( "smime.p7s" ) );
        return;
      }
      // fall through:
    default:
    case Kleo::InlineOpenPGPFormat:
    case Kleo::SMIMEOpaqueFormat:
      ;
  }
}


void CryptoJobPrivate::setNestedContentDisposition( KMime::Content* content, Kleo::CryptoMessageFormat format, bool sign )
{
  Q_Q( CryptoJob );
  if ( !sign && format & Kleo::OpenPGPMIMEFormat ) {
    content->contentDisposition()->setDisposition( KMime::Headers::CDinline );
    content->contentDisposition()->setFilename( QString::fromAscii( "msg.asc" ) );
  } else if ( sign && format & Kleo::SMIMEFormat ) {
    content->contentDisposition()->setDisposition( KMime::Headers::CDattachment );
    content->contentDisposition()->setFilename( QString::fromAscii( "smime.p7s" ) );
  }
  content->assemble();
}


bool CryptoJobPrivate::makeMultiMime( Kleo::CryptoMessageFormat f, bool sign )
{
  Q_Q( CryptoJob );
  switch ( format ) {
    default:
    case Kleo::InlineOpenPGPFormat:
    case Kleo::SMIMEOpaqueFormat:   return false;
    case Kleo::OpenPGPMIMEFormat:   return true;
    case Kleo::SMIMEFormat:         return sign; // only on sign - there's no mp/encrypted for S/MIME
  }
}

CryptoJob::CryptoJob( QObject *parent )
  : ContentJobBase( *new CryptoJobPrivate( this ), parent )
{
}

CryptoJob::~CryptoJob()
{
}

void CryptoJob::setSignEncrypt( bool sign, bool encrypt )
{
  Q_D( CryptoJob );

  d->sign = sign;
  d->encrypt = encrypt;
}

void CryptoJob::setContent( KMime::Content* content )
{
  Q_D( CryptoJob );
  
  d->content = content;
}

void CryptoJob::setCryptoMessageFormat( Kleo::CryptoMessageFormat format)
{
  Q_D( CryptoJob );

  d->format = format;
}
    
void CryptoJob::setEncryptionItems( QStringList recipients, std::vector<GpgME::Key> keys )
{
  Q_D( CryptoJob );

  d->encRecipients = recipients;
  d->encKeys = keys;
}

void CryptoJob::setSigningKeys( std::vector<GpgME::Key>& signers )
{
  Q_D( CryptoJob );

  d->signers = signers;
}

void CryptoJob::doStart()
{
  Q_D( CryptoJob );

  if( d->sign && d->encrypt ) { // both, so we sign then encrypte
    EncryptJob* eJob = new EncryptJob( this );
    // TODO implement

  } 
  if( d->sign ) {
    SignJob* sJob = new SignJob( this );
    sJob->setContent( d->content );
    sJob->setCryptoMessageFormat( d->format );
    sJob->setSigningKeys( d->signers );
    
    appendSubjob( sJob );
  }
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
  if( d->sign )  {
    Q_ASSERT( d->subjobContents.count() == 1 );
    QByteArray sig = d->subjobContents.first()->body();

    d->resultContent = d->composeHeadersAndBody( d->content, sig, d->format, d->sign );
  }

  emitResult();
}

#include "cryptojob.moc"
