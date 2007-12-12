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
#include "signingcertificateselectiondialog.h"
#include "kleo-assuan.h"
#include "models/keycache.h"
#include "utils/formatting.h"

#include <kmime/kmime_header_parsing.h>

#include <gpgme++/key.h>

#include <KDialog>
#include <KLocale>

#include <QButtonGroup>
#include <QCheckBox>
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
    void selectCertificates();
    void setCertificates( const QMap<GpgME::Protocol, GpgME::Key>& certs );
    void updateModeSelectionWidgets();

private:
    enum SignEncryptMode {
        SignAndEncrypt=0,
        SignOnly,
        EncryptOnly
    };
    SignEncryptMode mode;
    QButtonGroup* signEncryptGroup;
    QRadioButton* signAndEncryptRB;
    QRadioButton* encryptOnlyRB;
    QRadioButton* signOnlyRB;
    QGroupBox* signingBox;
    QLabel * signerLabel;
    QLabel * pgpLabel;
    QLabel * cmsLabel;
    QGroupBox * encryptBox;
    QCheckBox * textArmorCO;
    QCheckBox * removeUnencryptedCO;
    QPushButton * selectCertificatesButton;
    GpgME::Protocol protocol;
    bool signingMutable;
    bool encryptionMutable;
    bool signingSelected;
    bool encryptionSelected;
    QMap<GpgME::Protocol,GpgME::Key> certificates;
};

SignerResolvePage::Private::Private( SignerResolvePage * qq )
    : q( qq ), mode( SignOnly ), protocol( GpgME::UnknownProtocol ),
      signingMutable( true ), encryptionMutable( true ), 
      signingSelected( false ), encryptionSelected( false )

{
    QVBoxLayout* layout = new QVBoxLayout( q );
    layout->setSpacing( KDialog::spacingHint() );

    signEncryptGroup = new QButtonGroup( q );
    q->connect( signEncryptGroup, SIGNAL( buttonClicked( int ) ), q, SLOT( setMode( int ) ) );

    signAndEncryptRB = new QRadioButton;
    signAndEncryptRB->setText( i18n( "Sign and encrypt (OpenPGP only)" ) );
    signAndEncryptRB->setChecked( true );
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
    encryptBox->setTitle( i18n( "Encryption Options" ) );
    QBoxLayout * const encryptLayout = new QVBoxLayout( encryptBox );
    textArmorCO = new QCheckBox;
    textArmorCO->setText( i18n( "Text output (ASCII armor)" ) );
    encryptLayout->addWidget( textArmorCO );
    removeUnencryptedCO = new QCheckBox;
    removeUnencryptedCO->setText( i18n( "Remove unencrypted original file when done" ) );
    removeUnencryptedCO->setChecked( true );
    encryptLayout->addWidget( removeUnencryptedCO );
    layout->addWidget( encryptBox );

    signingBox = new QGroupBox;
    signingBox->setTitle( i18n( "Signing Certificates" ) );
    QGridLayout* signerLayout = new QGridLayout( signingBox );
    signerLayout->setColumnStretch( 1, 1 );

    QLabel* signerLabelLabel = new QLabel;
    signerLabelLabel->setText( i18n( "Signer:" ) );
    signerLayout->addWidget( signerLabelLabel, 0, 0 );
    signerLabel = new QLabel;
    signerLayout->addWidget( signerLabel, 0, 1 );
    QLabel* const pgpLabelLabel = new QLabel;
    pgpLabelLabel->setText( i18n( "OpenPGP:" ) );
    signerLayout->addWidget( pgpLabelLabel, 1, 0 );
    pgpLabel = new QLabel;
    signerLayout->addWidget( pgpLabel, 1, 1, 1, 1, Qt::AlignLeft );
    QLabel* const cmsLabelLabel = new QLabel;
    cmsLabelLabel->setText( i18n( "S/MIME:" ) );
    signerLayout->addWidget( cmsLabelLabel, 2, 0 );
    cmsLabel = new QLabel;
    signerLayout->addWidget( cmsLabel, 2, 1, 1, 1, Qt::AlignLeft );
    
    selectCertificatesButton = new QPushButton;
    selectCertificatesButton->setText( i18n( "Change Signing Certificates..." ) );
    selectCertificatesButton->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
    signerLayout->addWidget( selectCertificatesButton, 3, 0, 1, -1, Qt::AlignLeft );
    q->connect( selectCertificatesButton, SIGNAL( clicked() ),
                q, SLOT( selectCertificates() ) );
    layout->addWidget( signingBox );
    layout->addStretch();

    setCertificates( QMap<GpgME::Protocol, GpgME::Key>() );
    updateModeSelectionWidgets();
}

SignerResolvePage::Private::~Private() {}


