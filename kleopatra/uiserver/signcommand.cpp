/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/signcommand.cpp

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

#include "signcommand.h"
#include "kleo-assuan.h"
#include "keyselectionjob.h"

#include "utils/stl_util.h"

#include <kleo/keylistjob.h>
#include <kleo/signjob.h>
#include <kleo/cryptobackendfactory.h>

#include <gpgme++/data.h>
#include <gpgme++/error.h>
#include <gpgme++/key.h>
#include <gpgme++/keylistresult.h>
#include <gpgme++/signingresult.h>

#include <KLocale>

#include <QIODevice>
#include <QString>
#include <QStringList>
#include <QObject>
#include <QDebug>

#include <boost/bind.hpp>

using namespace Kleo;

namespace {
    template <template <typename T> class Op>
    struct ByProtocol {
        typedef bool result_type;

        bool operator()( const GpgME::Key & lhs, const GpgME::Key & rhs ) const {
            return Op<int>()( qstricmp( lhs.protocolAsString(), rhs.protocolAsString() ), 0 );
        }
        bool operator()( const GpgME::Key & lhs, const char * rhs ) const {
            return Op<int>()( qstricmp( lhs.protocolAsString(), rhs ), 0 );
        }
        bool operator()( const char * lhs, const GpgME::Key & rhs ) const {
            return Op<int>()( qstricmp( lhs, rhs.protocolAsString() ), 0 );
        }
        bool operator()( const char * lhs, const char * rhs ) const {
            return Op<int>()( qstricmp( lhs, rhs ), 0 );
        }
    };

}

class SignCommand::Private
  : public AssuanCommandPrivateBaseMixin<SignCommand::Private, SignCommand>
{
    Q_OBJECT
public:
    Private( SignCommand * qq )
        :AssuanCommandPrivateBaseMixin<SignCommand::Private, SignCommand>()
        , q( qq ), m_statusSent(0)
    {}
    virtual ~Private() {}
    
    void checkInputs();
    void startKeySelection();
    void startSignJobs( const std::vector<GpgME::Key>& keys );
    void showKeySelectionDialog();
    
    struct Input {
        QIODevice* data;
        QString dataFileName;
        unsigned int id;
    };
    
    struct Result {
        GpgME::SigningResult result;
        QByteArray data;
        unsigned int id;
        unsigned int error;
        QString errorString;
    };

    SignCommand *q;

private Q_SLOTS:
    void slotKeySelectionResult( const std::vector<GpgME::Key>& );
    void slotKeySelectionError( const GpgME::Error& error, const GpgME::KeyListResult& );
    void slotSigningResult( const GpgME::SigningResult & result, const QByteArray & signature );
private:
    void trySendingStatus( const QString & str );
    
    std::vector<Input> m_inputs;
    QMap<int, Result> m_results;
    QMap<const SignJob*, unsigned int> m_jobs;
    int m_signJobs;
    int m_statusSent;
};

void SignCommand::Private::checkInputs()
{
    const int numInputs = q->numBulkInputDevices( "INPUT" );
    const int numOutputs = q->numBulkInputDevices( "OUTPUT" );
    const int numMessages = q->numBulkInputDevices( "MESSAGE" );

    //TODO use better error code if possible
    if ( numMessages != 0 )
        throw assuan_exception(makeError( GPG_ERR_ASS_NO_INPUT ), "Only --input and --output can be provided to the sign command, no --message"); 
       
    // either the output is discarded, or there ar as many as inputs
    //TODO use better error code if possible
    if ( numOutputs > 0 && numInputs != numOutputs )
        throw assuan_exception( makeError( GPG_ERR_ASS_NO_INPUT ),  "For each --input there needs to be an --output");

    for ( int i = 0; i < numInputs; ++i ) {
        Input input;
        input.id = i;
        input.data = q->bulkInputDevice( "INPUT", i );
        input.dataFileName = q->bulkInputDeviceFileName( "INPUT", i );

        m_inputs.push_back( input );
    }
}

