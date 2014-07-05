/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/newsignencryptemailcontroller.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2009,2010 Klarälvdalens Datakonsult AB

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

#include "newsignencryptemailcontroller.h"

#include "encryptemailtask.h"
#include "signemailtask.h"
#include "taskcollection.h"
#include "sender.h"
#include "recipient.h"

#include "emailoperationspreferences.h"

#include <crypto/gui/signencryptemailconflictdialog.h>


#include <utils/input.h>
#include <utils/output.h>
#include <utils/kleo_assert.h>

#include <kleo/stl_util.h>
#include <kleo/exception.h>

#include <gpgme++/key.h>

#include <kmime/kmime_header_parsing.h>

#include <KLocalizedString>
#include <KDebug>
#include <KMessageBox>

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

//
// BEGIN Conflict Detection
//

/*
  This code implements the following conflict detection algortihm:

  1. There is no conflict if and only if we have a Perfect Match.
  2. A Perfect Match is defined as:
    a. either a Perfect OpenPGP-Match and not even a Partial S/MIME Match
    b. or a Perfect S/MIME-Match and not even a Partial OpenPGP-Match
    c. or a Perfect OpenPGP-Match and preselected protocol=OpenPGP
    d. or a Perfect S/MIME-Match and preselected protocol=S/MIME
  3. For Protocol \in {OpenPGP,S/MIME}, a Perfect Protocol-Match is defined as:
    a. If signing, \foreach Sender, there is exactly one
         Matching Protocol-Certificate with
     i. can-sign=true
     ii. has-secret=true
    b. and, if encrypting, \foreach Recipient, there is exactly one
         Matching Protocol-Certificate with
     i. can-encrypt=true
     ii. (validity is not considered, cf. msg 24059)
  4. For Protocol \in {OpenPGP,S/MIME}, a Partial Protocol-Match is defined as:
    a. If signing, \foreach Sender, there is at least one
         Matching Protocol-Certificate with
     i. can-sign=true
     ii. has-secret=true
    b. and, if encrypting, \foreach Recipient, there is at least
     one Matching Protocol-Certificate with
     i. can-encrypt=true
     ii. (validity is not considered, cf. msg 24059)
  5. For Protocol \in {OpenPGP,S/MIME}, a Matching Protocol-Certificate is
     defined as matching by email-address. A revoked, disabled, or expired
     certificate is not considered a match.
  6. Sender is defined as those mailboxes that have been set with the SENDER
     command.
  7. Recipient is defined as those mailboxes that have been set with either the
     SENDER or the RECIPIENT commands.
*/

namespace {

    struct count_signing_certificates {
        typedef size_t result_type;
        const Protocol proto;
        explicit count_signing_certificates( Protocol proto ) : proto( proto ) {}
        size_t operator()( const Sender & sender ) const {
            const size_t result = sender.signingCertificateCandidates( proto ).size();
            qDebug( "count_signing_certificates( %9s %20s ) == %2lu",
                    proto == OpenPGP ? "OpenPGP," : proto == CMS ? "CMS," : "<unknown>,",
                    qPrintable( sender.mailbox().prettyAddress() ), result );
            return result;
        }
    };

    struct count_encrypt_certificates {
        typedef size_t result_type;
        const Protocol proto;
        explicit count_encrypt_certificates( Protocol proto ) : proto( proto ) {}
        size_t operator()( const Sender & sender ) const {
            const size_t result = sender.encryptToSelfCertificateCandidates( proto ).size();
            qDebug( "count_encrypt_certificates( %9s %20s ) == %2lu",
                    proto == OpenPGP ? "OpenPGP," : proto == CMS ? "CMS," : "<unknown>,",
                    qPrintable( sender.mailbox().prettyAddress() ), result );
            return result;
        }

        size_t operator()( const Recipient & recipient ) const {
            const size_t result = recipient.encryptionCertificateCandidates( proto ).size();
            qDebug( "count_encrypt_certificates( %9s %20s ) == %2lu",
                    proto == OpenPGP ? "OpenPGP," : proto == CMS ? "CMS," : "<unknown>,",
                    qPrintable( recipient.mailbox().prettyAddress() ), result );
            return result;
        }
    };

}

static bool has_perfect_match( bool sign, bool encrypt, Protocol proto, const std::vector<Sender> & senders, const std::vector<Recipient> & recipients ) {
    if ( sign )
        if ( !kdtools::all( senders,    boost::bind( count_signing_certificates( proto ), _1 ) == 1 ) )
            return false;
    if ( encrypt )
        if ( !kdtools::all( senders,    boost::bind( count_encrypt_certificates( proto ), _1 ) == 1 ) ||
             !kdtools::all( recipients, boost::bind( count_encrypt_certificates( proto ), _1 ) == 1 ) )
            return false;
    return true;
}

