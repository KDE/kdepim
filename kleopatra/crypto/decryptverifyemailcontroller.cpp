/* -*- mode: c++; c-basic-offset:4 -*-
    decryptverifyemailcontroller.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

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

#include "decryptverifyemailcontroller.h"
#include <crypto/gui/resultlistwidget.h>
#include <crypto/decryptverifytask.h>
#include <crypto/taskcollection.h>

#include <utils/classify.h>
#include <utils/gnupg-helper.h>
#include <utils/input.h>
#include <utils/output.h>
#include <utils/kleo_assert.h>

#include <kleo/cryptobackendfactory.h>

#include <KDebug>
#include <KLocalizedString>

#include <QPointer>
#include <QTimer>

#include <boost/shared_ptr.hpp>

#include <memory>
#include <vector>

using namespace boost;
using namespace GpgME;
using namespace Kleo;
using namespace Kleo::Crypto;
using namespace Kleo::Crypto::Gui;

class DecryptVerifyEMailController::Private {
    DecryptVerifyEMailController* const q;
public:

    explicit Private( DecryptVerifyEMailController* qq );

    void slotWizardCanceled();
    void slotTaskDone( const shared_ptr<const DecryptVerifyResult> & );
    void schedule();

    std::vector<shared_ptr<AbstractDecryptVerifyTask> > buildTasks();

    void ensureWizardCreated();
    void ensureWizardVisible();
    void reportError( int err, const QString & details ) {
        emit q->error( err, details );
    }

    void cancelAllTasks();

    std::vector<shared_ptr<Input> > m_inputs, m_signedDatas;
    std::vector<shared_ptr<Output> > m_outputs;
        
    QPointer<ResultListWidget> m_wizard;
    std::vector<shared_ptr<const DecryptVerifyResult> > m_results;
    std::vector<shared_ptr<AbstractDecryptVerifyTask> > m_runnableTasks, m_completedTasks;
    shared_ptr<AbstractDecryptVerifyTask> m_runningTask;
    bool m_silent;
    DecryptVerifyOperation m_operation;
    Protocol m_protocol;
    bool m_errorDetected;
    VerificationMode m_verificationMode;

};

DecryptVerifyEMailController::Private::Private( DecryptVerifyEMailController* qq )
    : q( qq ),
    m_silent( false ),
    m_operation( DecryptVerify ),
    m_protocol( UnknownProtocol ),
    m_errorDetected( false ),
    m_verificationMode( Detached )
{
    qRegisterMetaType<VerificationResult>();
}

void DecryptVerifyEMailController::Private::slotWizardCanceled()
{
    kDebug();
    reportError( gpg_error( GPG_ERR_CANCELED ), i18n("User canceled") );
}

void DecryptVerifyEMailController::Private::slotTaskDone(  const shared_ptr<const DecryptVerifyResult> & result )
{
    assert( q->sender() );
    
    // We could just delete the tasks here, but we can't use
    // Qt::QueuedConnection here (we need sender()) and other slots
    // might not yet have executed. Therefore, we push completed tasks
    // into a burial container

    if ( q->sender() == m_runningTask.get() ) {
        m_completedTasks.push_back( m_runningTask );
        m_results.push_back( result );
        m_runningTask.reset();
    }

    QTimer::singleShot( 0, q, SLOT(schedule()) );
}

void DecryptVerifyEMailController::Private::schedule()
{
    if ( !m_runningTask && !m_runnableTasks.empty() ) {
        const shared_ptr<AbstractDecryptVerifyTask> t = m_runnableTasks.back();
        m_runnableTasks.pop_back();
        t->start();
        m_runningTask = t;
    }
    if ( !m_runningTask ) {
        kleo_assert( m_runnableTasks.empty() );
        Q_FOREACH ( const shared_ptr<const DecryptVerifyResult> & i, m_results )
            emit q->verificationResult( i->verificationResult() );
        emit q->done();
    }
}

void DecryptVerifyEMailController::Private::ensureWizardCreated()
{
    if ( m_wizard )
        return;

    std::auto_ptr<ResultListWidget> w( new ResultListWidget );
    w->setWindowTitle( i18n( "Decrypt/Verify E-Mail" ) );
    w->setAttribute( Qt::WA_DeleteOnClose );
    w->setStandaloneMode( true );

#if 0
    connect( w.get(), SIGNAL(canceled()), q, SLOT(slotWizardCanceled()), Qt::QueuedConnection );
#endif
    m_wizard = w.release();

}

std::vector< shared_ptr<AbstractDecryptVerifyTask> > DecryptVerifyEMailController::Private::buildTasks()
{
    const uint numInputs = m_inputs.size();
    const uint numMessages = m_signedDatas.size();
    const uint numOutputs = m_outputs.size();
 
    // these are duplicated from DecryptVerifyCommandEMailBase::Private::checkForErrors with slightly modified error codes/messages
    if ( !numInputs )
        throw Kleo::Exception( makeGnuPGError( GPG_ERR_CONFLICT ),
                               i18n("At least one input needs to be provided") );

    if ( numMessages )
        if ( numMessages != numInputs )
            throw Kleo::Exception( makeGnuPGError( GPG_ERR_CONFLICT ),  //TODO use better error code if possible
                                   i18n("Signature/signed data count mismatch") );
        else if ( m_operation != Verify || m_verificationMode != Detached )
            throw Kleo::Exception( makeGnuPGError( GPG_ERR_CONFLICT ),
                                   i18n("Signed data can only be given for detached signature verification") );

    if ( numOutputs )
        if ( numOutputs != numInputs )
            throw Kleo::Exception( makeGnuPGError( GPG_ERR_CONFLICT ), //TODO use better error code if possible
                                   i18n("Input/Output count mismatch") );
        else if ( numMessages )
            throw Kleo::Exception( makeGnuPGError( GPG_ERR_CONFLICT ),
                                   i18n("Cannot use output and signed data simultaneously") );

    kleo_assert( m_protocol != UnknownProtocol );

    const CryptoBackend::Protocol * const backend = CryptoBackendFactory::instance()->protocol( m_protocol );
    if ( !backend )
        throw Kleo::Exception( makeGnuPGError( GPG_ERR_UNSUPPORTED_PROTOCOL ),
                               m_protocol == OpenPGP ? i18n("No backend support for OpenPGP") :
                               m_protocol == CMS     ? i18n("No backend support for S/MIME") : QString() );


    if ( m_operation != Decrypt && !m_silent )
        ensureWizardVisible();

    std::vector< shared_ptr<AbstractDecryptVerifyTask> > tasks;
    
    for ( unsigned int i = 0 ; i < numInputs ; ++i ) {
        shared_ptr<AbstractDecryptVerifyTask> task;
        switch ( m_operation ) {
            case Decrypt:
            {
                shared_ptr<DecryptTask> t( new DecryptTask );
                t->setInput( m_inputs.at( i ) );
                assert( numOutputs );
                t->setOutput( m_outputs.at( i ) );
                t->setProtocol( m_protocol );
                task = t;
            }
            break;
            case Verify:
            {
                if ( m_verificationMode == Detached ) {
                    shared_ptr<VerifyDetachedTask> t( new VerifyDetachedTask );
                    t->setInput( m_inputs.at( i ) );
                    t->setSignedData( m_signedDatas.at( i ) );
                    t->setProtocol( m_protocol );
                    task = t;
                } else {
                    shared_ptr<VerifyOpaqueTask> t( new VerifyOpaqueTask );
                    t->setInput( m_inputs.at( i ) );
                    if ( numOutputs )
                        t->setOutput( m_outputs.at( i ) );
                    t->setProtocol( m_protocol );
                    task = t;
                }
            }
            break;
            case DecryptVerify:
            {
                shared_ptr<DecryptVerifyTask> t( new DecryptVerifyTask );
                t->setInput( m_inputs.at( i ) );
                assert( numOutputs );
                t->setOutput( m_outputs.at( i ) );
                t->setProtocol( m_protocol );
                task = t;                
            }
        }

        assert( task );
        tasks.push_back( task );
    }
    
    return tasks;
}

void DecryptVerifyEMailController::Private::ensureWizardVisible()
{
    ensureWizardCreated();
    q->bringToForeground( m_wizard );
}

DecryptVerifyEMailController::DecryptVerifyEMailController( QObject* parent ) : Controller( parent ), d( new Private( this ) )
{
}

DecryptVerifyEMailController::DecryptVerifyEMailController( const shared_ptr<const ExecutionContext> & ctx, QObject* parent ) : Controller( ctx, parent ), d( new Private( this ) )
{
}

DecryptVerifyEMailController::~DecryptVerifyEMailController() { kDebug(); }

void DecryptVerifyEMailController::start()
{
    d->m_runnableTasks = d->buildTasks();
    int i = 0;

    shared_ptr<TaskCollection> coll( new TaskCollection );
    std::vector<shared_ptr<Task> > tsks;
    Q_FOREACH( const shared_ptr<Task> & i, d->m_runnableTasks ) { 
        connect( i.get(), SIGNAL(decryptVerifyResult(boost::shared_ptr<const Kleo::Crypto::DecryptVerifyResult>)),
                 this, SLOT(slotTaskDone(boost::shared_ptr<const Kleo::Crypto::DecryptVerifyResult>)) );
        tsks.push_back( i );
    }
    coll->setTasks( tsks );
    d->ensureWizardCreated();
    d->m_wizard->setTaskCollection( coll );

    d->ensureWizardVisible();
    QTimer::singleShot( 0, this, SLOT(schedule()) );
}

void DecryptVerifyEMailController::setInputs( const std::vector<shared_ptr<Input> > & inputs )
{
    d->m_inputs = inputs;
}

void DecryptVerifyEMailController::setSignedData( const std::vector<shared_ptr<Input> > & data )
{
    d->m_signedDatas = data;
}

void DecryptVerifyEMailController::setOutputs( const std::vector<shared_ptr<Output> > & outputs )
{
    d->m_outputs = outputs;
}

void DecryptVerifyEMailController::setWizardShown( bool shown )
{
    d->m_silent = !shown;
    if ( d->m_wizard )
        d->m_wizard->setVisible( shown );
}

void DecryptVerifyEMailController::setOperation( DecryptVerifyOperation operation )
{
    d->m_operation = operation;
}

void DecryptVerifyEMailController::setVerificationMode( VerificationMode vm )
{
    d->m_verificationMode = vm;
}

void DecryptVerifyEMailController::setProtocol( Protocol prot )
{
    d->m_protocol = prot;
}

void DecryptVerifyEMailController::cancel()
{
    kDebug();
    try {
        d->m_errorDetected = true;
        if ( d->m_wizard )
            d->m_wizard->close();
        d->cancelAllTasks();
    } catch ( const std::exception & e ) {
        qDebug( "Caught exception: %s", e.what() );
    }
}

void DecryptVerifyEMailController::Private::cancelAllTasks() {

    // we just kill all runnable tasks - this will not result in
    // signal emissions.
    m_runnableTasks.clear();

    // a cancel() will result in a call to 
    if ( m_runningTask )
        m_runningTask->cancel();
}

#include "decryptverifyemailcontroller.moc"
