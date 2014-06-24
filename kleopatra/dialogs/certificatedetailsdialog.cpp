/* -*- mode: c++; c-basic-offset:4 -*-
    dialogs/certificatedetailsdialog.cpp

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

#include "certificatedetailsdialog.h"

#include "ui_certificatedetailsdialog.h"

#include <models/useridlistmodel.h>
#include <models/subkeylistmodel.h>
#include <models/keycache.h>

#include <commands/changepassphrasecommand.h>
#include <commands/changeownertrustcommand.h>
#include <commands/changeexpirycommand.h>
#include <commands/adduseridcommand.h>
#include <commands/certifycertificatecommand.h>
#include <commands/dumpcertificatecommand.h>

#include <utils/formatting.h>
#include <utils/gnupg-helper.h>

#include <kleo/cryptobackendfactory.h>
#include <kleo/cryptobackend.h>
#include <kleo/keylistjob.h>
#include <kleo/dn.h>

#include <gpgme++/key.h>
#include <gpgme++/keylistresult.h>

#include <KDebug>
#include <KMessageBox>
#include <KLocalizedString>

#include <QPointer>
#include <QHeaderView>

#include <boost/mem_fn.hpp>

#include <algorithm>
#include <cassert>
#include <KSharedConfig>
#include <QFontDatabase>

using namespace Kleo;
using namespace Kleo::Dialogs;
using namespace Kleo::Commands;
using namespace GpgME;
using namespace boost;

static bool own( const std::vector<UserID::Signature> & sigs ) {
    const shared_ptr<const KeyCache> kc = KeyCache::instance();
    Q_FOREACH( const UserID::Signature & sig, sigs ) {
        const Key signer = kc->findByKeyIDOrFingerprint( sig.signerKeyID() );
        if ( signer.isNull() || !signer.hasSecret() )
            return false;
    }
    return !sigs.empty();
}

class CertificateDetailsDialog::Private {
    friend class ::Kleo::Dialogs::CertificateDetailsDialog;
    CertificateDetailsDialog * const q;
public:
    explicit Private( CertificateDetailsDialog * qq )
        : q( qq ),
          key(),
          certificationsModel(),
          subkeysModel(),
          ui( q )
    {
        ui.certificationsTV->setModel( &certificationsModel );
        connect( ui.certificationsTV->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                 q, SLOT(slotCertificationSelectionChanged()) );
        connect( ui.changePassphrasePB, SIGNAL(clicked()),
                 q, SLOT(slotChangePassphraseClicked()) );
        connect( ui.changeTrustLevelPB, SIGNAL(clicked()),
                 q, SLOT(slotChangeTrustLevelClicked()) );
        connect( ui.changeExpiryDatePB, SIGNAL(clicked()),
                 q, SLOT(slotChangeExpiryDateClicked()) );
        connect( ui.revokeCertificationPB, SIGNAL(clicked()),
                 q, SLOT(slotRevokeCertificationClicked()) );
        connect( ui.addUserIDPB, SIGNAL(clicked()),
                 q, SLOT(slotAddUserIDClicked()) );
        connect( ui.revokeUserIDPB, SIGNAL(clicked()),
                 q, SLOT(slotRevokeUserIDClicked()) );
        connect( ui.certifyUserIDPB, SIGNAL(clicked()),
                 q, SLOT(slotCertifyUserIDClicked()) );
        connect( ui.revokeCertificationPB, SIGNAL(clicked()),
                 q, SLOT(slotRevokeCertificationClicked()) );
        connect( ui.showCertificationsPB, SIGNAL(clicked()),
                 q, SLOT(slotShowCertificationsClicked()) );


        ui.subkeyTV->setModel( &subkeysModel );
        // no selection (yet)

        connect( KeyCache::instance().get(), SIGNAL(keysMayHaveChanged()),
                 q, SLOT(slotKeysMayHaveChanged()) );
    }

    void readConfig()
    {
        KConfigGroup dialog( KSharedConfig::openConfig(), "CertificateDetailsDialog" );
        const QSize size = dialog.readEntry( "Size", QSize(600, 400) );
        if ( size.isValid() ) {
            q->resize( size );
        }
    }

    void writeConfig()
    {
        KConfigGroup dialog( KSharedConfig::openConfig(), "CertificateDetailsDialog" );
        dialog.writeEntry( "Size", q->size() );
        dialog.sync();
    }

private:
    void startCommandImplementation( const QPointer<Command> & ptr, const char * slot ) {
        connect( ptr, SIGNAL(finished()), q, slot );
        ptr->start();
        enableDisableWidgets();
    }
    template <typename T, typename A>
    void startCommand( QPointer<Command> & ptr, const A & arg, const char * slot ) {
        if ( ptr )
            return;
        ptr = new T( arg );
        startCommandImplementation( ptr, slot );
    }
    template <typename T>
    void startCommand( QPointer<Command> & ptr, const char * slot ) {
        startCommand<T>( ptr, this->key, slot );
    }
    void commandFinished( QPointer<Command> & ptr ) {
        ptr = 0;
        enableDisableWidgets();
    }

    void slotChangePassphraseClicked() {
        startCommand<ChangePassphraseCommand>( changePassphraseCommand, SLOT(slotChangePassphraseCommandFinished()) );
    }
    void slotChangePassphraseCommandFinished() {
        commandFinished( changePassphraseCommand );
    }

    void slotChangeTrustLevelClicked() {
        startCommand<ChangeOwnerTrustCommand>( changeOwnerTrustCommand, SLOT(slotChangeOwnerTrustCommandFinished()) );
    }
    void slotChangeOwnerTrustCommandFinished() {
        commandFinished( changeOwnerTrustCommand );
    }

    void slotChangeExpiryDateClicked() {
        startCommand<ChangeExpiryCommand>( changeExpiryDateCommand, SLOT(slotChangeExpiryDateCommandFinished()) );
    }
    void slotChangeExpiryDateCommandFinished() {
        commandFinished( changeExpiryDateCommand );
    }

    void slotAddUserIDClicked() {
        startCommand<AddUserIDCommand>( addUserIDCommand, SLOT(slotAddUserIDCommandFinished()) );
    }
    void slotAddUserIDCommandFinished() {
        commandFinished( addUserIDCommand );
    }

    void slotCertifyUserIDClicked() {
        const std::vector<UserID> uids = selectedUserIDs();
        if ( uids.empty() )
            return;
        startCommand<CertifyCertificateCommand>( signCertificateCommand, uids, SLOT(slotSignCertificateCommandFinished()) );
    }
    void slotSignCertificateCommandFinished() {
        commandFinished( signCertificateCommand );
    }

    void slotRevokeCertificateClicked() {

    }

    void slotRevokeUserIDClicked() {

    }

    void slotRevokeCertificationClicked() {

    }

    void slotShowCertificationsClicked() {
        startSignatureListing();
        enableDisableWidgets();
    }

    void startSignatureListing() {
        if ( keyListJob )
            return;
        const CryptoBackend::Protocol * const protocol = CryptoBackendFactory::instance()->protocol( key.protocol() );
        if ( !protocol )
            return;
        KeyListJob * const job = protocol->keyListJob( /*remote*/false, /*includeSigs*/true, /*validate*/true );
        if ( !job )
            return;
        connect( job, SIGNAL(result(GpgME::KeyListResult)),
                 q, SLOT(slotSignatureListingDone(GpgME::KeyListResult)) );
        connect( job, SIGNAL(nextKey(GpgME::Key)),
                 q, SLOT(slotSignatureListingNextKey(GpgME::Key)) );
        if ( const Error err = job->start( QStringList( QString::fromLatin1( key.primaryFingerprint() ) ) ) )
            showSignatureListingErrorDialog( err );
        else
            keyListJob = job;
    }
    void slotSignatureListingNextKey( const Key & key ) {
        // don't lose the secret flags ...
        Key merged = key;
        merged.mergeWith( this->key );
        q->setKey( merged );

        // fixup the tree view
        ui.certificationsTV->expandAll();
        ui.certificationsTV->header()->resizeSections( QHeaderView::ResizeToContents );
    }
    void slotSignatureListingDone( const KeyListResult & result ) {
        if ( result.error().isCanceled() )
            ;
        else if ( result.error() )
            showSignatureListingErrorDialog( result.error() );
        else {
            ; // nothing to do
        }
        keyListJob = 0;
        enableDisableWidgets();
    }
    void showSignatureListingErrorDialog( const Error & err ) {
        KMessageBox::information( q, i18nc("@info",
                                           "<para>An error occurred while loading the certifications: "
                                           "<message>%1</message></para>",
                                           QString::fromLocal8Bit( err.asString() ) ),
                                  i18nc("@title","Certifications Loading Failed") );
    }

    void slotCertificationSelectionChanged() {
        enableDisableWidgets();
    }

    void slotKeysMayHaveChanged() {
        if ( const char * const fpr = key.primaryFingerprint() )
            if ( !(key.keyListMode() & Extern) )
                q->setKey( KeyCache::instance()->findByFingerprint( fpr ) );
    }

    void slotDumpCertificate() {

        if ( dumpCertificateCommand )
            return;

        if ( key.protocol() != CMS ) {
            ui.dumpLTW->clear();
            return;
        }

        ui.dumpLTW->setLines( QStringList( i18n("Please wait while generating the dump...") ) );

        dumpCertificateCommand = new DumpCertificateCommand( key );
        dumpCertificateCommand->setUseDialog( false );
        QPointer<Command> cmd = dumpCertificateCommand.data();
        startCommandImplementation( cmd, SLOT(slotDumpCertificateCommandFinished()) );
    }

    void slotDumpCertificateCommandFinished() {
        ui.dumpLTW->setLines( dumpCertificateCommand->output() );
    }

