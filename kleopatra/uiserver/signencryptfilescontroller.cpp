/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/signencryptfilescontroller.cpp

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

#include "signencryptfilescontroller.h"

#include "kleo-assuan.h"
#include "assuancommand.h"
#include "certificateresolver.h"
#include "signencryptfileswizard.h"
#include "signencryptfilestask.h"
#include "input.h"
#include "output.h"
#include "classify.h"

#include <utils/stl_util.h>

#include <kmime/kmime_header_parsing.h>

#include <KLocale>

#include <QPointer>
#include <QTimer>
#include <QFileInfo>

#include <boost/bind.hpp>

using namespace Kleo;
using namespace boost;
using namespace GpgME;
using namespace KMime::Types;

class SignEncryptFilesController::Private {
    friend class ::Kleo::SignEncryptFilesController;
    SignEncryptFilesController * const q;
public:
    explicit Private( SignEncryptFilesController * qq );

private:
    void slotWizardOperationPrepared();
    void slotWizardCanceled();
    void slotTaskDone();
    
private:
    void ensureWizardCreated();
    void ensureWizardVisible();
    void cancelAllTasks();
    void reportError( int err, const QString & details ) {
        errorDetected = true;
        emit q->error( err, details );
    }
    void removeInputFiles();

    void schedule();
    shared_ptr<SignEncryptFilesTask> takeRunnable( GpgME::Protocol proto );
    void connectTask( const shared_ptr<Task> & task, unsigned int idx );

    static void assertValidOperation( unsigned int );
    static QString titleForOperation( unsigned int op );
private:
    std::vector< shared_ptr<SignEncryptFilesTask> > runnable, completed;
    shared_ptr<SignEncryptFilesTask> cms, openpgp;
    weak_ptr<AssuanCommand> command;
    QPointer<SignEncryptFilesWizard> wizard;
    unsigned int operation;
    Protocol protocol;
    bool errorDetected : 1;
};

SignEncryptFilesController::Private::Private( SignEncryptFilesController * qq )
    : q( qq ),
      runnable(),
      cms(),
      openpgp(),
      command(),
      wizard(),
      operation( SignAllowed|EncryptAllowed ),
      protocol( UnknownProtocol ),
      errorDetected( false )
{

}


QString SignEncryptFilesController::Private::titleForOperation( unsigned int op ) {
    const bool signDisallowed = (op & SignMask) == SignDisallowed;
    const bool encryptDisallowed = (op & EncryptMask) == EncryptDisallowed;

    assuan_assert( !signDisallowed || !encryptDisallowed );
 
    if ( !signDisallowed && encryptDisallowed )
        return i18n( "Sign Files" );

    if ( signDisallowed && !encryptDisallowed )
        return i18n( "Encrypt Files" );

    return i18n( "Sign/Encrypt Files" );
}

SignEncryptFilesController::SignEncryptFilesController( QObject * p )
    : QObject( p ), d( new Private( this ) )
{

}

SignEncryptFilesController::~SignEncryptFilesController() {
    if ( d->wizard && !d->wizard->isVisible() )
        delete d->wizard;
        //d->wizard->close(); ### ?
}

void SignEncryptFilesController::setProtocol( Protocol proto ) {
    assuan_assert( d->protocol == UnknownProtocol ||
                   d->protocol == proto );
    d->protocol = proto;
    d->ensureWizardCreated();
    d->wizard->setPresetProtocol( proto );
}

Protocol SignEncryptFilesController::protocol() const {
    return d->protocol;
}

void SignEncryptFilesController::setCommand( const shared_ptr<AssuanCommand> & cmd ) {
    d->command = cmd;
}

// static
void SignEncryptFilesController::Private::assertValidOperation( unsigned int op ) {
    assuan_assert( ( op & SignMask )    == SignDisallowed    ||
                   ( op & SignMask )    == SignAllowed       ||
                   ( op & SignMask )    == SignForced );
    assuan_assert( ( op & EncryptMask ) == EncryptDisallowed ||
                   ( op & EncryptMask ) == EncryptAllowed    ||
                   ( op & EncryptMask ) == EncryptForced );
    assuan_assert( ( op & ~(SignMask|EncryptMask) ) == 0 );
}

void SignEncryptFilesController::setOperationMode( unsigned int mode ) {
    Private::assertValidOperation( mode );
    d->operation = mode;
    if ( d->wizard )
        d->wizard->setWindowTitle( d->titleForOperation( d->operation ) );
}

void SignEncryptFilesController::setFiles( const QStringList & files ) {
    assuan_assert( !files.empty() );
    d->ensureWizardVisible();
    d->wizard->setFiles( files );
}

void SignEncryptFilesController::Private::slotWizardCanceled() {
    reportError( gpg_error( GPG_ERR_CANCELED ), i18n("User cancel") );
}

void SignEncryptFilesController::start() {
    d->ensureWizardVisible();
}

