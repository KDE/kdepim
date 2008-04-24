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


#include <kleo/cryptobackendfactory.h>
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

#include <boost/bind.hpp>

#include <algorithm>

using namespace Kleo::Crypto;
using namespace Kleo;
using namespace GpgME;
using namespace boost;

namespace {

static const char * iconForSignature( const Signature & sig ) {
    if ( sig.summary() & Signature::Green )
        return "dialog-ok";
    if ( sig.summary() & Signature::Red )
        return "dialog-error";
    return "dialog-warning";
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

namespace {

    bool IsBad( const Signature & sig ) {
        return sig.summary() & Signature::Red;
    }
    bool IsValid( const Signature & sig ) {
        return sig.summary() & Signature::Valid;
    }
}

Task::Result::VisualCode codeForVerificationResult( const VerificationResult & res )
{
    if ( res.isNull() )
        return Task::Result::NeutralSuccess;

    const std::vector<Signature> sigs = res.signatures();
    if ( sigs.empty() )
        return Task::Result::Warning;

    if ( !std::count_if( sigs.begin(), sigs.end(), IsBad ) )
        return Task::Result::AllGood;
    if ( std::find_if( sigs.begin(), sigs.end(), IsBad ) != sigs.end() )
        return Task::Result::Danger;
    return Task::Result::Warning;
}

QString formatVerificationResultOverview( const VerificationResult & res ) {
    if ( res.isNull() )
        return QString();

    const Error err = res.error();

    if ( err.isCanceled() )
        return i18n("<b>Verification canceled.</b>");
    else if ( err )
        return i18n( "<b>Verification failed: %1.</b>", Qt::escape( QString::fromLocal8Bit( err.asString() ) ) );

    const std::vector<Signature> sigs = res.signatures();

    if ( sigs.empty() )
        return i18n( "<b>No signatures found</b>" );

    const uint bad = std::count_if( sigs.begin(), sigs.end(), IsBad );
    if ( bad > 0 )
        return i18np("<b>Bad signature</b>", "<b>%1 bad signatures</b>", bad );
    const uint invalid = std::count_if( sigs.begin(), sigs.end(), !bind( IsValid, _1 ) );
    if ( invalid > 0 )
            return i18np("<b>Invalid signature</b>", "<b>%1 invalid signatures</b>", invalid );
    return i18np("<b>Good signature</b>", "<b>%1 good signatures</b>", sigs.size() );
}

static QString formatDecryptionResultOverview( const DecryptionResult & result )
{
    const Error err = result.error();

    if ( err.isCanceled() )
        return i18n("<b>Decryption canceled.</b>");
    else if ( err )
        return i18n( "<b>Decryption failed: %1.</b>", Qt::escape( QString::fromLocal8Bit( err.asString() ) ) );
    return i18n("<b>Decryption succeeded.</b>" );
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
    if ( IsErrorOrCanceled( dr ) )
        return formatDecryptionResultOverview( dr );
    return formatVerificationResultOverview( vr );
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
    Private( DecryptVerifyOperation type,
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

    bool isDecryptOnly() const { return m_type == Decrypt; }
    bool isVerifyOnly() const { return m_type == Verify; }
    bool isDecryptVerify() const { return m_type == DecryptVerify; }
    DecryptVerifyOperation m_type;
    VerificationResult m_verificationResult;
    DecryptionResult m_decryptionResult;
    QByteArray m_stuff;
    int m_error;
    QString m_errorString;
};

shared_ptr<DecryptVerifyResult> DecryptVerifyResult::fromDecryptResult( int id, const DecryptionResult & dr, const QByteArray & plaintext ) {
    return shared_ptr<DecryptVerifyResult>( new DecryptVerifyResult(
        id,
        Decrypt,
        VerificationResult(),
        dr,
        plaintext,
        0,
        QString() ) );
}

shared_ptr<DecryptVerifyResult> DecryptVerifyResult::fromDecryptResult( int id, const GpgME::Error & err, const QString& what ) {
    return shared_ptr<DecryptVerifyResult>( new DecryptVerifyResult(
        id,
        Decrypt,
        VerificationResult(),
        DecryptionResult( err ),
        QByteArray(),
        err.code(),
        what ) );
}

shared_ptr<DecryptVerifyResult> DecryptVerifyResult::fromDecryptVerifyResult( int id, const DecryptionResult & dr, const VerificationResult & vr, const QByteArray & plaintext ) {
    return shared_ptr<DecryptVerifyResult>( new DecryptVerifyResult(
        id,
        DecryptVerify,
        vr,
        dr,
        plaintext,
        0,
        QString() ) );
}

shared_ptr<DecryptVerifyResult> DecryptVerifyResult::fromDecryptVerifyResult( int id, const GpgME::Error & err, const QString & details ) {
    return shared_ptr<DecryptVerifyResult>( new DecryptVerifyResult(
        id,
        DecryptVerify,
        VerificationResult(),
        DecryptionResult( err ),
        QByteArray(),
        err.code(),
        details ) );
}

shared_ptr<DecryptVerifyResult> DecryptVerifyResult::fromVerifyOpaqueResult( int id, const VerificationResult & vr, const QByteArray & plaintext ) {
    return shared_ptr<DecryptVerifyResult>( new DecryptVerifyResult(
        id,
        Verify,
        vr,
        DecryptionResult(),
        plaintext,
        0,
        QString() ) );
}
shared_ptr<DecryptVerifyResult> DecryptVerifyResult::fromVerifyOpaqueResult( int id, const GpgME::Error & err, const QString & details ) {
    return shared_ptr<DecryptVerifyResult>( new DecryptVerifyResult(
        id,
        Verify,
        VerificationResult( err ),
        DecryptionResult(),
        QByteArray(),
        err.code(),
        details ) );
}

shared_ptr<DecryptVerifyResult> DecryptVerifyResult::fromVerifyDetachedResult( int id, const VerificationResult & vr ) {
    return shared_ptr<DecryptVerifyResult>( new DecryptVerifyResult( 
        id,
        Verify,
        vr,
        DecryptionResult(),
        QByteArray(),
        0,
        QString() ) );
}
shared_ptr<DecryptVerifyResult> DecryptVerifyResult::fromVerifyDetachedResult( int id, const GpgME::Error & err, const QString & details ) {
    return shared_ptr<DecryptVerifyResult>( new DecryptVerifyResult(
        id,
        Verify,
        VerificationResult( err ),
        DecryptionResult(),
        QByteArray(),
        err.code(),
        details ) );
}

DecryptVerifyResult::DecryptVerifyResult( int id, DecryptVerifyOperation type,
                    const VerificationResult& vr,
                    const DecryptionResult& dr,
                    const QByteArray& stuff,
                    int errCode,
                    const QString& errString )
    : Task::Result( id ), d( new Private( type, vr, dr, stuff, errCode, errString ) )
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

Task::Result::VisualCode DecryptVerifyResult::code() const {
    if ( d->m_type == DecryptVerify || d->m_type == Verify )
            return codeForVerificationResult( d->m_verificationResult );
    return hasError() ? NeutralError : NeutralSuccess;
}

GpgME::VerificationResult DecryptVerifyResult::verificationResult() const
{
    return d->m_verificationResult;
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

class AbstractDecryptVerifyTask::Private {
    
};

AbstractDecryptVerifyTask::AbstractDecryptVerifyTask( QObject * parent ) : Task( parent ), d( new Private ) {}

AbstractDecryptVerifyTask::~AbstractDecryptVerifyTask() {}

class DecryptVerifyTask::Private {
    DecryptVerifyTask* const q;
public:
    explicit Private( DecryptVerifyTask* qq ) : q( qq ), m_backend( 0 ), m_protocol( UnknownProtocol )  {}

    void slotResult( const DecryptionResult&, const VerificationResult&, const QByteArray& );

    void registerJob( DecryptVerifyJob * job ) {
        q->connect( job, SIGNAL(result(GpgME::DecryptionResult,GpgME::VerificationResult,QByteArray)),
                    q, SLOT(slotResult(GpgME::DecryptionResult,GpgME::VerificationResult,QByteArray)) );
        q->connect( job, SIGNAL(progress(QString,int,int)),
                    q, SLOT(setProgress(QString,int,int)) );
    }

    void emitResult( const shared_ptr<DecryptVerifyResult>& result );
    
    shared_ptr<Input> m_input;
    shared_ptr<Output> m_output;
    const CryptoBackend::Protocol* m_backend;
    Protocol m_protocol;
};


void DecryptVerifyTask::Private::emitResult( const shared_ptr<DecryptVerifyResult>& result )
{
    q->emitResult( result );
    emit q->decryptVerifyResult( result );
}

void DecryptVerifyTask::Private::slotResult( const DecryptionResult& dr, const VerificationResult& vr, const QByteArray& plainText )
{
    if ( dr.error().code() || vr.error().code() ) {
        m_output->cancel();
    } else {
        try {
            m_output->finalize();
        } catch ( const GpgME::Exception & e ) {
            emitResult( DecryptVerifyResult::fromDecryptResult( q->id(), e.error(), QString::fromLocal8Bit( e.what() ) ) );
            return;
        }
    }

    emitResult( DecryptVerifyResult::fromDecryptVerifyResult( q->id(), dr, vr, plainText ) );
}


DecryptVerifyTask::DecryptVerifyTask( QObject* parent ) : AbstractDecryptVerifyTask( parent ), d( new Private( this ) )
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

void DecryptVerifyTask::setOutput( const shared_ptr<Output> & output )
{
    d->m_output = output;
    kleo_assert( d->m_output && d->m_output->ioDevice() );
}

void DecryptVerifyTask::setProtocol( Protocol prot )
{
    kleo_assert( prot != UnknownProtocol );
    d->m_protocol = prot;
    d->m_backend = CryptoBackendFactory::instance()->protocol( prot );
    kleo_assert( d->m_backend );
}

void DecryptVerifyTask::autodetectProtocolFromInput() 
{
    if ( d->m_input )
        setProtocol( findProtocol( d->m_input->classification() ) );
}

QString DecryptVerifyTask::label() const
{
    return i18n( "Decrypting: %1...", d->m_input->label() );
}

Protocol DecryptVerifyTask::protocol() const
{
    return d->m_protocol;
}

void DecryptVerifyTask::cancel()
{
    
}

void DecryptVerifyTask::doStart()
{
    kleo_assert( d->m_backend );
    try {
        DecryptVerifyJob * const job = d->m_backend->decryptVerifyJob();
        kleo_assert( job );
        d->registerJob( job );
        job->start( d->m_input->ioDevice(), d->m_output->ioDevice() );
    } catch ( const GpgME::Exception & e ) {
        d->emitResult( DecryptVerifyResult::fromDecryptVerifyResult( id(), e.error(), QString::fromLocal8Bit( e.what() ) ) );
    }
}

class DecryptTask::Private {
    DecryptTask* const q;
public:
    explicit Private( DecryptTask* qq ) : q( qq ), m_backend( 0 ), m_protocol( UnknownProtocol )  {}

    void slotResult( const DecryptionResult&, const QByteArray& );

    void registerJob( DecryptJob * job ) {
        q->connect( job, SIGNAL(result(GpgME::DecryptionResult,QByteArray)),
                    q, SLOT(slotResult(GpgME::DecryptionResult,QByteArray)) );
        q->connect( job, SIGNAL(progress(QString,int,int)),
                    q, SLOT(setProgress(QString,int,int)) );
    }

    void emitResult( const shared_ptr<DecryptVerifyResult>& result );
    
    shared_ptr<Input> m_input;
    shared_ptr<Output> m_output;
    const CryptoBackend::Protocol* m_backend;
    Protocol m_protocol;
};


void DecryptTask::Private::emitResult( const shared_ptr<DecryptVerifyResult>& result )
{
    q->emitResult( result );
    emit q->decryptVerifyResult( result );
}

void DecryptTask::Private::slotResult( const DecryptionResult& result, const QByteArray& plainText )
{
    if ( result.error().code() ) {
        m_output->cancel();
    } else {
        try {
            m_output->finalize();
        } catch ( const GpgME::Exception & e ) {
            emitResult( DecryptVerifyResult::fromDecryptResult( q->id(), e.error(), QString::fromLocal8Bit( e.what() ) ) );
            return;
        }
    }

    emitResult( DecryptVerifyResult::fromDecryptResult( q->id(), result, plainText ) );
}

DecryptTask::DecryptTask( QObject* parent ) : AbstractDecryptVerifyTask( parent ), d( new Private( this ) )
{
}

DecryptTask::~DecryptTask()
{
}

void DecryptTask::setInput( const shared_ptr<Input> & input )
{
    d->m_input = input;
    kleo_assert( d->m_input && d->m_input->ioDevice() );
}

void DecryptTask::setOutput( const shared_ptr<Output> & output )
{
    d->m_output = output;
    kleo_assert( d->m_output && d->m_output->ioDevice() );
}

void DecryptTask::setProtocol( Protocol prot )
{
    kleo_assert( prot != UnknownProtocol );
    d->m_protocol = prot;
    d->m_backend = CryptoBackendFactory::instance()->protocol( prot );
    kleo_assert( d->m_backend );
}

void DecryptTask::autodetectProtocolFromInput() 
{
    if ( d->m_input )
        setProtocol( findProtocol( d->m_input->classification() ) );
}

QString DecryptTask::label() const
{
    return i18n( "Decrypting: %1...", d->m_input->label() );
}

Protocol DecryptTask::protocol() const
{
    kleo_assert( !"not implemented" );
    return UnknownProtocol; // ### TODO
}

void DecryptTask::cancel()
{
    
}

void DecryptTask::doStart()
{
    kleo_assert( d->m_backend );

    try {
        DecryptJob * const job = d->m_backend->decryptJob();
        kleo_assert( job );
        d->registerJob( job );
        job->start( d->m_input->ioDevice(), d->m_output->ioDevice() );
    } catch ( const GpgME::Exception & e ) {
        d->emitResult( DecryptVerifyResult::fromDecryptResult( id(), e.error(), QString::fromLocal8Bit( e.what() ) ) );
    }
}

class VerifyOpaqueTask::Private {
    VerifyOpaqueTask* const q;
public:
    explicit Private( VerifyOpaqueTask* qq ) : q( qq ), m_backend( 0 ), m_protocol( UnknownProtocol )  {}

    void slotResult( const VerificationResult&, const QByteArray& );

    void registerJob( VerifyOpaqueJob* job ) {
        q->connect( job, SIGNAL(result(GpgME::VerificationResult,QByteArray)),
                    q, SLOT(slotResult(GpgME::VerificationResult,QByteArray)) );
        q->connect( job, SIGNAL(progress(QString,int,int)),
                    q, SLOT(setProgress(QString,int,int)) );
    }

    void emitResult( const shared_ptr<DecryptVerifyResult>& result );
    
    shared_ptr<Input> m_input;
    shared_ptr<Output> m_output;
    const CryptoBackend::Protocol* m_backend;
    Protocol m_protocol;
};


void VerifyOpaqueTask::Private::emitResult( const shared_ptr<DecryptVerifyResult>& result )
{
    q->emitResult( result );
    emit q->decryptVerifyResult( result );
}

void VerifyOpaqueTask::Private::slotResult( const VerificationResult& result, const QByteArray& plainText )
{
    if ( result.error().code() ) {
        m_output->cancel();
    } else {
        try {
            m_output->finalize();
        } catch ( const GpgME::Exception & e ) {
            emitResult( DecryptVerifyResult::fromDecryptResult( q->id(), e.error(), QString::fromLocal8Bit( e.what() ) ) );
            return;
        }
    }

    emitResult( DecryptVerifyResult::fromVerifyOpaqueResult( q->id(), result, plainText ) );
}

VerifyOpaqueTask::VerifyOpaqueTask( QObject* parent ) : AbstractDecryptVerifyTask( parent ), d( new Private( this ) )
{
}

VerifyOpaqueTask::~VerifyOpaqueTask()
{
}

void VerifyOpaqueTask::setInput( const shared_ptr<Input> & input )
{
    d->m_input = input;
    kleo_assert( d->m_input && d->m_input->ioDevice() );
}

void VerifyOpaqueTask::setOutput( const shared_ptr<Output> & output )
{
    d->m_output = output;
    kleo_assert( d->m_output && d->m_output->ioDevice() );
}

void VerifyOpaqueTask::setProtocol( Protocol prot )
{
    kleo_assert( prot != UnknownProtocol );
    d->m_protocol = prot;
    d->m_backend = CryptoBackendFactory::instance()->protocol( prot );
    kleo_assert( d->m_backend );
}

void VerifyOpaqueTask::autodetectProtocolFromInput() 
{
    if ( d->m_input )
        setProtocol( findProtocol( d->m_input->classification() ) );
}

QString VerifyOpaqueTask::label() const
{
    return i18n( "Verifying: %1...", d->m_input->label() );
}

Protocol VerifyOpaqueTask::protocol() const
{
    return d->m_protocol;
}

void VerifyOpaqueTask::cancel()
{
    
}

void VerifyOpaqueTask::doStart()
{
    kleo_assert( d->m_backend );

    try {
        VerifyOpaqueJob * const job = d->m_backend->verifyOpaqueJob();
        kleo_assert( job );
        d->registerJob( job );
        job->start( d->m_input->ioDevice(), d->m_output ? d->m_output->ioDevice() : shared_ptr<QIODevice>() );
    } catch ( const GpgME::Exception & e ) {
        d->emitResult( DecryptVerifyResult::fromVerifyOpaqueResult( id(), e.error(), QString::fromLocal8Bit( e.what() ) ) );
    }
}

class VerifyDetachedTask::Private {
    VerifyDetachedTask* const q;
public:
    explicit Private( VerifyDetachedTask* qq ) : q( qq ), m_backend( 0 ), m_protocol( UnknownProtocol ) {}

    void slotResult( const VerificationResult& );

    void registerJob( VerifyDetachedJob* job ) {
        q->connect( job, SIGNAL(result(GpgME::VerificationResult)),
                    q, SLOT(slotResult(GpgME::VerificationResult)) );
        q->connect( job, SIGNAL(progress(QString,int,int)),
                    q, SLOT(setProgress(QString,int,int)) );
    }

    void emitResult( const shared_ptr<DecryptVerifyResult>& result );
    
    shared_ptr<Input> m_input, m_signedData;
    const CryptoBackend::Protocol* m_backend;
    Protocol m_protocol;
};


void VerifyDetachedTask::Private::emitResult( const shared_ptr<DecryptVerifyResult>& result )
{
    q->emitResult( result );
    emit q->decryptVerifyResult( result );
}

void VerifyDetachedTask::Private::slotResult( const VerificationResult& result )
{
    emitResult( DecryptVerifyResult::fromVerifyDetachedResult( q->id(), result ) );
}

VerifyDetachedTask::VerifyDetachedTask( QObject* parent ) : AbstractDecryptVerifyTask( parent ), d( new Private( this ) )
{
}

VerifyDetachedTask::~VerifyDetachedTask()
{
}

void VerifyDetachedTask::setInput( const shared_ptr<Input> & input )
{
    d->m_input = input;
    kleo_assert( d->m_input && d->m_input->ioDevice() );
}

void VerifyDetachedTask::setSignedData( const shared_ptr<Input> & signedData )
{
    d->m_signedData = signedData;
    kleo_assert( d->m_signedData && d->m_signedData->ioDevice() );
}

void VerifyDetachedTask::setProtocol( Protocol prot )
{
    kleo_assert( prot != UnknownProtocol );
    d->m_protocol = prot;
    d->m_backend = CryptoBackendFactory::instance()->protocol( prot );
    kleo_assert( d->m_backend );
}

void VerifyDetachedTask::autodetectProtocolFromInput() 
{
    if ( d->m_input )
        setProtocol( findProtocol( d->m_input->classification() ) );
}

QString VerifyDetachedTask::label() const
{
    return i18n( "Verifying signature: %1...", d->m_input->label() );
}

Protocol VerifyDetachedTask::protocol() const
{
    kleo_assert( !"not implemented" );
    return UnknownProtocol; // ### TODO
}

void VerifyDetachedTask::cancel()
{
    
}

void VerifyDetachedTask::doStart()
{
    kleo_assert( d->m_backend );
    try {
        VerifyDetachedJob * const job = d->m_backend->verifyDetachedJob();
        kleo_assert( job );
        d->registerJob( job );
        job->start( d->m_input->ioDevice(), d->m_signedData->ioDevice() );
    } catch ( const GpgME::Exception & e ) {
        d->emitResult( DecryptVerifyResult::fromVerifyDetachedResult( id(), e.error(), QString::fromLocal8Bit( e.what() ) ) );
    }
}

#include "decryptverifytask.moc"
