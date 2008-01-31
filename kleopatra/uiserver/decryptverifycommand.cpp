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

#include <models/keycache.h>
#include <models/predicates.h>

#include <utils/input.h>
#include <utils/output.h>
#include <utils/detail_p.h>
#include <utils/hex.h>
#include <utils/classify.h>
#include <utils/formatting.h>
#include <utils/stl_util.h>
#include <utils/kleo_assert.h>
#include <utils/exception.h>

#include <kleo/verifyopaquejob.h>
#include <kleo/verifydetachedjob.h>
#include <kleo/decryptjob.h>
#include <kleo/decryptverifyjob.h>
#include <kleo/cryptobackendfactory.h>

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

    struct DVTask {
    private:
        DVTask( const DVTask & );
        DVTask & operator=( const DVTask & );
    public:

        DVTask() : type( VerifyDetached ), backend( 0 ) {}

        Type type;

        shared_ptr<Input> input, signedData;

        shared_ptr<Output> output;

        const CryptoBackend::Protocol* backend;

        shared_ptr<DVResult> result;

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
          taskList(),
          m_errorString(),
          m_error( 0U )
    {

    }

    ~Private() {
    }

    std::vector< shared_ptr<DVTask> > buildTaskList();
    static shared_ptr<DVTask> taskFromOperationWidget( const DecryptVerifyOperationWidget * w, const shared_ptr<QFile> & file, const QDir & outDir );

    void startTasks();

    static QString signatureToString( const Signature& sig, const Key & key );

    void createWizard() {
        if ( wizard )
            return;
        wizard =  new DecryptVerifyWizard;
        q->applyWindowID( wizard );
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
    std::vector< shared_ptr<DVTask> > taskList;
    QHash<QObject*, unsigned int> m_senderToId;
    QString m_errorString;
    unsigned int m_error;
};


DecryptVerifyCommand::DecryptVerifyCommand()
    : AssuanCommandMixin<DecryptVerifyCommand>(), d( new Private( this ) )
{

}

DecryptVerifyCommand::~DecryptVerifyCommand() {}

int DecryptVerifyCommand::doStart() {

    d->taskList = d->buildTaskList();

    if ( d->taskList.empty() )
        throw Kleo::Exception( makeError( GPG_ERR_ASS_NO_INPUT ),
                               i18n("No usable inputs found") );

    try {
        d->startTasks();
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

std::vector< shared_ptr<DVTask> > DecryptVerifyCommand::Private::buildTaskList()
{
    if ( !q->senders().empty() )
        throw Kleo::Exception( q->makeError( GPG_ERR_CONFLICT ),
                               i18n("Can't use SENDER") );

    if ( !q->recipients().empty() )
        throw Kleo::Exception( q->makeError( GPG_ERR_CONFLICT ),
                               i18n("Can't use RECIPIENT") );

    const unsigned int numInputs = q->inputs().size();
    const unsigned int numMessages = q->messages().size();
    const unsigned int numOutputs  = q->outputs().size();

    const unsigned int op = q->operation();
    const Mode mode = q->mode();
    const GpgME::Protocol proto = q->checkProtocol( mode );

    const unsigned int numFiles = q->numFiles();

    kleo_assert( op != 0 );

    std::vector< shared_ptr<DVTask> > tasks;

    if ( mode == EMail ) {

        if ( numFiles )
            throw Kleo::Exception( q->makeError( GPG_ERR_CONFLICT ), i18n("FILES present") );

        if ( !numInputs )
            throw Kleo::Exception( q->makeError( GPG_ERR_ASS_NO_INPUT ),
                                   i18n("At least one INPUT needs to be provided") );

        if ( numMessages )
            if ( numMessages != numInputs )
                throw Kleo::Exception( q->makeError( GPG_ERR_ASS_NO_INPUT ),  //TODO use better error code if possible
                                       i18n("INPUT/MESSAGE count mismatch") );
            else if ( op != VerifyImplied )
                throw Kleo::Exception( q->makeError( GPG_ERR_CONFLICT ),
                                       i18n("MESSAGE can only be given for detached signature verification") );

        if ( numOutputs )
            if ( numOutputs != numInputs )
                throw Kleo::Exception( q->makeError( GPG_ERR_ASS_NO_OUTPUT ), //TODO use better error code if possible
                                       i18n("INPUT/OUTPUT count mismatch") );
            else if ( numMessages )
                throw Kleo::Exception( q->makeError( GPG_ERR_CONFLICT ),
                                       i18n("Can't use OUTPUT and MESSAGE simultaneously") );

        kleo_assert( proto != UnknownProtocol );

        const CryptoBackend::Protocol * const backend = CryptoBackendFactory::instance()->protocol( proto );
        if ( !backend )
            throw Kleo::Exception( q->makeError( GPG_ERR_UNSUPPORTED_PROTOCOL ),
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

        if ( type != Decrypt && !q->hasOption("silent") ) {
            showWizard();
            wizard->next();
        }

        for ( unsigned int i = 0 ; i < numInputs ; ++i ) {

            shared_ptr<DVTask> task( new DVTask );
            task->type = type;

            task->input = q->inputs().at( i );
            kleo_assert( task->input->ioDevice() );

            if ( type == VerifyDetached ) {
                task->signedData = q->messages().at( i );
                kleo_assert( task->signedData->ioDevice() );
            }

            if ( numOutputs ) {
                task->output = q->outputs().at( i );
                kleo_assert( task->output->ioDevice() );
            }

            task->backend = backend;

            tasks.push_back( task );
        }

    } else {

        if ( numInputs )
            throw Kleo::Exception( q->makeError( GPG_ERR_CONFLICT ), i18n("INPUT present") );
        if ( numMessages )
            throw Kleo::Exception( q->makeError( GPG_ERR_CONFLICT ), i18n("MESSAGE present") );
        if ( numOutputs )
            throw Kleo::Exception( q->makeError( GPG_ERR_CONFLICT ), i18n("OUTPUT present") );

        createWizard();

        std::vector< shared_ptr<QFile> > files;
        unsigned int counter = 0;
        Q_FOREACH( const shared_ptr<QFile> & file, q->files() ) {

            kleo_assert( file );

            const QString fname = file->fileName();

            kleo_assert( !fname.isEmpty() );

            const unsigned int classification = classify( fname );

            if ( mayBeOpaqueSignature( classification ) || mayBeCipherText( classification ) ) {

                DecryptVerifyOperationWidget * const op = wizard->operationWidget( counter++ );
                kleo_assert( op != 0 );

                op->setMode( DecryptVerifyOperationWidget::DecryptVerifyOpaque );
                op->setInputFileName( fname );
                op->setSignedDataFileName( QString() );

                files.push_back( file );

            } else if ( mayBeDetachedSignature( classification ) ) {
                // heuristics say it's a detached signature

                DecryptVerifyOperationWidget * const op = wizard->operationWidget( counter++ );
                kleo_assert( op != 0 );

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
                    kleo_assert( op != 0 );

                    op->setMode( DecryptVerifyOperationWidget::VerifyDetachedWithSignedData );
                    op->setInputFileName( s );
                    op->setSignedDataFileName( fname );

                    files.push_back( file );

                }
            }
        }

        kleo_assert( counter == files.size() );

        if ( !counter )
            throw Kleo::Exception( q->makeError( GPG_ERR_ASS_NO_INPUT ), i18n("No usable inputs found") );

        wizard->setOutputDirectory( q->heuristicBaseDirectory() );
        showWizard();
        if ( !wizard->waitForOperationSelection() )
            throw Kleo::Exception( q->makeError( GPG_ERR_CANCELED ), i18n("Confirmation dialog canceled") );

        const QFileInfo outDirInfo( wizard->outputDirectory() );
        kleo_assert( outDirInfo.isDir() );

        const QDir outDir( outDirInfo.absoluteFilePath() );
        kleo_assert( outDir.exists() );

        for ( unsigned int i = 0 ; i < counter ; ++i )
            try {
                tasks.push_back( taskFromOperationWidget( wizard->operationWidget( i ), files[i], outDir ) );
            } catch ( const GpgME::Exception & e ) {
                addResult( i, DVResult::fromDecryptVerifyResult( e.error() ) );
            }

    }

    return tasks;
}

static const CryptoBackend::Protocol * backendFor( const shared_ptr<Input> & input ) {
    return CryptoBackendFactory::instance()->protocol( findProtocol( input->classification() ) );
}

// static
shared_ptr<DVTask> DecryptVerifyCommand::Private::taskFromOperationWidget( const DecryptVerifyOperationWidget * w, const shared_ptr<QFile> & file, const QDir & outDir) {

    kleo_assert( w );

    shared_ptr<DVTask> task( new DVTask );

    switch ( w->mode() ) {
    case DecryptVerifyOperationWidget::VerifyDetachedWithSignature:

        task->type = VerifyDetached;
        task->input = Input::createFromFile( file );
        task->signedData = Input::createFromFile( w->signedDataFileName() );

        kleo_assert( file->fileName() == w->inputFileName() );

        break;

    case DecryptVerifyOperationWidget::VerifyDetachedWithSignedData:

        task->type = VerifyDetached;
        task->input = Input::createFromFile( w->inputFileName() );
        task->signedData = Input::createFromFile( file );

        kleo_assert( file->fileName() == w->signedDataFileName() );

        break;

    case DecryptVerifyOperationWidget::DecryptVerifyOpaque:

        task->type = DecryptVerify;
        task->input = Input::createFromFile( file );
        task->output = Output::createFromFile( outDir.absoluteFilePath( outputFileName( QFileInfo( file->fileName() ).fileName() ) ), false );

        kleo_assert( file->fileName() == w->inputFileName() );

        break;
    }

    task->backend = backendFor( task->input );

    return task;
}



void DecryptVerifyCommand::Private::startTasks()
{
    kleo_assert( !taskList.empty() );

    unsigned int i = 0;
    Q_FOREACH ( shared_ptr<DVTask> task, taskList ) {

        kleo_assert( task->backend );

        QPointer<QObject> that = this;
        switch ( task->type ) {
        case Decrypt:
            try {
                DecryptJob * const job = task->backend->decryptJob();
                kleo_assert( job );
                registerJob( i, job );
                job->start( task->input->ioDevice(), task->output->ioDevice() );
            } catch ( const GpgME::Exception & e ) {
                addResult( i, DVResult::fromDecryptResult( e.error() ) );
            }
            break;
        case DecryptVerify:
            try {
                DecryptVerifyJob * const job = task->backend->decryptVerifyJob();
                kleo_assert( job );
                registerJob( i, job );
                job->start( task->input->ioDevice(), task->output->ioDevice() );
            } catch ( const GpgME::Exception & e ) {
                addResult( i, DVResult::fromDecryptVerifyResult( e.error() ) );
            }
            break;
        case VerifyOpaque:
            try {
                VerifyOpaqueJob * const job = task->backend->verifyOpaqueJob();
                kleo_assert( job );
                registerJob( i, job );
                job->start( task->input->ioDevice(), task->output->ioDevice() );
            } catch ( const GpgME::Exception & e ) {
                addResult( i, DVResult::fromVerifyOpaqueResult( e.error() ) );
            }
            break;
        case VerifyDetached:
            try {
                VerifyDetachedJob * const job = task->backend->verifyDetachedJob();
                kleo_assert( job );
                registerJob( i, job );
                job->start( task->input->ioDevice(), task->signedData->ioDevice() );
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

static const char * summaryToString( const Signature::Summary summary )
{
    if ( summary & Signature::Red )
        return "RED";
    if ( summary & Signature::Green )
        return "GREEN";
    return "YELLOW";
}

static QString keyToString( const Key & key ) {

    kleo_assert( !key.isNull() );

    const QString email = Formatting::prettyEMail( key );
    const QString name = Formatting::prettyName( key );

    if ( name.isEmpty() )
        return email;
    else if ( email.isEmpty() )
        return name;
    else
        return QString::fromLatin1( "%1 <%2>" ).arg( name, email );
}

QString DecryptVerifyCommand::Private::signatureToString( const Signature & sig, const Key & key )
{
    if ( sig.isNull() )
        return QString();

    const bool red   = (sig.summary() & Signature::Red);
    const bool valid = (sig.summary() & Signature::Valid);

    if ( red )
        if ( key.isNull() )
            if ( const char * fpr = sig.fingerprint() )
                return i18n("Bad signature by unknown key %1: %2", QString::fromLatin1( fpr ), QString::fromLocal8Bit( sig.status().asString() ) );
            else
                return i18n("Bad signature by an unknown key: %1", QString::fromLocal8Bit( sig.status().asString() ) );
        else
            return i18n("Bad signature by %1: %2", keyToString( key ), QString::fromLocal8Bit( sig.status().asString() ) );

    else if ( valid )
        if ( key.isNull() )
            if ( const char * fpr = sig.fingerprint() )
                return i18n("Good signature by unknown key %1.", QString::fromLatin1( fpr ) );
            else
                return i18n("Good signature by an unknown key.");
        else
            return i18n("Good signature by %1.", keyToString( key ) );

    else
        if ( key.isNull() )
            if ( const char * fpr = sig.fingerprint() )
                return i18n("Invalid signature by unknown key %1: %2", QString::fromLatin1( fpr ), QString::fromLocal8Bit( sig.status().asString() ) );
            else
                return i18n("Invalid signature by an unknown key: %1", QString::fromLocal8Bit( sig.status().asString() ) );
        else
            return i18n("Invalid signature by %1: %2", keyToString( key ), QString::fromLocal8Bit( sig.status().asString() ) );
}

static QStringList labels( const std::vector< shared_ptr<DVTask> > & taskList )
{
    QStringList labels;
    for ( unsigned int i = 0, end = taskList.size() ; i < end ; ++i ) {
        const shared_ptr<DVTask> & task = taskList[i];
        switch ( task->type ) {
        case Decrypt:
        case DecryptVerify:
            labels.push_back( i18n( "Decrypting: %1...", task->input->label() ) );
            break;
        case VerifyOpaque:
            labels.push_back( i18n( "Verifying: %1...", task->input->label() ) );
            break;
        case VerifyDetached:
            labels.push_back( i18n( "Verifying signature: %1...", task->input->label() ) );
            break;
        }
    }
    return labels;
}

void DecryptVerifyCommand::Private::slotProgress( const QString& what, int current, int total )
{
    // FIXME report progress, via sendStatus()
}

struct ResultPresentAndOutputFinalized {
    typedef bool result_type;

    bool operator()( const shared_ptr<DVTask> & task ) const {
        return task->result && ( !task->output || task->output->isFinalized() );
    }
};

void DecryptVerifyCommand::Private::addResult( unsigned int id, const DVResult & res )
{
    const bool taskExists = id < taskList.size();
    assert( !taskExists || !taskList[id]->result );

    const shared_ptr<DVResult> result( new DVResult( res ) );
    if ( taskExists )
    	taskList[id]->result = result;

    qDebug( "addResult: %d (%p)", id, result.get() );

    const VerificationResult & vResult = result->verificationResult;
    const DecryptionResult   & dResult = result->decryptionResult;

    if ( dResult.error().code() )
        result->error = dResult.error().encodedError();
    else if ( vResult.error().code() )
        result->error = vResult.error().encodedError();

    if ( !result->error && taskExists && taskList[id]->output )
        try {

            QPointer<QObject> that = this;
            taskList[id]->output->finalize( /*wizard*/ );
            if ( !that )
                return;

        } catch ( const Kleo::Exception & e ) {
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

    if ( kdtools::all( taskList.begin(), taskList.end(), ResultPresentAndOutputFinalized() ) )
        sendSigStatii();
}

void DecryptVerifyCommand::Private::sendSigStatii() const {

    Q_FOREACH( const shared_ptr<DVTask> & task, taskList ) {

        const shared_ptr<DVResult> result = task->result;
        if ( !result )
            continue;

        const VerificationResult & vResult = result->verificationResult;

        try {
            const std::vector<Signature> sigs = vResult.signatures();
            const std::vector<Key> signers = PublicKeyCache::instance()->findSigners( vResult );
            Q_FOREACH ( const Signature & sig, sigs ) {
                const QString s = signatureToString( sig, keyForSignature( sig, signers ) );
                const char * color = summaryToString( sig.summary() );
                q->sendStatusEncoded( "SIGSTATUS",
                                      color + ( ' ' + hexencode( s.toUtf8().constData() ) ) );
            }
        } catch ( ... ) {}

    }

    finishCommand();
}


#include "decryptverifycommand.moc"