private:
    void updateWidgetVisibility() {
        const bool x509 = key.protocol() == CMS;
        const bool pgp = key.protocol() == OpenPGP;
        const bool secret = key.hasSecret();
        const bool sigs = (key.keyListMode() & Signatures);
        const bool ultimateTrust = key.ownerTrust() == Key::Ultimate;
        const bool external = (key.keyListMode() & Extern);

        // Overview Tab
        ui.overviewActionsGB->setVisible( !external );
        ui.changePassphrasePB->setVisible(         secret );
        ui.changeTrustLevelPB->setVisible( pgp && ( !secret || !ultimateTrust ) );
        ui.changeExpiryDatePB->setVisible( pgp &&  secret );

        // Certifications Tab
        ui.userIDsActionsGB->setVisible( !external && pgp );
        ui.certificationsActionGB->setVisible( !external && pgp );
        ui.addUserIDPB->setVisible( secret );
        ui.expandAllCertificationsPB->setVisible( pgp && sigs );
        ui.collapseAllCertificationsPB->setVisible( pgp && sigs );
        ui.showCertificationsPB->setVisible( !external && pgp && !sigs );

        // Technical Details Tab
        ui.tabWidget->setTabEnabled( ui.tabWidget->indexOf( ui.detailsTab ), pgp );

        // Chain tab
        ui.tabWidget->setTabEnabled( ui.tabWidget->indexOf( ui.chainTab ), x509 );

        // Dump tab
        ui.tabWidget->setTabEnabled( ui.tabWidget->indexOf( ui.dumpTab ), x509 );

        // not implemented:
        ui.revokeCertificatePB->hide();
        ui.revokeUserIDPB->hide();
        ui.certificationsActionGB->hide();
    }

    QModelIndexList selectedCertificationsIndexes() const {
        return ui.certificationsTV->selectionModel()->selectedRows();
    }

    std::vector<UserID> selectedUserIDs() const {
        const QModelIndexList mil = selectedCertificationsIndexes();
        std::vector<UserID> uids = certificationsModel.userIDs( mil, true );
        uids.erase( std::remove_if( uids.begin(), uids.end(), mem_fn( &UserID::isNull ) ), uids.end() );
        return uids;
    }

    std::vector<UserID::Signature> selectedSignatures() const {
        const QModelIndexList mil = selectedCertificationsIndexes();
        std::vector<UserID::Signature> sigs = certificationsModel.signatures( mil );
        sigs.erase( std::remove_if( sigs.begin(), sigs.end(), mem_fn( &UserID::Signature::isNull ) ), sigs.end() );
        return sigs;
    }

    void enableDisableWidgets() {
        // Overview Tab
        ui.changePassphrasePB->setEnabled( !changePassphraseCommand );
        ui.changeTrustLevelPB->setEnabled( !changeOwnerTrustCommand );
        ui.changeExpiryDatePB->setEnabled( !changeExpiryDateCommand );

        // Certifications Tab
        ui.addUserIDPB->setEnabled( !addUserIDCommand );
        ui.showCertificationsPB->setEnabled( !keyListJob );
        ui.showCertificationsPB->setText( keyListJob
                                          ? i18n("(please wait while certifications are being loaded)")
                                          : i18n("Load Certifications (may take a while)") );

        const std::vector<UserID> uids = selectedUserIDs();
        const std::vector<UserID::Signature> sigs = selectedSignatures();

        ui.certifyUserIDPB->setEnabled(      !uids.empty() &&  sigs.empty() && !signCertificateCommand );
        ui.revokeUserIDPB->setEnabled(       !uids.empty() &&  sigs.empty() );
        ui.revokeCertificationPB->setEnabled( uids.empty() && !sigs.empty() && own( sigs ) );
    }

    void updateLabel() {
        ui.overviewLB->setText( Formatting::formatOverview( key ) );
    }

    void updateChainTab() {
        ui.chainTW->clear();

        if ( key.protocol() != CMS )
            return;

        QTreeWidgetItem * last = 0;
        const std::vector<Key> chain = KeyCache::instance()->findIssuers( key, KeyCache::RecursiveSearch|KeyCache::IncludeSubject );
        if ( chain.empty() )
            return;
        if ( !chain.back().isRoot() ) {
            last = new QTreeWidgetItem( ui.chainTW );
            last->setText( 0, i18n("Issuer Certificate Not Found (%1)",
                                   DN( chain.back().issuerName() ).prettyDN() ) );
            //last->setSelectable( false );
            const QBrush & fg = ui.chainTW->palette().brush( QPalette::Disabled, QPalette::WindowText );
            last->setForeground( 0, fg );
        }
        for ( std::vector<Key>::const_reverse_iterator it = chain.rbegin(), end = chain.rend() ; it != end ; ++it ) {
            last = last ? new QTreeWidgetItem( last ) : new QTreeWidgetItem( ui.chainTW ) ;
            last->setText( 0, DN( it->userID(0).id() ).prettyDN() );
            //last->setSelectable( true );
        }
        ui.chainTW->expandAll();
    }

    void propagateKey() {
        certificationsModel.setKey( key );
        const QModelIndexList uidIndexes = certificationsModel.indexes( key.userIDs() );
        Q_FOREACH( const QModelIndex & idx, uidIndexes )
            ui.certificationsTV->setFirstColumnSpanned( idx.row(), idx.parent(), true );

        subkeysModel.setKey( key );
        ui.subkeyTV->header()->resizeSections( QHeaderView::ResizeToContents );

        updateChainTab();
        slotDumpCertificate();
    }


