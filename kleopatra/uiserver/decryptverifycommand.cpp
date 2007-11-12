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
#include <utils/stl_util.h>

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

#include <boost/bind.hpp>

#include <cassert>
#include <algorithm>
#include <functional>

#include <errno.h>

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

    class TemporaryFile : public QTemporaryFile {
    public:
        explicit TemporaryFile() : QTemporaryFile() {}
        explicit TemporaryFile( const QString & templateName ) : QTemporaryFile( templateName ) {}
        explicit TemporaryFile( QObject * parent ) : QTemporaryFile( parent ) {}
        explicit TemporaryFile( const QString & templateName, QObject * parent ) : QTemporaryFile( templateName, parent ) {}

        /* reimp */ void close() {
            if ( isOpen() )
                m_oldFileName = fileName();
            QTemporaryFile::close();
        }

        QString oldFileName() const { return m_oldFileName; }

    private:
        QString m_oldFileName;
    };

    enum Type {
        Decrypt,
        DecryptVerify,
        VerifyOpaque,
        VerifyDetached
    } type;

    struct DVResult {

        Type type;
        VerificationResult verificationResult;
        DecryptionResult decryptionResult;
        QByteArray stuff;
        int error;
        QString errorString;

        static DVResult fromDecryptResult( const DecryptionResult & dr, const QByteArray & plaintext ) {
            const DVResult res = {
                Decrypt,
                VerificationResult(),
                dr,
                plaintext,
                0,
                QString()
            };
            return res;
        }
        static DVResult fromDecryptResult( const Error & err ) {
            return fromDecryptResult( DecryptionResult( err ), QByteArray() );
        }

        static DVResult fromDecryptVerifyResult( const DecryptionResult & dr, const VerificationResult & vr, const QByteArray & plaintext ) {
            const DVResult res = {
                DecryptVerify,
                vr,
                dr,
                plaintext,
                0,
                QString()
            };
            return res;
        }
        static DVResult fromDecryptVerifyResult( const Error & err ) {
            return fromDecryptVerifyResult( DecryptionResult( err ), VerificationResult(), QByteArray() );
        }

        static DVResult fromVerifyOpaqueResult( const VerificationResult & vr, const QByteArray & plaintext ) {
            const DVResult res = {
                VerifyOpaque,
                vr,
                DecryptionResult(),
                plaintext,
                0,
                QString()
            };
            return res;
        }
        static DVResult fromVerifyOpaqueResult( const Error & err ) {
            return fromVerifyOpaqueResult( VerificationResult( err ), QByteArray() );
        }
        
        static DVResult fromVerifyDetachedResult( const VerificationResult & vr ) {
            const DVResult res = {
                VerifyDetached,
                vr,
                DecryptionResult(),
                QByteArray(),
                0,
                QString()
            };
            return res;
        }
        static DVResult fromVerifyDetachedResult( const Error & err ) {
            return fromVerifyDetachedResult( VerificationResult( err ) );
        }
        
    };

    struct Input {
    private:
        Input( const Input & );
        Input & operator=( const Input & );
    public:

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
            shared_ptr<TemporaryFile> tmp;
            bool in_progress;
        } output;

        const CryptoBackend::Protocol* backend;

        shared_ptr<DVResult> result;

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
            const shared_ptr<TemporaryFile> tmp( new TemporaryFile( fname ) );
            if ( !tmp->open() )
                throw assuan_exception( gpg_error( GPG_ERR_ASS_NO_OUTPUT ),
                                        i18n("Can't open temporary file \"%1\": %2", tmp->fileName(), tmp->errorString() ) );
            const _Output output = {
                tmp, fname, tmp, false,
            };
            return output;
        }

        void finalizeOutput( QWidget * parent );

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
          m_statusSent( 0U ),
          m_errorString(),
          m_error( 0U )
    {
        
    }

    ~Private() {
    }

    std::vector< shared_ptr<Input> > buildInputList();
    static shared_ptr<Input> inputFromOperationWidget( const DecryptVerifyOperationWidget * w, const shared_ptr<QFile> & file, const QDir & outDir );

    void startJobs();

    void trySendingStatus( const char * tag, const QString & str ) const {
        // ### FIXME: make AssuanCommand::sendStatus() throw the exception
        if ( const int err = q->sendStatus( tag, str ) )
            throw assuan_exception( err, i18n("Problem writing out verification status.") );
    }

    QString signatureToString( const Signature& sig, const Key & key ) const;

    void createWizard() {
        if ( wizard )
            return;
        wizard = q->applyWindowID( new DecryptVerifyWizard );
        wizard->setAttribute( Qt::WA_DeleteOnClose );
        connect( wizard, SIGNAL(finished(int)), this, SLOT(slotDialogClosed()) );
        //if ( requestedWindowTitle().isEmpty() )
            wizard->setWindowTitle( i18n("Decrypt/Verify Wizard") );
        //else
        //    wizard->setWindowTitle( i18n("Decrypt/Verify Wizard: %1", requestedWindowTitle() ) );
    }

    void showWizard() {
        if ( !wizard ) {
            createWizard();
            wizard->next();
        }
        if ( !wizard->isVisible() )
            wizard->show();
        wizard->raise();
    }

