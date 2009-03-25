/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/signencryptfilestask.cpp

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

#include "signencryptfilestask.h"

#include <utils/formatting.h>
#include <utils/input.h>
#include <utils/output.h>
#include <utils/stl_util.h>
#include <utils/kleo_assert.h>
#include <utils/exception.h>

#include <kleo/cryptobackendfactory.h>
#include <kleo/cryptobackend.h>
#include <kleo/signjob.h>
#include <kleo/signencryptjob.h>
#include <kleo/encryptjob.h>

#include <gpgme++/signingresult.h>
#include <gpgme++/encryptionresult.h>
#include <gpgme++/key.h>

#include <KLocalizedString>

#include <QFile>
#include <QFileInfo>
#include <QPointer>
#include <QTextDocument> // for Qt::escape

#include <boost/bind.hpp>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace boost;
using namespace GpgME;

namespace {

    QString formatInputOutputLabel( const QString & input, const QString & output, bool inputDeleted, bool outputDeleted ) {
        return i18nc( "Input file --> Output file (rarr is arrow", "%1 &rarr; %2",
                      inputDeleted ? QString("<s>%1</s>").arg( Qt::escape( input ) ) : Qt::escape( input ),
                      outputDeleted ? QString("<s>%1</s>").arg( Qt::escape( output ) ) : Qt::escape( output ) );
    }

    class ErrorResult : public Task::Result {
    public:
        ErrorResult( bool sign, bool encrypt, const Error & err, const QString & errStr, const QString & input, const QString & output, const QString & auditLog )
            : Task::Result(), m_sign( sign ), m_encrypt( encrypt ), m_error( err ), m_errString( errStr ), m_inputLabel( input ), m_outputLabel( output ), m_auditLog( auditLog ) {}

        /* reimp */ QString overview() const;
        /* reimp */ QString details() const;
        /* reimp */ int errorCode() const { return m_error.encodedError(); }
        /* reimp */ QString errorString() const { return m_errString; }
        /* reimp */ VisualCode code() const { return NeutralError; }
        /* reimp */ QString auditLogAsHtml() const { return m_auditLog; }
    private:
        const bool m_sign;
        const bool m_encrypt;
        const Error m_error;
        const QString m_errString;
        const QString m_inputLabel;
        const QString m_outputLabel;
        const QString m_auditLog;
    };

    class SignEncryptFilesResult : public Task::Result {
    public:
        SignEncryptFilesResult( const SigningResult & sr, const QString & input, const QString & output, bool inputRemoved, bool outputCreated, const QString & auditLog )
            : Task::Result(), m_sresult( sr ), m_inputLabel( input ), m_outputLabel( output ), m_inputRemoved( inputRemoved ), m_outputCreated( outputCreated ), m_auditLog( auditLog ) {
            assert( !m_sresult.isNull() );
        }
        SignEncryptFilesResult( const EncryptionResult & er, const QString & input, const QString & output, bool inputRemoved, bool outputCreated, const QString & auditLog )
            : Task::Result(), m_eresult( er ), m_inputLabel( input ), m_outputLabel( output ), m_inputRemoved( inputRemoved ), m_outputCreated( outputCreated ), m_auditLog( auditLog ) {
            assert( !m_eresult.isNull() );
        }
        SignEncryptFilesResult( const SigningResult & sr, const EncryptionResult & er, const QString & input, const QString & output, bool inputRemoved, bool outputCreated,  const QString & auditLog )
            : Task::Result(), m_sresult( sr ), m_eresult( er ), m_inputLabel( input ), m_outputLabel( output ), m_inputRemoved( inputRemoved ), m_outputCreated( outputCreated ), m_auditLog( auditLog ) {
            assert( !m_sresult.isNull() || !m_eresult.isNull() );
        }

        /* reimp */ QString overview() const;
        /* reimp */ QString details() const;
        /* reimp */ int errorCode() const;
        /* reimp */ QString errorString() const;
        /* reimp */ VisualCode code() const;
        /* reimp */ QString auditLogAsHtml() const;

    private:
        const SigningResult m_sresult;
        const EncryptionResult m_eresult;
        const QString m_inputLabel;
        const QString m_outputLabel;
        const bool m_inputRemoved;
        const bool m_outputCreated;
        const QString m_auditLog;
    };

    static QString makeSigningOverview( const Error & err ) {
        if ( err.isCanceled() )
            return i18n("Signing canceled.");

        if ( err )
            return i18n("Signing failed." );
        return i18n("Signing succeeded.");
    }

    static QString makeResultOverview( const SigningResult & result ) {
        return makeSigningOverview( result.error() );
    }

    static QString makeEncryptionOverview( const Error & err ) {
        if ( err.isCanceled() )
            return i18n("Encryption canceled.");

        if ( err )
            return i18n("Encryption failed." );

        return i18n("Encryption succeeded.");
    }


