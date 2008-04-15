/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/decryptverifycommand.cpp

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

#include <config-kleopatra.h>

#include "decryptverifycommand.h"

#include <crypto/decryptverifytask.h>

#include <crypto/gui/decryptverifywizard.h>
#include <crypto/gui/decryptverifyresultwidget.h>
#include <crypto/gui/decryptverifyoperationwidget.h>

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
#include <KWindowSystem>

#include <QFileDialog>
#include <QObject>
#include <QIODevice>
#include <QMap>
#include <QHash>
#include <QPointer>
#include <QTimer>

#include <boost/bind.hpp>

#include <cassert>
#include <algorithm>
#include <functional>

#include <errno.h>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace Kleo::Crypto::Gui;
using namespace GpgME;
using namespace boost;

class DecryptVerifyCommand::Private : public QObject {
    Q_OBJECT
    friend class ::Kleo::DecryptVerifyCommand;
    DecryptVerifyCommand * const q;
public:
    explicit Private( DecryptVerifyCommand * qq )
        : QObject(),
          q( qq ),
          wizard(),
          m_runnableTasks(),
          m_completedTasks(),
          m_runningTask()
    {

    }

    ~Private() {
    }

    std::vector< shared_ptr<DecryptVerifyTask> > buildTaskList();
    static shared_ptr<DecryptVerifyTask> taskFromOperationWidget( const DecryptVerifyOperationWidget * w, const shared_ptr<QFile> & file, const QDir & outDir );


    void connectTask( const shared_ptr<DecryptVerifyTask> & t, unsigned int idx );

    void ensureWizardCreated() {
        if ( wizard )
            return;
        wizard =  new DecryptVerifyWizard;
        q->applyWindowID( wizard );
        wizard->setAttribute( Qt::WA_DeleteOnClose );
        connect( wizard, SIGNAL(finished(int)), this, SLOT(slotDialogClosed()) );
        wizard->setWindowTitle( i18n( "Decrypt/Verify" ) );
    }

    void showWizard() {
        if ( !wizard ) {
            ensureWizardCreated();
            wizard->next();
        }
        if ( !wizard->isVisible() )
            wizard->show();
        wizard->raise();
#ifdef Q_WS_WIN
        KWindowSystem::forceActiveWindow( wizard->winId() );
#endif
    }

public Q_SLOTS:
    void slotProgress( const QString & what, int current, int total );

public:

    std::vector<shared_ptr<const DecryptVerifyResult> >::const_iterator firstErrorResult() const;
    bool hasError() const;
    unsigned int error() const;
    QString errorString() const;
    
    void addStartErrorResult( unsigned int id, const shared_ptr<DecryptVerifyResult> & res );
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

    void schedule();
    void slotTaskDone( const boost::shared_ptr<const Kleo::Crypto::DecryptVerifyResult>& result );

private:
    QPointer<DecryptVerifyWizard> wizard;
    std::vector< shared_ptr<DecryptVerifyTask> > m_runnableTasks, m_completedTasks;
    shared_ptr<DecryptVerifyTask> m_runningTask;
    std::vector<shared_ptr<const DecryptVerifyResult> > m_results;
};


DecryptVerifyCommand::DecryptVerifyCommand()
    : AssuanCommandMixin<DecryptVerifyCommand>(), d( new Private( this ) )
{

}

void DecryptVerifyCommand::Private::schedule()
{
    if ( !m_runningTask && !m_runnableTasks.empty() ) {
        const shared_ptr<DecryptVerifyTask> t = m_runnableTasks.back();
        m_runnableTasks.pop_back();
        t->start(); // ### FIXME: this might throw
        m_runningTask = t;
    }
    if ( !m_runningTask ) {
        kleo_assert( m_runnableTasks.empty() );
#if KDAB_PENDING // remove encrypted inputs here if wanted?
        if ( wizard->removeUnencryptedFile() && wizard->encryptionSelected() && !errorDetected )
            removeInputFiles();
#endif
        sendSigStatii();
    }
}

void DecryptVerifyCommand::Private::slotTaskDone( const shared_ptr<const DecryptVerifyResult>& result )
{
    assert( sender() );
    
    // We could just delete the tasks here, but we can't use
    // Qt::QueuedConnection here (we need sender()) and other slots
    // might not yet have executed. Therefore, we push completed tasks
    // into a burial container

    if ( sender() == m_runningTask.get() ) {
        m_completedTasks.push_back( m_runningTask );
        m_results.push_back( result );
        m_runningTask.reset();
    }

    QTimer::singleShot( 0, this, SLOT(schedule()) );
}