public Q_SLOTS:
    void slotProgress( const QString & what, int current, int total );

public:
    void registerJob( int id, VerifyDetachedJob* job ) {
        connect( job, SIGNAL(result(GpgME::VerificationResult)),
                 this, SLOT(slotVerifyDetachedResult(GpgME::VerificationResult)) );
        m_senderToId[job] = id;
    }
    void registerJob( int id, VerifyOpaqueJob* job ) {
        connect( job, SIGNAL(result(GpgME::VerificationResult,QByteArray)),
                 this, SLOT(slotVerifyOpaqueResult(GpgME::VerificationResult,QByteArray)) );
        m_senderToId[job] = id;
    }
    void registerJob( int id, DecryptJob * job ) {
        connect( job, SIGNAL(result(GpgME::DecryptionResult,QByteArray)),
                 this, SLOT(slotDecryptResult(GpgME::DecryptionResult,QByteArray)) );
        m_senderToId[job] = id;
    }
    void registerJob( int id, DecryptVerifyJob * job ) {
        connect( job, SIGNAL(result(GpgME::DecryptionResult,GpgME::VerificationResult,QByteArray)),
                 this, SLOT(slotDecryptVerifyResult(GpgME::DecryptionResult,GpgME::VerificationResult,QByteArray)) );
        m_senderToId[job] = id;
    }

    bool hasError() const { return m_error; }
    unsigned int error() const { return m_error; }
    const QString & errorString() const { return m_errorString; }
    
    void addResult( unsigned int id, const DVResult & res );
    void sendSigStatii() const;

    void finishCommand() const {
        if ( hasError() )
            q->done( error(), errorString() );
        else
            q->done();
    }

private Q_SLOTS:
    void slotDialogClosed() {
        finishCommand();
    }

    void slotVerifyOpaqueResult( const GpgME::VerificationResult & result, const QByteArray & plainText ) {
        assert( m_senderToId.contains( sender() ) );
        const unsigned int id = m_senderToId[sender()];
        addResult( id, DVResult::fromVerifyOpaqueResult( result, plainText ) );
    }

    void slotVerifyDetachedResult( const GpgME::VerificationResult & result ) {
        assert( m_senderToId.contains( sender() ) );
        const unsigned int id = m_senderToId[sender()];
        addResult( id, DVResult::fromVerifyDetachedResult( result ) );
    }

    void slotDecryptResult( const GpgME::DecryptionResult & result, const QByteArray & plainText ) {
        assert( m_senderToId.contains( sender() ) );
        const unsigned int id = m_senderToId[sender()];
        addResult( id, DVResult::fromDecryptResult( result, plainText ) );
    }
    void slotDecryptVerifyResult( const GpgME::DecryptionResult & dr, const GpgME::VerificationResult & vr, const QByteArray & plainText ) {
        assert( m_senderToId.contains( sender() ) );
        const unsigned int id = m_senderToId[sender()];
        addResult( id, DVResult::fromDecryptVerifyResult( dr, vr, plainText ) );
    }

private:
    QPointer<DecryptVerifyWizard> wizard;
    std::vector< shared_ptr<Input> > inputList;
    QHash<QObject*, unsigned int> m_senderToId;
    unsigned int m_statusSent;
    QString m_errorString;
    unsigned int m_error;
};


DecryptVerifyCommand::DecryptVerifyCommand()
    : AssuanCommandMixin<DecryptVerifyCommand>(), d( new Private( this ) )
{

}

DecryptVerifyCommand::~DecryptVerifyCommand() {}

int DecryptVerifyCommand::doStart() {

    d->inputList = d->buildInputList();

    if ( d->inputList.empty() )
        throw assuan_exception( makeError( GPG_ERR_ASS_NO_INPUT ),
                                i18n("No usable inputs found") );

    try {
        d->startJobs();
        return 0;
    } catch ( ... ) {
        //d->showWizard();
        throw;
    }
}

void DecryptVerifyCommand::doCanceled() {
    // ### cancel all jobs?
    if ( d->wizard )
        d->wizard->close();
}

