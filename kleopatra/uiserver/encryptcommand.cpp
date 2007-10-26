/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/encryptcommand.cpp

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

#include "encryptcommand.h"
#include "keyselectionjob.h"
#include "kleo-assuan.h"

#include <kleo/encryptjob.h>

#include <gpgme++/encryptionresult.h>
#include <gpgme++/error.h>

#include <KLocale>

#include <QMap>
#include <QStringList>

using namespace Kleo;
using namespace boost;

class EncryptCommand::Private
  : public AssuanCommandPrivateBaseMixin<EncryptCommand::Private, EncryptCommand>
{
    Q_OBJECT
public:
    Private( EncryptCommand * qq )
        :AssuanCommandPrivateBaseMixin<EncryptCommand::Private, EncryptCommand>()
        , m_deleteInputFiles( false ), m_activeEncryptJobs( 0 ), m_statusSent( 0 ), q( qq )
    {}
    virtual ~Private() {}

    void checkInputs();
    void startKeySelection();
    void startEncryptJobs( const std::vector<GpgME::Key>& keys );


    struct Input {
        shared_ptr<QIODevice> input;
        QString inputFileName;
        int id;
        GpgME::Protocol protocol;
    };
    
    struct Result {
        GpgME::EncryptionResult result;
        QByteArray data;
        unsigned int id;
        unsigned int error;
        QString errorString;
    };

    bool m_deleteInputFiles;
    std::vector<Input> m_inputs;
    int m_activeEncryptJobs;
    QMap<const EncryptJob*, uint> m_jobs;
    QMap<uint, Result> m_results;
    int m_statusSent;

public Q_SLOTS:
    void slotKeySelectionError( const GpgME::Error&, const GpgME::KeyListResult& );
    void slotKeySelectionResult( const std::vector<GpgME::Key>& ); 
    void slotEncryptionResult( const GpgME::EncryptionResult& result, const QByteArray& cipherText );

private:
    bool trySendingStatus( const QString & str );

public:

    EncryptCommand *q;
};

void EncryptCommand::Private::checkInputs()
{
    const int numInputs = q->numBulkInputDevices();
    const int numOutputs = q->numBulkOutputDevices();
    const int numMessages = q->numBulkMessageDevices();

    //TODO use better error code if possible
    if ( numMessages != 0 )
        throw assuan_exception( makeError( GPG_ERR_ASS_NO_INPUT ), i18n( "Only INPUT and OUTPUT can be provided to the ENCRYPT command, no MESSAGE") ); 
       
    // for each input, we need an output
    //TODO use better error code if possible
    if ( numOutputs && numInputs != numOutputs )
        throw assuan_exception( makeError( GPG_ERR_ASS_NO_INPUT ),  i18n( "For each INPUT there needs to be an OUTPUT" ) );

    GpgME::Protocol protocol;
    try { protocol = q->checkProtocol( EMail ); }
    catch ( ... ) { protocol = GpgME::UnknownProtocol; }

    for ( int i = 0; i < numInputs; ++i ) {
        Input input;
        input.input = q->bulkInputDevice( i );
        assert( input.input );
        input.inputFileName = q->bulkInputDeviceFileName( i );
        input.id = i;
        input.protocol = protocol;
        m_inputs.push_back( input );
    }

    m_deleteInputFiles = q->hasOption( "delete-input-files" );
}

void EncryptCommand::Private::startKeySelection()
{
    KeySelectionJob* job = new KeySelectionJob( this );
    job->setSecretKeysOnly( false );
    job->setPatterns( q->recipients() );
    job->setSilent( q->hasOption( "silent" ) );
    connect( job, SIGNAL( error( GpgME::Error, GpgME::KeyListResult ) ),
             this, SLOT( slotKeySelectionError( GpgME::Error, GpgME::KeyListResult ) ) );
    connect( job, SIGNAL( result( std::vector<GpgME::Key> ) ),
             this, SLOT( slotKeySelectionResult( std::vector<GpgME::Key> ) ) );
    job->start();
}