static bool has_partial_match( bool sign, bool encrypt, Protocol proto, const std::vector<Sender> & senders, const std::vector<Recipient> & recipients ) {
    if ( sign )
        if ( !kdtools::all( senders,    boost::bind( count_signing_certificates( proto ), _1 ) >= 1 ) )
            return false;
    if ( encrypt )
        if ( !kdtools::all( senders,    boost::bind( count_encrypt_certificates( proto ), _1 ) >= 1 ) ||
             !kdtools::all( recipients, boost::bind( count_encrypt_certificates( proto ), _1 ) >= 1 ) )
            return false;
    return true;
}

static bool has_perfect_overall_match( bool sign, bool encrypt, const std::vector<Sender> & senders, const std::vector<Recipient> & recipients, Protocol presetProtocol ) {
    return presetProtocol == OpenPGP   &&   has_perfect_match( sign, encrypt, OpenPGP, senders, recipients )
        || presetProtocol == CMS       &&   has_perfect_match( sign, encrypt, CMS,     senders, recipients )
        || has_perfect_match( sign, encrypt, OpenPGP, senders, recipients )   &&   !has_partial_match( sign, encrypt, CMS,     senders, recipients )
        || has_perfect_match( sign, encrypt, CMS,     senders, recipients )   &&   !has_partial_match( sign, encrypt, OpenPGP, senders, recipients ) ;
}

static bool has_conflict( bool sign, bool encrypt, const std::vector<Sender> & senders, const std::vector<Recipient> & recipients, Protocol presetProtocol ) {
    return !has_perfect_overall_match( sign, encrypt, senders, recipients, presetProtocol );
}

//
// END Conflict Detection
//

static std::vector<Sender> mailbox2sender( const std::vector<Mailbox> & mbs ) {
    std::vector<Sender> senders;
    senders.reserve( mbs.size() );
    Q_FOREACH( const Mailbox & mb, mbs )
        senders.push_back( Sender( mb ) );
    return senders;
}

static std::vector<Recipient> mailbox2recipient( const std::vector<Mailbox> & mbs ) {
    std::vector<Recipient> recipients;
    recipients.reserve( mbs.size() );
    Q_FOREACH( const Mailbox & mb, mbs )
        recipients.push_back( Recipient( mb ) );
    return recipients;
}

class NewSignEncryptEMailController::Private {
    friend class ::Kleo::Crypto::NewSignEncryptEMailController;
    NewSignEncryptEMailController * const q;
public:
    explicit Private( NewSignEncryptEMailController * qq );
    ~Private();

private:
    void slotDialogAccepted();
    void slotDialogRejected();

private:
    void ensureDialogVisible();
    void cancelAllTasks();

    void startSigning();
    void startEncryption();
    void schedule();
    shared_ptr<Task> takeRunnable( GpgME::Protocol proto );

private:
    bool sign : 1;
    bool encrypt : 1;
    bool resolvingInProgress : 1;
    bool certificatesResolved : 1;
    bool detached : 1;
    Protocol presetProtocol;
    std::vector<Key> signers, recipients;
    std::vector< shared_ptr<Task> > runnable, completed;
    shared_ptr<Task> cms, openpgp;
    QPointer<SignEncryptEMailConflictDialog> dialog;
};

NewSignEncryptEMailController::Private::Private( NewSignEncryptEMailController * qq )
    : q( qq ),
      sign( false ),
      encrypt( false ),
      resolvingInProgress( false ),
      certificatesResolved( false ),
      detached( false ),
      presetProtocol( UnknownProtocol ),
      signers(),
      recipients(),
      runnable(),
      cms(),
      openpgp(),
      dialog( new SignEncryptEMailConflictDialog )
{
    connect( dialog, SIGNAL(accepted()), q, SLOT(slotDialogAccepted()) );
    connect( dialog, SIGNAL(rejected()), q, SLOT(slotDialogRejected()) );
}

NewSignEncryptEMailController::Private::~Private() {
    delete dialog;
}

NewSignEncryptEMailController::NewSignEncryptEMailController( const shared_ptr<ExecutionContext> & xc, QObject * p )
    : Controller( xc, p ), d( new Private( this ) )
{

}

NewSignEncryptEMailController::NewSignEncryptEMailController( QObject * p )
    : Controller( p ), d( new Private( this ) )
{

}

NewSignEncryptEMailController::~NewSignEncryptEMailController() { qDebug(); }

