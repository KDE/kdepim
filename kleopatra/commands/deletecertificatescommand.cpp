/* -*- mode: c++; c-basic-offset:4 -*-
    deleteCertificatescommand.cpp

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

#include "deletecertificatescommand.h"

#include "command_p.h"

#include <models/keycache.h>
#include <models/predicates.h>

#include <utils/stl_util.h>

#include <kleo/cryptobackend.h>
#include <kleo/cryptobackendfactory.h>
#include <kleo/multideletejob.h>
#include <kleo/deletejob.h>

#include <gpgme++/key.h>

#include <KLocale>
#include <KMessageBox>

#include <QPointer>
#include <QAbstractItemView>

#include <boost/bind.hpp>

#include <algorithm>
#include <vector>
#include <cassert>

using namespace boost;
using namespace GpgME;
using namespace Kleo;

class DeleteCertificatesCommand::Private : public Command::Private {
    friend class ::Kleo::DeleteCertificatesCommand;
    DeleteCertificatesCommand * q_func() const { return static_cast<DeleteCertificatesCommand*>(q); }
public:
    explicit Private( DeleteCertificatesCommand * qq, KeyListController * c );
    ~Private();

    void startDeleteJob( GpgME::Protocol protocol );

    void cancelJobs();
    void pgpDeleteResult( const GpgME::Error & );
    void cmsDeleteResult( const GpgME::Error & );
    void showErrorsAndFinish();

    bool canDelete( GpgME::Protocol proto ) const {
        if ( const CryptoBackend::Protocol * const cbp = CryptoBackendFactory::instance()->protocol( proto ) )
            if ( DeleteJob * const job = cbp->deleteJob() ) {
                job->slotCancel();
                return true;
            }
        return false;
    }

private:
    QPointer<MultiDeleteJob> cmsJob, pgpJob;
    GpgME::Error cmsError, pgpError;
    std::vector<Key> cmsKeys, pgpKeys;
};

DeleteCertificatesCommand::Private * DeleteCertificatesCommand::d_func() { return static_cast<Private*>(d.get()); }
const DeleteCertificatesCommand::Private * DeleteCertificatesCommand::d_func() const { return static_cast<const Private*>(d.get()); }

#define d d_func()
#define q q_func()

DeleteCertificatesCommand::Private::Private( DeleteCertificatesCommand * qq, KeyListController * c )
    : Command::Private( qq, c )
{
    
}

DeleteCertificatesCommand::Private::~Private() {}


DeleteCertificatesCommand::DeleteCertificatesCommand( KeyListController * p )
    : Command( new Private( this, p ) )
{
    
}

DeleteCertificatesCommand::DeleteCertificatesCommand( QAbstractItemView * v, KeyListController * p )
    : Command( v, new Private( this, p ) )
{
    
}

DeleteCertificatesCommand::~DeleteCertificatesCommand() {}

namespace {

    enum Action { Nothing = 0, Failure = 1, ClearCMS = 2, ClearPGP = 4 };

    // const unsigned int errorCase =
    //    openpgp.empty() << 3U | d->canDelete( OpenPGP ) << 2U |
    //        cms.empty() << 1U |     d->canDelete( CMS ) << 0U ;

    static const struct {
        const char * text;
        Action actions;
    } deletionErrorCases[16] = {
        // if havePGP
        //   if cantPGP
        //     if haveCMS
        { I18N_NOOP( "Neither the OpenPGP nor the CMS "
                     "backends support certificate deletion.\n"
                     "Check your installation." ),
          Failure }, // cantCMS
        { I18N_NOOP( "The OpenPGP backend does not support "
                     "certificate deletion.\n"
                     "Check your installation.\n"
                     "Only the selected CMS certificates "
                     "will be deleted." ),
          ClearPGP }, // canCMS
        //     if !haveCMS
        { I18N_NOOP( "The OpenPGP backend does not support "
                     "certificate deletion.\n"
                     "Check your installation." ),
          Failure },
        { I18N_NOOP( "The OpenPGP backend does not support "
                     "certificate deletion.\n"
                     "Check your installation." ),
          Failure },
        //   if canPGP
        //      if haveCMS
        { I18N_NOOP( "The CMS backend does not support "
                     "certificate deletion.\n"
                     "Check your installation.\n"
                     "Only the selected OpenPGP certificates "
                     "will be deleted." ),
          ClearCMS }, // cantCMS
        { 0,
          Nothing }, // canCMS
        //      if !haveCMS
        { 0,
          Nothing }, // cantCMS
        { 0,
          Nothing }, // canCMS
        // if !havePGP
        //   if cantPGP
        //     if haveCMS
        { I18N_NOOP( "The CMS backend does not support "
                     "certificate deletion.\n"
                     "Check your installation." ),
          Failure }, // cantCMS
        { 0,
          Nothing }, // canCMS
        //     if !haveCMS
        { 0,
          Nothing }, // cantCMS
        { 0,
          Nothing }, // canCMS
        //  if canPGP
        //     if haveCMS
        { I18N_NOOP( "The CMS backend does not support "
                     "certificate deletion.\n"
                     "Check your installation." ),
          Failure }, // cantCMS
        { 0,
          Nothing }, // canCMS
        //     if !haveCMS
        { 0,
          Nothing }, // cantCMS
        { 0,
          Nothing }, // canCMS
    };
} // anon namespace

void DeleteCertificatesCommand::doStart() {

    std::vector<Key> keys = d->keys();
    if ( keys.empty() ) {
        d->finished();
        return;
    }

    // 1. Remove secret keys:

    std::vector<Key>::iterator keysEnd
        = std::remove_if( keys.begin(), keys.end(), bind( &Key::hasSecret, _1 ) );

    if ( keysEnd == keys.begin() ) {
        KMessageBox::information( d->parentWidgetOrView(),
                                  i18np("The certificate to be deleted is your own. "
                                        "It contains private key material, "
                                        "which is needed to decrypt past communication "
                                        "encrypted to the certificate, and can therefore "
                                        "not be deleted.",

                                        "All of the certificates to be deleted "
                                        "are your own. "
                                        "They contain private key material, "
                                        "which is needed to decrypt past communication "
                                        "encrypted to the certificate, and can therefore "
                                        "not be deleted.",

                                        keys.size() ),
                                  i18n("Secret Key Deletion") );
        d->finished();
        return;
    }

    const bool hadSecretKeys = keysEnd != keys.end();

    // 2. Remove issuers of secret keys:

    const std::vector<Key> issuersOfSecretKeys = KeyCache::instance()->findIssuers( KeyCache::instance()->keys() );

    std::sort( keys.begin(), keysEnd, _detail::ByFingerprint<std::less>() );

    const std::vector<Key>::iterator it
        = std::set_difference( keys.begin(), keysEnd,
                               issuersOfSecretKeys.begin(), issuersOfSecretKeys.end(),
                               keys.begin(),
                               _detail::ByFingerprint<std::less>() );

    if ( it == keys.begin() ) {
        KMessageBox::information( d->parentWidgetOrView(),
                                  hadSecretKeys ?
                                  i18n("All of the certificates to be deleted "
                                       "are either your own, or are issuers of one of your own certificates. "
                                       "Your own certificates contain private key material, "
                                       "which is needed to decrypt past communication "
                                       "encrypted to the certificate. They can therefore not be deleted, "
                                       "and neither can their issuers.")
                                  :
                                  i18np("The certificate to be deleted "
                                        "is an issuer of one of your own certificates. "
                                        "Your own certificates contain private key material, "
                                        "which is needed to decrypt past communication "
                                        "encrypted to the certificate. They can therefore not be deleted, "
                                        "and neither can their issuers.",
                                        
                                        "All of the certificates to be deleted "
                                        "are issuers of one of your own certificates. "
                                        "Your own certificates contain private key material, "
                                        "which is needed to decrypt past communication "
                                        "encrypted to the certificate. They can therefore not be deleted, "
                                        "and neither can their issuers.",

                                        keys.size() ),
                                  i18n("Secret Key Deletion") );
        d->finished();
        return;
    }

    const bool hadSecretKeyIssuers = it != keysEnd;
    keysEnd = it;

    if ( hadSecretKeys || hadSecretKeyIssuers )
        if ( KMessageBox::Continue != 
             KMessageBox::warningContinueCancel( d->parentWidgetOrView(),
                                                 hadSecretKeys ?
                                                 hadSecretKeyIssuers ?
                                                 i18n("Some of the certificates to be deleted "
                                                      "are your own, or are issuers of one of your own certificates. "
                                                      "Your own certificates contain private key material, "
                                                      "which is needed to decrypt past communication "
                                                      "encrypted to the certificate. They can therefore not be deleted, "
                                                      "and neither can their issuers.\n"
                                                      "If you choose to continue, your own, "
                                                      "as well as any issuer certificates, will be "
                                                      "skipped from deletion.")
                                                 :
                                                 i18n("Some of the certificates to be deleted "
                                                      "are your own. "
                                                      "They contain private key material, "
                                                      "which is needed to decrypt past communication "
                                                      "encrypted to the certificate. They can therefore not be deleted.\n"
                                                      "If you choose to continue, they will be "
                                                      "skipped from deletion.")
                                                 :
                                                 i18n("Some of the certificates to be deleted "
                                                      "are issuers of one of your own certificates. "
                                                      "Your own certificates contain private key material, "
                                                      "which is needed to decrypt past communication "
                                                      "encrypted to the certificate. They can therefore not be deleted, "
                                                      "and neither can their issuers.\n"
                                                      "If you choose to continue, they will be "
                                                      "skipped from deletion."),
                                                 i18n("Secret Key Deletion" ) ) )
        {
            emit canceled();
            d->finished();
            return;
        }

    std::sort( keys.begin(), keysEnd, _detail::ByFingerprint<std::less>() );

    std::vector<Key>::iterator
        pgpBegin = keys.begin(),
        pgpEnd = std::stable_partition( pgpBegin, keysEnd,
                                        bind( &GpgME::Key::protocol, _1 ) != CMS ),
        cmsBegin = pgpEnd,
        cmsEnd = std::set_difference( cmsBegin, keysEnd,
                                      issuersOfSecretKeys.begin(), issuersOfSecretKeys.end(),
                                      cmsBegin,
                                      _detail::ByFingerprint<std::less>() );

    const std::vector<Key> subjects = KeyCache::instance()->findSubjects( cmsBegin, cmsEnd );

    assert( !kdtools::any( subjects.begin(), subjects.end(), bind( &Key::hasSecret, _1 ) ) );

    std::vector<Key> cms;
    cms.reserve( cmsEnd - cmsBegin + subjects.size() );
    std::set_union( cmsBegin, cmsEnd,
                    subjects.begin(), subjects.end(),
                    std::back_inserter( cms ),
                    _detail::ByFingerprint<std::less>() );

    std::vector<Key> openpgp( pgpBegin, pgpEnd ); 

    if ( cms.size() > static_cast<std::size_t>( cmsEnd - cmsBegin ) &&
         KMessageBox::warningContinueCancel( d->parentWidgetOrView(),
					     i18n("Some or all of the selected "
						  "certificates are issuers (CA certificates) "
						  "for other, non-selected certificates.\n"
						  "Deleting a CA certificate will also delete "
						  "all certificates issued by it."),
					     i18n("Deleting CA Certificates") )
	 != KMessageBox::Continue ) {
        emit canceled();
        d->finished();
        return;
    }

    assert( !kdtools::any( openpgp.begin(), openpgp.end(), bind( &Key::hasSecret, _1 ) ) );
    assert( !kdtools::any( cms.begin(), cms.end(), bind( &Key::hasSecret, _1 ) ) );

    const unsigned int errorCase =
        openpgp.empty() << 3U | d->canDelete( OpenPGP ) << 2U |
            cms.empty() << 1U |     d->canDelete( CMS ) << 0U ;

    if ( const unsigned int actions = deletionErrorCases[errorCase].actions ) {
        KMessageBox::information( d->parentWidgetOrView(),
                                  i18n( deletionErrorCases[errorCase].text ),
                                  (actions & Failure)
                                  ? i18n( "Certificate Deletion Failed" )
                                  : i18n( "Certificate Deletion Problem" ) );
        if ( actions & ClearCMS )
            cms.clear();
        if ( actions & ClearPGP )
            openpgp.clear();
        if ( actions & Failure ) {
            emit canceled();
            d->finished();
            return;
        }
    }

    assert( !openpgp.empty() || !cms.empty() );

    d->pgpKeys.swap( openpgp );
    d->cmsKeys.swap( cms );

    if ( !d->pgpKeys.empty() )
        d->startDeleteJob( GpgME::OpenPGP );
    if ( !d->cmsKeys.empty() )
        d->startDeleteJob( GpgME::CMS );

    if ( ( d->pgpKeys.empty() || d->pgpError.code() ) &&
         ( d->cmsKeys.empty() || d->cmsError.code() ) )
        d->showErrorsAndFinish();
}
    
void DeleteCertificatesCommand::Private::startDeleteJob( GpgME::Protocol protocol ) {
    assert( protocol != GpgME::UnknownProtocol );

    const std::vector<Key> & keys = protocol == CMS ? cmsKeys : pgpKeys ;

    const CryptoBackend::Protocol * const backend = CryptoBackendFactory::instance()->protocol( protocol );
    assert( backend );

    std::auto_ptr<MultiDeleteJob> job( new MultiDeleteJob( backend ) );

    connect( job.get(), SIGNAL(result(GpgME::Error,GpgME::Key)),
             q, protocol == CMS ? SLOT(cmsDeleteResult(GpgME::Error)) : SLOT(pgpDeleteResult(GpgME::Error)) );

    connect( job.get(), SIGNAL(progress(QString,int,int)),
             q, SIGNAL(progress(QString,int,int)) );

    if ( const Error err = job->start( keys ) )
        ( protocol == CMS ? cmsError : pgpError ) = err;
    else
        ( protocol == CMS ? cmsJob : pgpJob ) = job.release();
}

void DeleteCertificatesCommand::Private::showErrorsAndFinish() {

    assert( !pgpJob ); assert( !cmsJob );

    if ( pgpError || cmsError ) {
        QString pgpErrorString;
        if ( pgpError )
            pgpErrorString = i18n( "OpenPGP backend: %1", QString::fromLocal8Bit( pgpError.asString() ) );
        QString cmsErrorString;
        if ( cmsError )
            cmsErrorString = i18n( "CMS backend: %1", QString::fromLocal8Bit( cmsError.asString() ) );

        const QString msg = i18n("<qt><p>An error occurred while trying to delete "
                                 "the certificate:</p>"
                                 "<p><b>%1</b></p></qt>",
                                 pgpError ? cmsError ? pgpErrorString + "</br>" + cmsErrorString : pgpErrorString : cmsErrorString );
        KMessageBox::error( parentWidgetOrView(), msg, i18n("Certificate Deletion Failed") );
    } else {
        std::vector<Key> keys = pgpKeys;
        keys.insert( keys.end(), cmsKeys.begin(), cmsKeys.end() );
        KeyCache::mutableInstance()->remove( keys );
    }

    finished();
}

void DeleteCertificatesCommand::doCancel() {
    d->cancelJobs();
}

void DeleteCertificatesCommand::Private::pgpDeleteResult( const Error & err ) {
    pgpError = err;
    pgpJob = 0;
    if ( !cmsJob )
        showErrorsAndFinish();
}

void DeleteCertificatesCommand::Private::cmsDeleteResult( const Error & err ) {
    cmsError = err;
    cmsJob = 0;
    if ( !pgpJob )
        showErrorsAndFinish();
}

void DeleteCertificatesCommand::Private::cancelJobs()
{
    if ( cmsJob )
        cmsJob->slotCancel();
    if ( pgpJob )
        pgpJob->slotCancel();
}

#undef d
#undef q

#include "moc_deletecertificatescommand.cpp"
