/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/verifycommand.cpp

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

#include "decryptverifycommand.h"

#include "decryptverifywizard.h"
#include "decryptverifyresultwidget.h"
#include "decryptverifyoperationwidget.h"

#include "kleo-assuan.h"
#include "classify.h"
#include "detail_p.h"

#include <models/keycache.h>
#include <models/predicates.h>

#include <utils/formatting.h>

#include <kleo/verifyopaquejob.h>
#include <kleo/verifydetachedjob.h>
#include <kleo/decryptjob.h>
#include <kleo/decryptverifyjob.h>

#include <gpgme++/error.h>
#include <gpgme++/key.h>
#include <gpgme++/verificationresult.h>
#include <gpgme++/decryptionresult.h>

#include <gpg-error.h>

#include <KLocale>
#include <KIconLoader>
#include <KMessageBox>

#include <QFileDialog>
#include <QObject>
#include <QIODevice>
#include <QMap>
#include <QHash>
#include <QPointer>
#include <QTemporaryFile>

#include <cassert>
#include <algorithm>
#include <functional>

using namespace Kleo;
using namespace GpgME;
using namespace boost;

// -- helpers ---
static
// dup of same function in decryptverifyresultwidget.cpp
const GpgME::Key & keyForSignature( const Signature & sig, const std::vector<Key> & keys ) {
    if ( const char * const fpr = sig.fingerprint() ) {
        const std::vector<Key>::const_iterator it
            = std::lower_bound( keys.begin(), keys.end(), fpr, _detail::ByFingerprint<std::less>() );
        if ( it != keys.end() && _detail::ByFingerprint<std::equal_to>()( *it, fpr ) )
            return *it;
    }
    static const Key null;
    return null;
}

namespace {

    enum Type {
        Decrypt,
        DecryptVerify,
        VerifyOpaque,
        VerifyDetached
    } type;

    struct Input {

        Input() : type( VerifyDetached ), backend( 0 ) {}

        Type type;

        struct _Input {
            shared_ptr<QIODevice> io;
            QString fileName;
            shared_ptr<QFile> file; // only used when != {INPUT,MESSAGE} (shared_ptr b/c QFile isn't copyable)
        } input, signedData;

        struct _Output {
            shared_ptr<QIODevice> io;
            QString fileName;
            shared_ptr<QTemporaryFile> tmp;
        } output;

        const CryptoBackend::Protocol* backend;

        static _Input fromOpenFile( const shared_ptr<QFile> & file ) {
            assuan_assert( file->isOpen() );
            const _Input input = {
                file, file->fileName(), file
            };
            return input;
        }

        static _Input openExistingFile( const QString & fname ) {
            if ( fname.isEmpty() )
                throw assuan_exception( gpg_error( GPG_ERR_ASS_NO_INPUT ), i18n("Empty filename provided") );
            const shared_ptr<QFile> f( new QFile( fname ) );
            if ( !f->open( QIODevice::ReadOnly ) ) // ### ask again?
                throw assuan_exception( gpg_error( GPG_ERR_ASS_NO_INPUT ),
                                        i18n("Can't open file \"%1\" for reading: %2", fname, f->errorString() ) );
            const _Input input = {
                f, fname, f
            };
            return input;
        }

        static _Output makeTemporaryOutput( const QString & fname ) {
            const shared_ptr<QTemporaryFile> tmp( new QTemporaryFile( fname ) );
            if ( !tmp->open() )
                throw assuan_exception( gpg_error( GPG_ERR_ASS_NO_OUTPUT ),
                                        i18n("Can't open temporary file \"%1\": %2", tmp->fileName(), tmp->errorString() ) );
            const _Output output = {
                tmp, fname, tmp,
            };
            return output;
        }

        void finalizeOutput();
    };

    struct DVResult {

        Type type;
        int id;
        VerificationResult verificationResult;
        DecryptionResult decryptionResult;
        QByteArray stuff;
        int error;
        QString errorString;