void NewSignEncryptEMailController::setSubject( const QString & subject ) {
    d->dialog->setSubject( subject );
}

void NewSignEncryptEMailController::setProtocol( Protocol proto ) {
    d->presetProtocol = proto;
    d->dialog->setPresetProtocol( proto );
}

Protocol NewSignEncryptEMailController::protocol() const {
    return d->dialog->selectedProtocol();
}


const char * NewSignEncryptEMailController::protocolAsString() const {
    switch ( protocol() ) {
    case OpenPGP: return "OpenPGP";
    case CMS:     return "CMS";
    default:
        throw Kleo::Exception( gpg_error( GPG_ERR_INTERNAL ),
                               i18n("Call to NewSignEncryptEMailController::protocolAsString() is ambiguous.") );
    }
}

void NewSignEncryptEMailController::setSigning( bool sign ) {
    d->sign = sign;
    d->dialog->setSign( sign );
}

bool NewSignEncryptEMailController::isSigning() const {
    return d->sign;
}

void NewSignEncryptEMailController::setEncrypting( bool encrypt ) {
    d->encrypt = encrypt;
    d->dialog->setEncrypt( encrypt );
}

bool NewSignEncryptEMailController::isEncrypting() const {
    return d->encrypt;
}

void NewSignEncryptEMailController::setDetachedSignature( bool detached ) {
    d->detached = detached;
}

bool NewSignEncryptEMailController::isResolvingInProgress() const {
    return d->resolvingInProgress;
}

bool NewSignEncryptEMailController::areCertificatesResolved() const {
    return d->certificatesResolved;
}

static bool is_dialog_quick_mode( bool sign, bool encrypt ) {
    const EMailOperationsPreferences prefs;
    return ( !sign    || prefs.quickSignEMail() )
        && ( !encrypt || prefs.quickEncryptEMail() )
        ;
}

static void save_dialog_quick_mode( bool on ) {
    EMailOperationsPreferences prefs;
    prefs.setQuickSignEMail( on );
    prefs.setQuickEncryptEMail( on );
    prefs.save();
}

void NewSignEncryptEMailController::startResolveCertificates( const std::vector<Mailbox> & r, const std::vector<Mailbox> & s ) {
    d->certificatesResolved = false;
    d->resolvingInProgress = true;

    const std::vector<Sender> senders = mailbox2sender( s );
    const std::vector<Recipient> recipients = mailbox2recipient( r );
    const bool quickMode = is_dialog_quick_mode( d->sign, d->encrypt );

    const bool conflict = quickMode && has_conflict( d->sign, d->encrypt, senders, recipients, d->presetProtocol );

    d->dialog->setQuickMode( quickMode );
    d->dialog->setSenders( senders );
    d->dialog->setRecipients( recipients );
    d->dialog->pickProtocol();
    d->dialog->setConflict( conflict );

    if ( quickMode && !conflict )
        QMetaObject::invokeMethod( this, "slotDialogAccepted", Qt::QueuedConnection );
    else
        d->ensureDialogVisible();
}

void NewSignEncryptEMailController::Private::slotDialogAccepted() {
    if ( dialog->isQuickMode() != is_dialog_quick_mode( sign, encrypt ) )
        save_dialog_quick_mode( dialog->isQuickMode() );
    resolvingInProgress = false;
    certificatesResolved = true;
    signers = dialog->resolvedSigningKeys();
    recipients = dialog->resolvedEncryptionKeys();
    QMetaObject::invokeMethod( q, "certificatesResolved", Qt::QueuedConnection );
}

void NewSignEncryptEMailController::Private::slotDialogRejected() {
    resolvingInProgress = false;
    certificatesResolved = false;
    QMetaObject::invokeMethod( q, "error", Qt::QueuedConnection,
                               Q_ARG( int, gpg_error( GPG_ERR_CANCELED ) ),
                               Q_ARG( QString, i18n("User cancel") ) );
}

void NewSignEncryptEMailController::startEncryption( const std::vector< shared_ptr<Input> > & inputs, const std::vector< shared_ptr<Output> > & outputs ) {

    kleo_assert( d->encrypt );
    kleo_assert( !d->resolvingInProgress );

    kleo_assert( !inputs.empty() );
    kleo_assert( outputs.size() == inputs.size() );

    std::vector< shared_ptr<Task> > tasks;
    tasks.reserve( inputs.size() );

    kleo_assert( !d->recipients.empty() );

    for ( unsigned int i = 0, end = inputs.size() ; i < end ; ++i ) {

        const shared_ptr<EncryptEMailTask> task( new EncryptEMailTask );

        task->setInput( inputs[i] );
        task->setOutput( outputs[i] );
        task->setRecipients( d->recipients );

        tasks.push_back( task );
    }

    // append to runnable stack
    d->runnable.insert( d->runnable.end(), tasks.begin(), tasks.end() );

    d->startEncryption();
}