std::vector<shared_ptr<const DecryptVerifyResult> >::const_iterator DecryptVerifyCommand::Private::firstErrorResult() const
{
    return std::find_if( m_results.begin(), m_results.end(), bind( &DecryptVerifyResult::hasError, _1 ) );
}

bool DecryptVerifyCommand::Private::hasError() const
{
    return firstErrorResult() != m_results.end();
}

QString DecryptVerifyCommand::Private::errorString() const
{
    const std::vector<shared_ptr<const DecryptVerifyResult> >::const_iterator it = firstErrorResult();
    return it != m_results.end() ? (*it)->errorString() : QString();
}

unsigned int DecryptVerifyCommand::Private::error() const
{
    const std::vector<shared_ptr<const DecryptVerifyResult> >::const_iterator it = firstErrorResult();
    return it != m_results.end() ? (*it)->errorCode() : 0;
}

DecryptVerifyCommand::~DecryptVerifyCommand() {}

void DecryptVerifyCommand::Private::connectTask( const shared_ptr<DecryptVerifyTask> & t, unsigned int idx ) {
    connect( t.get(), SIGNAL(decryptVerifyResult(boost::shared_ptr<const Kleo::Crypto::DecryptVerifyResult>)),
             this, SLOT(slotTaskDone(boost::shared_ptr<const Kleo::Crypto::DecryptVerifyResult>)) );
    ensureWizardCreated();
    wizard->connectTask( t, idx );
}

