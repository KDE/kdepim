/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/encryptemailtask.cpp

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

#include "encryptemailtask.h"

#include <utils/input.h>
#include <utils/output.h>
#include <utils/kleo_assert.h>
#include <utils/auditlog.h>

#include <kleo/stl_util.h>
#include <kleo/cryptobackendfactory.h>
#include <kleo/cryptobackend.h>
#include <kleo/encryptjob.h>

#include <gpgme++/encryptionresult.h>
#include <gpgme++/key.h>

#include <KLocalizedString>

#include <QPointer>
#include <QTextDocument> // for Qt::escape

#include <boost/bind.hpp>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace boost;
using namespace GpgME;

namespace {

    class EncryptEMailResult : public Task::Result {
        const EncryptionResult m_result;
        const AuditLog m_auditLog;
    public:
        EncryptEMailResult( const EncryptionResult & r, const AuditLog & auditLog )
            : Task::Result(), m_result( r ), m_auditLog( auditLog ) {}

        /* reimp */ QString overview() const;
        /* reimp */ QString details() const;
        /* reimp */ int errorCode() const;
        /* reimp */ QString errorString() const;
        /* reimp */ VisualCode code() const;
        /* reimp */ AuditLog auditLog() const;
    };

    QString makeResultString( const EncryptionResult& res )
    {
        const Error err = res.error();

        if ( err.isCanceled() )
            return i18n( "Encryption canceled." );

        if ( err )
            return i18n( "Encryption failed: %1", Qt::escape( QString::fromLocal8Bit( err.asString() ) ) );

        return i18n( "Encryption succeeded." );
    }

}

class EncryptEMailTask::Private {
    friend class ::Kleo::Crypto::EncryptEMailTask;
    EncryptEMailTask * const q;
public:
    explicit Private( EncryptEMailTask * qq );

private:
    std::auto_ptr<Kleo::EncryptJob> createJob( GpgME::Protocol proto );

private:
    void slotResult( const EncryptionResult & );

private:
    shared_ptr<Input> input;
    shared_ptr<Output> output;
    std::vector<Key> recipients;

    QPointer<Kleo::EncryptJob> job;
};

EncryptEMailTask::Private::Private( EncryptEMailTask * qq )
    : q( qq ),
      input(),
      output(),
      job( 0 )
{

}

EncryptEMailTask::EncryptEMailTask( QObject * p )
    : Task( p ), d( new Private( this ) )
{

}

EncryptEMailTask::~EncryptEMailTask() {}

void EncryptEMailTask::setInput( const shared_ptr<Input> & input ) {
    kleo_assert( !d->job );
    kleo_assert( input );
    d->input = input;
}

void EncryptEMailTask::setOutput( const shared_ptr<Output> & output ) {
    kleo_assert( !d->job );
    kleo_assert( output );
    d->output = output;
}

void EncryptEMailTask::setRecipients( const std::vector<Key> & recipients ) {
    kleo_assert( !d->job );
    kleo_assert( !recipients.empty() );
    d->recipients = recipients;
}

Protocol EncryptEMailTask::protocol() const {
    kleo_assert( !d->recipients.empty() );
    return d->recipients.front().protocol();
}

QString EncryptEMailTask::label() const
{
    return d->input ? d->input->label() : QString();
}

unsigned long long EncryptEMailTask::inputSize() const {
    return d->input ? d->input->size() : 0;
}

void EncryptEMailTask::doStart() {
    kleo_assert( !d->job );
    kleo_assert( d->input );
    kleo_assert( d->output );
    kleo_assert( !d->recipients.empty() );

    std::auto_ptr<Kleo::EncryptJob> job = d->createJob( protocol() );
    kleo_assert( job.get() );

    job->start( d->recipients,
                d->input->ioDevice(), d->output->ioDevice(),
                /*alwaysTrust=*/true );

    d->job = job.release();
}

void EncryptEMailTask::cancel() {
    if ( d->job )
        d->job->slotCancel();
}

std::auto_ptr<Kleo::EncryptJob> EncryptEMailTask::Private::createJob( GpgME::Protocol proto ) {
    const CryptoBackend::Protocol * const backend = CryptoBackendFactory::instance()->protocol( proto );
    kleo_assert( backend );
    bool shouldArmor = ( proto == OpenPGP || q->asciiArmor() ) && !output->binaryOpt();
    std::auto_ptr<Kleo::EncryptJob> encryptJob( backend->encryptJob( shouldArmor, /*textmode=*/false ) );
    kleo_assert( encryptJob.get() );
    if ( proto == CMS && !q->asciiArmor() && !output->binaryOpt() )
        encryptJob->setOutputIsBase64Encoded( true );
    connect( encryptJob.get(), SIGNAL(progress(QString,int,int)),
             q, SLOT(setProgress(QString,int,int)) );
    connect( encryptJob.get(), SIGNAL(result(GpgME::EncryptionResult,QByteArray)),
             q, SLOT(slotResult(GpgME::EncryptionResult)) );
    return encryptJob;
}

void EncryptEMailTask::Private::slotResult( const EncryptionResult & result ) {
    const Job * const job = qobject_cast<const Job*>( q->sender() );
    if ( result.error().code() ) {
        output->cancel();
    } else {
        output->finalize();
    }
    q->emitResult( shared_ptr<Result>( new EncryptEMailResult( result, AuditLog::fromJob( job ) ) ) );
}

QString EncryptEMailResult::overview() const {
    return makeOverview( makeResultString( m_result ) );
}

QString EncryptEMailResult::details() const {
    return QString();
}

int EncryptEMailResult::errorCode() const {
    return m_result.error().encodedError();
}

QString EncryptEMailResult::errorString() const {
    return hasError() ? makeResultString( m_result ) : QString();
}

AuditLog EncryptEMailResult::auditLog() const {
    return m_auditLog;
}

Task::Result::VisualCode EncryptEMailResult::code() const
{
    if ( m_result.error().isCanceled() )
        return Warning;
    return m_result.error().code() ? NeutralError : NeutralSuccess;
}


#include "moc_encryptemailtask.cpp"


