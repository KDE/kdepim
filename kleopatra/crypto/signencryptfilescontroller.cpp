/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/signencryptfilescontroller.cpp

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

#include "signencryptfilescontroller.h"

#include "signencryptfilestask.h"
#include "certificateresolver.h"

#include <crypto/gui/newsignencryptfileswizard.h>
#include <crypto/taskcollection.h>

#include <utils/input.h>
#include <utils/output.h>
#include <utils/classify.h>
#include <utils/stl_util.h>
#include <utils/kleo_assert.h>
#include <utils/exception.h>

#include <kmime/kmime_header_parsing.h>

#include <KLocale>
#include <kdebug.h>

#include <QPointer>
#include <QTimer>
#include <QFileInfo>

#include <boost/bind.hpp>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace Kleo::Crypto::Gui;
using namespace boost;
using namespace GpgME;
using namespace KMime::Types;

class SignEncryptFilesController::Private {
    friend class ::Kleo::Crypto::SignEncryptFilesController;
    SignEncryptFilesController * const q;
public:
    explicit Private( SignEncryptFilesController * qq );
    ~Private();

private:
    void slotWizardOperationPrepared();
    void slotWizardCanceled();

private:
    void ensureWizardCreated();
    void ensureWizardVisible();
    void updateWizardMode();
    void cancelAllTasks();
    void reportError( int err, const QString & details ) {
        emit q->error( err, details );
    }

    void schedule();
    shared_ptr<SignEncryptFilesTask> takeRunnable( GpgME::Protocol proto );

    static void assertValidOperation( unsigned int );
    static QString titleForOperation( unsigned int op );
private:
    std::vector< shared_ptr<SignEncryptFilesTask> > runnable, completed;
    shared_ptr<SignEncryptFilesTask> cms, openpgp;
    QPointer<NewSignEncryptFilesWizard> wizard;
    QStringList files;
    unsigned int operation;
    Protocol protocol;
};

SignEncryptFilesController::Private::Private( SignEncryptFilesController * qq )
    : q( qq ),
      runnable(),
      cms(),
      openpgp(),
      wizard(),
      files(),
      operation( SignAllowed|EncryptAllowed ),
      protocol( UnknownProtocol )
{

}

SignEncryptFilesController::Private::~Private() { kDebug(); }

QString SignEncryptFilesController::Private::titleForOperation( unsigned int op ) {
    const bool signDisallowed = (op & SignMask) == SignDisallowed;
    const bool encryptDisallowed = (op & EncryptMask) == EncryptDisallowed;

    kleo_assert( !signDisallowed || !encryptDisallowed );
 
    if ( !signDisallowed && encryptDisallowed )
        return i18n( "Sign Files" );

    if ( signDisallowed && !encryptDisallowed )
        return i18n( "Encrypt Files" );

    return i18n( "Sign/Encrypt Files" );
}

SignEncryptFilesController::SignEncryptFilesController( QObject * p )
    : Controller( p ), d( new Private( this ) )
{

}

SignEncryptFilesController::SignEncryptFilesController( const shared_ptr<const ExecutionContext> & ctx, QObject * p )
    : Controller( ctx, p ), d( new Private( this ) )
{

}

SignEncryptFilesController::~SignEncryptFilesController() {
    kDebug();
    if ( d->wizard && !d->wizard->isVisible() )
        delete d->wizard;
        //d->wizard->close(); ### ?
}

void SignEncryptFilesController::setProtocol( Protocol proto ) {
    kleo_assert( d->protocol == UnknownProtocol ||
                 d->protocol == proto );
    d->protocol = proto;
    d->ensureWizardCreated();
    d->wizard->setPresetProtocol( proto );
}

Protocol SignEncryptFilesController::protocol() const {
    return d->protocol;
}

// static
void SignEncryptFilesController::Private::assertValidOperation( unsigned int op ) {
    kleo_assert( ( op & SignMask )    == SignDisallowed    ||
                 ( op & SignMask )    == SignAllowed       ||
                 ( op & SignMask )    == SignForced );
    kleo_assert( ( op & EncryptMask ) == EncryptDisallowed ||
                 ( op & EncryptMask ) == EncryptAllowed    ||
                 ( op & EncryptMask ) == EncryptForced );
    kleo_assert( ( op & ~(SignMask|EncryptMask) ) == 0 );
}

void SignEncryptFilesController::setOperationMode( unsigned int mode ) {
    Private::assertValidOperation( mode );
    d->operation = mode;
    d->updateWizardMode();
}

void SignEncryptFilesController::Private::updateWizardMode() {
    if ( !wizard )
        return;
    wizard->setWindowTitle( titleForOperation( operation ) );
    const unsigned int signOp = (operation & SignMask);
    const unsigned int encrOp = (operation & EncryptMask);
    switch ( signOp ) {
    case SignForced:
    case SignDisallowed:
        wizard->setSigningPreset( signOp == SignForced );
        wizard->setSigningUserMutable( false );
        break;
    default:
        assert( !"Should not happen" );
    case SignAllowed:
        wizard->setSigningPreset( encrOp == EncryptDisallowed );
        wizard->setSigningUserMutable( true );
        break;
    }
    switch ( encrOp ) {
    case EncryptForced:
    case EncryptDisallowed:
        wizard->setEncryptionPreset( encrOp == EncryptForced );
        wizard->setEncryptionUserMutable( false );
        break;
    default:
        assert( !"Should not happen" );
    case EncryptAllowed:
        wizard->setEncryptionPreset( true );
        wizard->setEncryptionUserMutable( true );
        break;
    }
}

