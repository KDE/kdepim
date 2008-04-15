/* -*- mode: c++; c-basic-offset:4 -*-
    decryptverifytask.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "decryptverifytask.h"

#include <kleo/verifyopaquejob.h>
#include <kleo/verifydetachedjob.h>
#include <kleo/decryptjob.h>
#include <kleo/decryptverifyjob.h>

#include <models/keycache.h>
#include <models/predicates.h>

#include <utils/detail_p.h>
#include <utils/input.h>
#include <utils/output.h>
#include <utils/classify.h>
#include <utils/formatting.h>
#include <utils/stl_util.h>
#include <utils/kleo_assert.h>
#include <utils/exception.h>

#include <gpgme++/error.h>
#include <gpgme++/key.h>
#include <gpgme++/verificationresult.h>
#include <gpgme++/decryptionresult.h>

#include <gpg-error.h>

#include <KIconLoader>
#include <KLocalizedString>

#include <QByteArray>
#include <QColor>
#include <QTextDocument> // Qt::escape

#include <algorithm>

using namespace Kleo::Crypto;
using namespace Kleo;
using namespace GpgME;
using namespace boost;

namespace {

static const char * iconForSignature( const Signature & sig ) {
    if ( sig.summary() & GpgME::Signature::Green )
        return "dialog-ok";
    if ( sig.summary() & GpgME::Signature::Red )
        return "dialog-error";
    return "dialog-warning";
}


static QString image( const char * img ) {
    // ### escape?
    return "<img src=\"" + KIconLoader::global()->iconPath( img, KIconLoader::Small ) + "\"/>";
}

static QColor color( const DecryptionResult & dr, const VerificationResult & vr ) {
    if ( !dr.isNull() && dr.error() )
        return Qt::red;
    if ( !vr.isNull() && vr.error() )
        return Qt::red;
    return Qt::gray;
}

static QColor color( const Signature & sig ) {
    if ( sig.summary() & GpgME::Signature::Red )
        return Qt::red;
    if ( sig.summary() & GpgME::Signature::Green )
        return Qt::green;
    return Qt::yellow;
}

static bool IsErrorOrCanceled( const GpgME::Error & err )
{
    return err || err.isCanceled();
}

static bool IsErrorOrCanceled( const Result & res )
{
    return IsErrorOrCanceled( res.error() );
}

QString renderKey( const Key & key ) {
    if ( key.isNull() )
        return i18n( "Unknown key" );
    return QString::fromLatin1( "<a href=\"key:%1\">%2</a>" ).arg( key.primaryFingerprint(), Formatting::prettyName( key ) );
}

QString formatVerificationResultOverview( const VerificationResult & res ) {
    if ( res.isNull() )
        return QString();

    const Error err = res.error();

    QString overview;

    // Icon:
    overview += image( err ? "dialog-error" : "dialog-ok" );

    // Summary:
    overview += "<b>";
    if ( err.isCanceled() )
        overview += i18n("Verification canceled.");
    else if ( err )
        overview += i18n( "Verification failed: %1.", Qt::escape( QString::fromLocal8Bit( err.asString() ) ) );
    else
        overview += i18n("Verification succeeded.");
    overview += "</b>";

    return overview;
}

static QString formatDecryptionResultOverview( const DecryptionResult & result )
{
    const Error err = result.error();
    QString overview;

    // Icon:
    overview += image( err ? "dialog-error" : "dialog-ok" );

    overview += "<b>";
    if ( err.isCanceled() )
        overview += i18n("Decryption canceled.");
    else if ( err )
        overview += i18n( "Decryption failed: %1.", Qt::escape( QString::fromLocal8Bit( err.asString() ) ) );
    else
        overview += i18n("Decryption succeeded." );
    overview += "</b>";
    return overview;
}


QString formatSignature( const Signature & sig, const Key & key ) {
    if ( sig.isNull() )
        return QString();

    const bool red   = (sig.summary() & Signature::Red);
    //const bool green = (sig.summary() & Signature::Green);
    const bool valid = (sig.summary() & Signature::Valid);

    if ( red )
        if ( key.isNull() )
            if ( const char * fpr = sig.fingerprint() )
                return i18n("Bad signature by unknown key %1: %2", QString::fromLatin1( fpr ), QString::fromLocal8Bit( sig.status().asString() ) );
            else
                return i18n("Bad signature by an unknown key: %1", QString::fromLocal8Bit( sig.status().asString() ) );
        else
            return i18n("Bad signature by %1: %2", renderKey( key ), QString::fromLocal8Bit( sig.status().asString() ) );

    else if ( valid )
        if ( key.isNull() )
            if ( const char * fpr = sig.fingerprint() )
                return i18n("Good signature by unknown key %1.", QString::fromLatin1( fpr ) );
            else
                return i18n("Good signature by an unknown key.");
        else
            return i18n("Good signature by %1.", renderKey( key ) );

    else
        if ( key.isNull() )
            if ( const char * fpr = sig.fingerprint() )
                return i18n("Invalid signature by unknown key %1: %2", QString::fromLatin1( fpr ), QString::fromLocal8Bit( sig.status().asString() ) );
            else
                return i18n("Invalid signature by an unknown key: %1", QString::fromLocal8Bit( sig.status().asString() ) );
        else
            return i18n("Invalid signature by %1: %2", renderKey( key ), QString::fromLocal8Bit( sig.status().asString() ) );
}

static QString formatVerificationResultDetails( const VerificationResult & res )
{
    const std::vector<Signature> sigs = res.signatures();
    const std::vector<Key> signers = KeyCache::instance()->findSigners( res );
    QString details;
    Q_FOREACH ( const Signature & sig, sigs )
        details += formatSignature( sig, DecryptVerifyResult::keyForSignature( sig, signers ) ) + '\n';
    details = details.trimmed();
    details.replace( '\n', "<br/>" );
    return details;
}

static QString formatDecryptionResultDetails( const DecryptionResult & res, const std::vector<Key> & recipients )
{
    if ( res.isNull() || !res.error() || res.error().isCanceled() )
        return QString();

    if ( recipients.empty() )
        return QString( "<i>" + i18np( "One unknown recipient.", "%1 unknown recipients.", res.numRecipients() ) + "</i>" );

    QString details;
    details += i18np( "Recipients:", "Recipients:", res.numRecipients() );
    if ( res.numRecipients() == 1 )
        return details + renderKey( recipients.front() );

    details += "<ul>";
    Q_FOREACH( const Key & key, recipients )
        details += "<li>" + renderKey( key ) + "</li>";
    if ( recipients.size() < res.numRecipients() )
        details += "<li><i>" + i18np( "One unknown recipient", "%1 unknown recipients",
                                   res.numRecipients() - recipients.size() ) + "</i></li>";

    return details + "</ul>";
}

static QString formatDecryptVerifyResultOverview( const DecryptionResult & dr, const VerificationResult & vr )
{
    const QString drOverview = formatDecryptionResultOverview( dr );
    if ( IsErrorOrCanceled( dr ) )
        return drOverview;
    return drOverview + "<br/>" + formatVerificationResultOverview( vr );
}


static QString formatDecryptVerifyResultDetails( const DecryptionResult & dr,
                                                 const VerificationResult & vr,
                                                 const std::vector<Key> & recipients )
{
    const QString drDetails = formatDecryptionResultDetails( dr, recipients );
    if ( IsErrorOrCanceled( dr ) )
        return drDetails;
    return drDetails + ( drDetails.isEmpty() ? "" : "<br/>" ) + formatVerificationResultDetails( vr );
}

}

class DecryptVerifyResult::Private {
public:
    Private( DecryptVerifyTask::Type type,
             const VerificationResult & vr,
             const DecryptionResult & dr,
             const QByteArray & stuff,
             int errCode,
             const QString& errString ) :
                 m_type( type ),
                 m_verificationResult( vr ),
                 m_decryptionResult( dr ),
                 m_stuff( stuff ),
                 m_error( errCode ),
                 m_errorString( errString )
    {
    }

    bool isDecryptOnly() const { return m_type == DecryptVerifyTask::Decrypt; }
    bool isVerifyOnly() const { return m_type == DecryptVerifyTask::VerifyOpaque || m_type == DecryptVerifyTask::VerifyOpaque; }
    bool isDecryptVerify() const { return m_type == DecryptVerifyTask::DecryptVerify; }
    DecryptVerifyTask::Type m_type;
    VerificationResult m_verificationResult;
    DecryptionResult m_decryptionResult;
    QByteArray m_stuff;
    int m_error;
    QString m_errorString;
};

shared_ptr<DecryptVerifyResult> DecryptVerifyResult::fromDecryptResult( const DecryptionResult & dr, const QByteArray & plaintext ) {
    return shared_ptr<DecryptVerifyResult>( new DecryptVerifyResult(
        DecryptVerifyTask::Decrypt,
        VerificationResult(),
        dr,
        plaintext,
        0,
        QString() ) );
}

shared_ptr<DecryptVerifyResult> DecryptVerifyResult::fromDecryptResult( const GpgME::Error & err, const QString& what ) {
    return shared_ptr<DecryptVerifyResult>( new DecryptVerifyResult(
        DecryptVerifyTask::Decrypt,
        VerificationResult(),
        DecryptionResult( err ),
        QByteArray(),
        err.code(),
        what ) );
}

shared_ptr<DecryptVerifyResult> DecryptVerifyResult::fromDecryptVerifyResult( const DecryptionResult & dr, const VerificationResult & vr, const QByteArray & plaintext ) {
    return shared_ptr<DecryptVerifyResult>( new DecryptVerifyResult(
        DecryptVerifyTask::DecryptVerify,
        vr,
        dr,
        plaintext,
        0,
        QString() ) );
}

shared_ptr<DecryptVerifyResult> DecryptVerifyResult::fromDecryptVerifyResult( const GpgME::Error & err, const QString & details ) {
    return shared_ptr<DecryptVerifyResult>( new DecryptVerifyResult(
        DecryptVerifyTask::DecryptVerify,
        VerificationResult(),
        DecryptionResult( err ),
        QByteArray(),
        err.code(),
        details ) );
}

shared_ptr<DecryptVerifyResult> DecryptVerifyResult::fromVerifyOpaqueResult( const VerificationResult & vr, const QByteArray & plaintext ) {
    return shared_ptr<DecryptVerifyResult>( new DecryptVerifyResult(
        DecryptVerifyTask::VerifyOpaque,
        vr,
        DecryptionResult(),
        plaintext,
        0,
        QString() ) );
}
shared_ptr<DecryptVerifyResult> DecryptVerifyResult::fromVerifyOpaqueResult( const GpgME::Error & err, const QString & details ) {
    return shared_ptr<DecryptVerifyResult>( new DecryptVerifyResult(
        DecryptVerifyTask::VerifyOpaque,
        VerificationResult( err ),
        DecryptionResult(),
        QByteArray(),
        err.code(),
        details ) );
}

shared_ptr<DecryptVerifyResult> DecryptVerifyResult::fromVerifyDetachedResult( const VerificationResult & vr ) {
    return shared_ptr<DecryptVerifyResult>( new DecryptVerifyResult( 
        DecryptVerifyTask::VerifyDetached,
        vr,
        DecryptionResult(),
        QByteArray(),
        0,
        QString() ) );
}
shared_ptr<DecryptVerifyResult> DecryptVerifyResult::fromVerifyDetachedResult( const GpgME::Error & err, const QString & details ) {
    return shared_ptr<DecryptVerifyResult>( new DecryptVerifyResult( 
        DecryptVerifyTask::VerifyDetached,
        VerificationResult( err ),
        DecryptionResult(),
        QByteArray(),
        err.code(),
        details ) );
}

DecryptVerifyResult::DecryptVerifyResult( DecryptVerifyTask::Type type,
                    const VerificationResult& vr,
                    const DecryptionResult& dr,
                    const QByteArray& stuff,
                    int errCode,
                    const QString& errString )
    : Task::Result(), d( new Private( type, vr, dr, stuff, errCode, errString ) )
{
}

QString DecryptVerifyResult::overview() const
{
    if ( d->isDecryptOnly() )
        return formatDecryptionResultOverview( d->m_decryptionResult );
    if ( d->isVerifyOnly() )
        return formatVerificationResultOverview( d->m_verificationResult );
    return formatDecryptVerifyResultOverview( d->m_decryptionResult, d->m_verificationResult );
}

QString DecryptVerifyResult::details() const
{
    if ( d->isDecryptOnly() )
        return formatDecryptionResultDetails( d->m_decryptionResult, KeyCache::instance()->findRecipients( d->m_decryptionResult ) );
    if ( d->isVerifyOnly() )
        return formatVerificationResultDetails( d->m_verificationResult );
    return formatDecryptVerifyResultDetails( d->m_decryptionResult, d->m_verificationResult, KeyCache::instance()->findRecipients( d->m_decryptionResult ) );
}

bool DecryptVerifyResult::hasError() const
{
    return d->m_error != 0;
}

int DecryptVerifyResult::errorCode() const
{
    return d->m_error;
}

QString DecryptVerifyResult::errorString() const
{
    return d->m_errorString;
}

GpgME::VerificationResult DecryptVerifyResult::verificationResult() const
{
    return d->m_verificationResult;
}


const char * DecryptVerifyResult::summaryToString( const Signature::Summary summary )
{
    if ( summary & Signature::Red )
        return "RED";
    if ( summary & Signature::Green )
        return "GREEN";
    return "YELLOW";
}

QString DecryptVerifyResult::keyToString( const Key & key ) {

    kleo_assert( !key.isNull() );

    const QString email = Formatting::prettyEMail( key );
    const QString name = Formatting::prettyName( key );

    if ( name.isEmpty() )
        return email;
    else if ( email.isEmpty() )
        return name;
    else
        return QString::fromLatin1( "%1 <%2>" ).arg( name, email );
}


const Key & DecryptVerifyResult::keyForSignature( const Signature & sig, const std::vector<Key> & keys ) {
    if ( const char * const fpr = sig.fingerprint() ) {
        const std::vector<Key>::const_iterator it
            = std::lower_bound( keys.begin(), keys.end(), fpr, _detail::ByFingerprint<std::less>() );
        if ( it != keys.end() && _detail::ByFingerprint<std::equal_to>()( *it, fpr ) )
            return *it;
    }
    static const Key null;
    return null;
}

QString DecryptVerifyResult::signatureToString( const Signature & sig, const Key & key )
{
    if ( sig.isNull() )
        return QString();

    const bool red   = (sig.summary() & Signature::Red);
    const bool valid = (sig.summary() & Signature::Valid);

    if ( red )
        if ( key.isNull() )
            if ( const char * fpr = sig.fingerprint() )
                return i18n("Bad signature by unknown key %1: %2", QString::fromLatin1( fpr ), QString::fromLocal8Bit( sig.status().asString() ) );
            else
                return i18n("Bad signature by an unknown key: %1", QString::fromLocal8Bit( sig.status().asString() ) );
        else
            return i18n("Bad signature by %1: %2", keyToString( key ), QString::fromLocal8Bit( sig.status().asString() ) );

    else if ( valid )
        if ( key.isNull() )
            if ( const char * fpr = sig.fingerprint() )
                return i18n("Good signature by unknown key %1.", QString::fromLatin1( fpr ) );
            else
                return i18n("Good signature by an unknown key.");
        else
            return i18n("Good signature by %1.", keyToString( key ) );

    else
        if ( key.isNull() )
            if ( const char * fpr = sig.fingerprint() )
                return i18n("Invalid signature by unknown key %1: %2", QString::fromLatin1( fpr ), QString::fromLocal8Bit( sig.status().asString() ) );
            else
                return i18n("Invalid signature by an unknown key: %1", QString::fromLocal8Bit( sig.status().asString() ) );
        else
            return i18n("Invalid signature by %1: %2", keyToString( key ), QString::fromLocal8Bit( sig.status().asString() ) );
}

class DecryptVerifyTask::Private {
    DecryptVerifyTask* const q;
public:
    explicit Private( Type type, DecryptVerifyTask* qq ) : q( qq ), m_type( type ) {}

    void slotResult( const DecryptionResult&, const QByteArray& );
    void slotResult( const DecryptionResult&, const VerificationResult&, const QByteArray& );
    void slotResult( const VerificationResult&, const QByteArray& );
    void slotResult( const VerificationResult& );

    void registerJob( VerifyDetachedJob* job ) {
        q->connect( job, SIGNAL(result(GpgME::VerificationResult)),
                    q, SLOT(slotResult(GpgME::VerificationResult)) );
    }

    void registerJob( VerifyOpaqueJob* job ) {
        q->connect( job, SIGNAL(result(GpgME::VerificationResult,QByteArray)),
                    q, SLOT(slotResult(GpgME::VerificationResult,QByteArray)) );
    }

    void registerJob( DecryptJob * job ) {
        q->connect( job, SIGNAL(result(GpgME::DecryptionResult,QByteArray)),
                    q, SLOT(slotResult(GpgME::DecryptionResult,QByteArray)) );
    }

    void registerJob( DecryptVerifyJob * job ) {
        q->connect( job, SIGNAL(result(GpgME::DecryptionResult,GpgME::VerificationResult,QByteArray)),
                    q, SLOT(slotResult(GpgME::DecryptionResult,GpgME::VerificationResult,QByteArray)) );
    }

    void emitResult( const shared_ptr<DecryptVerifyResult>& result );
    
    Type m_type;
    shared_ptr<Input> m_input, m_signedData;
    shared_ptr<Output> m_output;
    const CryptoBackend::Protocol* m_backend;
};


void DecryptVerifyTask::Private::emitResult( const shared_ptr<DecryptVerifyResult>& result )
{
    emit q->result( result );
    emit q->decryptVerifyResult( result );
}

void DecryptVerifyTask::Private::slotResult( const DecryptionResult& result, const QByteArray& plainText )
{
    if ( result.error().code() ) {
        m_output->cancel();
    } else {
        try {
            m_output->finalize();
        } catch ( const GpgME::Exception & e ) {
            emitResult( DecryptVerifyResult::fromDecryptResult( e.error(), QString::fromLocal8Bit( e.what() ) ) );
            return;
        }
    }

    emitResult( DecryptVerifyResult::fromDecryptResult( result, plainText ) );
}

void DecryptVerifyTask::Private::slotResult( const DecryptionResult& dr, const VerificationResult& vr, const QByteArray& plainText )
{
    if ( dr.error().code() || vr.error().code() ) {
        m_output->cancel();
    } else {
        try {
            m_output->finalize();
        } catch ( const GpgME::Exception & e ) {
            emitResult( DecryptVerifyResult::fromDecryptResult( e.error(), QString::fromLocal8Bit( e.what() ) ) );
            return;
        }
    }

    emitResult( DecryptVerifyResult::fromDecryptVerifyResult( dr, vr, plainText ) );
}

void DecryptVerifyTask::Private::slotResult( const VerificationResult& result, const QByteArray& plainText )
{
    if ( result.error().code() ) {
        m_output->cancel();
    } else {
        try {
            m_output->finalize();
        } catch ( const GpgME::Exception & e ) {
            emitResult( DecryptVerifyResult::fromDecryptResult( e.error(), QString::fromLocal8Bit( e.what() ) ) );
            return;
        }
    }

    emitResult( DecryptVerifyResult::fromVerifyOpaqueResult( result, plainText ) );
}

void DecryptVerifyTask::Private::slotResult( const VerificationResult& result )
{
    assert( !m_output );
    emitResult( DecryptVerifyResult::fromVerifyDetachedResult( result ) );
}

DecryptVerifyTask::DecryptVerifyTask( Type type, QObject* parent ) : Task( parent ), d( new Private( type, this ) )
{
}

DecryptVerifyTask::~DecryptVerifyTask()
{
}

void DecryptVerifyTask::setInput( const shared_ptr<Input> & input )
{
    d->m_input = input;
    kleo_assert( d->m_input && d->m_input->ioDevice() );
}

void DecryptVerifyTask::setSignedData( const shared_ptr<Input> & signedData )
{
    d->m_signedData = signedData;
    kleo_assert( d->m_signedData && d->m_signedData->ioDevice() );
}

void DecryptVerifyTask::setOutput( const shared_ptr<Output> & output )
{
    d->m_output = output;
    kleo_assert( d->m_output && d->m_output->ioDevice() );
}

void DecryptVerifyTask::setBackend( const CryptoBackend::Protocol* backend )
{
    d->m_backend = backend;
}

void DecryptVerifyTask::autodetectBackendFromInput() 
{
    if ( d->m_input )
        setBackend( CryptoBackendFactory::instance()->protocol( findProtocol( d->m_input->classification() ) ) );
}

QString DecryptVerifyTask::label() const
{
    switch ( d->m_type ) {
      case Decrypt:
      case DecryptVerify:
          return i18n( "Decrypting: %1...", d->m_input->label() );
      case VerifyOpaque:
          return i18n( "Verifying: %1...", d->m_input->label() );
      case VerifyDetached:
          return i18n( "Verifying signature: %1...", d->m_input->label() );
    }
    return i18n( "not implemented" );
}

Protocol DecryptVerifyTask::protocol() const
{
    kleo_assert( !"not implemented" );
    return UnknownProtocol; // ### TODO
}

void DecryptVerifyTask::cancel()
{
    
}

void DecryptVerifyTask::doStart()
{
    kleo_assert( d->m_backend );

    switch ( d->m_type ) {
    case Decrypt:
        try {
            DecryptJob * const job = d->m_backend->decryptJob();
            kleo_assert( job );
            d->registerJob( job );
            job->start( d->m_input->ioDevice(), d->m_output->ioDevice() );
        } catch ( const GpgME::Exception & e ) {
            d->emitResult( DecryptVerifyResult::fromDecryptResult( e.error(), QString::fromLocal8Bit( e.what() ) ) );
        }
        break;
    case DecryptVerify:
        try {
            DecryptVerifyJob * const job = d->m_backend->decryptVerifyJob();
            kleo_assert( job );
            d->registerJob( job );
            job->start( d->m_input->ioDevice(), d->m_output->ioDevice() );
        } catch ( const GpgME::Exception & e ) {
            d->emitResult( DecryptVerifyResult::fromDecryptVerifyResult( e.error(), QString::fromLocal8Bit( e.what() ) ) );
        }
        break;
    case VerifyOpaque:
        try {
            VerifyOpaqueJob * const job = d->m_backend->verifyOpaqueJob();
            kleo_assert( job );
            d->registerJob( job );
            job->start( d->m_input->ioDevice(), d->m_output ? d->m_output->ioDevice() : shared_ptr<QIODevice>() );
        } catch ( const GpgME::Exception & e ) {
            d->emitResult( DecryptVerifyResult::fromVerifyOpaqueResult( e.error(), QString::fromLocal8Bit( e.what() ) ) );
        }
        break;
    case VerifyDetached:
        try {
            VerifyDetachedJob * const job = d->m_backend->verifyDetachedJob();
            kleo_assert( job );
            d->registerJob( job );
            job->start( d->m_input->ioDevice(), d->m_signedData->ioDevice() );
        } catch ( const GpgME::Exception & e ) {
            d->emitResult( DecryptVerifyResult::fromVerifyDetachedResult( e.error(), QString::fromLocal8Bit( e.what() ) ) );
        }
        break;
    }
}

#include "decryptverifytask.moc"