        static DVResult fromDecryptResult( int id, const DecryptionResult & dr, const QByteArray & plaintext ) {
            const DVResult res = {
                Decrypt,
                id,
                VerificationResult(),
                dr,
                plaintext,
                0,
                QString()
            };
            return res;
        }
        static DVResult fromDecryptResult( int id, const Error & err ) {
            return fromDecryptResult( id, DecryptionResult( err ), QByteArray() );
        }

        static DVResult fromDecryptVerifyResult( int id, const DecryptionResult & dr, const VerificationResult & vr, const QByteArray & plaintext ) {
            const DVResult res = {
                DecryptVerify,
                id,
                vr,
                dr,
                plaintext,
                0,
                QString()
            };
            return res;
        }
        static DVResult fromDecryptVerifyResult( int id, const Error & err ) {
            return fromDecryptVerifyResult( id, DecryptionResult( err ), VerificationResult(), QByteArray() );
        }

        static DVResult fromVerifyOpaqueResult( int id, const VerificationResult & vr, const QByteArray & plaintext ) {
            const DVResult res = {
                VerifyOpaque,
                id,
                vr,
                DecryptionResult(),
                plaintext,
                0,
                QString()
            };
            return res;
        }
        static DVResult fromVerifyOpaqueResult( int id, const Error & err ) {
            return fromVerifyOpaqueResult( id, VerificationResult( err ), QByteArray() );
        }
        
        static DVResult fromVerifyDetachedResult( int id, const VerificationResult & vr ) {
            const DVResult res = {
                VerifyDetached,
                id,
                vr,
                DecryptionResult(),
                QByteArray(),
                0,
                QString()
            };
            return res;
        }
        static DVResult fromVerifyDetachedResult( int id, const Error & err ) {
            return fromVerifyDetachedResult( id, VerificationResult( err ) );
        }
        
    };

} // anon namespace 

class DecryptVerifyCommand::Private : public QObject {
    Q_OBJECT
    friend class ::Kleo::DecryptVerifyCommand;
    DecryptVerifyCommand * const q;
public:
    explicit Private( DecryptVerifyCommand * qq )
        : QObject(),
          q( qq ),
          wizard(),
          inputList(),
          m_unfinished( 0 ),
          m_statusSent( 0 ),
          m_error( 0U )
    {
        
    }

    std::vector<Input> buildInputList();
    static Input inputFromOperationWidget( const DecryptVerifyOperationWidget * w, const shared_ptr<QFile> & file, const QDir & outDir );

    void startJobs();

    void trySendingStatus( const char * tag, const QString & str );
    QString signatureToString( const Signature& sig, const Key & key ) const;

    void createWizard() {
        if ( wizard )
            return;
        wizard = q->applyWindowID( new DecryptVerifyWizard );
        //if ( requestedWindowTitle().isEmpty() )
            wizard->setWindowTitle( i18n("Decrypt/Verify Wizard") );
        //else
        //    wizard->setWindowTitle( i18n("Decrypt/Verify Wizard: %1", requestedWindowTitle() ) );
    }

    void showWizard() {
        if ( wizard ) {
            if ( !wizard->isVisible() )
                wizard->show();
            wizard->raise();
        }
    }

public Q_SLOTS:
    void finished( const QMap<int,DVResult> & results );
    void slotProgress( const QString & what, int current, int total );

public:
    void registerJob( int id, VerifyDetachedJob * job );
    void registerJob( int id, VerifyOpaqueJob * job );
    void registerJob( int id, DecryptJob * job );
    void registerJob( int id, DecryptVerifyJob * job );

    bool hasError() const { return m_error; }
    unsigned int error() const { return m_error; }
    
    void addResult( const DVResult & res );

private Q_SLOTS:
    void slotDialogClosed();

    void slotVerifyOpaqueResult( const GpgME::VerificationResult & result, const QByteArray & plainText ) {
        assert( m_senderToId.contains( sender() ) );
        const int id = m_senderToId[sender()];
        addResult( DVResult::fromVerifyOpaqueResult( id, result, plainText ) );
    }

    void slotVerifyDetachedResult( const GpgME::VerificationResult & result ) {
        assert( m_senderToId.contains( sender() ) );
        const int id = m_senderToId[sender()];
        addResult( DVResult::fromVerifyDetachedResult( id, result ) );
    }