int DecryptVerifyCommand::doStart() {

    d->m_runnableTasks = d->buildTaskList();

    if ( d->m_runnableTasks.empty() )
        throw Kleo::Exception( makeError( GPG_ERR_ASS_NO_INPUT ),
                               i18n("No usable inputs found") );


    d->ensureWizardCreated();
    uint i = d->m_results.size();
    Q_FOREACH( const shared_ptr<DecryptVerifyTask> & task, d->m_runnableTasks )
        d->connectTask( task, i++ );

    try {
        d->schedule();
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

std::vector< shared_ptr<DecryptVerifyTask> > DecryptVerifyCommand::Private::buildTaskList()
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

    std::vector< shared_ptr<DecryptVerifyTask> > tasks;

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

        DecryptVerifyTask::Type type;
        if ( numMessages )
            type = DecryptVerifyTask::VerifyDetached;
        else if ( (op&DecryptMask) == DecryptOff )
            type = DecryptVerifyTask::VerifyOpaque;
        else if ( (op&VerifyMask) == VerifyOff )
            type = DecryptVerifyTask::Decrypt;
        else
            type = DecryptVerifyTask::DecryptVerify;

        if ( type != DecryptVerifyTask::Decrypt && !q->hasOption("silent") ) {
            showWizard();
            wizard->next();
        }

        for ( unsigned int i = 0 ; i < numInputs ; ++i ) {

            shared_ptr<DecryptVerifyTask> task( new DecryptVerifyTask( type ) );

            task->setInput( q->inputs().at( i ) );

            if ( type == DecryptVerifyTask::VerifyDetached )
                task->setSignedData( q->messages().at( i ) );

            if ( numOutputs )
                task->setOutput( q->outputs().at( i ) );

            task->setBackend( backend );

            tasks.push_back( task );
        }

    } else {

        if ( numInputs )
            throw Kleo::Exception( q->makeError( GPG_ERR_CONFLICT ), i18n("INPUT present") );
        if ( numMessages )
            throw Kleo::Exception( q->makeError( GPG_ERR_CONFLICT ), i18n("MESSAGE present") );
        if ( numOutputs )
            throw Kleo::Exception( q->makeError( GPG_ERR_CONFLICT ), i18n("OUTPUT present") );

        ensureWizardCreated();

        std::vector< shared_ptr<QFile> > files;
        unsigned int counter = 0;
        Q_FOREACH( const shared_ptr<QFile> & file, q->files() ) {

            kleo_assert( file );

            const QString fname = file->fileName();

            kleo_assert( !fname.isEmpty() );

            const unsigned int classification = classify( fname );

            if ( mayBeOpaqueSignature( classification ) || mayBeCipherText( classification ) || mayBeDetachedSignature( classification ) ) {

                DecryptVerifyOperationWidget * const op = wizard->operationWidget( counter++ );
                kleo_assert( op != 0 );

                if ( mayBeOpaqueSignature( classification ) || mayBeCipherText( classification ) )
                    op->setMode( DecryptVerifyOperationWidget::DecryptVerifyOpaque );
                else
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

        int failed = 0;
        for ( unsigned int i = 0 ; i < counter ; ++i )
            try {
                tasks.push_back( taskFromOperationWidget( wizard->operationWidget( i ), files[i], outDir ) );
            } catch ( const GpgME::Exception & e ) {
                addStartErrorResult( failed++, DecryptVerifyResult::fromDecryptVerifyResult( e.error(), QString::fromLocal8Bit( e.what() ) ) );
            }
    }

    return tasks;
}

// static
shared_ptr<DecryptVerifyTask> DecryptVerifyCommand::Private::taskFromOperationWidget( const DecryptVerifyOperationWidget * w, const shared_ptr<QFile> & file, const QDir & outDir) {

    kleo_assert( w );

    shared_ptr<DecryptVerifyTask> task;

    switch ( w->mode() ) {
    case DecryptVerifyOperationWidget::VerifyDetachedWithSignature:

        task.reset( new DecryptVerifyTask( DecryptVerifyTask::VerifyDetached ) );
        task->setInput( Input::createFromFile( file ) );
        task->setSignedData( Input::createFromFile( w->signedDataFileName() ) );

        kleo_assert( file->fileName() == w->inputFileName() );

        break;

    case DecryptVerifyOperationWidget::VerifyDetachedWithSignedData:
        task.reset( new DecryptVerifyTask( DecryptVerifyTask::VerifyDetached ) );
        task->setInput( Input::createFromFile( w->inputFileName() ) );
        task->setSignedData( Input::createFromFile( file ) );

        kleo_assert( file->fileName() == w->signedDataFileName() );

        break;

    case DecryptVerifyOperationWidget::DecryptVerifyOpaque:

        task.reset( new DecryptVerifyTask( DecryptVerifyTask::DecryptVerify ) );
        task->setInput( Input::createFromFile( file ) );
        task->setOutput( Output::createFromFile( outDir.absoluteFilePath( outputFileName( QFileInfo( file->fileName() ).fileName() ) ), false ) );

        kleo_assert( file->fileName() == w->inputFileName() );

        break;
    }

    task->autodetectBackendFromInput();
    return task;
}

void DecryptVerifyCommand::Private::slotProgress( const QString& what, int current, int total )
{
    // ### FIXME report progress, via sendStatus()
}

#if KDAB_PENDING

struct ResultPresentAndOutputFinalized {
    typedef bool result_type;

    bool operator()( const shared_ptr<DecryptVerifyTask> & task ) const {
        return task->result && ( !task->output || task->output->isFinalized() );
    }
};

void DecryptVerifyCommand::Private::addResult( unsigned int id, const shared_ptr<DecryptVerifyResult> & res )
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
#endif // KDAB_PENDING

void DecryptVerifyCommand::Private::addStartErrorResult( unsigned int id, const shared_ptr<DecryptVerifyResult> & res )
{
    ensureWizardCreated();
    wizard->resultWidget( id )->setResult( res );
    m_results.push_back( res );
}

void DecryptVerifyCommand::Private::sendSigStatii() const {

    Q_FOREACH( const shared_ptr<const DecryptVerifyResult> & result, m_results ) {
        const VerificationResult vResult = result->verificationResult();
        try {
            const std::vector<Signature> sigs = vResult.signatures();
            const std::vector<Key> signers = KeyCache::instance()->findSigners( vResult );
            Q_FOREACH ( const Signature & sig, sigs ) {
                const QString s = DecryptVerifyResult::signatureToString( sig, DecryptVerifyResult::keyForSignature( sig, signers ) );
                const char * color = DecryptVerifyResult::summaryToString( sig.summary() );
                q->sendStatusEncoded( "SIGSTATUS",
                                      color + ( ' ' + hexencode( s.toUtf8().constData() ) ) );
            }
        } catch ( ... ) {}
    }
    finishCommand();
}


#include "decryptverifycommand.moc"
