/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/decryptemailcommand.cpp

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

#include "decryptemailcommand.h"

#include <QObject>
#include <QIODevice>

#include <kleo/decryptjob.h>
#include <kleo/cryptobackendfactory.h>

#include <gpgme++/error.h>
#include <gpgme++/decryptionresult.h>

#include <cassert>

class Kleo::DecryptEmailCommand::Private : public QObject
{
    Q_OBJECT
public:
    Private( DecryptEmailCommand * qq )
        :q( qq ), backend(0)
    {}

    DecryptEmailCommand *q;
    const CryptoBackend::Protocol *backend;
    void findCryptoBackend();

public Q_SLOTS:
    void slotDecryptionResult( const GpgME::DecryptionResult &, const QByteArray & plainText );
    void slotProgress( const QString& what, int current, int total );

};

Kleo::DecryptEmailCommand::DecryptEmailCommand()
    : AssuanCommandMixin<DecryptEmailCommand>(),
      d( new Private( this ) )
{
}

Kleo::DecryptEmailCommand::~DecryptEmailCommand() {}

void Kleo::DecryptEmailCommand::Private::findCryptoBackend()
{
    // FIXME this could be either SMIME or OpenPGP, find out from headers
    const bool isSMIME = true;
    if ( isSMIME )
        backend = Kleo::CryptoBackendFactory::instance()->smime();
    else
        backend = Kleo::CryptoBackendFactory::instance()->openpgp();
}


void Kleo::DecryptEmailCommand::Private::slotProgress( const QString& what, int current, int total )
{
    // FIXME report progress, via sendStatus()
}

void Kleo::DecryptEmailCommand::Private::slotDecryptionResult( const GpgME::DecryptionResult & result, const QByteArray & plainText )
{
    //handle result, send status?
    q->bulkOutputDevice()->write( plainText );
}

int Kleo::DecryptEmailCommand::start( const std::string & line )
{
    // FIXME parse line
    Q_UNUSED(line)

    // FIXME check options

    d->findCryptoBackend(); // decide on smime or openpgp
    assert(d->backend);

    // get encrypted data
    const QByteArray encrypted = bulkInputDevice()->readAll(); // FIXME safe enough?

    //fire off appropriate kleo decrypt job
    Kleo::DecryptJob * const job = d->backend->decryptJob();
    assert(job);

    QObject::connect( job, SIGNAL(result(GpgME::DecryptionResult,QByteArray)),
                      d.get(), SLOT(slotDecryptionResult(GpgME::DecryptionResult,QByteArray)) );
    QObject::connect( job, SIGNAL(progress(QString,int,int)),
                      d.get(), SLOT(slotProgress(QString,int,int)) );

    // FIXME handle cancelled, let job show dialog? both done and return error?
    const GpgME::Error error = job->start( encrypted );
    if ( error )
        done( error );
    return error;
}

void Kleo::DecryptEmailCommand::canceled()
{
}

#include "decryptemailcommand.moc"