    void slotDecryptResult( const GpgME::DecryptionResult & result, const QByteArray & plainText ) {
        assert( m_senderToId.contains( sender() ) );
        const int id = m_senderToId[sender()];
        addResult( DVResult::fromDecryptResult( id, result, plainText ) );
    }
    void slotDecryptVerifyResult( const GpgME::DecryptionResult & dr, const GpgME::VerificationResult & vr, const QByteArray & plainText ) {
        assert( m_senderToId.contains( sender() ) );
        const int id = m_senderToId[sender()];
        addResult( DVResult::fromDecryptVerifyResult( id, dr, vr, plainText ) );
    }

private:
    QPointer<DecryptVerifyWizard> wizard;
    std::vector<Input> inputList;
    QMap<int, DVResult> m_results;
    QHash<QObject*, int> m_senderToId;
    int m_unfinished;
    int m_statusSent;
    unsigned int m_error;
};


void DecryptVerifyCommand::Private::registerJob( int id, VerifyDetachedJob* job )
{
    connect( job, SIGNAL(result(GpgME::VerificationResult)),
             this, SLOT(slotVerifyDetachedResult(GpgME::VerificationResult)) );
    m_senderToId[job] = id;
    ++m_unfinished;
}

void DecryptVerifyCommand::Private::registerJob( int id, VerifyOpaqueJob* job )
{
    connect( job, SIGNAL(result(GpgME::VerificationResult,QByteArray)),
             this, SLOT(slotVerifyOpaqueResult(GpgME::VerificationResult,QByteArray)) );
    m_senderToId[job] = id;
    ++m_unfinished;
}

void DecryptVerifyCommand::Private::registerJob( int id, DecryptJob * job )
{
    connect( job, SIGNAL(result(GpgME::DecryptionResult,QByteArray)),
             this, SLOT(slotDecryptResult(GpgME::DecryptionResult,QByteArray)) );
    m_senderToId[job] = id;
    ++m_unfinished;
}

void DecryptVerifyCommand::Private::registerJob( int id, DecryptVerifyJob * job )
{
    connect( job, SIGNAL(result(GpgME::DecryptionResult,GpgME::VerificationResult,QByteArray)),
             this, SLOT(slotDecryptVerifyResult(GpgME::DecryptionResult,GpgME::VerificationResult,QByteArray)) );
    m_senderToId[job] = id;
    ++m_unfinished;
}

void DecryptVerifyCommand::Private::addResult( const DVResult & res )
{
    m_results[res.id] = res;

    // send status for all results received so far, but in order of id
    while ( m_results.contains( m_statusSent ) ) {
       DVResult result = m_results[m_statusSent];
       const VerificationResult & vResult = result.verificationResult;
       const DecryptionResult   & dResult = result.decryptionResult;

       if ( dResult.error().code() )
           result.error = dResult.error();
       else if ( vResult.error().code() )
           result.error = vResult.error();

       if ( !result.error )
           try {

               try {
                   inputList[res.id].finalizeOutput();
               } catch ( const assuan_exception & e ) {
                   // record these errors, but ignore them:
                   result.error = e.error_code();
                   result.errorString = e.message();
               }

               if ( !vResult.isNull() ) {
                   const std::vector<Signature> sigs = vResult.signatures();
                   assuan_assert( !sigs.empty() );
                   const std::vector<Key> signers = KeyCache::instance()->findSigners( vResult );
                   Q_FOREACH ( const Signature & sig, sigs ) {
                       const QString s = signatureToString( sig, keyForSignature( sig, signers ) );
                       trySendingStatus( "SIGSTATUS", s );
                   }
               }
           } catch ( const assuan_exception& e ) {
               result.error = e.error_code();
               result.errorString = e.message();
               // FIXME ask to continue or cancel
           }

       m_results[result.id] = result;

       wizard->resultWidget( m_statusSent )->setResult( result.decryptionResult, result.verificationResult );
       m_statusSent++;

       if ( result.error && !m_error )
           m_error = result.error;
    }

    --m_unfinished;
    assert( m_unfinished >= 0 );
    if ( m_unfinished == 0 )
        finished( m_results );
}

