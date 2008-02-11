/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/signemailcontroller.cpp

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

#include "signemailcontroller.h"

#include "assuancommand.h"

#include <crypto/gui/signemailwizard.h>

#include <crypto/signemailtask.h>
#include <crypto/certificateresolver.h>

#include <utils/input.h>
#include <utils/output.h>
#include <utils/stl_util.h>
#include <utils/kleo_assert.h>

#include <kmime/kmime_header_parsing.h>

#include <KLocale>

#include <QPointer>
#include <QTimer>

#include <boost/bind.hpp>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace Kleo::Crypto::Gui;
using namespace boost;
using namespace GpgME;
using namespace KMime::Types;

class SignEMailController::Private {
    friend class ::Kleo::SignEMailController;
    SignEMailController * const q;
public:
    explicit Private( SignEMailController * qq );

private:
    void slotWizardSignersResolved();
    void slotWizardCanceled(); // ### extract to base
    void slotTaskDone();       // ### extract to base

private:
    void ensureWizardCreated(); // ### extract to base
    void ensureWizardVisible(); // ### extract to base
    void cancelAllJobs();       // ### extract to base

    void schedule();            // ### extract to base
    shared_ptr<SignEMailTask> takeRunnable( GpgME::Protocol proto ); // ### extract to base
    void connectTask( const shared_ptr<Task> & task, unsigned int idx ); // ### extract to base

private:
    std::vector< shared_ptr<SignEMailTask> > runnable, completed; // ### extract to base
    shared_ptr<SignEMailTask> cms, openpgp; // ### extract to base
    QPointer<SignEncryptWizard> wizard; // ### extract to base
    Protocol protocol;                  // ### extract to base
    bool detached : 1;
};

SignEMailController::Private::Private( SignEMailController * qq )
    : q( qq ),
      runnable(),
      cms(),
      openpgp(),
      wizard(),
      protocol( UnknownProtocol ),
      detached( false )
{

}

SignEMailController::SignEMailController( const boost::shared_ptr<AssuanCommand> & cmd, QObject * p )
    : Controller( cmd, p ), d( new Private( this ) )
{

}

SignEMailController::~SignEMailController() {
    /// ### extract to base
    if ( d->wizard && !d->wizard->isVisible() )
        delete d->wizard;
        //d->wizard->close(); ### ?
}

// ### extract to base
void SignEMailController::setProtocol( Protocol proto ) {
    kleo_assert( d->protocol == UnknownProtocol ||
                 d->protocol == proto );
    d->protocol = proto;
    d->ensureWizardCreated();
    d->wizard->setPresetProtocol( proto );
}

Protocol SignEMailController::protocol() const {
    return d->protocol;
}

void SignEMailController::startResolveSigners( const std::vector<Mailbox> & signers ) {
    const std::vector< std::vector<Key> > keys = CertificateResolver::resolveSigners( signers, d->protocol );

    if ( !signers.empty() )
        kleo_assert( keys.size() == static_cast<size_t>( signers.size() ) );

    d->ensureWizardCreated();

    d->wizard->setSignersAndCandidates( signers, keys );
  
    d->ensureWizardVisible();
}

void SignEMailController::setDetachedSignature( bool detached ) {
    kleo_assert( !d->openpgp );
    kleo_assert( !d->cms );
    kleo_assert( d->completed.empty() );
    kleo_assert( d->runnable.empty() );

    d->detached = detached;
}

void SignEMailController::Private::slotWizardSignersResolved() {
    emit q->signersResolved();
}

// ### extract to base
void SignEMailController::Private::slotWizardCanceled() {
    emit q->error( gpg_error( GPG_ERR_CANCELED ), i18n("User cancel") );
}

// ### extract to base
void SignEMailController::importIO() {
    const shared_ptr<AssuanCommand> cmd = command().lock();
    kleo_assert( cmd );

    const std::vector< shared_ptr<Input> > & inputs = cmd->inputs();
    kleo_assert( !inputs.empty() );

    const std::vector< shared_ptr<Output> > & outputs = cmd->outputs();
    kleo_assert( !outputs.empty() );

    std::vector< shared_ptr<SignEMailTask> > tasks;
    tasks.reserve( inputs.size() );

    d->ensureWizardCreated();

    const std::vector<Key> keys = d->wizard->resolvedSigners();
    kleo_assert( !keys.empty() );

    for ( unsigned int i = 0, end = inputs.size() ; i < end ; ++i ) {

        const shared_ptr<SignEMailTask> task( new SignEMailTask );
        task->setInput( inputs[i] );
        task->setOutput( outputs[i] );
        task->setSigners( keys );
        task->setDetachedSignature( d->detached );

        tasks.push_back( task );
    }

    d->runnable.swap( tasks );
}