private:
    Key key;
    UserIDListModel certificationsModel;
    SubkeyListModel subkeysModel;

    QPointer<Command> changePassphraseCommand;
    QPointer<Command> changeOwnerTrustCommand;
    QPointer<Command> changeExpiryDateCommand;

    QPointer<Command> addUserIDCommand;
    QPointer<Command> signCertificateCommand;

    QPointer<DumpCertificateCommand> dumpCertificateCommand;

    QPointer<KeyListJob> keyListJob;

    struct UI : public Ui_CertificateDetailsDialog {
        explicit UI( Dialogs::CertificateDetailsDialog * qq )
            : Ui_CertificateDetailsDialog()
        {
            setupUi( qq->mainWidget() );
            qq->setButtons( KDialog::Help | KDialog::Close );
            qq->setHelp(QString(), QLatin1String("kleopatra"));
            chainTW->header()->setResizeMode( 0, QHeaderView::Stretch );

            dumpLTW->setFont( QFontDatabase::systemFont(QFontDatabase::FixedFont) );
            dumpLTW->setMinimumVisibleLines( 15 );
            dumpLTW->setMinimumVisibleColumns( 40 );

            subkeyHLine->setTitle( i18nc("@title","Subkeys") );
        }
    } ui;
};

CertificateDetailsDialog::CertificateDetailsDialog( QWidget * p, Qt::WindowFlags f )
    : KDialog( p, f ), d( new Private( this ) )
{
    d->readConfig();
}

CertificateDetailsDialog::~CertificateDetailsDialog()
{
    d->writeConfig();
}


void CertificateDetailsDialog::setKey( const Key & key ) {
    d->key = key;
    d->updateWidgetVisibility();
    d->updateLabel();
    d->propagateKey();
    d->enableDisableWidgets();
}

Key CertificateDetailsDialog::key() const {
    return d->key;
}


#include "moc_certificatedetailsdialog.cpp"
