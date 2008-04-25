/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/signemailtask.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

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

#include "signemailtask.h"

#include <utils/input.h>
#include <utils/output.h>
#include <utils/stl_util.h>
#include <utils/kleo_assert.h>

#include <kleo/cryptobackendfactory.h>
#include <kleo/cryptobackend.h>
#include <kleo/signjob.h>

#include <gpgme++/signingresult.h>
#include <gpgme++/key.h>

#include <KLocale>

#include <QPointer>
#include <QTextDocument> // for Qt::escape

#include <boost/bind.hpp>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace boost;
using namespace GpgME;

namespace {

    class SignEMailResult : public Task::Result {
        const SigningResult m_result;
    public:
        explicit SignEMailResult( const SigningResult & r )
            : Task::Result(), m_result( r ) {}

        /* reimp */ QString overview() const;
        /* reimp */ QString details() const;
        /* reimp */ int errorCode() const;
        /* reimp */ QString errorString() const;
        /* reimp */ VisualCode code() const;
    };

    QString makeResultString( const SigningResult& res )
    {
        const Error err = res.error();

        if ( err.isCanceled() )
            return i18n( "Signing canceled." );

        if ( err )
            return i18n( "Signing failed: %1", Qt::escape( QString::fromLocal8Bit( err.asString() ) ) );

        return i18n( "Signing succeeded." );
    }
}

class SignEMailTask::Private {
    friend class ::Kleo::Crypto::SignEMailTask;
    SignEMailTask * const q;
public:
    explicit Private( SignEMailTask * qq );

private:
    std::auto_ptr<Kleo::SignJob> createJob( GpgME::Protocol proto );

private:
    void slotResult( const SigningResult & );

private:
    shared_ptr<Input> input;
    shared_ptr<Output> output;
    std::vector<Key> signers;
    bool detached;

    QString micAlg;

    QPointer<Kleo::SignJob> job;
};

SignEMailTask::Private::Private( SignEMailTask * qq )
    : q( qq ),
      input(),
      output(),
      signers(),
      detached( false ),
      micAlg(),
      job( 0 )
{

}

SignEMailTask::SignEMailTask( QObject * p )
    : Task( p ), d( new Private( this ) )
{

}

SignEMailTask::~SignEMailTask() {}

void SignEMailTask::setInput( const shared_ptr<Input> & input ) {
    kleo_assert( !d->job );
    kleo_assert( input );
    d->input = input;
}

void SignEMailTask::setOutput( const shared_ptr<Output> & output ) {
    kleo_assert( !d->job );
    kleo_assert( output );
    d->output = output;
}

void SignEMailTask::setSigners( const std::vector<Key> & signers ) {
    kleo_assert( !d->job );
    kleo_assert( !signers.empty() );
    d->signers = signers;
}

void SignEMailTask::setDetachedSignature( bool detached ) {
    kleo_assert( !d->job );
    d->detached = detached;
}

Protocol SignEMailTask::protocol() const {
    kleo_assert( !d->signers.empty() );
    return d->signers.front().protocol();
}

QString SignEMailTask::label() const
{
    return d->input ? d->input->label() : QString();
}

unsigned long long SignEMailTask::inputSize() const
{
    return d->input ? d->input->size() : 0;
}

void SignEMailTask::doStart() {
    kleo_assert( !d->job );
    kleo_assert( d->input );
    kleo_assert( d->output );
    kleo_assert( !d->signers.empty() );

    d->micAlg.clear();

    std::auto_ptr<Kleo::SignJob> job = d->createJob( protocol() );
    kleo_assert( job.get() );

    job->start( d->signers,
                d->input->ioDevice(), d->output->ioDevice(),
                d->detached ? GpgME::Detached : GpgME::NormalSignatureMode );

    d->job = job.release();
}

void SignEMailTask::cancel() {
    if ( d->job )
        d->job->slotCancel();
}

std::auto_ptr<Kleo::SignJob> SignEMailTask::Private::createJob( GpgME::Protocol proto ) {
    const CryptoBackend::Protocol * const backend = CryptoBackendFactory::instance()->protocol( proto );
    kleo_assert( backend );
    std::auto_ptr<Kleo::SignJob> signJob( backend->signJob( /*armor=*/proto == OpenPGP || q->asciiArmor(), /*textmode=*/false ) );
    kleo_assert( signJob.get() );
    if ( proto == CMS && !q->asciiArmor() )
        signJob->setOutputIsBase64Encoded( true );
    connect( signJob.get(), SIGNAL(progress(QString,int,int)),
             q, SLOT(setProgress(QString,int,int)) );
    connect( signJob.get(), SIGNAL(result(GpgME::SigningResult,QByteArray)),
             q, SLOT(slotResult(GpgME::SigningResult)) );
    return signJob;
}

static QString collect_micalgs( const GpgME::SigningResult & result, GpgME::Protocol proto ) {
    const std::vector<GpgME::CreatedSignature> css = result.createdSignatures();
    QStringList micalgs;
#ifndef DASHBOT_HAS_STABLE_COMPILER
    Q_FOREACH( const GpgME::CreatedSignature & sig, css )
        micalgs.push_back( QString::fromLatin1( sig.hashAlgorithmAsString() ).toLower() );
#else
    std::transform( css.begin(), css.end(),
                    std::back_inserter( micalgs ),
                    bind( &QString::toLower, bind( &QString::fromLatin1, bind( &GpgME::CreatedSignature::hashAlgorithmAsString, _1 ), -1 ) ) );
#endif
    if ( proto == GpgME::OpenPGP )
        for ( QStringList::iterator it = micalgs.begin(), end = micalgs.end() ; it != end ; ++it )
            it->prepend( "pgp-" );
    micalgs.sort();
    micalgs.erase( std::unique( micalgs.begin(), micalgs.end() ), micalgs.end() );
    return micalgs.join( QLatin1String(",") );
}

void SignEMailTask::Private::slotResult( const SigningResult & result ) {
    if ( result.error().code() ) {
        output->cancel();
    } else {
        output->finalize();
        micAlg = collect_micalgs( result, q->protocol() );
    }
    q->emitResult( shared_ptr<Result>( new SignEMailResult( result ) ) );
}

QString SignEMailTask::micAlg() const {
    return d->micAlg;
}


QString SignEMailResult::overview() const {
    return makeOverview( makeResultString( m_result ) );
}

QString SignEMailResult::details() const {
    return QString();
}

int SignEMailResult::errorCode() const {
    return m_result.error().encodedError();
}

QString SignEMailResult::errorString() const {
    return hasError() ? makeResultString( m_result ) : QString();
}

Task::Result::VisualCode SignEMailResult::code() const {
    if ( m_result.error().isCanceled() )
        return Warning;
    return m_result.error().code() ? NeutralError : NeutralSuccess;
}

#include "moc_signemailtask.cpp"