static bool obtainOverwritePermission( const QString & fileName ) {
    return KMessageBox::questionYesNo( 0, i18n("The file <b>%1</b> already exists.\n"
                                               "Overwrite?", fileName ),
                                       i18n("Overwrite Existing File?") ) == KMessageBox::Yes ;
}

void Input::finalizeOutput() {
    if ( !output.tmp )
        return;
    if ( output.tmp->isOpen() )
        output.tmp->close();

#warning implement
    // 1. if output.fileName present -> must be this

    // 2. is suggestedFileName not present, get one heuristically

    // 2a. if not silent, ask user for confirmation

    // 3. if target exists:

    // if !silent, ask, otherwise, append .n

    output.tmp->setAutoRemove( false );
    bool overwrite = false;
    static const int maxtries = 5;
    for ( int i = 0 ; i < maxtries ; ++i ) {
        if ( QFile::rename( output.tmp->fileName(), output.fileName ) )
            return;
        else if ( overwrite || ( overwrite = obtainOverwritePermission( output.fileName ) ) )
            QFile::remove( output.fileName );
        else
            throw;
    }
    output.tmp->setAutoRemove( true );
}

DecryptVerifyCommand::DecryptVerifyCommand()
    : AssuanCommandMixin<DecryptVerifyCommand>(), d( new Private( this ) )
{

}

DecryptVerifyCommand::~DecryptVerifyCommand() {}