void SignCommand::Private::startKeySelection()
{
    KeySelectionJob* job = new KeySelectionJob( this );
    job->setSecretKeysOnly( true );
    job->setPatterns( QStringList() ); // FIXME
    connect( job, SIGNAL( error( GpgME::Error, GpgME::KeyListResult ) ),
             this, SLOT( slotKeySelectionError( GpgME::Error, GpgME::KeyListResult ) ) );
    connect( job, SIGNAL( result( std::vector<GpgME::Key> ) ),
             this, SLOT( slotKeySelectionResult( std::vector<GpgME::Key> ) ) );
    job->start();
}

void SignCommand::Private::startSignJobs( const std::vector<GpgME::Key>& keys )
{
    // make sure the keys are all of the same type
    // FIXME reasonable assumption?
    if ( keys.empty() || !kdtools::all( keys.begin(), keys.end(), boost::bind( ByProtocol<std::equal_to>(), _1, keys.front() )  ) ) {
        q->done();
    }
    const CryptoBackend::Protocol* backend = CryptoBackendFactory::instance()->protocol( keys.front().protocolAsString() );
    
    assert( backend );
    
    Q_FOREACH( const Input input, m_inputs ) {
        SignJob *job = backend->signJob();
        connect( job, SIGNAL( result( GpgME::SigningResult, QByteArray ) ),
                 this, SLOT( slotSigningResult( GpgME::SigningResult, QByteArray ) ) );
        // FIXME port to iodevice
        // FIXME mode?
        if ( const GpgME::Error err = job->start( keys, input.data->readAll(), GpgME::NormalSignatureMode ) ) {
            q->done( err );
        }
        m_jobs.insert( job, input.id );
        m_signJobs++;
    }
}

void SignCommand::Private::slotKeySelectionResult( const std::vector<GpgME::Key>& keys )
{
    // fire off the sign jobs
    startSignJobs( keys );
}


void SignCommand::Private::slotKeySelectionError( const GpgME::Error& error, const GpgME::KeyListResult& )
{
    assert( error );
    if ( error == q->makeError( GPG_ERR_CANCELED ) ) 
        q->done( error, "User canceled key selection" );
    else
        q->done( error, "Error while listing and selecting private keys" );

}

void SignCommand::Private::trySendingStatus( const QString & str )
{
    if ( const int err = q->sendStatus( "SIGN", str ) ) {
        QString errorString = i18n("Problem writing out the signature.");
        q->done( err, errorString ) ;
    }
}

void SignCommand::Private::slotSigningResult( const GpgME::SigningResult & result, const QByteArray & signature )
{
    const SignJob * const job = qobject_cast<SignJob*>( sender() );
    assert( job );
    assert( m_jobs.contains( job ) );
    const unsigned int id = m_jobs[job];
    
    {
        Result res;
        res.result = result;
        res.data = signature;
        res.id = id;
        m_results.insert( id, res );
    }
    // send status for all results received so far, but in order of id
    while ( m_results.contains( m_statusSent ) ) {
       SignCommand::Private::Result result = m_results[m_statusSent];
       QString resultString;
       try {
           const GpgME::SigningResult & signres = result.result; 
           assert( !signres.isNull() );
               
           const GpgME::Error signError = signres.error();
           if ( signError )
               throw assuan_exception( signError, "Signing failed: " );
           // FIXME adjust for smime?
           const QString filename = q->bulkInputDeviceFileName( "INPUT", m_statusSent ) + ".sig";
           writeToOutputDeviceOrAskForFileName( result.id, result.data, filename );
           resultString = "OK - Super Duper Weenie\n";
       } catch ( const assuan_exception& e ) {
           result.error = e.error_code();
           result.errorString = e.what();
           m_results[result.id] = result;
           resultString = "ERR " + result.errorString;
           // FIXME ask to continue or cancel
       }
       trySendingStatus( resultString );
       m_statusSent++;
    }
    if ( --m_signJobs == 0 )
        q->done();
}

SignCommand::SignCommand()
:d( new Private( this ) )
{
}

SignCommand::~SignCommand()
{
}

int SignCommand::doStart()
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


void SignCommand::doCanceled()
{
}

#include "signcommand.moc"
