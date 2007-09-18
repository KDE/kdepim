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
#include "assuancommandprivatebase_p.h"

#include <QObject>
#include <QIODevice>
#include <QHash>

#include <kleo/decryptjob.h>

#include <gpgme++/error.h>
#include <gpgme++/decryptionresult.h>

#include <gpg-error.h>

#include <cassert>

using namespace Kleo;

class DecryptionResultCollector : public QObject
{
    Q_OBJECT
public:
    explicit DecryptionResultCollector( QObject* parent = 0 );

    struct Result {
        QString id;
        GpgME::DecryptionResult result;
        QByteArray stuff;
    };

    void registerJob( const QString& id, DecryptJob* job );
Q_SIGNALS:
    void finished( const QHash<QString, DecryptionResultCollector::Result> & );

private Q_SLOTS:
    void slotDecryptResult(const GpgME::DecryptionResult &, const QByteArray &);

private:
    QHash<QString, Result> m_results;
    QHash<QObject*, QString> m_senderToId;
    int m_unfinished;
};

DecryptionResultCollector::DecryptionResultCollector( QObject* parent )
: QObject( parent ), m_unfinished( 0 )
{
}

void DecryptionResultCollector::registerJob( const QString& id, DecryptJob* job )
{
    connect( job, SIGNAL( result( GpgME::DecryptionResult,QByteArray ) ),
             this, SLOT( slotDecryptResult( GpgME::DecryptionResult, QByteArray ) ) );
    m_senderToId[job] = id;
    ++m_unfinished;
}

void DecryptionResultCollector::slotDecryptResult(const GpgME::DecryptionResult & result,
                                                  const QByteArray & stuff )
{
    const QString id = m_senderToId[sender()];

    Result res;
    res.id = id;
    res.stuff = stuff;
    res.result = result;
    m_results[id] = res;

    --m_unfinished;
    assert( m_unfinished >= 0 );
    if ( m_unfinished == 0 ) {
        emit finished( m_results );
        deleteLater();
    }
}

class DecryptCommand::Private
  : public AssuanCommandPrivateBaseMixin<DecryptCommand::Private, DecryptCommand>
{
    Q_OBJECT
public:
    Private( DecryptCommand * qq )
        :AssuanCommandPrivateBaseMixin<DecryptCommand::Private, DecryptCommand>(), q( qq )
    {}

    DecryptCommand *q;
    QList<Input> analyzeInput( GpgME::Error& error, QString& errorDetails ) const;
    int startDecryption();

public Q_SLOTS:
    void slotDecryptionCollectionResult( const QHash<QString, DecryptionResultCollector::Result>& );
    void slotProgress( const QString& what, int current, int total );

};

DecryptCommand::DecryptCommand()
    : AssuanCommandMixin<DecryptCommand>(),
      d( new Private( this ) )
{
}

DecryptCommand::~DecryptCommand() {}

QList<AssuanCommandPrivateBase::Input> DecryptCommand::Private::analyzeInput( GpgME::Error& error, QString& errorDetails ) const
{
    error = GpgME::Error();
    errorDetails = QString();

    const int numInputs = q->numBulkInputDevices( "INPUT" );
    const int numOutputs = q->numBulkInputDevices( "OUTPUT" );
    const int numMessages = q->numBulkInputDevices( "MESSAGE" );

    if ( numMessages != 0 )
    {
        error = GpgME::Error( GPG_ERR_ASS_NO_INPUT ); //TODO use better error code if possible
        errorDetails = "Only --input can be provided to the decrypt command, no --message";
        return QList<Input>();
    }

    // either the output is discarded, or there ar as many as inputs
    if ( numOutputs > 0 && numInputs != numOutputs )
    {
        error = GpgME::Error( GPG_ERR_ASS_NO_INPUT ); //TODO use better error code if possible
        errorDetails = "For each --input there needs to be an --output";
        return QList<Input>();
    }

    // FIXME TEMPORARY
    if ( numInputs > 1 ) {
         error = GpgME::Error( GPG_ERR_ASS_NO_INPUT ); //TODO use better error code if possible
        errorDetails = "The decrypt command currently only accepts one input at at time.";
        return QList<Input>();
    }

    QList<Input> inputs;

    for ( int i = 0; i < numInputs; ++i )
    {
        Input input;
        input.message = q->bulkInputDevice( "INPUT", i );
        input.messageFileName = q->bulkInputDeviceFileName( "INPUT", i );
        assert( input.message );
        input.type = Input::Opaque; // by definition
        inputs.append( input );
    }

    return inputs;
}

void DecryptCommand::Private::slotProgress( const QString& what, int current, int total )
{
    // FIXME report progress, via sendStatus()
}


void DecryptCommand::Private::slotDecryptionCollectionResult( const QHash<QString, DecryptionResultCollector::Result>& results )
{
    assert( !results.isEmpty() );
    QList<DecryptionResultCollector::Result>::const_iterator it = results.values().begin();
    QList<DecryptionResultCollector::Result>::const_iterator end = results.values().end();
    for ( int i = 0; it != end; ++it, ++i ) {
        const DecryptionResultCollector::Result result = *it;

        const GpgME::Error decryptionError = result.result.error();
        if ( decryptionError ) {
            q->done( decryptionError );
            return;
        }

        //handle result, send status
        QIODevice * const outdevice = q->bulkOutputDevice( "OUTPUT", result.id.toInt() );
        if ( outdevice ) {
            if ( const int err = outdevice->write( result.stuff ) ) {
                q->done( err );
                return;
            }
        }
    }
    q->done();
}

int DecryptCommand::Private::startDecryption()
{
    assert( !inputList.isEmpty() );
    DecryptionResultCollector* collector = new DecryptionResultCollector;
    connect( collector, SIGNAL( finished( QHash<QString, DecryptionResultCollector::Result> ) ),
             SLOT( slotDecryptionCollectionResult( QHash<QString, DecryptionResultCollector::Result> ) ) );

    try {

        int i = 0;
        Q_FOREACH ( const Private::Input input, inputList )
        {
            ++i;
            assert( input.backend );

            //fire off appropriate kleo decrypt verify job
            DecryptJob * const job = input.backend->decryptJob();
            assert(job);
            collector->registerJob( QString::number(i), job );

            // FIXME handle file names
            const QByteArray encrypted = input.message->readAll(); // FIXME safe enough?
            const GpgME::Error error = job->start( encrypted );
            if ( error ) throw error;

        }
    } catch ( const GpgME::Error & error ) {
        delete collector;
        q->done( error );
        return error;
    }

    return 0;
}



int DecryptCommand::doStart()
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
    if ( err ) {
        done( err, details );
        return err;
    }
    err = d->startDecryption();
    return 0;

}

void DecryptCommand::doCanceled()
{
}

#include "decryptcommand.moc"

