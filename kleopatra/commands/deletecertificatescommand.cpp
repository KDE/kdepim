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

#include <dialogs/deletecertificatesdialog.h>

#include <models/keycache.h>
#include <models/predicates.h>

#include <kleo/stl_util.h>
#include <kleo/cryptobackend.h>
#include <kleo/cryptobackendfactory.h>
#include <kleo/multideletejob.h>
#include <kleo/deletejob.h>

#include <gpgme++/key.h>

#include <KLocale>

#include <QPointer>
#include <QAbstractItemView>

#include <boost/bind.hpp>

#include <algorithm>
#include <vector>
#include <cassert>

using namespace boost;
using namespace GpgME;
using namespace Kleo;
using namespace Kleo::Dialogs;

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

    void ensureDialogCreated() {
        if ( dialog )
            return;
        dialog = new DeleteCertificatesDialog;
        applyWindowID( dialog );
        dialog->setAttribute( Qt::WA_DeleteOnClose );
        dialog->setWindowTitle( i18nc("@title:window", "Delete Certificates") );
        connect( dialog, SIGNAL(accepted()), q_func(), SLOT(slotDialogAccepted()) );
        connect( dialog, SIGNAL(rejected()), q_func(), SLOT(slotDialogRejected()) );
    }
    void ensureDialogShown() {
        if ( dialog )
            dialog->show();
    }

    void slotDialogAccepted();
    void slotDialogRejected() {
        canceled();
    }

private:
    QPointer<DeleteCertificatesDialog> dialog;
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

    std::vector<Key> selected = d->keys();
    if ( selected.empty() ) {
        d->finished();
        return;
    }

    kdtools::sort( selected, _detail::ByFingerprint<std::less>() );

    // Calculate the closure of the selected keys (those that need to
    // be deleted with them, though not selected themselves):

    std::vector<Key> toBeDeleted = KeyCache::instance()->findSubjects( selected );
    kdtools::sort( toBeDeleted, _detail::ByFingerprint<std::less>() );

    std::vector<Key> unselected;
    unselected.reserve( toBeDeleted.size() );
    std::set_difference( toBeDeleted.begin(), toBeDeleted.end(),
                         selected.begin(), selected.end(),
                         std::back_inserter( unselected ),
                         _detail::ByFingerprint<std::less>() );

    d->ensureDialogCreated();

    d->dialog->setSelectedKeys( selected );
    d->dialog->setUnselectedKeys( unselected );

    d->ensureDialogShown();

}

void DeleteCertificatesCommand::Private::slotDialogAccepted() {
    std::vector<Key> keys = dialog->keys();
    assert( !keys.empty() );

    std::vector<Key>::iterator
        pgpBegin = keys.begin(),
        pgpEnd = std::stable_partition( pgpBegin, keys.end(),
                                        boost::bind( &GpgME::Key::protocol, _1 ) != CMS ),
        cmsBegin = pgpEnd,
        cmsEnd = keys.end() ;

    std::vector<Key> openpgp( pgpBegin, pgpEnd );
    std::vector<Key>     cms( cmsBegin, cmsEnd );

    const unsigned int errorCase =
        openpgp.empty() << 3U | canDelete( OpenPGP ) << 2U |
            cms.empty() << 1U |     canDelete( CMS ) << 0U ;

    if ( const unsigned int actions = deletionErrorCases[errorCase].actions ) {
        information( i18n( deletionErrorCases[errorCase].text ),
                     (actions & Failure)
                     ? i18n( "Certificate Deletion Failed" )
                     : i18n( "Certificate Deletion Problem" ) );
        if ( actions & ClearCMS )
            cms.clear();
        if ( actions & ClearPGP )
            openpgp.clear();
        if ( actions & Failure ) {
            canceled();
            return;
        }
    }

    assert( !openpgp.empty() || !cms.empty() );

    pgpKeys.swap( openpgp );
    cmsKeys.swap( cms );

    if ( !pgpKeys.empty() )
        startDeleteJob( GpgME::OpenPGP );
    if ( !cmsKeys.empty() )
        startDeleteJob( GpgME::CMS );

    if ( ( pgpKeys.empty() || pgpError.code() ) &&
         ( cmsKeys.empty() || cmsError.code() ) )
        showErrorsAndFinish();
}
    
void DeleteCertificatesCommand::Private::startDeleteJob( GpgME::Protocol protocol ) {
    assert( protocol != GpgME::UnknownProtocol );

    const std::vector<Key> & keys = protocol == CMS ? cmsKeys : pgpKeys ;

    const CryptoBackend::Protocol * const backend = CryptoBackendFactory::instance()->protocol( protocol );
    assert( backend );

    std::auto_ptr<MultiDeleteJob> job( new MultiDeleteJob( backend ) );

    if ( protocol == CMS )
        connect( job.get(), SIGNAL(result(GpgME::Error,GpgME::Key)),
                 q_func(), SLOT(cmsDeleteResult(GpgME::Error)) );
    else
        connect( job.get(), SIGNAL(result(GpgME::Error,GpgME::Key)),
                 q_func(), SLOT(pgpDeleteResult(GpgME::Error)) );

    connect( job.get(), SIGNAL(progress(QString,int,int)),
             q, SIGNAL(progress(QString,int,int)) );

    if ( const Error err = job->start( keys, true /*allowSecretKeyDeletion*/ ) )
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
                                 pgpError ? cmsError ? pgpErrorString + QLatin1String("</br>") + cmsErrorString : pgpErrorString : cmsErrorString );
        error( msg, i18n("Certificate Deletion Failed") );
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