std::vector<Input> DecryptVerifyCommand::Private::buildInputList()
{
    if ( !q->senders().empty() )
        throw assuan_exception( q->makeError( GPG_ERR_CONFLICT ),
                                i18n("Can't use SENDER") );

    if ( !q->recipients().empty() )
        throw assuan_exception( q->makeError( GPG_ERR_CONFLICT ),
                                i18n("Can't use RECIPIENT") );

    const unsigned int numInputs = q->numBulkInputDevices();
    const unsigned int numMessages = q->numBulkMessageDevices();
    const unsigned int numOutputs  = q->numBulkOutputDevices();

    const unsigned int op = q->operation();
    const Mode mode = q->mode();
    const GpgME::Protocol proto = q->checkProtocol( mode );

    const unsigned int numFiles = q->numFiles();

    assuan_assert( op != 0 );

    std::vector<Input> inputs;

    if ( mode == EMail ) {

        if ( numFiles )
            throw assuan_exception( q->makeError( GPG_ERR_CONFLICT ), i18n("FILES present") );

        if ( !numInputs )
            throw assuan_exception( q->makeError( GPG_ERR_ASS_NO_INPUT ),
                                    i18n("At least one INPUT needs to be provided") );

        if ( numMessages )
            if ( numMessages != numInputs )
                throw assuan_exception( q->makeError( GPG_ERR_ASS_NO_INPUT ),  //TODO use better error code if possible
                                        i18n("INPUT/MESSAGE count mismatch") );
            else if ( op != VerifyImplied )
                throw assuan_exception( q->makeError( GPG_ERR_CONFLICT ),
                                        i18n("MESSAGE can only be given for detached signature verification") );

        if ( numOutputs )
            if ( numOutputs != numInputs )
                throw assuan_exception( q->makeError( GPG_ERR_ASS_NO_OUTPUT ), //TODO use better error code if possible
                                        i18n("INPUT/OUTPUT count mismatch") );
            else if ( numMessages )
                throw assuan_exception( q->makeError( GPG_ERR_CONFLICT ),
                                        i18n("Can't use OUTPUT and MESSAGE simultaneously") );

        assuan_assert( proto != UnknownProtocol );

        const CryptoBackend::Protocol * const backend = CryptoBackendFactory::instance()->protocol( proto );
        if ( !backend )
            throw assuan_exception( q->makeError( GPG_ERR_UNSUPPORTED_PROTOCOL ),
                                    proto == OpenPGP ? i18n("No backend support for OpenPGP") :
                                    proto == CMS     ? i18n("No backend support for S/MIME") : QString() );

        Type type;
        if ( numMessages )
            type = VerifyDetached;
        else if ( (op&DecryptMask) == DecryptOff )
            type = VerifyOpaque;
        else if ( (op&VerifyMask) == VerifyOff )
            type = Decrypt;
        else
            type = DecryptVerify;

        for ( unsigned int i = 0 ; i < numInputs ; ++i ) {

            Input input;
            input.type = type;

            input.input.io = q->bulkInputDevice( i );
            input.input.fileName = q->bulkInputDeviceFileName( i );
            assuan_assert( input.input.io );

            if ( type == VerifyDetached ) {
                input.signedData.io = q->bulkInputDevice( i );
                input.signedData.fileName = q->bulkInputDeviceFileName( i );
                assuan_assert( input.signedData.io );
            }

            if ( numOutputs ) {
                input.output.io = q->bulkOutputDevice( i );
                input.output.fileName = q->bulkOutputDeviceFileName( i );
                assuan_assert( input.output.io );
            }

            input.backend = backend;

            inputs.push_back( input );
        }

    } else {

        if ( numInputs )
            throw assuan_exception( q->makeError( GPG_ERR_CONFLICT ), i18n("INPUT present") );
        if ( numMessages )
            throw assuan_exception( q->makeError( GPG_ERR_CONFLICT ), i18n("MESSAGE present") );
        if ( numOutputs )
            throw assuan_exception( q->makeError( GPG_ERR_CONFLICT ), i18n("OUTPUT present") );

        createWizard();

        std::vector< shared_ptr<QFile> > files;
        unsigned int counter = 0;
        Q_FOREACH( const shared_ptr<QFile> & file, q->files() ) {

            assuan_assert( file );

            const QString fname = file->fileName();

            assuan_assert( !fname.isEmpty() );

            const unsigned int classification = classify( fname );

            if ( mayBeOpaqueSignature( classification ) || mayBeCipherText( classification ) ) {

                DecryptVerifyOperationWidget * const op = wizard->operationWidget( counter++ );
                assuan_assert( op != 0 );

                op->setMode( DecryptVerifyOperationWidget::DecryptVerifyOpaque );
                op->setInputFileName( fname );
                op->setSignedDataFileName( QString() );

                files.push_back( file );

            } else if ( mayBeDetachedSignature( classification ) ) {
                // heuristics say it's a detached signature

                DecryptVerifyOperationWidget * const op = wizard->operationWidget( counter++ );
                assuan_assert( op != 0 );

                op->setMode( DecryptVerifyOperationWidget::VerifyDetachedWithSignature );
                op->setInputFileName( fname );
                op->setSignedDataFileName( findSignedData( fname ) );

                files.push_back( file );

            } else {

                // probably the signed data file was selected:
                QStringList signatures = findSignatures( fname );
                if ( signatures.empty() )
                    signatures.push_back( QString() );

                Q_FOREACH( const QString s, signatures ) {
                    DecryptVerifyOperationWidget * op = wizard->operationWidget( counter++ );
                    assuan_assert( op != 0 );

                    op->setMode( DecryptVerifyOperationWidget::VerifyDetachedWithSignedData );
                    op->setInputFileName( s );
                    op->setSignedDataFileName( fname );

                    files.push_back( file );

                }
            }
        }

        assuan_assert( counter == files.size() );

        if ( !counter )
            throw assuan_exception( q->makeError( GPG_ERR_ASS_NO_INPUT ), i18n("No usable inputs found") );

        wizard->setOutputDirectory( q->heuristicBaseDirectory() );
        showWizard();
        if ( !wizard->waitForOperationSelection() )
            throw assuan_exception( q->makeError( GPG_ERR_CANCELED ), i18n("Confirmation dialog canceled") );

        const QFileInfo outDirInfo( wizard->outputDirectory() );
        assuan_assert( outDirInfo.isDir() );

        const QDir outDir( outDirInfo.absoluteFilePath() );
        assuan_assert( outDir.exists() );

        for ( unsigned int i = 0 ; i < counter ; ++i )
            try {
                inputs.push_back( inputFromOperationWidget( wizard->operationWidget( i ), files[i], outDir ) );
            } catch ( const GpgME::Exception & e ) {
                //wizard->addResult( DVResult::.... )
            }
        
    }

    return inputs;
}

