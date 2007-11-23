/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/encryptemailcontroller.cpp

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

#include "encryptemailcontroller.h"

#include "kleo-assuan.h"
#include "assuancommand.h"
#include "certificateresolver.h"
#include "signencryptwizard.h"
#include "encryptemailtask.h"
#include "input.h"
#include "output.h"

#include <utils/stl_util.h>

#include <kmime/kmime_header_parsing.h>

#include <KLocale>

#include <QPointer>
#include <QTimer>

#include <boost/bind.hpp>

using namespace Kleo;
using namespace boost;
using namespace GpgME;
using namespace KMime::Types;

class EncryptEMailController::Private {
    friend class ::Kleo::EncryptEMailController;
    EncryptEMailController * const q;
public:
    explicit Private( EncryptEMailController * qq );

private:
    void slotWizardRecipientsResolved();
    void slotWizardCanceled();
    void slotTaskDone();

private:
    void ensureWizardCreated();
    void ensureWizardVisible();
    void cancelAllTasks();

    void schedule();
    shared_ptr<EncryptEMailTask> takeRunnable( GpgME::Protocol proto );
    void connectTask( const shared_ptr<Task> & task, unsigned int idx );

private:
    std::vector< shared_ptr<EncryptEMailTask> > runnable, completed;
    shared_ptr<EncryptEMailTask> cms, openpgp;
    weak_ptr<AssuanCommand> command;
    QPointer<SignEncryptWizard> wizard;
    Protocol protocol;
};

EncryptEMailController::Private::Private( EncryptEMailController * qq )
    : q( qq ),
      runnable(),
      cms(),
      openpgp(),
      command(),
      wizard(),
      protocol( UnknownProtocol )
{

}

EncryptEMailController::EncryptEMailController( QObject * p )
    : QObject( p ), d( new Private( this ) )
{

}

EncryptEMailController::~EncryptEMailController() {
    if ( d->wizard && !d->wizard->isVisible() )
        delete d->wizard;
        //d->wizard->close(); ### ?
}

void EncryptEMailController::setProtocol( Protocol proto ) {
    assuan_assert( d->protocol == UnknownProtocol ||
                   d->protocol == proto );
    d->protocol = proto;
    if ( d->wizard )
        d->wizard->setProtocol( proto );
}

Protocol EncryptEMailController::protocol() const {
    return d->protocol;
}

const char * EncryptEMailController::protocolAsString() const {
    switch ( d->protocol ) {
    case OpenPGP: return "OpenPGP";
    case CMS:     return "CMS";
    default:
        throw assuan_exception( gpg_error( GPG_ERR_INTERNAL ),
                                i18n("Call to EncryptEMailController::protocolAsString() is ambiguous.") );
    }
}

void EncryptEMailController::setCommand( const shared_ptr<AssuanCommand> & cmd ) {
    d->command = cmd;
}

void EncryptEMailController::startResolveRecipients( const std::vector<Mailbox> & recipients ) {
    const std::vector< std::vector<Key> > keys = CertificateResolver::resolveRecipients( recipients, d->protocol );

    assuan_assert( keys.size() == static_cast<size_t>( recipients.size() ) );

    d->ensureWizardCreated();

    d->wizard->setRecipientsAndCandidates( recipients, keys );

    if ( d->wizard->canGoToNextPage() ) {
        d->wizard->next();
    }
    d->ensureWizardVisible();
}

void EncryptEMailController::Private::slotWizardRecipientsResolved() {
    emit q->recipientsResolved();
}

void EncryptEMailController::Private::slotWizardCanceled() {
    emit q->error( gpg_error( GPG_ERR_CANCELED ), i18n("User cancel") );
}