void SignerResolvePage::Private::setCertificates( const QMap<GpgME::Protocol, GpgME::Key>& certs )
{
    certificates = certs;
    const GpgME::Key cmsKey = certs[GpgME::CMS];
    cmsLabel->setText( !cmsKey.isNull() ? Formatting::formatForComboBox( cmsKey ) : i18n( "No certificate selected" ) );
    const GpgME::Key pgpKey = certs[GpgME::OpenPGP];
    pgpLabel->setText( !pgpKey.isNull() ? Formatting::formatForComboBox( pgpKey ) : i18n( "No certificate selected" )  );
    emit q->completeChanged();
}

void SignerResolvePage::Private::updateModeSelectionWidgets()
{
    const bool bothMutable = signingMutable && encryptionMutable;
    const bool noSigning = !signingSelected && !signingMutable;
    const bool noEncryption = !encryptionSelected && !encryptionMutable;
    signAndEncryptRB->setChecked( signingSelected && encryptionSelected );
    signOnlyRB->setChecked( signingSelected && !encryptionSelected );
    encryptOnlyRB->setChecked( encryptionSelected && !signingSelected );
    const bool canSignAndEncrypt = bothMutable && protocol != GpgME::CMS;
    const bool canSignOnly = !encryptionSelected || encryptionMutable;
    const bool canEncryptOnly = !signingSelected || signingMutable;
    signAndEncryptRB->setEnabled( canSignAndEncrypt );
    signOnlyRB->setEnabled( canSignOnly );
    encryptOnlyRB->setEnabled( canEncryptOnly );
    const bool buttonsVisible = signingMutable || encryptionMutable;
    signOnlyRB->setVisible( buttonsVisible );
    encryptOnlyRB->setVisible( buttonsVisible );
    signAndEncryptRB->setVisible( buttonsVisible );
    signingBox->setVisible( !noSigning );
    encryptBox->setVisible( !noEncryption );
}

void SignerResolvePage::Private::selectCertificates()
{
    QPointer<SigningCertificateSelectionDialog> dlg = new SigningCertificateSelectionDialog( q );
    if ( dlg->exec() == QDialog::Accepted )
    {
        setCertificates( dlg->selectedCertificates() );
    }

    delete dlg;
}

void SignerResolvePage::Private::setMode( int mode_ )
{
    mode = static_cast<SignEncryptMode>( mode_ );
    signingBox->setEnabled( mode != EncryptOnly );
    encryptBox->setEnabled( mode != SignOnly );
}

SignerResolvePage::SignerResolvePage( QWidget * parent, Qt::WFlags f )
  : WizardPage( parent, f ), d( new Private( this ) )
{
    setTitle( i18n( "<b>Choose Operation to be Performed</b>" ) );
    setSubTitle( i18n( "TODO" ) );
}

SignerResolvePage::~SignerResolvePage() {}

void SignerResolvePage::setSignersAndCandidates( const std::vector<KMime::Types::Mailbox> & signers, 
                                                 const std::vector< std::vector<GpgME::Key> > & keys )
{
    assuan_assert( signers.empty() || signers.size() == keys.size() );

    switch ( signers.size() )
    {
    case 0:
        d->signerLabel->setText( QString() ); // TODO: use default identity?
        break;        
    case 1:
        d->signerLabel->setText( signers.front().prettyAddress() );
        break;
    default: // > 1
        assuan_assert( !"Resolving multiple signers not implemented" );
    }
    emit completeChanged();
}


void SignerResolvePage::setProtocol( GpgME::Protocol protocol )
{
    d->protocol = protocol;
    d->updateModeSelectionWidgets();
}

GpgME::Protocol SignerResolvePage::protocol() const
{
    return d->protocol;
}

std::vector<GpgME::Key> SignerResolvePage::resolvedSigners() const
{
    assuan_assert( isComplete() );
    std::vector<GpgME::Key> result;
    if ( d->protocol == GpgME::UnknownProtocol )
    {
        if ( !d->certificates[GpgME::OpenPGP].isNull() )
            result.push_back( d->certificates[GpgME::OpenPGP] );
        if ( !d->certificates[GpgME::CMS].isNull() )
            result.push_back( d->certificates[GpgME::CMS] );
    }
    else
    {
        result.push_back( d->certificates[d->protocol] );
    }
    return result;
}

bool SignerResolvePage::isComplete() const
{
    //TODO: factor out mode-specific code
    if ( d->mode != Private::EncryptOnly )
    {
        return true;
#if 0 // TODO: implement this correctly, dependent on selected mode and signing certificates
        return d->certificates.contains( d->protocol ) && !d->certificates[d->protocol].isNull();
#endif
    }
    else
    {
        return true;
    }
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


bool SignerResolvePage::isAsciiArmorEnabled() const
{
    return d->textArmorCO->isChecked();
}

void SignerResolvePage::setAsciiArmorEnabled( bool enabled )
{
    d->textArmorCO->setChecked( enabled );
}

bool SignerResolvePage::removeUnencryptedFile() const
{
    return d->removeUnencryptedCO->isChecked();
}

void SignerResolvePage::setRemoveUnencryptedFile( bool remove )
{
    d->removeUnencryptedCO->setChecked( remove );
}

#include "moc_signerresolvepage.cpp"