    static QString makeResultOverview( const EncryptionResult & result ) {
        return makeEncryptionOverview( result.error() );
    }

    static QString makeResultOverview( const SigningResult & sr, const EncryptionResult & er ) {
        if ( er.isNull() && sr.isNull() )
            return QString();
        if ( er.isNull() )
            return makeResultOverview( sr );
        if ( sr.isNull() )
            return makeResultOverview( er );
        if ( sr.error().isCanceled() || sr.error() )
            return makeResultOverview( sr );
        if ( er.error().isCanceled() || er.error() )
            return makeResultOverview( er );
        return i18n( "Signing and encryption succeeded." );
    }

    static QString makeResultDetails( const SigningResult & result ) {
        const Error err = result.error();
        if ( err )
            return Qt::escape( QString::fromLocal8Bit( err.asString() ) );
        return QString();
    }

    static QString makeResultDetails( const EncryptionResult & result ) {
        const Error err = result.error();
        if ( err )
            return Qt::escape( QString::fromLocal8Bit( err.asString() ) );
        return i18n(" Encryption succeeded." );
    }

}


QString ErrorResult::overview() const {
    assert( m_error || m_error.isCanceled() );
    assert( m_sign || m_encrypt );
    const QString label = formatInputOutputLabel( m_inputLabel, m_outputLabel, false, true );
    const bool canceled = m_error.isCanceled();
    if ( m_sign && m_encrypt )
        return canceled ? i18n( "%1: <b>Sign/encrypt canceled.</b>", label ) : i18n( " %1: Sign/encrypt failed.", label );
    return i18nc( "label: result. Example: foo -> foo.gpg: Encryption failed.", "%1: <b>%2</b>", label,
                  m_sign ? makeSigningOverview( m_error ) :makeEncryptionOverview( m_error ) );
}

QString ErrorResult::details() const {
    return m_errString;
}

class SignEncryptFilesTask::Private {
    friend class ::Kleo::Crypto::SignEncryptFilesTask;
    SignEncryptFilesTask * const q;
public:
    explicit Private( SignEncryptFilesTask * qq );

private:
    std::auto_ptr<Kleo::SignJob> createSignJob( GpgME::Protocol proto );
    std::auto_ptr<Kleo::SignEncryptJob> createSignEncryptJob( GpgME::Protocol proto );
    std::auto_ptr<Kleo::EncryptJob> createEncryptJob( GpgME::Protocol proto );
    shared_ptr<const Task::Result> makeErrorResult( const Error & err, const QString & errStr, const QString & auditLog );

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
    bool detached : 1;
    bool removeInput : 1;