void EncryptEMailController::importIO() {

    const shared_ptr<AssuanCommand> cmd = d->command.lock();
    assuan_assert( cmd );

    const std::vector< shared_ptr<Input> > & inputs = cmd->inputs();
    assuan_assert( !inputs.empty() );

    const std::vector< shared_ptr<Output> > & outputs = cmd->outputs();
    assuan_assert( outputs.size() == inputs.size() );

    std::vector< shared_ptr<EncryptEMailTask> > tasks;
    tasks.reserve( inputs.size() );

    d->ensureWizardCreated();

    const std::vector<Key> keys = d->wizard->resolvedCertificates();
    assuan_assert( !keys.empty() );

    for ( unsigned int i = 0, end = inputs.size() ; i < end ; ++i ) {

        const shared_ptr<EncryptEMailTask> task( new EncryptEMailTask );
        task->setInput( inputs[i] );
        task->setOutput( outputs[i] );
        task->setRecipients( keys );

        tasks.push_back( task );
    }

    d->runnable.swap( tasks );
}

void EncryptEMailController::start() {
    int i = 0;
    Q_FOREACH( const shared_ptr<Task> task, d->runnable )
        d->connectTask( task, i++ );
    d->schedule();
}

void EncryptEMailController::Private::schedule() {

    if ( !cms )
        if ( const shared_ptr<EncryptEMailTask> t = takeRunnable( CMS ) ) {
            t->start();
            cms = t;
        }

    if ( !openpgp )
        if ( const shared_ptr<EncryptEMailTask> t = takeRunnable( OpenPGP ) ) {
            t->start();
            openpgp = t;
        }

    if ( !cms && !openpgp ) {
        assuan_assert( runnable.empty() );
        emit q->done();
    }
    
}

shared_ptr<EncryptEMailTask> EncryptEMailController::Private::takeRunnable( GpgME::Protocol proto ) {
    const std::vector< shared_ptr<EncryptEMailTask> >::iterator it
        = std::find_if( runnable.begin(), runnable.end(),
                        bind( &Task::protocol, _1 ) == proto );
    if ( it == runnable.end() )
        return shared_ptr<EncryptEMailTask>();

    const shared_ptr<EncryptEMailTask> result = *it;
    runnable.erase( it );
    return result;
}

void EncryptEMailController::Private::connectTask( const shared_ptr<Task> & t, unsigned int idx ) {
    connect( t.get(), SIGNAL(result(boost::shared_ptr<const Kleo::Task::Result>)),
             q, SLOT(slotTaskDone()) );
    connect( t.get(), SIGNAL(error(int,QString)), q, SLOT(slotTaskDone()) );
    ensureWizardCreated();
    wizard->connectTask( t, idx );
}

void EncryptEMailController::Private::slotTaskDone() {
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

void EncryptEMailController::cancel() {
    try {
        if ( d->wizard )
            d->wizard->close();
        d->cancelAllTasks();
    } catch ( const std::exception & e ) {
        qDebug( "Caught exception: %s", e.what() );
    }
}

void EncryptEMailController::Private::cancelAllTasks() {

    // we just kill all runnable tasks - this will not result in
    // signal emissions.
    runnable.clear();

    // a cancel() will result in a call to 
    if ( cms )
        cms->cancel();
    if ( openpgp )
        openpgp->cancel();
}

void EncryptEMailController::Private::ensureWizardCreated() {
    if ( wizard )
        return;

    std::auto_ptr<SignEncryptWizard> w( new SignEncryptWizard );
    if ( const shared_ptr<AssuanCommand> cmd = command.lock() )
        w = cmd->applyWindowID( w );
    w->setWindowTitle( i18n("Encrypt Mail Message") );
    w->setMode( SignEncryptWizard::EncryptEMail );
    w->setAttribute( Qt::WA_DeleteOnClose );
    connect( w.get(), SIGNAL(recipientsResolved()), q, SLOT(slotWizardRecipientsResolved()) );
    connect( w.get(), SIGNAL(canceled()), q, SLOT(slotWizardCanceled()) );

    w->setProtocol( protocol );
    wizard = w.release();
}

void EncryptEMailController::Private::ensureWizardVisible() {
    ensureWizardCreated();
    if ( wizard->isVisible() )
        wizard->raise();
    else
        wizard->show();
}

#include "moc_encryptemailcontroller.cpp"