unsigned int SignEncryptFilesController::operationMode() const {
    return d->operation;
}

void SignEncryptFilesController::setFiles( const QStringList & files ) {
    kleo_assert( !files.empty() );
    d->files = files;
    d->ensureWizardCreated();
    d->wizard->setFiles( files );
}

void SignEncryptFilesController::Private::slotWizardCanceled() {
    kDebug();
    reportError( gpg_error( GPG_ERR_CANCELED ), i18n("User cancel") );
}

void SignEncryptFilesController::start() {
    d->ensureWizardVisible();
}

static shared_ptr<SignEncryptFilesTask>
createSignEncryptTaskForFileInfo( const QFileInfo & fi, bool pgp, bool sign, bool encrypt, bool ascii, bool removeUnencrypted, const std::vector<Key> & recipients, const std::vector<Key> & signers ) {
    const shared_ptr<SignEncryptFilesTask> task( new SignEncryptFilesTask );
    task->setSign( sign );
    task->setEncrypt( encrypt );
    task->setAsciiArmor( ascii );
    task->setRemoveInputFileOnSuccess( removeUnencrypted );
    if ( sign ) {
        task->setSigners( signers );
        task->setDetachedSignature( true );
    }
    if ( encrypt )
        task->setRecipients( recipients );

    unsigned int cls = pgp ? Class::OpenPGP : Class::CMS ;
    if ( encrypt )
        cls |= Class::CipherText;
    else if ( sign )
        cls |= Class::DetachedSignature;
    cls |= ascii ? Class::Ascii : Class::Binary ;

    const QString input = fi.absoluteFilePath();
    task->setInputFileName( input );

    const char * ext = outputFileExtension( cls );
    if ( !ext )
        ext = "out"; // ### error out?

    const QString output = input + '.' + ext;
    task->setOutputFileName( output );

    return task;
}

static std::vector< shared_ptr<SignEncryptFilesTask> >
createSignEncryptTasksForFileInfo( const QFileInfo & fi, bool sign, bool encrypt, bool ascii, bool removeUnencrypted, const std::vector<Key> & pgpRecipients, const std::vector<Key> & pgpSigners, const std::vector<Key> & cmsRecipients, const std::vector<Key> & cmsSigners ) {
    std::vector< shared_ptr<SignEncryptFilesTask> > result;

    const bool shallPgpSign = sign && !pgpSigners.empty();
    const bool shallPgpEncrypt = encrypt && !pgpRecipients.empty();
    const bool pgp = ( shallPgpEncrypt && ( !sign || shallPgpSign ) ) || ( !encrypt && shallPgpSign );

    const bool shallCmsSign = sign && !cmsSigners.empty();
    const bool shallCmsEncrypt = encrypt && !cmsRecipients.empty();
    const bool cms = ( shallCmsEncrypt && ( !sign || shallCmsSign ) ) || ( !encrypt && shallCmsSign );

    result.reserve( pgp + cms );

    if ( pgp )
        result.push_back( createSignEncryptTaskForFileInfo( fi, true, sign, encrypt, ascii, removeUnencrypted, pgpRecipients, pgpSigners ) );
    if ( cms )
        result.push_back( createSignEncryptTaskForFileInfo( fi, false, sign, encrypt, ascii, removeUnencrypted, cmsRecipients, cmsSigners ) );

    return result;
}

