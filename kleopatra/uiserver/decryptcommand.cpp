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

#include "decryptcommand.h"

#include <QObject>
#include <QIODevice>

#include <kleo/decryptjob.h>

#include <gpgme++/error.h>
#include <gpgme++/decryptionresult.h>
#include <gpgme++/verificationresult.h>

#include <gpg-error.h>

#include <cassert>

class Kleo::DecryptCommand::Private
  : public AssuanCommandPrivateBaseMixin<Kleo::DecryptCommand::Private, DecryptCommand>
{
    Q_OBJECT
public:
    Private( DecryptCommand * qq )
        :AssuanCommandPrivateBaseMixin<Kleo::DecryptCommand::Private, DecryptCommand>(), q( qq )
    {}

    DecryptCommand *q;

public Q_SLOTS:
    void slotDecryptionResult( const GpgME::DecryptionResult &, const QByteArray & plainText );
    void slotProgress( const QString& what, int current, int total );

};

Kleo::DecryptCommand::DecryptCommand()
    : AssuanCommandMixin<DecryptCommand>(),
      d( new Private( this ) )
{
}

Kleo::DecryptCommand::~DecryptCommand() {}

void Kleo::DecryptCommand::Private::slotProgress( const QString& what, int current, int total )
{
    // FIXME report progress, via sendStatus()
}

void Kleo::DecryptCommand::Private::slotDecryptionResult( const GpgME::DecryptionResult & decryptionResult, const QByteArray & plainText )
{
    const GpgME::Error decryptionError = decryptionResult.error();
    if ( decryptionError )
    {
        q->done( decryptionError );
        return;
    }

    //handle result, send status
    q->bulkOutputDevice( "OUTPUT" )->write( plainText );
    q->done();
}

int Kleo::DecryptCommand::doStart()
{
    /*
    d->parseCommandLine("");
    d->showDetails = !hasOption("silent");
    */

    GpgME::Error error;
    QString details;
    d->inputList = d->analyzeInput( error, details );
    if ( error ) {
        done( error, details );
        return error;
    }

    int err = d->determineInputsAndProtocols( details );
    if ( err )
        done( err, details );

    try {

        Q_FOREACH ( const Private::Input input, d->inputList )
        {
            assert( input.backend );

            //fire off appropriate kleo decrypt verify job
            Kleo::DecryptJob * const job = input.backend->decryptJob();
            assert(job);

            QObject::connect( job, SIGNAL( result( GpgME::DecryptionResult, QByteArray ) ),
                    d.get(), SLOT( slotDecryptionResult( GpgME::DecryptionResult, QByteArray ) ) );
            QObject::connect( job, SIGNAL( progress( QString, int, int ) ),
                    d.get(), SLOT( slotProgress( QString, int, int ) ) );

            const QByteArray encrypted = bulkInputDevice( "INPUT" )->readAll(); // FIXME safe enough?

            // FIXME handle cancelled, let job show dialog? both done and return error?
            const GpgME::Error error = job->start( encrypted );
            if ( error )
                done( error );
        }
    } catch ( const GpgME::Error & error ) {
        //delete collector;
        done( error );
        return error;
    }


    return error;
}

void Kleo::DecryptCommand::doCanceled()
{
}

#include "decryptcommand.moc"

