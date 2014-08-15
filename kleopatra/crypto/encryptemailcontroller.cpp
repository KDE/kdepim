/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/encryptemailcontroller.cpp

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

#include "encryptemailcontroller.h"

#include "encryptemailtask.h"
#include "taskcollection.h"

#include <crypto/gui/encryptemailwizard.h>


#include <utils/input.h>
#include <utils/output.h>
#include <utils/kleo_assert.h>

#include <kleo/stl_util.h>
#include <kleo/exception.h>

#include "emailoperationspreferences.h"

#include <gpgme++/key.h>

#include <kmime/kmime_header_parsing.h>

#include <KLocalizedString>

#include <QPointer>
#include <QTimer>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace Kleo::Crypto::Gui;
using namespace boost;
using namespace GpgME;
using namespace KMime::Types;

class EncryptEMailController::Private {
    friend class ::Kleo::Crypto::EncryptEMailController;
    EncryptEMailController * const q;
public:
    explicit Private( Mode mode, EncryptEMailController * qq );

private:
    void slotWizardRecipientsResolved();
    void slotWizardCanceled();

private:
    void ensureWizardCreated() const;
    void ensureWizardVisible();
    void cancelAllTasks();

    void schedule();
    shared_ptr<EncryptEMailTask> takeRunnable( GpgME::Protocol proto );

private:
    const Mode mode;
    std::vector< shared_ptr<EncryptEMailTask> > runnable, completed;
    shared_ptr<EncryptEMailTask> cms, openpgp;
    mutable QPointer<EncryptEMailWizard> wizard;
};

EncryptEMailController::Private::Private( Mode m, EncryptEMailController * qq )
    : q( qq ),
      mode( m ),
      runnable(),
      cms(),
      openpgp(),
      wizard()
{

}

EncryptEMailController::EncryptEMailController( const shared_ptr<ExecutionContext> & xc, Mode mode, QObject * p )
    : Controller( xc, p ), d( new Private( mode, this ) )
{

}

EncryptEMailController::EncryptEMailController( Mode mode, QObject * p )
    : Controller( p ), d( new Private( mode, this ) )
{

}

EncryptEMailController::~EncryptEMailController() {
    if ( d->wizard && !d->wizard->isVisible() )
        delete d->wizard;
        //d->wizard->close(); ### ?
}

EncryptEMailController::Mode EncryptEMailController::mode() const {
    return d->mode;
}

void EncryptEMailController::setProtocol( Protocol proto ) {
    d->ensureWizardCreated();
    const Protocol protocol = d->wizard->presetProtocol();
    kleo_assert( protocol == UnknownProtocol ||
                 protocol == proto );

    d->wizard->setPresetProtocol( proto );
}

Protocol EncryptEMailController::protocol() const {
    d->ensureWizardCreated();
    return d->wizard->selectedProtocol();
}


const char * EncryptEMailController::protocolAsString() const {
    switch ( protocol() ) {
    case OpenPGP: return "OpenPGP";
    case CMS:     return "CMS";
    default:
        throw Kleo::Exception( gpg_error( GPG_ERR_INTERNAL ),
                               i18n("Call to EncryptEMailController::protocolAsString() is ambiguous.") );
    }
}

void EncryptEMailController::startResolveRecipients() {
    startResolveRecipients( std::vector<Mailbox>(), std::vector<Mailbox>() );
}

void EncryptEMailController::startResolveRecipients( const std::vector<Mailbox> & recipients, const std::vector<Mailbox> & senders ) {
    d->ensureWizardCreated();
    d->wizard->setRecipients( recipients, senders );
    d->ensureWizardVisible();
}

void EncryptEMailController::Private::slotWizardRecipientsResolved() {
    emit q->recipientsResolved();
}

void EncryptEMailController::Private::slotWizardCanceled() {
    q->setLastError( gpg_error( GPG_ERR_CANCELED ), i18n("User cancel") );
    q->emitDoneOrError();
}

