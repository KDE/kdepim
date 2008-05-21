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

#include <utils/formatting.h>

#include <gpgme++/key.h>

#include <KDebug>
#include <KMessageBox>
#include <KLocalizedString>

#include <boost/mem_fn.hpp>

#include <algorithm>
#include <cassert>

using namespace Kleo;
using namespace Kleo::Dialogs;
using namespace GpgME;
using namespace boost;

static bool own( const std::vector<UserID::Signature> & sigs ) {
    // ### check whether we have secret keys for all sigs (by fingerprint)
    return false;
}

class CertificateDetailsDialog::Private {
    friend class ::Kleo::Dialogs::CertificateDetailsDialog;
    CertificateDetailsDialog * const q;
public:
    explicit Private( CertificateDetailsDialog * qq )
        : q( qq ),
          key(),
          certificationsModel(),
          ui( q )
    {
        ui.certificationsTV->setModel( &certificationsModel );
        connect( ui.certificationsTV->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                 q, SLOT(slotCertificationSelectionChanged()) );
    }

private:
    void slotChangePassphraseClicked();
    void slotChangeTrustLevelClicked();
    void slotChangeExpiryDateClicked();
    void slotRevokeCertificateClicked();

    void slotAddUserIDClicked();
    void slotRevokeUserIDClicked();
    void slotCertifyUserIDClicked();

    void slotRevokeCertificationClicked();

    void slotShowCertificationsToggled( bool );

    void slotCertificationSelectionChanged() {
        enableDisableWidgets();
    }

private:
    void updateWidgetVisibility() {
        const bool x509 = key.protocol() == CMS;
        const bool pgp = key.protocol() == OpenPGP;
        const bool secret = key.hasSecret();

        // Overview Tab
        ui.changePassphrasePB->setVisible(         secret );
        ui.changeTrustLevelPB->setVisible( pgp && !secret );
        ui.changeExpiryDatePB->setVisible( pgp &&  secret );

        // Certifications Tab
        ui.userIDsActionsGB->setVisible( pgp );
        ui.certificationsActionGB->setVisible( pgp );
        ui.addUserIDPB->setVisible( secret );
        ui.certifyUserIDPB->setVisible( !secret );

        // ...
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
        // Certifications Tab

        const std::vector<UserID> uids = selectedUserIDs();
        const std::vector<UserID::Signature> sigs = selectedSignatures();

        ui.certifyUserIDPB->setEnabled(      !uids.empty() &&  sigs.empty() );
        ui.revokeUserIDPB->setEnabled(       !uids.empty() &&  sigs.empty() );
        ui.revokeCertificationPB->setEnabled( uids.empty() && !sigs.empty() && own( sigs ) );
    }

    void updateLabel() {
        ui.overviewLB->setText( Formatting::formatOverview( key ) );
    }

    void propagateKey() {
        certificationsModel.setKey( key );
    }


private:
    Key key;
    UserIDListModel certificationsModel;

    struct UI : public Ui_CertificateDetailsDialog {
        explicit UI( Dialogs::CertificateDetailsDialog * qq )
            : Ui_CertificateDetailsDialog()
        {
            setupUi( qq );
        }
    } ui;
};

CertificateDetailsDialog::CertificateDetailsDialog( QWidget * p, Qt::WindowFlags f )
    : QDialog( p, f ), d( new Private( this ) )
{

}

CertificateDetailsDialog::~CertificateDetailsDialog() {}


void CertificateDetailsDialog::setKey( const Key & key ) {
    if ( qstricmp( key.primaryFingerprint(), d->key.primaryFingerprint() ) == 0 )
        return;
    d->key = key;
    d->updateWidgetVisibility();
    d->enableDisableWidgets();
    d->updateLabel();
    d->propagateKey();
}

Key CertificateDetailsDialog::key() const {
    return d->key;
}


void CertificateDetailsDialog::Private::slotChangePassphraseClicked() {

}

void CertificateDetailsDialog::Private::slotChangeTrustLevelClicked() {

}

void CertificateDetailsDialog::Private::slotChangeExpiryDateClicked() {

}

void CertificateDetailsDialog::Private::slotRevokeCertificateClicked() {

}

void CertificateDetailsDialog::Private::slotAddUserIDClicked() {

}

void CertificateDetailsDialog::Private::slotRevokeUserIDClicked() {

}

void CertificateDetailsDialog::Private::slotCertifyUserIDClicked() {

}

void CertificateDetailsDialog::Private::slotRevokeCertificationClicked() {

}

void CertificateDetailsDialog::Private::slotShowCertificationsToggled( bool ) {

}



#include "moc_certificatedetailsdialog.cpp"