    QPointer<Kleo::Job> job;
    shared_ptr<OverwritePolicy> m_overwritePolicy;
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
      detached( false ),
      removeInput( false ),
      job( 0 ),
      m_overwritePolicy( new OverwritePolicy( 0 ) )
{
    q->setAsciiArmor( true );
}

shared_ptr<const Task::Result> SignEncryptFilesTask::Private::makeErrorResult( const Error & err, const QString & errStr, const QString & auditLog )
{
    return shared_ptr<const ErrorResult>( new ErrorResult( sign, encrypt, err, errStr, input->label(), output->label(), auditLog ) );
}

SignEncryptFilesTask::SignEncryptFilesTask( QObject * p )
    : Task( p ), d( new Private( this ) )
{

}

SignEncryptFilesTask::~SignEncryptFilesTask() {}

void SignEncryptFilesTask::setInputFileName( const QString & fileName ) {
    kleo_assert( !d->job );
    kleo_assert( !fileName.isEmpty() );
    d->inputFileName = fileName;
}

void SignEncryptFilesTask::setOutputFileName( const QString & fileName ) {
    kleo_assert( !d->job );
    kleo_assert( !fileName.isEmpty() );
    d->outputFileName = fileName;
}

void SignEncryptFilesTask::setSigners( const std::vector<Key> & signers ) {
    kleo_assert( !d->job );
    d->signers = signers;
}

void SignEncryptFilesTask::setRecipients( const std::vector<Key> & recipients ) {
    kleo_assert( !d->job );
    d->recipients = recipients;
}


void SignEncryptFilesTask::setOverwritePolicy( const shared_ptr<OverwritePolicy> & policy ) {
    kleo_assert( !d->job );
    d->m_overwritePolicy = policy;
}

void SignEncryptFilesTask::setSign( bool sign ) {
    kleo_assert( !d->job );
    d->sign = sign;
}

void SignEncryptFilesTask::setEncrypt( bool encrypt ) {
    kleo_assert( !d->job );
    d->encrypt = encrypt;
}

void SignEncryptFilesTask::setRemoveInputFileOnSuccess( bool remove )
{
    kleo_assert( !d->job );
    d->removeInput = remove;
}

void SignEncryptFilesTask::setDetachedSignature( bool detached ) {
    kleo_assert( !d->job );
    d->detached = detached;
}

Protocol SignEncryptFilesTask::protocol() const {
    if ( d->sign && !d->signers.empty() )
        return d->signers.front().protocol();
    if ( d->encrypt ) {
        if ( !d->recipients.empty() )
            return d->recipients.front().protocol();
        else
            return GpgME::OpenPGP; // symmetric OpenPGP encryption
    }
    throw Kleo::Exception( gpg_error( GPG_ERR_INTERNAL ),
                           i18n("Cannot determine protocol for task") );
}

QString SignEncryptFilesTask::label() const {
    return d->input ? d->input->label() : QString();
}

QString SignEncryptFilesTask::tag() const {
    return Formatting::displayName( protocol() );
}

unsigned long long SignEncryptFilesTask::inputSize() const
{
    return QFileInfo( d->inputFileName ).size();
}

void SignEncryptFilesTask::doStart() {
    kleo_assert( !d->job );
    if ( d->sign ) {
        kleo_assert( !d->signers.empty() );
    }

    d->input = Input::createFromFile( d->inputFileName );
    d->output = Output::createFromFile( d->outputFileName, d->m_overwritePolicy );

    if ( d->encrypt )
        if ( d->sign ) {
            std::auto_ptr<Kleo::SignEncryptJob> job = d->createSignEncryptJob( protocol() );
            kleo_assert( job.get() );

            job->start( d->signers, d->recipients,
                        d->input->ioDevice(), d->output->ioDevice(), true );

            d->job = job.release();
        } else {
            std::auto_ptr<Kleo::EncryptJob> job = d->createEncryptJob( protocol() );
            kleo_assert( job.get() );

            job->start( d->recipients, d->input->ioDevice(), d->output->ioDevice(), true );

            d->job = job.release();
        }
    else
        if ( d->sign ) {
            std::auto_ptr<Kleo::SignJob> job = d->createSignJob( protocol() );
            kleo_assert( job.get() );

            job->start( d->signers,
                        d->input->ioDevice(), d->output->ioDevice(),
                        d->detached ? GpgME::Detached : GpgME::NormalSignatureMode );

            d->job = job.release();
        } else {
            kleo_assert( !"Either 'sign' or 'encrypt' must be set!" );
        }
}

void SignEncryptFilesTask::cancel() {
    if ( d->job )
        d->job->slotCancel();
}

std::auto_ptr<Kleo::SignJob> SignEncryptFilesTask::Private::createSignJob( GpgME::Protocol proto ) {
    const CryptoBackend::Protocol * const backend = CryptoBackendFactory::instance()->protocol( proto );
    kleo_assert( backend );
    std::auto_ptr<Kleo::SignJob> signJob( backend->signJob( q->asciiArmor(), /*textmode=*/false ) );
    kleo_assert( signJob.get() );
    connect( signJob.get(), SIGNAL(progress(QString,int,int)),
             q, SLOT(setProgress(QString,int,int)) );
    connect( signJob.get(), SIGNAL(result(GpgME::SigningResult,QByteArray)),
             q, SLOT(slotResult(GpgME::SigningResult)) );
    return signJob;
}

std::auto_ptr<Kleo::SignEncryptJob> SignEncryptFilesTask::Private::createSignEncryptJob( GpgME::Protocol proto ) {
    const CryptoBackend::Protocol * const backend = CryptoBackendFactory::instance()->protocol( proto );
    kleo_assert( backend );
    std::auto_ptr<Kleo::SignEncryptJob> signEncryptJob( backend->signEncryptJob( q->asciiArmor(), /*textmode=*/false ) );
    kleo_assert( signEncryptJob.get() );
    connect( signEncryptJob.get(), SIGNAL(progress(QString,int,int)),
             q, SLOT(setProgress(QString,int,int)) );
    connect( signEncryptJob.get(), SIGNAL(result(GpgME::SigningResult,GpgME::EncryptionResult,QByteArray)),
             q, SLOT(slotResult(GpgME::SigningResult,GpgME::EncryptionResult)) );
    return signEncryptJob;
}

std::auto_ptr<Kleo::EncryptJob> SignEncryptFilesTask::Private::createEncryptJob( GpgME::Protocol proto ) {
    const CryptoBackend::Protocol * const backend = CryptoBackendFactory::instance()->protocol( proto );
    kleo_assert( backend );
    std::auto_ptr<Kleo::EncryptJob> encryptJob( backend->encryptJob( q->asciiArmor(), /*textmode=*/false ) );
    kleo_assert( encryptJob.get() );
    connect( encryptJob.get(), SIGNAL(progress(QString,int,int)),
             q, SLOT(setProgress(QString,int,int)) );
    connect( encryptJob.get(), SIGNAL(result(GpgME::EncryptionResult,QByteArray)),
             q, SLOT(slotResult(GpgME::EncryptionResult)) );
    return encryptJob;
}

void SignEncryptFilesTask::Private::slotResult( const SigningResult & result ) {
    const Job * const job = qobject_cast<const Job*>( q->sender() );
    const QString auditLog = job ? job->auditLogAsHtml() : QString();
    bool inputRemoved = false;
    bool outputCreated = false;
    if ( result.error().code() ) {
        output->cancel();
    } else {
        try {
            kleo_assert( !result.isNull() );
            output->finalize();
            outputCreated = true;
            input->finalize();
            if ( removeInput ) {
                inputRemoved = QFile::remove( inputFileName );
            }
        } catch ( const GpgME::Exception & e ) {
            q->emitResult( makeErrorResult( e.error(), QString::fromLocal8Bit( e.what() ), auditLog ) );
            return;
        }
    }

    q->emitResult( shared_ptr<Result>( new SignEncryptFilesResult( result, input->label(), output->label(), inputRemoved, outputCreated, auditLog ) ) );
}

void SignEncryptFilesTask::Private::slotResult( const SigningResult & sresult, const EncryptionResult & eresult ) {
    const Job * const job = qobject_cast<const Job*>( q->sender() );
    const QString auditLog = job ? job->auditLogAsHtml() : QString();
    bool inputRemoved = false;
    bool outputCreated = false;
    if ( sresult.error().code() || eresult.error().code() ) {
        output->cancel();
    } else {
        try {
            kleo_assert( !sresult.isNull() || !eresult.isNull() );
            output->finalize();
            outputCreated = true;
            input->finalize();
            if ( removeInput ) {
                inputRemoved = QFile::remove( inputFileName );
            }
        } catch ( const GpgME::Exception & e ) {
            q->emitResult( makeErrorResult( e.error(), QString::fromLocal8Bit( e.what() ), auditLog ) );
            return;
        }
    }

    q->emitResult( shared_ptr<Result>( new SignEncryptFilesResult( sresult, eresult, input->label(), output->label(), inputRemoved, outputCreated, auditLog ) ) );
}

void SignEncryptFilesTask::Private::slotResult( const EncryptionResult & result ) {
    const Job * const job = qobject_cast<const Job*>( q->sender() );
    const QString auditLog = job ? job->auditLogAsHtml() : QString();
    bool inputRemoved = false;
    bool outputCreated = false;
    if ( result.error().code() ) {
        output->cancel();
    } else {
        try {
            kleo_assert( !result.isNull() );
            output->finalize();
            outputCreated = true;
            input->finalize();
            if ( removeInput ) {
                inputRemoved = QFile::remove( inputFileName );
            }
        } catch ( const GpgME::Exception & e ) {
            q->emitResult( makeErrorResult( e.error(), QString::fromLocal8Bit( e.what() ), auditLog ) );
            return;
        }
    }
    q->emitResult( shared_ptr<Result>( new SignEncryptFilesResult( result, input->label(), output->label(), inputRemoved, outputCreated, auditLog ) ) );
}

QString SignEncryptFilesResult::overview() const {
    const QString files = formatInputOutputLabel( m_inputLabel, m_outputLabel, m_inputRemoved, !m_outputCreated );
    return files + ": " + makeOverview( makeResultOverview( m_sresult, m_eresult ) );
}

QString SignEncryptFilesResult::details() const {
    return errorString();
}

int SignEncryptFilesResult::errorCode() const {
   if ( m_sresult.error().code() )
       return m_sresult.error().encodedError();
   if ( m_eresult.error().code() )
       return m_eresult.error().encodedError();
   return 0;
}

QString SignEncryptFilesResult::errorString() const {
    const bool sign = !m_sresult.isNull();
    const bool encrypt = !m_eresult.isNull();

    kleo_assert( sign || encrypt );

    if ( sign && encrypt ) {
        return
            m_sresult.error().code() ? makeResultDetails( m_sresult ) :
            m_eresult.error().code() ? makeResultDetails( m_eresult ) :
            QString();
    }

    return sign ? makeResultDetails( m_sresult ) : makeResultDetails( m_eresult );
}

Task::Result::VisualCode SignEncryptFilesResult::code() const
{
    if ( m_sresult.error().isCanceled() || m_eresult.error().isCanceled() )
        return Warning;
    return ( m_sresult.error().code() || m_eresult.error().code() ) ? NeutralError : NeutralSuccess;
}

QString SignEncryptFilesResult::auditLogAsHtml() const {
    return m_auditLog;
}

#include "moc_signencryptfilestask.cpp"