void EncryptEMailController::setInputAndOutput( const shared_ptr<Input> & input, const shared_ptr<Output> & output ) {
    setInputsAndOutputs( std::vector< shared_ptr<Input> >( 1, input ), std::vector< shared_ptr<Output> >( 1, output ) );
}

void EncryptEMailController::setInputsAndOutputs( const std::vector< shared_ptr<Input> > & inputs, const std::vector< shared_ptr<Output> > & outputs ) {

    kleo_assert( !inputs.empty() );
    kleo_assert( outputs.size() == inputs.size() );

    std::vector< shared_ptr<EncryptEMailTask> > tasks;
    tasks.reserve( inputs.size() );

    d->ensureWizardCreated();

    const std::vector<Key> keys = d->wizard->resolvedCertificates();
    kleo_assert( !keys.empty() );

    for ( unsigned int i = 0, end = inputs.size() ; i < end ; ++i ) {

        const shared_ptr<EncryptEMailTask> task( new EncryptEMailTask );
        task->setInput( inputs[i] );
        task->setOutput( outputs[i] );
        if ( d->mode == ClipboardMode )
            task->setAsciiArmor( true );
        task->setRecipients( keys );

        tasks.push_back( task );
    }

    d->runnable.swap( tasks );
}

void EncryptEMailController::start() {
    shared_ptr<TaskCollection> coll( new TaskCollection );
    std::vector<shared_ptr<Task> > tmp;
    std::copy( d->runnable.begin(), d->runnable.end(), std::back_inserter( tmp ) );
    coll->setTasks( tmp );
    d->ensureWizardCreated();
    d->wizard->setTaskCollection( coll );
    Q_FOREACH( const shared_ptr<Task> & t, tmp )
        connectTask( t );
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

    if ( cms || openpgp )
        return;
    kleo_assert( runnable.empty() );
    q->emitDoneOrError();
}

shared_ptr<EncryptEMailTask> EncryptEMailController::Private::takeRunnable( GpgME::Protocol proto ) {
    const std::vector< shared_ptr<EncryptEMailTask> >::iterator it
        = std::find_if( runnable.begin(), runnable.end(),
                        boost::bind( &Task::protocol, _1 ) == proto );
    if ( it == runnable.end() )
        return shared_ptr<EncryptEMailTask>();

    const shared_ptr<EncryptEMailTask> result = *it;
    runnable.erase( it );
    return result;
}

void EncryptEMailController::doTaskDone( const Task * task, const shared_ptr<const Task::Result> & result )
{
    Q_UNUSED( result );
    assert( task );

    // We could just delete the tasks here, but we can't use
    // Qt::QueuedConnection here (we need sender()) and other slots
    // might not yet have executed. Therefore, we push completed tasks
    // into a burial container

    if ( task == d->cms.get() ) {
        d->completed.push_back( d->cms );
        d->cms.reset();
    } else if ( task == d->openpgp.get() ) {
        d->completed.push_back( d->openpgp );
        d->openpgp.reset();
    }

    QTimer::singleShot( 0, this, SLOT(schedule()) );
}

void EncryptEMailController::cancel() {
    try {
        if ( d->wizard )
            d->wizard->close();
        d->cancelAllTasks();
    } catch ( const std::exception & e ) {
        qDebug() << "Caught exception: " << e.what();
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

void EncryptEMailController::Private::ensureWizardCreated() const {
    if ( wizard )
        return;

    std::auto_ptr<EncryptEMailWizard> w( new EncryptEMailWizard );
    w->setAttribute( Qt::WA_DeleteOnClose );
    Kleo::EMailOperationsPreferences prefs;
    w->setQuickMode( prefs.quickEncryptEMail() );
    connect( w.get(), SIGNAL(recipientsResolved()), q, SLOT(slotWizardRecipientsResolved()), Qt::QueuedConnection );
    connect( w.get(), SIGNAL(canceled()), q, SLOT(slotWizardCanceled()), Qt::QueuedConnection );

    wizard = w.release();
}

void EncryptEMailController::Private::ensureWizardVisible() {
    ensureWizardCreated();
    q->bringToForeground( wizard );
}

#include "moc_encryptemailcontroller.cpp"


