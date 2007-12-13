/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/signencryptfilestask.cpp

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

#include "signencryptfilestask.h"

#include "kleo-assuan.h"

#include "input.h"
#include "output.h"

#include <utils/stl_util.h>

#include <kleo/cryptobackendfactory.h>
#include <kleo/cryptobackend.h>
#include <kleo/signjob.h>
#include <kleo/signencryptjob.h>
#include <kleo/encryptjob.h>

#include <gpgme++/signingresult.h>
#include <gpgme++/encryptionresult.h>
#include <gpgme++/key.h>

#include <KLocale>

#include <QPointer>
#include <QTextDocument> // for Qt::escape

#include <boost/bind.hpp>

using namespace Kleo;
using namespace boost;
using namespace GpgME;

namespace {

    class SignEncryptFilesResult : public Task::Result {
        const SigningResult m_sresult;
        const EncryptionResult m_eresult;
    public:
        explicit SignEncryptFilesResult( const SigningResult & sr )
            : Task::Result(), m_sresult( sr ) {}
        explicit SignEncryptFilesResult( const EncryptionResult & er )
            : Task::Result(), m_eresult( er ) {}
        explicit SignEncryptFilesResult( const SigningResult & sr, const EncryptionResult & er )
            : Task::Result(), m_sresult( sr ), m_eresult( er ) {}

        /* reimp */ QString overview() const;
        /* reimp */ QString details() const;
    };


    static QString makeErrorString( const SigningResult & result ) {
        const Error err = result.error();

        assuan_assert( err || err.isCanceled() );

        if ( err.isCanceled() )
            return i18n("Signing canceled.");
        else // if ( err )
            return i18n("Signing failed: %1.", Qt::escape( QString::fromLocal8Bit( err.asString() ) ) );
    }

    static QString makeErrorString( const EncryptionResult & result ) {
        const Error err = result.error();

        assuan_assert( err || err.isCanceled() );

        if ( err.isCanceled() )
            return i18n("Encryption canceled.");
        else // if ( err )
            return i18n("Encryption failed: %1.", Qt::escape( QString::fromLocal8Bit( err.asString() ) ) );
    }

}

class SignEncryptFilesTask::Private {
    friend class ::Kleo::SignEncryptFilesTask;
    SignEncryptFilesTask * const q;
public:
    explicit Private( SignEncryptFilesTask * qq );

private:
    std::auto_ptr<Kleo::SignJob> createSignJob( GpgME::Protocol proto );
    std::auto_ptr<Kleo::SignEncryptJob> createSignEncryptJob( GpgME::Protocol proto );
    std::auto_ptr<Kleo::EncryptJob> createEncryptJob( GpgME::Protocol proto );

private:
    void slotResult( const SigningResult & );
    void slotResult( const SigningResult &, const EncryptionResult & );
    void slotResult( const EncryptionResult & );

private:
    shared_ptr<Input> input;
    shared_ptr<Output> output;
    QString inputFileName, outputFileName;
    std::vector<Key> signers;
    std::vector<Key> recipients;

    bool sign     : 1;
    bool encrypt  : 1;
    bool ascii    : 1;
    bool detached : 1;

    QPointer<Kleo::Job> job;
};

SignEncryptFilesTask::Private::Private( SignEncryptFilesTask * qq )
    : q( qq ),
      input(),
      output(),
      inputFileName(),
      outputFileName(),
      signers(),
      recipients(),
      sign( true ),
      encrypt( true ),
      ascii( true ),
      detached( false ),
      job( 0 )
{

}

SignEncryptFilesTask::SignEncryptFilesTask( QObject * p )
    : Task( p ), d( new Private( this ) )
{

}

SignEncryptFilesTask::~SignEncryptFilesTask() {}

void SignEncryptFilesTask::setInputFileName( const QString & fileName ) {
    assuan_assert( !d->job );
    assuan_assert( !fileName.isEmpty() );
    d->inputFileName = fileName;
}

