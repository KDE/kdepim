/* -*- mode: c++; c-basic-offset:4 -*-
    dialogs/signcertificatedialog.cpp

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

#include "certifycertificatedialog.h"

#include "certifycertificatedialog_p.h"

#include <utils/formatting.h>
#include <utils/kleo_assert.h>

#include <kleo/stl_util.h>

#include <KDebug>
#include <KLocalizedString>

#include <QGridLayout>
#include <QStandardItem>
#include <QListView>
#include <QVBoxLayout>
#include <QWizardPage>
#include <QCheckBox>
#include <QLabel>

#include <QTextDocument> // Qt::escape

#include <boost/bind.hpp>

#include <gpg-error.h>

#include <cassert>

using namespace boost;
using namespace GpgME;
using namespace Kleo;
using namespace Kleo::Dialogs;
using namespace Kleo::Dialogs::CertifyCertificateDialogPrivate;


void UserIDModel::setCertificateToCertify( const Key & key ) {
    m_key = key;
    clear();
    const std::vector<UserID> ids = key.userIDs();
    for ( unsigned int i = 0; i < ids.size(); ++i ) {
        QStandardItem * const item = new QStandardItem;
        item->setText( Formatting::prettyUserID( key.userID( i ) ) );
        item->setData( i, UserIDIndex );
        item->setCheckable( true );
        item->setEditable( false );
        appendRow( item );
    }
}

void UserIDModel::setCheckedUserIDs( const std::vector<unsigned int> & uids ) {
    const std::vector<unsigned int> sorted = kdtools::sorted( uids );
    for ( unsigned int i = 0, end = rowCount() ; i != end ; ++i )
        item( i )->setCheckState( kdtools::binary_search( sorted, i ) ? Qt::Checked : Qt::Unchecked );
}

std::vector<unsigned int> UserIDModel::checkedUserIDs() const {
    std::vector<unsigned int> ids;
    for ( int i = 0; i < rowCount(); ++i )
        if ( item( i )->checkState() == Qt::Checked )
            ids.push_back( item( i )->data( UserIDIndex ).toUInt() );
    return ids;
}

void SecretKeysModel::setSecretKeys( const std::vector<Key> & keys ) {
    clear();
    m_secretKeys = keys;
    for ( unsigned int i = 0; i < m_secretKeys.size(); ++i ) {
        const Key key = m_secretKeys[i];
        QStandardItem * const item = new QStandardItem;
        item->setText( Formatting::formatForComboBox( key ) );
        item->setData( i, IndexRole );
        item->setEditable( false );
        appendRow( item );
    }
}

std::vector<GpgME::Key> SecretKeysModel::secretKeys() const {
    return m_secretKeys;
}

Key SecretKeysModel::keyFromItem( const QStandardItem * item ) const {
    assert( item );
    const unsigned int idx = item->data( IndexRole ).toUInt();
    assert( idx < m_secretKeys.size() );
    return m_secretKeys[idx];
}

Key SecretKeysModel::keyFromIndex( const QModelIndex & idx ) const {
    return keyFromItem( itemFromIndex( idx ) );
}

SelectUserIDsPage::SelectUserIDsPage( QWidget * parent ) : QWizardPage( parent ), m_userIDModel() {
    QVBoxLayout * const layout = new QVBoxLayout ( this );
    QLabel * const label = new QLabel;
    label->setText( i18n( "<b>Step 1:</b> Please select the user IDs you wish to certify." ) );
    layout->addWidget( label );
    m_listView = new QListView;
    m_listView->setModel( &m_userIDModel );
    layout->addWidget( m_listView, 1 );
    m_label = new QLabel;
    layout->addWidget( m_label );
    m_checkbox = new QCheckBox;
    m_checkbox->setChecked( false );
    m_checkbox->setText( i18n("I have verified the fingerprint") );
    layout->addWidget( m_checkbox );
    connect( m_checkbox, SIGNAL(toggled(bool)), this, SIGNAL(completeChanged()) );
    connect( &m_userIDModel, SIGNAL(itemChanged(QStandardItem*)), this, SIGNAL(completeChanged()) );
}

bool SelectUserIDsPage::isComplete() const {
    return m_checkbox->isChecked() && !selectedUserIDs().empty();
}

void SelectUserIDsPage::setSelectedUserIDs( const std::vector<unsigned int> & uids ) {
    m_userIDModel.setCheckedUserIDs( uids );
}

std::vector<unsigned int> SelectUserIDsPage::selectedUserIDs() const {
    return m_userIDModel.checkedUserIDs();
}

void SelectUserIDsPage::setCertificateToCertify( const Key & key ) {
    m_label->setText( i18n( "Certificate: %1\nFingerprint: %2",
                            Formatting::formatForComboBox( key ),
                            QLatin1String(key.primaryFingerprint()) ) );
    m_userIDModel.setCertificateToCertify( key );

}

SelectCheckLevelPage::SelectCheckLevelPage( QWidget * parent ) : QWizardPage( parent ), m_ui() {
    m_ui.setupUi( this );
}

unsigned int SelectCheckLevelPage::checkLevel() const {
    if ( m_ui.checkLevelNotCheckedRB->isChecked() )
        return 1;
    if ( m_ui.checkLevelCasualRB->isChecked() )
        return 2;
    if ( m_ui.checkLevelThoroughlyRB->isChecked() )
        return 3;
    assert( !"No check level radiobutton checked" );
    return 0;
}

OptionsPage::OptionsPage( QWidget * parent ) : QWizardPage( parent ), m_ui() {
    m_ui.setupUi( this );
    m_ui.keyListView->setModel( &m_model );
    connect( m_ui.keyListView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SIGNAL(completeChanged()) );
    setCommitPage( true );
    setButtonText( QWizard::CommitButton, i18n( "Certify" ) );
}


bool OptionsPage::exportableCertificationSelected() const {
    return m_ui.exportableSignatureRB->isChecked();
}

void OptionsPage::setCertificatesWithSecretKeys( const std::vector<Key> & keys ) {
    assert( !keys.empty() );
    m_model.setSecretKeys( keys );
    if ( keys.size() == 1 ) {
        m_ui.stackedWidget->setCurrentWidget( m_ui.singleKeyPage );
        m_ui.singleKeyLabel->setText( i18n( "Certification will be performed using certificate %1.", Formatting::prettyNameAndEMail( keys[0] ) ) );
    } else {
        m_ui.stackedWidget->setCurrentWidget( m_ui.multipleKeysPage );
    }
    emit completeChanged();
}

Key OptionsPage::selectedSecretKey() const {
    if ( m_model.secretKeys().size() == 1 )
        return m_model.secretKeys().at( 0 );
    const QModelIndexList idxs = m_ui.keyListView->selectionModel()->selectedIndexes();
    assert( idxs.size() <= 1 );
    return idxs.isEmpty() ? Key() : m_model.keyFromIndex( idxs[0] );
}

bool OptionsPage::sendToServer() const {
    return m_ui.sendToServerCB->isChecked();
}

bool OptionsPage::validatePage() {
    emit nextClicked();
    return true;
}

bool OptionsPage::isComplete() const {
    return !selectedSecretKey().isNull();
}

SummaryPage::SummaryPage( QWidget * parent ) : QWizardPage( parent ), m_complete( false ) {
    QGridLayout * const layout = new QGridLayout( this );
    QLabel * const uidLabelLabel = new QLabel( i18n( "Signed user IDs:" ) );
    uidLabelLabel->setAlignment( Qt::AlignTop );
    int row = 0;
    layout->addWidget( new QLabel( i18n( "<b>Summary:</b>" ) ), row, 0, 1, 2 );
    layout->addWidget( uidLabelLabel, ++row, 0 );
    layout->addWidget( m_userIDsLabel = new QLabel, row, 1 );
#ifdef KLEO_SIGN_KEY_CERTLEVEL_SUPPORT
    layout->addWidget( new QLabel( i18n( "Check level:" ) ), ++row, 0 );
    layout->addWidget( m_checkLevelLabel = new QLabel, row, 1 );
#else
    m_checkLevelLabel = 0;
#endif
    layout->addWidget( new QLabel( i18n( "Selected secret key:" ) ), ++row, 0 );
    layout->addWidget( m_secretKeyLabel = new QLabel, row, 1 );
    m_secretKeyLabel->setTextFormat( Qt::PlainText );
    layout->addWidget( m_resultLabel = new QLabel, ++row, 0, 1, 2, Qt::AlignCenter );
    m_resultLabel->setWordWrap( true );
    layout->setRowStretch( row, 1 );
    m_resultLabel->setAlignment( Qt::AlignCenter );
}

bool SummaryPage::isComplete() const {
    return m_complete;
}

void SummaryPage::setSummary( const SummaryPage::Summary & sum ) {
    const Key key = sum.certificateToCertify;
    QStringList ids;
    Q_FOREACH ( const unsigned int i, sum.selectedUserIDs )
        ids += Formatting::prettyUserID( key.userID( i ) ).toHtmlEscaped();
    m_userIDsLabel->setText( QLatin1String("<qt>") + ids.join( QLatin1String("<br/>") ) + QLatin1String("</qt>") );
    m_secretKeyLabel->setText( sum.secretKey.isNull() ? i18n( "Default certificate" ) : Formatting::prettyNameAndEMail( sum.secretKey ) );
#ifdef KLEO_SIGN_KEY_CERTLEVEL_SUPPORT
    switch( sum.checkLevel ) {
        case 0:
            m_checkLevelLabel->setText( i18n( "No statement made" ) );
            break;
        case 1:
            m_checkLevelLabel->setText( i18n( "Not checked" ) );
            break;
        case 2:
            m_checkLevelLabel->setText( i18n( "Casually checked" ) );
            break;
        case 3:
            m_checkLevelLabel->setText( i18n( "Thoroughly checked" ) );
            break;
    }
#endif
}

void SummaryPage::setComplete( bool complete ) {
    if ( complete == m_complete )
        return;
    m_complete = complete;
    emit completeChanged();
}
void SummaryPage::setResult( const Error & err ) {
    if ( err && !err.isCanceled() )
        if ( err.code() == GPG_ERR_USER_1 )
            m_resultLabel->setText( i18n( "The certificate was not certified because it was already certified by the same certificate." ) );
        else
            m_resultLabel->setText( i18n( "The certificate could not be certified. <b>Error</b>: %1", QString::fromLocal8Bit( err.asString() ).toHtmlEscaped() ) );
    else if ( err.isCanceled() )
        m_resultLabel->setText( i18n("Certification canceled.") );
    else
        m_resultLabel->setText(i18n("Certification successful.") );
}

class CertifyCertificateDialog::Private {
    friend class ::Kleo::Dialogs::CertifyCertificateDialog;
    CertifyCertificateDialog * const q;

public:
    explicit Private( CertifyCertificateDialog * qq )
        : q( qq ),
        summaryPageId( 0 ),
        selectUserIDsPage( 0 ),
        selectCheckLevelPage( 0 ),
        optionsPage( 0 ),
        summaryPage( 0 )
    {
        selectUserIDsPage = new SelectUserIDsPage( q );
        q->addPage( selectUserIDsPage );
        //selectCheckLevelPage = new SelectCheckLevelPage( q );
        //setting the cert level explicitly is not supported by the backend,
        //thus we omit the page from the UI
        //q->addPage( selectCheckLevelPage );
        optionsPage = new OptionsPage( q );
        q->addPage( optionsPage );
        summaryPage = new SummaryPage( q );
        summaryPageId = q->addPage( summaryPage );
        connect( optionsPage, SIGNAL(nextClicked()), q, SIGNAL(certificationPrepared()) );
    }

    Key key() const {
        return selectUserIDsPage ? selectUserIDsPage->certificateToCertify() : Key() ;
    }

    void ensureSummaryPageVisible();

    void certificationResult( const Error & error );

    void setOperationCompleted() {
        summaryPage->setComplete( true );
    }

    SummaryPage::Summary createSummary() const {
        SummaryPage::Summary sum;
        sum.selectedUserIDs = selectUserIDsPage->selectedUserIDs();
        sum.secretKey = optionsPage->selectedSecretKey();
        sum.certificateToCertify = selectUserIDsPage->certificateToCertify();
        //PENDING
#ifdef KLEO_SIGN_KEY_CERTLEVEL_SUPPORT
        sum.checkLevel = selectCheckLevelPage->checkLevel();
#else
        sum.checkLevel = 0; 
#endif

        sum.exportable = optionsPage->exportableCertificationSelected();
        sum.sendToServer = optionsPage->sendToServer();
        return sum;
    }

    int summaryPageId;
    SelectUserIDsPage * selectUserIDsPage;
    SelectCheckLevelPage * selectCheckLevelPage;
    OptionsPage * optionsPage;
    SummaryPage * summaryPage;
};



CertifyCertificateDialog::CertifyCertificateDialog( QWidget * p, Qt::WindowFlags f )
    : QWizard( p, f ), d( new Private( this ) )
{
}

CertifyCertificateDialog::~CertifyCertificateDialog() {}

void CertifyCertificateDialog::setCertificateToCertify( const Key & key ) {
    setWindowTitle( i18nc( "arg is name, email of certificate holder", "Certify Certificate: %1", Formatting::prettyName( key ) ) );
    d->selectUserIDsPage->setCertificateToCertify( key );
}

void CertifyCertificateDialog::setCertificatesWithSecretKeys( const std::vector<Key> & keys ) {
    d->optionsPage->setCertificatesWithSecretKeys( keys );
}

bool CertifyCertificateDialog::exportableCertificationSelected() const {
    return d->optionsPage->exportableCertificationSelected();
}

bool CertifyCertificateDialog::trustCertificationSelected() const {
    return false;
}

bool CertifyCertificateDialog::nonRevocableCertificationSelected() const {
    return false;
}

Key CertifyCertificateDialog::selectedSecretKey() const {
    return d->optionsPage->selectedSecretKey();
}

bool CertifyCertificateDialog::sendToServer() const {
    return d->optionsPage->sendToServer();
}

unsigned int CertifyCertificateDialog::selectedCheckLevel() const {
    //PENDING
#ifdef KLEO_SIGN_KEY_CERTLEVEL_SUPPORT
    return d->selectCheckLevelPage->checkLevel();
#endif
    return 0;
}

void CertifyCertificateDialog::connectJob( SignKeyJob * job ) {
    connect( job, SIGNAL(result(GpgME::Error)), this, SLOT(certificationResult(GpgME::Error)) );
    d->summaryPage->setSummary( d->createSummary() );
}

void CertifyCertificateDialog::setError( const Error & error ) {
    d->setOperationCompleted();
    d->summaryPage->setResult( error );
    d->ensureSummaryPageVisible();
    if ( error.isCanceled() )
        close();
}

void CertifyCertificateDialog::Private::certificationResult( const Error & err ) {
    setOperationCompleted();
    summaryPage->setResult( err );
    ensureSummaryPageVisible();
}

namespace {
    struct UidEqual : std::binary_function<UserID,UserID,bool> {
        bool operator()( const UserID & lhs, const UserID & rhs ) const {
            return qstrcmp( lhs.parent().primaryFingerprint(),
                            rhs.parent().primaryFingerprint() ) == 0
                && qstrcmp( lhs.id(), rhs.id() ) == 0 ;
        }
    };
}

void CertifyCertificateDialog::setSelectedUserIDs( const std::vector<UserID> & uids ) {
    const Key key = d->key();
    const char * const fpr = key.primaryFingerprint();

    const std::vector<UserID> all = key.userIDs();

    std::vector<unsigned int> indexes;
    indexes.reserve( uids.size() );

    Q_FOREACH( const UserID & uid, uids ) {
        kleo_assert( qstrcmp( uid.parent().primaryFingerprint(), fpr ) == 0 );
        const unsigned int idx =
            std::distance( all.begin(), kdtools::find_if( all, boost::bind( UidEqual(), _1, uid ) ) );
        if ( idx < all.size() )
            indexes.push_back( idx );
    }

    d->selectUserIDsPage->setSelectedUserIDs( indexes );
}

std::vector<unsigned int> CertifyCertificateDialog::selectedUserIDs() const {
    return d->selectUserIDsPage->selectedUserIDs();
}

void CertifyCertificateDialog::Private::ensureSummaryPageVisible() {
    while ( q->currentId() != summaryPageId )
        q->next();
}

#include "moc_certifycertificatedialog.cpp"
#include "moc_certifycertificatedialog_p.cpp"
