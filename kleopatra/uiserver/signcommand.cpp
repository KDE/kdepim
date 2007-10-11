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
#include "detail_p.h"

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

#include <algorithm>
#include <iterator>
#include <vector>

using namespace Kleo;

class SignCommand::Private
  : public AssuanCommandPrivateBaseMixin<SignCommand::Private, SignCommand>
{
    Q_OBJECT
public:
    Private( SignCommand * qq )
        :AssuanCommandPrivateBaseMixin<SignCommand::Private, SignCommand>()
        , q( qq ), m_signJobs( 0 ), m_statusSent( 0 )
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
        GpgME::Protocol protocol;
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
    bool trySendingStatus( const QString & str );
    
    std::vector<Input> m_inputs;
    QMap<int, Result> m_results;
    QMap<const SignJob*, unsigned int> m_jobs;
    int m_signJobs;
    int m_statusSent;
};

void SignCommand::Private::checkInputs()
{
    const int numInputs = q->numBulkInputDevices( "INPUT" );
    const int numOutputs = q->numBulkOutputDevices( "OUTPUT" );
    const int numMessages = q->numBulkInputDevices( "MESSAGE" );

    //TODO use better error code if possible
    if ( numMessages != 0 )
        throw assuan_exception(makeError( GPG_ERR_ASS_NO_INPUT ), i18n( "Only INPUT and OUTPUT can be provided to the sign command, MESSAGE") ); 
       
    // either the output is discarded, or there ar as many as inputs
    //TODO use better error code if possible
    if ( numOutputs > 0 && numInputs != numOutputs )
        throw assuan_exception( makeError( GPG_ERR_ASS_NO_INPUT ),  i18n( "For each INPUT there needs to be an OUTPUT") );

    const GpgME::Protocol protocol = q->checkProtocol();

    for ( int i = 0; i < numInputs; ++i ) {
        Input input;
        input.id = i;
        input.data = q->bulkInputDevice( "INPUT", i );
        input.dataFileName = q->bulkInputDeviceFileName( "INPUT", i );
        input.protocol = protocol;

        m_inputs.push_back( input );
    }
}

void SignCommand::Private::startKeySelection()
{
    KeySelectionJob* job = new KeySelectionJob( this );
    job->setSecretKeysOnly( true );
    job->setPatterns( q->senders() );
    job->setSilent( q->hasOption( "silent" ) );
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
    if ( keys.empty() || !kdtools::all( keys.begin(), keys.end(), boost::bind( _detail::ByProtocol<std::equal_to>(), _1, keys.front() )  ) ) {
        q->done();
        return;
    }
    Q_FOREACH( const Input input, m_inputs ) {

        const CryptoBackend::Protocol* backend = CryptoBackendFactory::instance()->protocol( input.protocol == GpgME::OpenPGP ? "openpgp" : "smime" );
        assert( backend ); // FIXME - this should be checked somewhere before
    
        SignJob *job = backend->signJob( true, true );
        connect( job, SIGNAL( result( GpgME::SigningResult, QByteArray ) ),
                 this, SLOT( slotSigningResult( GpgME::SigningResult, QByteArray ) ) );
        // FIXME port to iodevice
        if ( const GpgME::Error err = job->start( keys, input.data->readAll(), q->hasOption( "detached" ) ? GpgME::Detached : GpgME::NormalSignatureMode ) ) {
            q->done( err );
            return;
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
    assert( error || error.isCanceled() );
    if ( error.isCanceled() )
        q->done( error, i18n( "User canceled key selection" ) );
    else
        q->done( error, i18n( "Error while listing and selecting private keys" ) );

}

bool SignCommand::Private::trySendingStatus( const QString & str )
{
    if ( const int err = q->sendStatus( "SIGN", str ) ) {
        QString errorString = i18n("Problem writing out the signature.");
        q->done( err, errorString );
        return false;
    }
    return true;
}

static QString collect_micalgs( const GpgME::SigningResult & result, GpgME::Protocol proto ) {
    const std::vector<GpgME::CreatedSignature> css = result.createdSignatures();
    QStringList micalgs;
    std::transform( css.begin(), css.end(),
                    std::back_inserter( micalgs ),
                    bind( &QString::toLower, bind( &QString::fromLatin1, bind( &GpgME::CreatedSignature::hashAlgorithmAsString, _1 ), -1 ) ) );
    if ( proto == GpgME::OpenPGP )
        for ( QStringList::iterator it = micalgs.begin(), end = micalgs.end() ; it != end ; ++it )
            it->prepend( "pgp-" );
    micalgs.sort();
    micalgs.erase( std::unique( micalgs.begin(), micalgs.end() ), micalgs.end() );
    return micalgs.join( QLatin1String(",") );
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
               throw assuan_exception( signError, i18n( "Signing failed: " ) );

           // send MICALG status message:
           const QString micalg = collect_micalgs( signres, m_inputs[m_statusSent].protocol );
           if ( !micalg.isEmpty() )
               if ( const int err = q->sendStatus( "MICALG", micalg ) )
                   throw assuan_exception( err, i18n( "Couldn't send MICALG status string: " ) );

           // FIXME adjust for smime?
           const QString filename = q->bulkInputDeviceFileName( "INPUT", m_statusSent ) + ".sig";
           writeToOutputDeviceOrAskForFileName( result.id, result.data, filename );
           resultString = "OK - Signature written";
       } catch ( const assuan_exception& e ) {
           result.error = e.error_code();
           result.errorString = e.what();
           m_results[result.id] = result;
           resultString = "ERR " + result.errorString;
           // FIXME ask to continue or cancel
       }
       if ( !trySendingStatus( resultString ) ) // emit done on error
           return;
       
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