void EncryptCommand::Private::startEncryptJobs( const std::vector<GpgME::Key>& keys )
{
    assert( m_activeEncryptJobs == 0 );

    if ( keys.empty() ) {
        q->done();
        return;
    }

    const GpgME::Protocol defaultProtocol = keys.front().protocol();

    Q_FOREACH( const Input i, m_inputs ) {
        const GpgME::Protocol proto = i.protocol == GpgME::UnknownProtocol ? defaultProtocol : i.protocol ;
        const CryptoBackend::Protocol* const backend = CryptoBackendFactory::instance()->protocol( proto == GpgME::OpenPGP ? "openpgp" : "smime" );
        assert( backend ); // FIXME: this should be checked upstream

        EncryptJob* job = backend->encryptJob( true, true );
        connect( job, SIGNAL( result( GpgME::EncryptionResult, QByteArray ) ), 
                 this, SLOT( slotEncryptionResult( GpgME::EncryptionResult, QByteArray ) ) ); 

        if ( const GpgME::Error error = job->start( keys, i.input->readAll(), /*always trust*/true ) ) { //TODO how to handle trust arg? Add an option?
            q->done( error );
            return;
        }
        m_jobs.insert( job, i.id );
        ++m_activeEncryptJobs;
    }
}

void EncryptCommand::Private::slotEncryptionResult( const GpgME::EncryptionResult& result, const QByteArray& cipherText )
{
    assert( m_activeEncryptJobs > 0 );
    const EncryptJob * const job = qobject_cast<EncryptJob*>( sender() );
    assert( job );
    assert( m_jobs.contains( job ) );
    const unsigned int id = m_jobs[job];
    
    {
        Result res;
        res.result = result;
        res.data = cipherText;
        res.id = id;
        m_results.insert( id, res );
    }
    // send status for all results received so far, but in order of id
    while ( m_results.contains( m_statusSent ) ) {
       EncryptCommand::Private::Result result = m_results[m_statusSent];
       QString resultString;
       try {
           const GpgME::EncryptionResult encres = result.result; 
           assert( !encres.isNull() );
               
           const GpgME::Error encryptError = encres.error();
           if ( encryptError )
               throw assuan_exception( encryptError, i18n( "Encryption failed: " ) );
           // FIXME adjust for smime?
           writeToOutputDeviceOrAskForFileName( result.id, result.data, QString() );
           resultString = "OK - Super Duper Weenie\n";
       } catch ( const assuan_exception& e ) {
           result.error = e.error_code();
           result.errorString = e.what();
           m_results[result.id] = result;
           resultString = "ERR " + result.errorString;
           // FIXME ask to continue or cancel
       }
       if ( !trySendingStatus( resultString ) )
           return; // trySendingStatus calls done() if it fails
       ++m_statusSent;
    }
    --m_activeEncryptJobs;
    if ( m_activeEncryptJobs == 0 )
        q->done();
}


bool EncryptCommand::Private::trySendingStatus( const QString & str )
{
    if ( const int err = q->sendStatus( "ENCRYPT", str ) ) {
        const QString errorString = i18n("Problem writing out the cipher text.");
        q->done( err, errorString );
        return false;
    }
    return true;
}

void EncryptCommand::Private::slotKeySelectionResult( const std::vector<GpgME::Key>& keys )
{
    startEncryptJobs( keys );
}


void EncryptCommand::Private::slotKeySelectionError( const GpgME::Error& error, const GpgME::KeyListResult& )
{
    assert( error || error.isCanceled() );
    if ( error.isCanceled() )
        q->done( error, i18n( "User canceled key selection" ) );
    else
        q->done( error, i18n( "Error while listing and selecting keys" ) );
}


EncryptCommand::EncryptCommand()
:d( new Private( this ) )
{
}

EncryptCommand::~EncryptCommand()
{
}

int EncryptCommand::doStart()
{
    try {
        d->checkInputs();
       d->startKeySelection();
    } catch ( const assuan_exception& e ) {
        done( e.error_code(), e.what());
        return e.error_code();
    }
    
    return 0;
}

void EncryptCommand::doCanceled()
{
}

#include "encryptcommand.moc"