std::vector< shared_ptr<Input> > DecryptVerifyCommand::Private::buildInputList()
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

    std::vector< shared_ptr<Input> > inputs;

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

            shared_ptr<Input> input( new Input );
            input->type = type;

            input->input.io = q->bulkInputDevice( i );
            input->input.fileName = q->bulkInputDeviceFileName( i );
            assuan_assert( input->input.io );

            if ( type == VerifyDetached ) {
                input->signedData.io = q->bulkInputDevice( i );
                input->signedData.fileName = q->bulkInputDeviceFileName( i );
                assuan_assert( input->signedData.io );
            }

            if ( numOutputs ) {
                input->output.io = q->bulkOutputDevice( i );
                input->output.fileName = q->bulkOutputDeviceFileName( i );
                assuan_assert( input->output.io );
            }

            input->backend = backend;

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
                //addResult( DVResult::fromDecryptVerifyResult( e.error(), e.what(), i ) );
            }
        
    }

    return inputs;
}

static const CryptoBackend::Protocol * backendFor( const QString & str ) {
    return CryptoBackendFactory::instance()->protocol( findProtocol( str ) );
}

// static
shared_ptr<Input> DecryptVerifyCommand::Private::inputFromOperationWidget( const DecryptVerifyOperationWidget * w, const shared_ptr<QFile> & file, const QDir & outDir) {

    assuan_assert( w );

    shared_ptr<Input> input( new Input );

    switch ( w->mode() ) {
    case DecryptVerifyOperationWidget::VerifyDetachedWithSignature:

        input->type = VerifyDetached;
        input->input = Input::fromOpenFile( file );
        input->signedData = Input::openExistingFile( w->signedDataFileName() );

        assuan_assert( file->fileName() == w->inputFileName() );

        break;

    case DecryptVerifyOperationWidget::VerifyDetachedWithSignedData:

        input->type = VerifyDetached;
        input->input = Input::openExistingFile( w->inputFileName() );
        input->signedData = Input::fromOpenFile( file );

        assuan_assert( file->fileName() == w->signedDataFileName() );

        break;

    case DecryptVerifyOperationWidget::DecryptVerifyOpaque:

        input->type = DecryptVerify;
        input->input = Input::fromOpenFile( file );
        input->output = Input::makeTemporaryOutput( outDir.absoluteFilePath( outputFileName( QFileInfo( file->fileName() ).fileName() ) ) );

        assuan_assert( file->fileName() == w->inputFileName() );

        break;
    }

    input->backend = backendFor( input->input.fileName );

    return input;
}