// static
Input DecryptVerifyCommand::Private::inputFromOperationWidget( const DecryptVerifyOperationWidget * w, const shared_ptr<QFile> & file, const QDir & outDir) {

    assuan_assert( w );

    Input input;

    switch ( w->mode() ) {
    case DecryptVerifyOperationWidget::VerifyDetachedWithSignature:

        input.type = VerifyDetached;
        input.input = Input::fromOpenFile( file );
        input.signedData = Input::openExistingFile( w->signedDataFileName() );

        assuan_assert( file->fileName() == w->inputFileName() );

        break;

    case DecryptVerifyOperationWidget::VerifyDetachedWithSignedData:

        input.type = VerifyDetached;
        input.input = Input::openExistingFile( w->inputFileName() );
        input.signedData = Input::fromOpenFile( file );

        assuan_assert( file->fileName() == w->signedDataFileName() );

        break;

    case DecryptVerifyOperationWidget::DecryptVerifyOpaque:

        input.type = DecryptVerify;
        input.input = Input::fromOpenFile( file );
        input.output = Input::makeTemporaryOutput( outDir.absoluteFilePath( QFileInfo( file->fileName() ).fileName() ) );

        assuan_assert( file->fileName() == w->inputFileName() );

        break;
    }

    return input;
}

static QString summaryToString( const Signature::Summary summary )
{
    if ( summary & Signature::Green )
        return "GREEN";
    else if ( summary & Signature::Red )
        return "RED";
    else
        return "YELLOW";
}

static QString keyToString( const Key & key ) {

    assuan_assert( !key.isNull() );

    const QString email = Formatting::prettyEMail( key );
    const QString name = Formatting::prettyName( key );

    if ( name.isEmpty() )
        return email;
    else if ( email.isEmpty() )
        return name;
    else
        return QString::fromLatin1( "%1 <%2>" ).arg( name, email );
}

QString DecryptVerifyCommand::Private::signatureToString( const Signature & sig, const Key & key ) const
{
    if ( sig.isNull() )
        return QString();

    const bool red   = (sig.summary() & Signature::Red);
    const bool green = (sig.summary() & Signature::Green);
    const bool valid = (sig.summary() & Signature::Valid);

    if ( red )
        if ( key.isNull() )
            if ( const char * fpr = sig.fingerprint() )
                return "RED " + i18n("Bad signature by unknown key %1: %2", QString::fromLatin1( fpr ), QString::fromLocal8Bit( sig.status().asString() ) );
            else
                return "RED " + i18n("Bad signature by an unknown key: %1", QString::fromLocal8Bit( sig.status().asString() ) );
        else
            return "RED " + i18n("Bad signature by %1: %2", keyToString( key ), QString::fromLocal8Bit( sig.status().asString() ) );

    else if ( valid )
        if ( key.isNull() )
            if ( const char * fpr = sig.fingerprint() )
                return ( green ? "GREEN " : "YELLOW " ) + i18n("Good signature by unknown key %1.", QString::fromLatin1( fpr ) );
            else
                return ( green ? "GREEN " : "YELLOW " ) + i18n("Good signature by an unknown key.");
        else
            return ( green ? "GREEN " : "YELLOW " ) + i18n("Good signature by %1.", keyToString( key ) );

    else
        if ( key.isNull() )
            if ( const char * fpr = sig.fingerprint() )
                return "YELLOW " + i18n("Invalid signature by unknown key %1: %2", QString::fromLatin1( fpr ), QString::fromLocal8Bit( sig.status().asString() ) );
            else
                return "YELLOW " + i18n("Invalid signature by an unknown key: %1", QString::fromLocal8Bit( sig.status().asString() ) );
        else
            return "YELLOW " + i18n("Invalid signature by %1: %2", keyToString( key ), QString::fromLocal8Bit( sig.status().asString() ) );
}