static shared_ptr<SignEncryptFilesTask>
createSignEncryptTaskForFileInfo( const QFileInfo & fi, bool pgp, bool sign, bool encrypt, bool ascii, const std::vector<Key> & recipients, const std::vector<Key> & signers ) {
    const shared_ptr<SignEncryptFilesTask> task( new SignEncryptFilesTask );
    task->setSign( sign );
    task->setEncrypt( encrypt );
    task->setAsciiArmor( ascii );
    if ( sign )
        task->setSigners( signers );
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
createSignEncryptTasksForFileInfo( const QFileInfo & fi, bool sign, bool encrypt, bool ascii, const std::vector<Key> & pgpRecipients, const std::vector<Key> & pgpSigners, const std::vector<Key> & cmsRecipients, const std::vector<Key> & cmsSigners ) {
    std::vector< shared_ptr<SignEncryptFilesTask> > result;

    const bool shallPgpSign = sign && !pgpSigners.empty();
    const bool shallPgpEncrypt = encrypt && !pgpRecipients.empty();
    const bool pgp = shallPgpEncrypt && ( !sign || shallPgpSign ) || !encrypt && shallPgpSign;

    const bool shallCmsSign = sign && !cmsSigners.empty();
    const bool shallCmsEncrypt = encrypt && !cmsRecipients.empty();
    const bool cms = shallCmsEncrypt && ( !sign || shallCmsSign ) || !encrypt && shallCmsSign;

    result.reserve( pgp + cms );

    if ( pgp )
        result.push_back( createSignEncryptTaskForFileInfo( fi, true, sign, encrypt, ascii, pgpRecipients, pgpSigners ) );
    if ( cms )
        result.push_back( createSignEncryptTaskForFileInfo( fi, false, sign, encrypt, ascii, cmsRecipients, cmsSigners ) );

    return result;
}

void SignEncryptFilesController::Private::slotWizardOperationPrepared() {

    try {

        assuan_assert( wizard );

        const bool sign = wizard->signingSelected();
        const bool encrypt = wizard->encryptionSelected();

        const bool ascii = wizard->isAsciiArmorEnabled();
        const QFileInfoList files = wizard->resolvedFiles();

        std::vector<Key> pgpRecipients, cmsRecipients, pgpSigners, cmsSigners;
        if ( encrypt ) {
            const std::vector<Key> recipients = wizard->resolvedCertificates();
            kdtools::copy_if( recipients.begin(), recipients.end(),
                              std::back_inserter( pgpRecipients ),
                              bind( &Key::protocol, _1 ) == GpgME::OpenPGP );
            kdtools::copy_if( recipients.begin(), recipients.end(),
                              std::back_inserter( cmsRecipients ),
                              bind( &Key::protocol, _1 ) == GpgME::CMS );
            assuan_assert( pgpRecipients.size() + cmsRecipients.size() == recipients.size() );
        }
        if ( sign ) {
            const std::vector<Key> signers = wizard->resolvedSigners();
            kdtools::copy_if( signers.begin(), signers.end(),
                              std::back_inserter( pgpSigners ),
                              bind( &Key::protocol, _1 ) == GpgME::OpenPGP );
            kdtools::copy_if( signers.begin(), signers.end(),
                              std::back_inserter( cmsSigners ),
                              bind( &Key::protocol, _1 ) == GpgME::CMS );
            assuan_assert( pgpSigners.size() + cmsSigners.size() == signers.size() );
        }

        assuan_assert( !files.empty() );

        std::vector< shared_ptr<SignEncryptFilesTask> > tasks;
        tasks.reserve( files.size() );

        Q_FOREACH( const QFileInfo & fi, files ) {
            const std::vector< shared_ptr<SignEncryptFilesTask> > created = 
                createSignEncryptTasksForFileInfo( fi, sign, encrypt, ascii, pgpRecipients, pgpSigners, cmsRecipients, cmsSigners );
            tasks.insert( tasks.end(), created.begin(), created.end() );
        }

        assuan_assert( runnable.empty() );

        runnable.swap( tasks );

        int i = 0;
        Q_FOREACH( const shared_ptr<Task> task, runnable )
            connectTask( task, i++ );

        schedule();
        
    } catch ( const assuan_exception & e ) {
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
        assuan_assert( runnable.empty() );
        if ( wizard->removeUnencryptedFile() && wizard->encryptionSelected() && !errorDetected )
            removeInputFiles();
        emit q->done();
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

void SignEncryptFilesController::Private::connectTask( const shared_ptr<Task> & t, unsigned int idx ) {
    connect( t.get(), SIGNAL(result(boost::shared_ptr<const Kleo::Task::Result>)),
             q, SLOT(slotTaskDone()) );
    connect( t.get(), SIGNAL(error(int,QString)), q, SLOT(slotTaskDone()) );
    ensureWizardCreated();
    wizard->connectTask( t, idx );
}

void SignEncryptFilesController::Private::slotTaskDone() {
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

void SignEncryptFilesController::cancel() {
    try {
        d->errorDetected = true;
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

    std::auto_ptr<SignEncryptFilesWizard> w( new SignEncryptFilesWizard );
    if ( const shared_ptr<AssuanCommand> cmd = command.lock() )
        w = cmd->applyWindowID( w );

    w->setWindowTitle( titleForOperation( operation ) );
    w->setAttribute( Qt::WA_DeleteOnClose );

    connect( w.get(), SIGNAL(operationPrepared()), q, SLOT(slotWizardOperationPrepared()), Qt::QueuedConnection );
    connect( w.get(), SIGNAL(canceled()), q, SLOT(slotWizardCanceled()), Qt::QueuedConnection );
    wizard = w.release();
}

void SignEncryptFilesController::Private::ensureWizardVisible() {
    ensureWizardCreated();
    if ( wizard->isVisible() )
        wizard->raise();
    else
        wizard->show();
}

void SignEncryptFilesController::Private::removeInputFiles() {
    if ( !wizard ) {
        qWarning( "SignEncryptFilesController::Private::removeInputFiles: no wizard!" );
        return;
    }
    if ( errorDetected )
        return;

    const QFileInfoList files = wizard->resolvedFiles();
    Q_FOREACH( const QFileInfo & fi, files )
        QFile::remove( fi.absoluteFilePath() );
}

#include "moc_signencryptfilescontroller.cpp"