void SignEncryptFilesTask::setOutputFileName( const QString & fileName ) {
    assuan_assert( !d->job );
    assuan_assert( !fileName.isEmpty() );
    d->outputFileName = fileName;
}

void SignEncryptFilesTask::setSigners( const std::vector<Key> & signers ) {
    assuan_assert( !d->job );
    d->signers = signers;
}

void SignEncryptFilesTask::setRecipients( const std::vector<Key> & recipients ) {
    assuan_assert( !d->job );
    d->recipients = recipients;
}

void SignEncryptFilesTask::setSign( bool sign ) {
    assuan_assert( !d->job );
    d->sign = sign;
}

void SignEncryptFilesTask::setEncrypt( bool encrypt ) {
    assuan_assert( !d->job );
    d->encrypt = encrypt;
}

void SignEncryptFilesTask::setAsciiArmor( bool ascii ) {
    assuan_assert( !d->job );
    d->ascii = ascii;
}

void SignEncryptFilesTask::setDetachedSignature( bool detached ) {
    assuan_assert( !d->job );
    d->detached = detached;
}

Protocol SignEncryptFilesTask::protocol() const {
    if ( d->sign && !d->signers.empty() )
        return d->signers.front().protocol();
    if ( d->encrypt )
        if ( !d->recipients.empty() )
            return d->recipients.front().protocol();
        else
            return GpgME::OpenPGP; // symmetric OpenPGP encryption
    throw assuan_exception( gpg_error( GPG_ERR_INTERNAL ),
                            i18n("Cannot determine protocol for task") );
}

QString SignEncryptFilesTask::label() const {
    return d->input ? d->input->label() : QString();
}

void SignEncryptFilesTask::doStart() {
    assuan_assert( !d->job );
    if ( d->sign )
        assuan_assert( !d->signers.empty() );

    d->input = Input::createFromFile( d->inputFileName );
    d->output = Output::createFromFile( d->outputFileName, false );

    if ( d->encrypt )
        if ( d->sign ) {
            std::auto_ptr<Kleo::SignEncryptJob> job = d->createSignEncryptJob( protocol() );
            assuan_assert( job.get() );

            job->start( d->signers, d->recipients,
                        d->input->ioDevice(), d->output->ioDevice(), true );

            d->job = job.release();
        } else {
            std::auto_ptr<Kleo::EncryptJob> job = d->createEncryptJob( protocol() );
            assuan_assert( job.get() );

            job->start( d->recipients, d->input->ioDevice(), d->output->ioDevice(), true );

            d->job = job.release();
        }
    else
        if ( d->sign ) {
            std::auto_ptr<Kleo::SignJob> job = d->createSignJob( protocol() );
            assuan_assert( job.get() );

            job->start( d->signers,
                        d->input->ioDevice(), d->output->ioDevice(),
                        d->detached ? GpgME::Detached : GpgME::NormalSignatureMode );

            d->job = job.release();
        } else {
            assuan_assert( !"Either 'sign' or 'encrypt' must be set!" );
        }
}

void SignEncryptFilesTask::cancel() {
    if ( d->job )
        d->job->slotCancel();
}

std::auto_ptr<Kleo::SignJob> SignEncryptFilesTask::Private::createSignJob( GpgME::Protocol proto ) {
    const CryptoBackend::Protocol * const backend = CryptoBackendFactory::instance()->protocol( proto );
    assuan_assert( backend );
    std::auto_ptr<Kleo::SignJob> signJob( backend->signJob( /*armor=*/true, /*textmode=*/false ) );
    assuan_assert( signJob.get() );
    connect( signJob.get(), SIGNAL(progress(QString,int,int)),
             q, SIGNAL(progress(QString,int,int)) );
    connect( signJob.get(), SIGNAL(result(GpgME::SigningResult,QByteArray)),
             q, SLOT(slotResult(GpgME::SigningResult)) );
    return signJob;
}