void SignEncryptFilesController::Private::slotWizardOperationPrepared() {

    try {

        kleo_assert( wizard );
        kleo_assert( !files.empty() );

        const bool sign = wizard->isSigningSelected();
        const bool encrypt = wizard->isEncryptionSelected();

        const bool ascii = wizard->isAsciiArmorEnabled();
        const bool removeUnencrypted = wizard->isRemoveUnencryptedFilesEnabled();

        std::vector<Key> pgpRecipients, cmsRecipients, pgpSigners, cmsSigners;
        if ( encrypt ) {
            const std::vector<Key> recipients = wizard->resolvedRecipients();
            kdtools::copy_if( recipients.begin(), recipients.end(),
                              std::back_inserter( pgpRecipients ),
                              bind( &Key::protocol, _1 ) == GpgME::OpenPGP );
            kdtools::copy_if( recipients.begin(), recipients.end(),
                              std::back_inserter( cmsRecipients ),
                              bind( &Key::protocol, _1 ) == GpgME::CMS );
            kleo_assert( pgpRecipients.size() + cmsRecipients.size() == recipients.size() );
        }
        if ( sign ) {
            const std::vector<Key> signers = wizard->resolvedSigners();
            kdtools::copy_if( signers.begin(), signers.end(),
                              std::back_inserter( pgpSigners ),
                              bind( &Key::protocol, _1 ) == GpgME::OpenPGP );
            kdtools::copy_if( signers.begin(), signers.end(),
                              std::back_inserter( cmsSigners ),
                              bind( &Key::protocol, _1 ) == GpgME::CMS );
            kleo_assert( pgpSigners.size() + cmsSigners.size() == signers.size() );
        }

        std::vector< shared_ptr<SignEncryptFilesTask> > tasks;
        tasks.reserve( files.size() );

        ensureWizardCreated();
        const shared_ptr<OverwritePolicy> overwritePolicy( new OverwritePolicy( wizard ) );

        Q_FOREACH( const QString & file, files ) {
            const std::vector< shared_ptr<SignEncryptFilesTask> > created = 
                createSignEncryptTasksForFileInfo( QFileInfo( file ), sign, encrypt, ascii, removeUnencrypted, pgpRecipients, pgpSigners, cmsRecipients, cmsSigners );
            Q_FOREACH( const shared_ptr<SignEncryptFilesTask> & i, created )
                i->setOverwritePolicy( overwritePolicy );
            tasks.insert( tasks.end(), created.begin(), created.end() );
        }

        kleo_assert( runnable.empty() );

        runnable.swap( tasks );
        
        Q_FOREACH( const shared_ptr<Task> task, runnable )
            q->connectTask( task );

        shared_ptr<TaskCollection> coll( new TaskCollection );
        std::vector<shared_ptr<Task> > tmp;
        std::copy( runnable.begin(), runnable.end(), std::back_inserter( tmp ) );
        coll->setTasks( tmp );
        wizard->setTaskCollection( coll );

        QTimer::singleShot( 0, q, SLOT(schedule()) );

    } catch ( const Kleo::Exception & e ) {
        reportError( e.error().encodedError(), e.message() );
    } catch ( const std::exception & e ) {
        reportError( gpg_error( GPG_ERR_UNEXPECTED ),
                     i18n("Caught unexpected exception in SignEncryptFilesController::Private::slotWizardOperationPrepared: %1",
                          QString::fromLocal8Bit( e.what() ) ) );
    } catch ( ... ) {
        reportError( gpg_error( GPG_ERR_UNEXPECTED ),
                     i18n("Caught unknown exception in SignEncryptFilesController::Private::slotWizardOperationPrepared") );
    }
}

void SignEncryptFilesController::Private::schedule() {

    if ( !cms )
        if ( const shared_ptr<SignEncryptFilesTask> t = takeRunnable( CMS ) ) {
            t->start();
            cms = t;
        }

    if ( !openpgp )
        if ( const shared_ptr<SignEncryptFilesTask> t = takeRunnable( OpenPGP ) ) {
            t->start();
            openpgp = t;
        }

    if ( !cms && !openpgp ) {
        kleo_assert( runnable.empty() );
        q->emitDoneOrError();
    }
}

shared_ptr<SignEncryptFilesTask> SignEncryptFilesController::Private::takeRunnable( GpgME::Protocol proto ) {
    const std::vector< shared_ptr<SignEncryptFilesTask> >::iterator it
        = std::find_if( runnable.begin(), runnable.end(),
                        bind( &Task::protocol, _1 ) == proto );
    if ( it == runnable.end() )
        return shared_ptr<SignEncryptFilesTask>();

    const shared_ptr<SignEncryptFilesTask> result = *it;
    runnable.erase( it );
    return result;
}

void SignEncryptFilesController::doTaskDone( const Task * task, const shared_ptr<const Task::Result> & result ) {
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

void SignEncryptFilesController::cancel() {
    kDebug();
    try {
        if ( d->wizard )
            d->wizard->close();
        d->cancelAllTasks();
    } catch ( const std::exception & e ) {
        qDebug( "Caught exception: %s", e.what() );
    }
}

void SignEncryptFilesController::Private::cancelAllTasks() {

    // we just kill all runnable tasks - this will not result in
    // signal emissions.
    runnable.clear();

    // a cancel() will result in a call to 
    if ( cms )
        cms->cancel();
    if ( openpgp )
        openpgp->cancel();
}

void SignEncryptFilesController::Private::ensureWizardCreated() {
    if ( wizard )
        return;

    std::auto_ptr<NewSignEncryptFilesWizard> w( new NewSignEncryptFilesWizard );
    w->setAttribute( Qt::WA_DeleteOnClose );

    connect( w.get(), SIGNAL(operationPrepared()), q, SLOT(slotWizardOperationPrepared()), Qt::QueuedConnection );
    connect( w.get(), SIGNAL(rejected()), q, SLOT(slotWizardCanceled()), Qt::QueuedConnection );
    wizard = w.release();

    updateWizardMode();
}

void SignEncryptFilesController::Private::ensureWizardVisible() {
    ensureWizardCreated();
    q->bringToForeground( wizard );
}

#include "moc_signencryptfilescontroller.cpp"