void DecryptVerifyCommand::Private::trySendingStatus( const char * tag, const QString & str )
{
    // ### FIXME: make AssuanCommand::sendStatus() throw the exception
    if ( const int err = q->sendStatus( tag, str ) )
        throw assuan_exception( err, i18n("Problem writing out verification status.") );
}

void DecryptVerifyCommand::Private::slotDialogClosed()
{
    if ( hasError() )
        q->done( error() );
    else
        q->done();
}

static QStringList labels( const std::vector<Input> & inputList )
{
    QStringList labels;
    for ( unsigned int i = 0, end = inputList.size() ; i < end ; ++i ) {
        const Input & input = inputList[i];
        switch ( input.type ) {
        case Decrypt:
        case DecryptVerify:
            labels.push_back( input.input.fileName.isEmpty()
                              ? i18n( "Decrypting message #%1...", i )
                              : i18n( "Decrypting file %1...", input.input.fileName ) );
            break;
        case VerifyOpaque:
            labels.push_back( input.input.fileName.isEmpty()
                              ? i18n( "Verifying message #%1...", i )
                              : i18n( "Verifying file %1...", input.input.fileName ) );
            break;
        case VerifyDetached:
            labels.push_back( input.input.fileName.isEmpty()
                              ? i18n( "Verifying message #%1...", i )
                              : i18n( "Verifying signature %1...", input.input.fileName ) );
            break;
        }
    }
    return labels;
}

void DecryptVerifyCommand::Private::finished( const QMap<int,DVResult> & results )
{
    if ( q->hasOption("silent") ) //otherwise we'll be ending when the dialog closes
        slotDialogClosed();
}



void DecryptVerifyCommand::Private::slotProgress( const QString& what, int current, int total )
{
    // FIXME report progress, via sendStatus()
}

void DecryptVerifyCommand::Private::startJobs()
{
    assuan_assert( !inputList.empty() );

    int i = 0;
    Q_FOREACH ( const Input input, inputList ) {

        assuan_assert( input.backend );

        switch ( input.type ) {
        case Decrypt:
            try {
                DecryptJob * const job = input.backend->decryptJob();
                assuan_assert( job );
                registerJob( i, job );
                job->start( input.input.io, input.output.io );
            } catch ( const GpgME::Exception & e ) {
                addResult( DVResult::fromDecryptResult( i, e.error() ) );
            }
            break;
        case DecryptVerify:
            try {
                DecryptVerifyJob * const job = input.backend->decryptVerifyJob();
                assuan_assert( job );
                registerJob( i, job );
                job->start( input.input.io, input.output.io );
            } catch ( const GpgME::Exception & e ) {
                addResult( DVResult::fromDecryptVerifyResult( i, e.error() ) );
            }
            break;
        case VerifyOpaque:
            try {
                VerifyOpaqueJob * const job = input.backend->verifyOpaqueJob();
                assuan_assert( job );
                registerJob( i, job );
                job->start( input.input.io, input.output.io );
            } catch ( const GpgME::Exception & e ) {
                addResult( DVResult::fromVerifyOpaqueResult( i, e.error() ) );
            }
            break;
        case VerifyDetached:
            try {
                VerifyDetachedJob * const job = input.backend->verifyDetachedJob();
                assuan_assert( job );
                registerJob( i, job );
                job->start( input.input.io, input.signedData.io );
            } catch ( const GpgME::Exception & e ) {
                addResult( DVResult::fromVerifyDetachedResult( i, e.error() ) );
            }
            break;
        }
        ++i;
    }

}

int DecryptVerifyCommand::doStart()
{
    d->inputList = d->buildInputList();

    if ( d->inputList.empty() )
        throw assuan_exception( makeError( GPG_ERR_ASS_NO_INPUT ),
                                i18n("No useable inputs found") );

    try {
        if( !hasOption("silent") )
            ;//d->showWizard();
        //d->waitForOperationSelection();
        d->startJobs();
        return 0;
    } catch ( ... ) {
        //d->showWizard();
        throw;
    }
}

void DecryptVerifyCommand::doCanceled()
{
    delete d->wizard;
    d->wizard = 0;
}

//
//
// Details....
//
//

#include "decryptverifycommand.moc"
