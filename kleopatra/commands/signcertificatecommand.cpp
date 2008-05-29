/* -*- mode: c++; c-basic-offset:4 -*-
    commands/signcertificatecommand.cpp

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
    along with this program; if not, write to the Free Softwarls   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#include "signcertificatecommand.h"

#include "command_p.h"

#include <dialogs/signcertificatedialog.h>

#include <models/keycache.h>

#include <utils/formatting.h>

#include <kleo/cryptobackendfactory.h>
#include <kleo/cryptobackend.h>
#include <kleo/signkeyjob.h>

#include <gpgme++/key.h>

#include <KLocale>
#include <KMessageBox>
#include <kdebug.h>

#include <boost/bind.hpp>

#include <cassert>

using namespace Kleo;
using namespace Kleo::Commands;
using namespace Kleo::Dialogs;
using namespace GpgME;

class SignCertificateCommand::Private : public Command::Private {
    friend class ::Kleo::Commands::SignCertificateCommand;
    SignCertificateCommand * q_func() const { return static_cast<SignCertificateCommand*>( q ); }
public:
    explicit Private( SignCertificateCommand * qq, KeyListController * c );
    ~Private();

    void init();

private:
    void slotDialogRejected();
    void slotResult( const Error & err );
    void slotCertificationPrepared();

private:
    void ensureDialogCreated();
    void createJob();
    void showErrorDialog( const Error & error );
    void showSuccessDialog();

private:
    std::vector<UserID> uids;
    QPointer<SignCertificateDialog> dialog;
    QPointer<SignKeyJob> job;
};


SignCertificateCommand::Private * SignCertificateCommand::d_func() { return static_cast<Private*>( d.get() ); }
const SignCertificateCommand::Private * SignCertificateCommand::d_func() const { return static_cast<const Private*>( d.get() ); }

#define d d_func()
#define q q_func()

SignCertificateCommand::Private::Private( SignCertificateCommand * qq, KeyListController * c )
    : Command::Private( qq, c ),
      uids(),
      dialog(),
      job()
{

}

SignCertificateCommand::Private::~Private() { kDebug(); }

SignCertificateCommand::SignCertificateCommand( KeyListController * c )
    : Command( new Private( this, c ) )
{
    d->init();
}

SignCertificateCommand::SignCertificateCommand( QAbstractItemView * v, KeyListController * c )
    : Command( v, new Private( this, c ) )
{
    d->init();
}

SignCertificateCommand::SignCertificateCommand( const Key & key )
    : Command( key, new Private( this, 0 ) )
{
    d->init();
}

SignCertificateCommand::SignCertificateCommand( const UserID & uid )
    : Command( uid.parent(), new Private( this, 0 ) )
{
    std::vector<UserID>( 1, uid ).swap( d->uids );
    d->init();
}

SignCertificateCommand::SignCertificateCommand( const std::vector<UserID> & uids )
    : Command( uids.empty() ? Key() : uids.front().parent(), new Private( this, 0 ) )
{
    d->uids = uids;
    d->init();
}

void SignCertificateCommand::Private::init() {

}

SignCertificateCommand::~SignCertificateCommand() { kDebug(); }

void SignCertificateCommand::setSignatureExportable( bool on ) {

}

void SignCertificateCommand::setSignatureRevocable( bool on ) {

}

void SignCertificateCommand::setSigningKey( const Key & signer ) {

}

void SignCertificateCommand::setUserIDs( const std::vector<UserID> & uids ) {
    d->uids = uids;
    if ( !uids.empty() && d->key().isNull() )
        setKey( uids.front().parent() );
}

void SignCertificateCommand::setUserID( const UserID & uid ) {
    setUserIDs( std::vector<UserID>( 1, uid ) );
}

void SignCertificateCommand::doStart() {

    const std::vector<Key> keys = d->keys();
    if ( keys.size() != 1 ||
         keys.front().protocol() != GpgME::OpenPGP ) {
        d->finished();
        return;
    }

    std::vector<Key> secKeys = KeyCache::instance()->secretKeys();
    std::vector<Key>::iterator it = std::remove_if( secKeys.begin(), secKeys.end(), !bind( &Key::canCertify, _1 ) );
    it = std::remove_if( it, secKeys.end(), bind( &Key::protocol, _1 ) != OpenPGP );
    secKeys.erase( it );

    if ( secKeys.empty() ) {
        KMessageBox::error( d->view(),
                            i18n( "To certify other certificates, you first need to create an OpenPGP certificate for yourself. Choose <interface>File->New Certificate...</interface> to create one." ),
                            i18n( "Certification Not Possible" ) );
        d->finished();
        return;
    }
    const Key & key = keys.front();

    Q_FOREACH( const UserID & uid, d->uids )
        if ( qstricmp( uid.parent().primaryFingerprint(), key.primaryFingerprint() ) != 0 ) {
            kWarning() << "User-ID <-> Key mismatch!";
            d->finished();
            return;
        }

    d->ensureDialogCreated();
    //d->dialog->setKey( key );
    //d->dialog->setUserIDs( uids );
    assert( d->dialog );
    d->dialog->setCertificateToCertify( d->key() );
    d->dialog->setCertificatesWithSecretKeys( secKeys );
    d->dialog->show();
}

void SignCertificateCommand::Private::slotDialogRejected() {
    emit q->canceled();
    finished();
}

void SignCertificateCommand::Private::slotResult( const Error & err ) {
#if 0
    if ( err.isCanceled() )
        ;
    else if ( err )
        showErrorDialog( err );
    else
        showSuccessDialog();
#endif
    finished();
}

void SignCertificateCommand::Private::slotCertificationPrepared() {
    assert( dialog );

    const SignKeyJob::SigningOption opt = dialog->signingOption();

    createJob();
    assert( job );
    dialog->connectJob( job );

#if 0
    if ( const Error err = job->start( key(), std::vector<unsigned int>(), Key(), 0u, opt ) ) {
        dialog->setError( err );
        finished();
    }
#endif
}

void SignCertificateCommand::doCancel() {
    kDebug();
    if ( d->job )
        d->job->slotCancel();
}

void SignCertificateCommand::Private::ensureDialogCreated() {
    if ( dialog )
        return;

    dialog = new SignCertificateDialog( view() );
    dialog->setAttribute( Qt::WA_DeleteOnClose );

    connect( dialog, SIGNAL(rejected()), q, SLOT(slotDialogRejected()) );
    connect( dialog, SIGNAL(certificationPrepared()), q, SLOT(slotCertificationPrepared()) );
}

void SignCertificateCommand::Private::createJob() {
    assert( !job );

    assert( key().protocol() == OpenPGP );
    const CryptoBackend::Protocol * const backend = CryptoBackendFactory::instance()->protocol( key().protocol() );
    if ( !backend )
        return;

    SignKeyJob * const j = backend->signKeyJob();
    if ( !j )
        return;

    connect( j, SIGNAL(progress(QString,int,int)),
             q, SIGNAL(progress(QString,int,int)) );
    connect( j, SIGNAL(result(GpgME::Error)),
             q, SLOT(slotResult(GpgME::Error)) );

    job = j;
}

void SignCertificateCommand::Private::showErrorDialog( const Error & err ) {
    KMessageBox::error( view(),
                        i18n("<p>An error occurred while trying to sign the certificate <b>%1</b>:</p><p>%2</p>",
                             Formatting::formatForComboBox( key() ),
                             QString::fromLocal8Bit( err.asString() ) ),
                        i18n("Certificate Signing Error") );
}

void SignCertificateCommand::Private::showSuccessDialog() {
    KMessageBox::information( view(),
                              i18n("Certificate successfully signed."),
                              i18n("Signing Certificate Succeeded") );
}

#undef d
#undef q

#include "moc_signcertificatecommand.cpp"