void DecryptVerifyCommand::Private::startJobs()
{
    assuan_assert( !inputList.empty() );

    unsigned int i = 0;
    Q_FOREACH ( shared_ptr<Input> input, inputList ) {

        assuan_assert( input->backend );

        QPointer<QObject> that = this;
        switch ( input->type ) {
        case Decrypt:
            try {
                DecryptJob * const job = input->backend->decryptJob();
                assuan_assert( job );
                registerJob( i, job );
                job->start( input->input.io, input->output.io );
            } catch ( const GpgME::Exception & e ) {
                addResult( i, DVResult::fromDecryptResult( e.error() ) );
            }
            break;
        case DecryptVerify:
            try {
                DecryptVerifyJob * const job = input->backend->decryptVerifyJob();
                assuan_assert( job );
                registerJob( i, job );
                job->start( input->input.io, input->output.io );
            } catch ( const GpgME::Exception & e ) {
                addResult( i, DVResult::fromDecryptVerifyResult( e.error() ) );
            }
            break;
        case VerifyOpaque:
            try {
                VerifyOpaqueJob * const job = input->backend->verifyOpaqueJob();
                assuan_assert( job );
                registerJob( i, job );
                job->start( input->input.io, input->output.io );
            } catch ( const GpgME::Exception & e ) {
                addResult( i, DVResult::fromVerifyOpaqueResult( e.error() ) );
            }
            break;
        case VerifyDetached:
            try {
                VerifyDetachedJob * const job = input->backend->verifyDetachedJob();
                assuan_assert( job );
                registerJob( i, job );
                job->start( input->input.io, input->signedData.io );
            } catch ( const GpgME::Exception & e ) {
                addResult( i, DVResult::fromVerifyDetachedResult( e.error() ) );
            }
            break;
        }
        if ( !that )
            return;
        ++i;
    }

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

static QStringList labels( const std::vector< shared_ptr<Input> > & inputList )
{
    QStringList labels;
    for ( unsigned int i = 0, end = inputList.size() ; i < end ; ++i ) {
        const shared_ptr<Input> & input = inputList[i];
        switch ( input->type ) {
        case Decrypt:
        case DecryptVerify:
            labels.push_back( input->input.fileName.isEmpty()
                              ? i18n( "Decrypting message #%1...", i )
                              : i18n( "Decrypting file %1...", input->input.fileName ) );
            break;
        case VerifyOpaque:
            labels.push_back( input->input.fileName.isEmpty()
                              ? i18n( "Verifying message #%1...", i )
                              : i18n( "Verifying file %1...", input->input.fileName ) );
            break;
        case VerifyDetached:
            labels.push_back( input->input.fileName.isEmpty()
                              ? i18n( "Verifying message #%1...", i )
                              : i18n( "Verifying signature %1...", input->input.fileName ) );
            break;
        }
    }
    return labels;
}

void DecryptVerifyCommand::Private::slotProgress( const QString& what, int current, int total )
{
    // FIXME report progress, via sendStatus()
}

void DecryptVerifyCommand::Private::addResult( unsigned int id, const DVResult & res )
{
    assert( !inputList[id]->result );

    const shared_ptr<DVResult> result( new DVResult( res ) );
    inputList[id]->result = result;

    qDebug( "addResult: %d (%p)", id, result.get() );

    const VerificationResult & vResult = result->verificationResult;
    const DecryptionResult   & dResult = result->decryptionResult;

    if ( dResult.error().code() )
        result->error = dResult.error();
    else if ( vResult.error().code() )
        result->error = vResult.error();

    if ( !result->error )
        try {

            QPointer<QObject> that = this;
            inputList[id]->finalizeOutput( wizard );
            if ( !that )
                return;

        } catch ( const assuan_exception& e ) {
            result->error = e.error_code();
            result->errorString = e.message();
            // FIXME ask to continue or cancel?
        }

    if ( wizard )
        wizard->resultWidget( id )->setResult( result->decryptionResult, result->verificationResult );

    if ( result->error && !m_error ) {
        m_error = result->error;
        m_errorString = result->errorString;
    }

    if ( kdtools::all( inputList.begin(), inputList.end(), bind( &Input::result, _1 ) ) &&
         !kdtools::any( inputList.begin(), inputList.end(),
                        bind( &Input::_Output::in_progress, bind( &Input::output, _1 ) ) ) )
        sendSigStatii();
}

void DecryptVerifyCommand::Private::sendSigStatii() const {

    Q_FOREACH( const shared_ptr<Input> & input, inputList ) {

        const shared_ptr<DVResult> result = input->result;
        if ( !result )
            continue;

        const VerificationResult & vResult = result->verificationResult;

        try {
            const std::vector<Signature> sigs = vResult.signatures();
            const std::vector<Key> signers = KeyCache::instance()->findSigners( vResult );
            Q_FOREACH ( const Signature & sig, sigs ) {
                const QString s = signatureToString( sig, keyForSignature( sig, signers ) );
                trySendingStatus( "SIGSTATUS", s );
            }
        } catch ( ... ) {}

    }
}

struct OutputGuard {
    Input::_Output & o;
    explicit OutputGuard( Input::_Output & o_ ) : o( o_ ) { o.in_progress = true; }
    ~OutputGuard() { o.in_progress = false; }
};

static bool obtainOverwritePermission( const QString & fileName, QWidget * parent ) {
    return KMessageBox::questionYesNo( parent, i18n("The file <b>%1</b> already exists.\n"
                                                    "Overwrite?", fileName ),
                                       i18n("Overwrite Existing File?"), KStandardGuiItem::overwrite(), KStandardGuiItem::cancel() ) == KMessageBox::Yes ;
}

void Input::finalizeOutput( QWidget * parent ) {
    if ( !output.tmp )
        return;
    if ( output.in_progress )
        return;

    const OutputGuard guard( output );

    if ( output.tmp->isOpen() )
        output.tmp->close();

    const QString tmpFileName = output.tmp->oldFileName();

    if ( QFile::rename( tmpFileName, output.fileName ) ) {
        output.tmp->setAutoRemove( false );
        return;
    }

    if ( !obtainOverwritePermission( output.fileName, parent ) )
        throw assuan_exception( gpg_error( GPG_ERR_CANCELED ),
                                i18n( "Overwriting declined" ) );

    if ( !QFile::remove( output.fileName ) )
        throw assuan_exception( errno ? gpg_error_from_errno( errno ) : gpg_error( GPG_ERR_EIO ),
                                i18n("Couldn't remove file \"%1\" for overwriting.", output.fileName ) );

    if ( QFile::rename( tmpFileName, output.fileName ) ) {
        output.tmp->setAutoRemove( false );
        return;
    }

    throw assuan_exception( errno ? gpg_error_from_errno( errno ) : gpg_error( GPG_ERR_EIO ),
                            i18n( "Couldn't rename file \"%1\" to \"%2\"",
                                  tmpFileName, output.fileName ) );
}

#include "decryptverifycommand.moc"
