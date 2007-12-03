/* -*- mode: c++; c-basic-offset:4 -*-
    signerresolvepage.cpp

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

#include "signerresolvepage.h"
#include "keyselectiondialog.h"
#include "kleo-assuan.h"
#include "models/keycache.h"
#include "utils/formatting.h"

#include <kmime/kmime_header_parsing.h>

#include <gpgme++/key.h>

#include <KDialog>
#include <KLocale>

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

#include <cassert>

using namespace Kleo;

class SignerResolvePage::Private {
    friend class ::SignerResolvePage;
    SignerResolvePage * const q;
public:
    explicit Private( SignerResolvePage * qq );
    ~Private();

    void setMode( int mode );
    void selectCertificate();
    void addCertificate( const GpgME::Key& key );
    void updateModeSelectionWidgets();

private:
    enum SignEncryptMode {
        SignAndEncrypt=0,
        SignOnly,
        EncryptOnly
    };
    QButtonGroup* signEncryptGroup;
    QRadioButton* signAndEncryptRB;
    QRadioButton* encryptOnlyRB;
    QRadioButton* signOnlyRB;
    QGroupBox* signingBox;
    QLabel * signerLabel;
    QComboBox* signerCombo;
    QGroupBox * encryptBox;
    QCheckBox * textArmorCO;
    QCheckBox * removeUnencryptedCO;
    QPushButton * selectCertificateButton;
    GpgME::Protocol protocol;
    bool signingMutable;
    bool encryptionMutable;
    bool signingSelected;
    bool encryptionSelected;
};

SignerResolvePage::Private::Private( SignerResolvePage * qq )
    : q( qq ), protocol( GpgME::UnknownProtocol ),
      signingMutable( true ), encryptionMutable( true ), 
      signingSelected( false ), encryptionSelected( false )

{
    QVBoxLayout* layout = new QVBoxLayout( q );
    layout->setSpacing( KDialog::spacingHint() );

    signEncryptGroup = new QButtonGroup( q );
    q->connect( signEncryptGroup, SIGNAL( buttonClicked( int ) ), q, SLOT( setMode( int ) ) );

    signAndEncryptRB = new QRadioButton;
    signAndEncryptRB->setText( i18n( "Sign and encrypt (OpenPGP only)" ) );
    signEncryptGroup->addButton( signAndEncryptRB, SignAndEncrypt );
    layout->addWidget( signAndEncryptRB );

    encryptOnlyRB = new QRadioButton;
    encryptOnlyRB->setText( i18n( "Encrypt only" ) );
    signEncryptGroup->addButton( encryptOnlyRB, EncryptOnly );
    layout->addWidget( encryptOnlyRB );

    signOnlyRB = new QRadioButton;
    signOnlyRB->setText( i18n( "Sign only" ) );
    signEncryptGroup->addButton( signOnlyRB, SignOnly );
    layout->addWidget( signOnlyRB );

    encryptBox = new QGroupBox;
    encryptBox->setEnabled( false );
    encryptBox->setTitle( i18n( "Encryption Options" ) );
    QBoxLayout * const encryptLayout = new QVBoxLayout( encryptBox );
    textArmorCO = new QCheckBox;
    textArmorCO->setText( i18n( "Text output (ASCII armor)" ) );
    encryptLayout->addWidget( textArmorCO );
    removeUnencryptedCO = new QCheckBox;
    removeUnencryptedCO->setText( i18n( "Remove unencrypted original file when done" ) );
    encryptLayout->addWidget( removeUnencryptedCO );
    layout->addWidget( encryptBox );

    signingBox = new QGroupBox;
    signingBox->setEnabled( false );
    signingBox->setTitle( i18n( "Signing Certificates" ) );
    QGridLayout* signerLayout = new QGridLayout( signingBox );
    signerLayout->setColumnStretch( 1, 1 );

    QLabel* label = new QLabel;
    label->setText( i18n( "Signer:" ) );
    signerLayout->addWidget( label, 0, 0 );
    signerLabel = new QLabel;
    signerLayout->addWidget( signerLabel, 0, 1 );
    QLabel* certLabel = new QLabel;
    certLabel->setText( i18n ( "Certificate: " ) );
    signerLayout->addWidget( certLabel, 1, 0 );
    signerCombo = new QComboBox;
    signerLayout->addWidget( signerCombo, 1, 1 );
    selectCertificateButton = new QPushButton;
    selectCertificateButton->setText( i18n("...") );
    selectCertificateButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred );
    signerLayout->addWidget( selectCertificateButton, 1, 2 );
    q->connect( selectCertificateButton, SIGNAL( clicked() ),
                q, SLOT( selectCertificate() ) );
    layout->addWidget( signingBox );
    layout->addStretch();

    //PENDING
    updateModeSelectionWidgets();
}

SignerResolvePage::Private::~Private() {}

void SignerResolvePage::Private::addCertificate( const GpgME::Key& key )
{
    signerCombo->addItem( Formatting::formatForComboBox( key ), 
                          QByteArray( key.keyID() ) );
}

void SignerResolvePage::Private::updateModeSelectionWidgets()
{
    const bool ismutable = signingMutable || encryptionMutable;
    signAndEncryptRB->setChecked( signingSelected && encryptionSelected );
    signOnlyRB->setChecked( signingSelected && !encryptionSelected );
    encryptOnlyRB->setChecked( encryptionSelected && !signingSelected );
}

void SignerResolvePage::Private::selectCertificate()
{
    QPointer<KeySelectionDialog> dlg = new KeySelectionDialog( q );
    dlg->setSelectionMode( KeySelectionDialog::SingleSelection );
    dlg->setProtocol( protocol );
    dlg->setAllowedKeys( KeySelectionDialog::SignOnly );
    dlg->addKeys( KeyCache::instance()->keys() );
    if ( dlg->exec() == QDialog::Accepted )
    {
        const std::vector<GpgME::Key> keys = dlg->selectedKeys();
        if ( !keys.empty() )
        {
            addCertificate( keys[0] );
            emit q->completeChanged();
            //TODO: make sure keys[0] gets selected
        }
    }

    delete dlg;
}

void SignerResolvePage::Private::setMode( int mode )
{

}

SignerResolvePage::SignerResolvePage( QWidget * parent, Qt::WFlags f )
  : WizardPage( parent, f ), d( new Private( this ) )
{
    
}

SignerResolvePage::~SignerResolvePage() {}

void SignerResolvePage::setSignersAndCandidates( const std::vector<KMime::Types::Mailbox> & signers, 
                                                 const std::vector< std::vector<GpgME::Key> > & keys )
{
    assuan_assert( !keys.empty() );
    assuan_assert( signers.empty() || signers.size() == keys.size() );
    if ( signers.size() > 1 )
        assuan_assert( !"Resolving multiple signers not implemented" );
    d->signerLabel->setText( signers.front().prettyAddress() );
    d->signerCombo->clear();
    Q_FOREACH( const GpgME::Key& i, keys.front() )
    {
        d->addCertificate( i );
    }
    emit completeChanged();
}


void SignerResolvePage::setProtocol( GpgME::Protocol protocol )
{
    d->protocol = protocol;
}

GpgME::Protocol SignerResolvePage::protocol() const
{
    return d->protocol;
}

std::vector<GpgME::Key> SignerResolvePage::resolvedSigners() const
{
    assuan_assert( isComplete() );
    std::vector<GpgME::Key> result;
    const QByteArray id = d->signerCombo->itemData( d->signerCombo->currentIndex() ).toByteArray();
    result.push_back( KeyCache::instance()->findByKeyIDOrFingerprint( id.constData() ) );
    return result;
}

bool SignerResolvePage::isComplete() const
{
    return !d->signerCombo->itemData( d->signerCombo->currentIndex() ).isNull();
}

bool SignerResolvePage::encryptionSelected() const
{
    return !d->signOnlyRB->isChecked();
}

void SignerResolvePage::setEncryptionSelected( bool selected )
{
    d->encryptionSelected = selected;
    d->updateModeSelectionWidgets();
}

bool SignerResolvePage::signingSelected() const
{
    return !d->encryptOnlyRB->isChecked();
}

void SignerResolvePage::setSigningSelected( bool selected )
{
    d->signingSelected = selected;
    d->updateModeSelectionWidgets();
}

bool SignerResolvePage::isEncryptionUserMutable() const
{
    return d->encryptionMutable;
}

bool SignerResolvePage::isSigningUserMutable() const
{
    return d->signingMutable;
}

void SignerResolvePage::setEncryptionUserMutable( bool ismutable )
{
    d->encryptionMutable = ismutable;
    d->updateModeSelectionWidgets();
}
        
void SignerResolvePage::setSigningUserMutable( bool ismutable )
{
    d->signingMutable = ismutable;
    d->updateModeSelectionWidgets();

}

#include "moc_signerresolvepage.cpp"

