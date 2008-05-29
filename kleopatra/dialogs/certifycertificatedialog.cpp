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

#include <KDebug>
#include <KLocalizedString>

#include <QStandardItem>
#include <QStandardItemModel>
#include <QListView>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QWizardPage>

#include <boost/bind.hpp>

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

std::vector<UserID> UserIDModel::checkedUserIDs() const {
    std::vector<UserID> ids;
    for ( int i = 0; i < rowCount(); ++i )
        if ( item( i )->checkState() == Qt::Checked )
            ids.push_back( m_key.userID( item( i )->data( UserIDIndex ).toUInt() ) );
    return ids;
}

void SecretKeysModel::setSecretKeys( const std::vector<Key> & keys ) {
    clear();
    m_secretKeys = keys;
    for ( unsigned int i = 0; i < m_secretKeys.size(); ++i ) {
        const Key key = m_secretKeys[i];
        QStandardItem * const item = new QStandardItem;
        item->setText( Formatting::prettyNameAndEMail( key ) );
        item->setData( i, IndexRole );
        item->setEditable( false );
        appendRow( item );
    }
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
    setTitle( i18n( "Please select the user IDs you wish to certify:" ) );
    QVBoxLayout * const layout = new QVBoxLayout ( this );
    m_listView = new QListView;
    m_listView->setModel( &m_userIDModel );
    connect( &m_userIDModel, SIGNAL(itemChanged(QStandardItem*)), this, SIGNAL(completeChanged()) );
    layout->addWidget( m_listView );
}

bool SelectUserIDsPage::isComplete() const {
    return !selectedUserIDs().empty();
}

std::vector<UserID> SelectUserIDsPage::selectedUserIDs() const {
    return m_userIDModel.checkedUserIDs();
}

void SelectUserIDsPage::setCertificateToCertify( const Key & key ) {
    m_userIDModel.setCertificateToCertify( key );

}

SelectCheckLevelPage::SelectCheckLevelPage( QWidget * parent ) : QWizardPage( parent ), m_ui() {
    m_ui.setupUi( this );
}

unsigned int SelectCheckLevelPage::checkLevel() const {
    if ( m_ui.checkLevelNotCheckedRB->isChecked() )
        return 0;
    if ( m_ui.checkLevelCasualRB->isChecked() )
        return 1;
    if ( m_ui.checkLevelThoroughlyRB->isChecked() )
        return 2;
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

void OptionsPage::OptionsPage::setCertificationOption( SignKeyJob::SigningOption opt ) {
    if ( opt == SignKeyJob::LocalSignature )
        m_ui.localSignatureRB->setChecked( true );
    else
        m_ui.exportableSignatureRB->setChecked( true );
}

SignKeyJob::SigningOption OptionsPage::selectedCertificationOption() const {
    return m_ui.exportableSignatureRB->isChecked() ? SignKeyJob::ExportableSignature : SignKeyJob::LocalSignature;
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
    QVBoxLayout * const layout = new QVBoxLayout( this );
    m_resultLabel = new QLabel;
    layout->addWidget( m_resultLabel );
}

bool SummaryPage::isComplete() const {
    return m_complete;
}

void SummaryPage::setComplete( bool complete ) {
    if ( complete == m_complete )
        return;
    m_complete = complete;
    emit completeChanged();
}
void SummaryPage::setError( const Error & err ) {

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
        selectCheckLevelPage = new SelectCheckLevelPage( q );
        q->addPage( selectCheckLevelPage );
        optionsPage = new OptionsPage( q );
        q->addPage( optionsPage );
        summaryPage = new SummaryPage( q );
        summaryPageId = q->addPage( summaryPage );
        connect( optionsPage, SIGNAL(nextClicked()), q, SIGNAL(certificationPrepared()) );
    }

    void ensureSummaryPageVisible();

    void certificationResult( const Error & error );

    void setOperationCompleted() {
        summaryPage->setComplete( true );
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
    d->selectUserIDsPage->setCertificateToCertify( key );
}

void CertifyCertificateDialog::setCertificatesWithSecretKeys( const std::vector<Key> & keys ) {
    d->optionsPage->setCertificatesWithSecretKeys( keys );
}


void CertifyCertificateDialog::setSigningOption( SignKeyJob::SigningOption option ) {
    d->optionsPage->setCertificationOption( option );
}

SignKeyJob::SigningOption CertifyCertificateDialog::signingOption() const {
    return d->optionsPage->selectedCertificationOption();
}

Key CertifyCertificateDialog::selectedSecretKey() const {
    return d->optionsPage->selectedSecretKey();
}

bool CertifyCertificateDialog::sendToServer() const {
    return d->optionsPage->sendToServer();
}

void CertifyCertificateDialog::connectJob( SignKeyJob * job ) {
    connect( job, SIGNAL(result(GpgME::Error)), this, SLOT(certificationResult(GpgME::Error)) );
}

void CertifyCertificateDialog::setError( const Error & error ) {
    d->setOperationCompleted();
    d->summaryPage->setError( error );
    d->ensureSummaryPageVisible();
    if ( error.isCanceled() )
        close();
}

void CertifyCertificateDialog::Private::certificationResult( const Error & err ) {
    setOperationCompleted();
    ensureSummaryPageVisible();
}

void CertifyCertificateDialog::Private::ensureSummaryPageVisible() {
    while ( q->currentId() != summaryPageId )
        q->next();
}

#include "moc_certifycertificatedialog.cpp"
#include "moc_certifycertificatedialog_p.cpp"