// ### extract to base
void SignEMailController::start() {
    int i = 0;
    Q_FOREACH( const shared_ptr<Task> task, d->runnable )
        d->connectTask( task, i++ );
    d->schedule();
}

// ### extract to base
void SignEMailController::Private::schedule() {

    if ( !cms )
        if ( const shared_ptr<SignEMailTask> t = takeRunnable( CMS ) ) {
            t->start();
            cms = t;
        }

    if ( !openpgp )
        if ( const shared_ptr<SignEMailTask> t = takeRunnable( OpenPGP ) ) {
            t->start();
            openpgp = t;
        }

    if ( !cms && !openpgp ) {
        kleo_assert( runnable.empty() );
        QPointer<QObject> Q = q;
        Q_FOREACH( const shared_ptr<SignEMailTask> t, completed ) {
            emit q->reportMicAlg( t->micAlg() );
            if ( !Q )
                return;
        }
        emit q->done();
    }
    
}

// ### extract to base
shared_ptr<SignEMailTask> SignEMailController::Private::takeRunnable( GpgME::Protocol proto ) {
    const std::vector< shared_ptr<SignEMailTask> >::iterator it
        = std::find_if( runnable.begin(), runnable.end(),
                        bind( &Task::protocol, _1 ) == proto );
    if ( it == runnable.end() )
        return shared_ptr<SignEMailTask>();

    const shared_ptr<SignEMailTask> result = *it;
    runnable.erase( it );
    return result;
}

// ### extract to base
void SignEMailController::Private::connectTask( const shared_ptr<Task> & t, unsigned int idx ) {
    connect( t.get(), SIGNAL(result(boost::shared_ptr<const Kleo::Crypto::Task::Result>)),
             q, SLOT(slotTaskDone()) );
    ensureWizardCreated();
    wizard->connectTask( t, idx );
}

// ### extract to base
void SignEMailController::Private::slotTaskDone() {
    assert( q->sender() );
    
    // We could just delete the tasks here, but we can't use
    // Qt::QueuedConnection here (we need sender()) and other slots
    // might not yet have executed. Therefore, we push completed tasks
    // into a burial container

    if ( q->sender() == cms.get() ) {
        completed.push_back( cms );
        cms.reset();
    } else if ( q->sender() == openpgp.get() ) {
        completed.push_back( openpgp );
        openpgp.reset();
    }

    QTimer::singleShot( 0, q, SLOT(schedule()) );
}

// ### extract to base
void SignEMailController::cancel() {
    try {
        if ( d->wizard )
            d->wizard->close();
        d->cancelAllJobs();
    } catch ( const std::exception & e ) {
        qDebug( "Caught exception: %s", e.what() );
    }
}

// ### extract to base
void SignEMailController::Private::cancelAllJobs() {

    // we just kill all runnable tasks - this will not result in
    // signal emissions.
    runnable.clear();

    // a cancel() will result in a call to 
    if ( cms )
        cms->cancel();
    if ( openpgp )
        openpgp->cancel();
}

// ### extract to base
void SignEMailController::Private::ensureWizardCreated() {
    if ( wizard )
        return;

    std::auto_ptr<SignEncryptWizard> w( new SignEMailWizard );
    w->setAttribute( Qt::WA_DeleteOnClose );
    connect( w.get(), SIGNAL(signersResolved()), q, SLOT(slotWizardSignersResolved()), Qt::QueuedConnection );
    connect( w.get(), SIGNAL(canceled()), q, SLOT(slotWizardCanceled()), Qt::QueuedConnection );
    w->setPresetProtocol( protocol );

    wizard = w.release();
}

// ### extract to base
void SignEMailController::Private::ensureWizardVisible() {
    ensureWizardCreated();
    q->bringToForeground( wizard );
}

#include "moc_signemailcontroller.cpp"


