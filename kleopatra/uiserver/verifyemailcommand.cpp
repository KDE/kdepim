/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/verifyemailcommand.cpp

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

#include "verifyemailcommand.h"

#include <QObject>
#include <QIODevice>

#include <kleo/verifyopaquejob.h>
#include <kleo/verifydetachedjob.h>
#include <kleo/cryptobackendfactory.h>

#include <gpgme++/error.h>
#include <gpgme++/verificationresult.h>

using namespace Kleo;

class VerifyEmailCommand::Private : public QObject
{
    Q_OBJECT
public:
    Private( VerifyEmailCommand* _q)
    :q(_q), backend(0)
        {}
    VerifyEmailCommand *q;
    const CryptoBackend::Protocol *backend;
    void findCryptoBackend();

public slots:
    void slotDetachedSignature( int, QByteArray, QByteArray );
    void slotVerifyOpaqueResult(const GpgME::VerificationResult &, const QByteArray &);
    void slotVerifyDetachedResult(const GpgME::VerificationResult &);
    void slotProgress( const QString& what, int current, int total );

};

VerifyEmailCommand::VerifyEmailCommand()
    : AssuanCommandMixin<VerifyEmailCommand>(),
      d( new Private( this ) )
{
}

VerifyEmailCommand::~VerifyEmailCommand() {}

void VerifyEmailCommand::Private::findCryptoBackend()
{
    // FIXME this could be either SMIME or OpenPGP, find out from headers
    const bool isSMIME = true;
    if ( isSMIME )
        backend = Kleo::CryptoBackendFactory::instance()->smime();
    else
        backend = Kleo::CryptoBackendFactory::instance()->openpgp();
}

void VerifyEmailCommand::Private::slotDetachedSignature( int, QByteArray, QByteArray )
{
    const QByteArray signature; // FIXME
    const QByteArray signedData; // FIXME
    // we now have the detached signature, verify it
    VerifyDetachedJob *job = backend->verifyDetachedJob();
    assert(job);

    QObject::connect( job,
                      SIGNAL( result(const GpgME::VerificationResult &) ),
                      this,
                      SLOT( slotVerifyDetachedResult(const GpgME::VerificationResult &) ) );
    QObject::connect( job,
                      SIGNAL( progress( const QString & , int, int ) ),
                      this,
                      SLOT( slotProgress( const QString&, int, int ) ) );
    GpgME::Error error = job->start( signature, signedData );
    if (error)
        q->done(error);
}

void VerifyEmailCommand::Private::slotVerifyOpaqueResult( const GpgME::VerificationResult &,
                                                          const QByteArray &)
{
    // present result
}

void VerifyEmailCommand::Private::slotVerifyDetachedResult( const GpgME::VerificationResult & )
{
   // present result
}

void VerifyEmailCommand::Private::slotProgress( const QString& what, int current, int total )
{
    // FIXME report progress, via sendStatus()
}

int VerifyEmailCommand::start( const std::string & line )
{
    // FIXME parse line
    Q_UNUSED(line)

    // FIXME check options

    d->findCryptoBackend(); // decide on smime or openpgp
    assert(d->backend);

    // FIXME figure out if it's an opaque or a detached signature
    const bool detached = true;

    if ( detached ) {
        // we need to inquire for the signature data
        const int err = inquire( "DETACHED_SIGNATURE",
                                 d.get(), SLOT(slotDetachedSignature(int,QByteArray,QByteArray)) );
        if ( err )
            done( err );
        return err; // 0 is all is ok, err otherwise 
    }

    // this is an opaque signature, get the data for it
    const QByteArray data = bulkInputDevice()->readAll(); // FIXME safe enough?

    //fire off appropriate kleo verification job
    VerifyOpaqueJob *job = d->backend->verifyOpaqueJob();
    assert(job);

    QObject::connect( job,
                      SIGNAL( result(GpgME::VerificationResult,QByteArray) ),
                      d.get(),
                      SLOT( slotVerifyOpaqueResult(const GpgME::VerificationResult &, const QByteArray &) ) );
    QObject::connect( job,
                      SIGNAL( progress( const QString & , int, int ) ),
                      d.get(),
                      SLOT( slotProgress( const QString&, int, int ) ) );

    // FIXME handle cancelled, let job show dialog? both done and return error?
    GpgME::Error error = job->start( data );
    if ( error )
        done( error );
    return error;
}

void VerifyEmailCommand::canceled()
{
}

void VerifyEmailCommand::reset()
{
}

#include "verifyemailcommand.moc"
