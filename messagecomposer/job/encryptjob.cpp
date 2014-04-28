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

#include <qdebug.h>

#include <KMime/kmime_message.h>
#include <KMime/kmime_content.h>
#include <QBuffer>

#include <gpgme++/global.h>
#include <gpgme++/signingresult.h>
#include <gpgme++/encryptionresult.h>
#include <sstream>

using namespace MessageComposer;

class MessageComposer::EncryptJobPrivate : public ContentJobBasePrivate
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

void EncryptJob::setEncryptionKeys( const std::vector<GpgME::Key>& keys )
{
    Q_D( EncryptJob );

    d->keys = keys;
}

void EncryptJob::setRecipients( const QStringList& recipients ) {
    Q_D( EncryptJob );

    d->recipients = recipients;
}

QStringList EncryptJob::recipients() const {
    Q_D( const EncryptJob );

    return d->recipients;
}

std::vector<GpgME::Key> EncryptJob::encryptionKeys() const {
    Q_D( const EncryptJob );

    return d->keys;
}

void EncryptJob::process()
{
    Q_D( EncryptJob );
    Q_ASSERT( d->resultContent == 0 ); // Not processed before.

    if( d->keys.size() == 0 ) { // should not happen---resolver should have dealt with it earlier
        qDebug() << "HELP! Encrypt job but have no keys to encrypt with.";
        return;
    }

    // if setContent hasn't been called, we assume that a subjob was added
    // and we want to use that
    if( !d->content || !d->content->hasContent() ) {
        Q_ASSERT( d->subjobContents.size() == 1 );
        d->content = d->subjobContents.first();
    }

    //d->resultContent = new KMime::Content;

    const Kleo::CryptoBackend::Protocol *proto = 0;
    if( d->format & Kleo::AnyOpenPGP ) {
        proto = Kleo::CryptoBackendFactory::instance()->openpgp();
    } else if( d->format & Kleo::AnySMIME ) {
        proto = Kleo::CryptoBackendFactory::instance()->smime();
    } else {
        qDebug() << "HELP! Encrypt job but have protocol to encrypt with.";
        return;
    }

    Q_ASSERT( proto );

    qDebug() << "got backend, starting job";
    Kleo::EncryptJob* seJob = proto->encryptJob( !d->binaryHint( d->format ), d->format == Kleo::InlineOpenPGPFormat );

    // for now just do the main recipients
    QByteArray encryptedBody;
    QByteArray content;
    d->content->assemble();
    if( d->format & Kleo::InlineOpenPGPFormat ) {
        content = d->content->body();
    } else {
        content = d->content->encodedContent();
    }

    // FIXME: Make async!
    const GpgME::EncryptionResult res = seJob->exec( d->keys,
                                                     content,
                                                     true, // 'alwaysTrust' provided keys
                                                     encryptedBody );

    // exec'ed jobs don't delete themselves
    seJob->deleteLater();

    if ( res.error() ) {
        setError( res.error().code() );
        setErrorText( QString::fromLocal8Bit( res.error().asString() ) );
        emitResult();
        return;
    }
    d->resultContent = MessageComposer::Util::composeHeadersAndBody( d->content, encryptedBody, d->format, false );

    emitResult();
    return;

}