void NewSignEncryptEMailController::Private::startEncryption() {
    shared_ptr<TaskCollection> coll( new TaskCollection );
    const std::vector<shared_ptr<Task> > tmp
        = kdtools::copy< std::vector<shared_ptr<Task> > >( runnable );
    coll->setTasks( tmp );
#if 0
#warning use a new result dialog
    // ### use a new result dialog
    dialog->setTaskCollection( coll );
#endif
    Q_FOREACH( const shared_ptr<Task> & t, tmp )
        q->connectTask( t );
    schedule();
}

void NewSignEncryptEMailController::startSigning( const std::vector< shared_ptr<Input> > & inputs, const std::vector< shared_ptr<Output> > & outputs ) {

    kleo_assert( d->sign );
    kleo_assert( !d->resolvingInProgress );

    kleo_assert( !inputs.empty() );
    kleo_assert( !outputs.empty() );

    std::vector< shared_ptr<Task> > tasks;
    tasks.reserve( inputs.size() );

    kleo_assert( !d->signers.empty() );
    kleo_assert( kdtools::none_of( d->signers, mem_fn( &Key::isNull ) ) );

    for ( unsigned int i = 0, end = inputs.size() ; i < end ; ++i ) {

        const shared_ptr<SignEMailTask> task( new SignEMailTask );

        task->setInput( inputs[i] );
        task->setOutput( outputs[i] );
        task->setSigners( d->signers );
        task->setDetachedSignature( d->detached );

        tasks.push_back( task );
    }

    // append to runnable stack
    d->runnable.insert( d->runnable.end(), tasks.begin(), tasks.end() );

    d->startSigning();
}

void NewSignEncryptEMailController::Private::startSigning() {
    shared_ptr<TaskCollection> coll( new TaskCollection );
    const std::vector<shared_ptr<Task> > tmp
        = kdtools::copy< std::vector<shared_ptr<Task> > >( runnable );
    coll->setTasks( tmp );
#if 0
#warning use a new result dialog
    // ### use a new result dialog
    dialog->setTaskCollection( coll );
#endif
    Q_FOREACH( const shared_ptr<Task> & t, tmp )
        q->connectTask( t );
    schedule();
}

void NewSignEncryptEMailController::Private::schedule() {

    if ( !cms )
        if ( const shared_ptr<Task> t = takeRunnable( CMS ) ) {
            t->start();
            cms = t;
        }

    if ( !openpgp )
        if ( const shared_ptr<Task> t = takeRunnable( OpenPGP ) ) {
            t->start();
            openpgp = t;
        }

    if ( cms || openpgp )
        return;
    kleo_assert( runnable.empty() );
    q->emitDoneOrError();
}

shared_ptr<Task> NewSignEncryptEMailController::Private::takeRunnable( GpgME::Protocol proto ) {
    const std::vector< shared_ptr<Task> >::iterator it
        = std::find_if( runnable.begin(), runnable.end(),
                        boost::bind( &Task::protocol, _1 ) == proto );
    if ( it == runnable.end() )
        return shared_ptr<Task>();

    const shared_ptr<Task> result = *it;
    runnable.erase( it );
    return result;
}

void NewSignEncryptEMailController::doTaskDone( const Task * task, const shared_ptr<const Task::Result> & result ) {
    assert( task );

    if ( result && result->hasError() ) {
        QPointer<QObject> that = this;
        if ( result->details().isEmpty() )
            KMessageBox::        sorry( 0,
                                        result->overview(),
                                        i18nc("@title:window","Error") );
        else
            KMessageBox::detailedSorry( 0,
                                        result->overview(),
                                        result->details(),
                                        i18nc("@title:window","Error") );
        if ( !that )
            return;
    }

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

void NewSignEncryptEMailController::cancel() {
    try {
        d->dialog->close();
        d->cancelAllTasks();
    } catch ( const std::exception & e ) {
        qDebug() << "Caught exception: " << e.what();
    }
}

void NewSignEncryptEMailController::Private::cancelAllTasks() {

    // we just kill all runnable tasks - this will not result in
    // signal emissions.
    runnable.clear();

    // a cancel() will result in a call to
    if ( cms )
        cms->cancel();
    if ( openpgp )
        openpgp->cancel();
}

void NewSignEncryptEMailController::Private::ensureDialogVisible() {
    q->bringToForeground( dialog );
}

#include "moc_newsignencryptemailcontroller.cpp"