std::auto_ptr<Kleo::SignEncryptJob> SignEncryptFilesTask::Private::createSignEncryptJob( GpgME::Protocol proto ) {
    const CryptoBackend::Protocol * const backend = CryptoBackendFactory::instance()->protocol( proto );
    assuan_assert( backend );
    std::auto_ptr<Kleo::SignEncryptJob> signEncryptJob( backend->signEncryptJob( /*armor=*/true, /*textmode=*/false ) );
    assuan_assert( signEncryptJob.get() );
    connect( signEncryptJob.get(), SIGNAL(progress(QString,int,int)),
             q, SIGNAL(progress(QString,int,int)) );
    connect( signEncryptJob.get(), SIGNAL(result(GpgME::SigningResult,GpgME::EncryptionResult,QByteArray)),
             q, SLOT(slotResult(GpgME::SigningResult,GpgME::EncryptionResult)) );
    return signEncryptJob;
}

std::auto_ptr<Kleo::EncryptJob> SignEncryptFilesTask::Private::createEncryptJob( GpgME::Protocol proto ) {
    const CryptoBackend::Protocol * const backend = CryptoBackendFactory::instance()->protocol( proto );
    assuan_assert( backend );
    std::auto_ptr<Kleo::EncryptJob> encryptJob( backend->encryptJob( /*armor=*/true, /*textmode=*/false ) );
    assuan_assert( encryptJob.get() );
    connect( encryptJob.get(), SIGNAL(progress(QString,int,int)),
             q, SIGNAL(progress(QString,int,int)) );
    connect( encryptJob.get(), SIGNAL(result(GpgME::EncryptionResult,QByteArray)),
             q, SLOT(slotResult(GpgME::EncryptionResult)) );
    return encryptJob;
}

void SignEncryptFilesTask::Private::slotResult( const SigningResult & result ) {
    if ( result.error().code() ) {
        output->cancel();
        emit q->error( result.error(), makeErrorString( result ) );
    } else
        try {
            output->finalize();
            emit q->result( shared_ptr<Result>( new SignEncryptFilesResult( result ) ) );
        } catch ( const GpgME::Exception & e ) {
            emit q->error( e.error(), QString::fromLocal8Bit( e.what() ) );
        }
}

void SignEncryptFilesTask::Private::slotResult( const SigningResult & sresult, const EncryptionResult & eresult ) {
    if ( sresult.error().code() ) {
        output->cancel();
        emit q->error( sresult.error(), makeErrorString( sresult ) );
    } else if ( eresult.error().code() ) {
        output->cancel();
        emit q->error( eresult.error(), makeErrorString( eresult ) );
    } else
        try {
            output->finalize();
            emit q->result( shared_ptr<Result>( new SignEncryptFilesResult( sresult, eresult ) ) );
        } catch ( const GpgME::Exception & e ) {
            emit q->error( e.error(), QString::fromLocal8Bit( e.what() ) );
        }
}

void SignEncryptFilesTask::Private::slotResult( const EncryptionResult & result ) {
    if ( result.error().code() ) {
        output->cancel();
        emit q->error( result.error(), makeErrorString( result ) );
    } else
        try {
            output->finalize();
            emit q->result( shared_ptr<Result>( new SignEncryptFilesResult( result ) ) );
        } catch ( const GpgME::Exception & e ) {
            emit q->error( e.error(), QString::fromLocal8Bit( e.what() ) );
        }
}

QString SignEncryptFilesResult::overview() const {
    const bool sign = !m_sresult.isNull();
    const bool encrypt = !m_eresult.isNull();
    assuan_assert( sign || encrypt );

    if ( sign && encrypt )
        return
            m_sresult.error().code() ? makeErrorString( m_sresult ) : 
            m_eresult.error().code() ? makeErrorString( m_eresult ) :
            i18n( "Combined signing/encryption succeeded" ) ;
    if ( sign )
        return m_sresult.error().code() ? makeErrorString( m_sresult ) : i18n("Signing succeeded") ;
    else
        return m_eresult.error().code() ? makeErrorString( m_eresult ) : i18n("Encryption succeeded") ;
}

QString SignEncryptFilesResult::details() const {
    return i18n("Not yet implemented");
}


#include "moc_signencryptfilestask.cpp"


