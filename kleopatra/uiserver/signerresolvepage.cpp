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
#include "certificateresolver.h"
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

using namespace GpgME;
using namespace Kleo;
using namespace boost;

namespace {

    class ValidatorImpl : public SignerResolvePage::Validator {
    public:
        QString explanation() const { return QString(); }
        bool isComplete() const { return true; }
    };
}

class SignerResolvePage::Private {
    friend class ::SignerResolvePage;
    SignerResolvePage * const q;
public:
    explicit Private( SignerResolvePage * qq );
    ~Private();

    void setOperation( int operation );
    void selectCertificates();
    void setCertificates( const QMap<GpgME::Protocol, GpgME::Key>& certs );
    void updateModeSelectionWidgets();
    void updateUi();

private:

    Operation operation;
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
    shared_ptr<SignerResolvePage::Validator> validator;
    shared_ptr<SigningPreferences> signingPreferences;
};

SignerResolvePage::Private::Private( SignerResolvePage * qq )
    : q( qq ), operation( SignAndEncrypt ), protocol( GpgME::UnknownProtocol ),
      signingMutable( true ), encryptionMutable( true ), 
      signingSelected( false ), encryptionSelected( false ), validator( new ValidatorImpl )

{
    QVBoxLayout* layout = new QVBoxLayout( q );
    layout->setSpacing( KDialog::spacingHint() );

    signEncryptGroup = new QButtonGroup( q );
    q->connect( signEncryptGroup, SIGNAL( buttonClicked( int ) ), q, SLOT( setOperation( int ) ) );

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
    removeUnencryptedCO->setChecked( false );
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
    updateUi();
}

void SignerResolvePage::setValidator( const boost::shared_ptr<SignerResolvePage::Validator>& validator )
{
    assert( validator );
    d->validator = validator;
    d->updateUi();
}

boost::shared_ptr<SignerResolvePage::Validator> SignerResolvePage::validator() const
{
    return d->validator;
}

SignerResolvePage::Private::~Private() {}


void SignerResolvePage::Private::setCertificates( const QMap<GpgME::Protocol, GpgME::Key>& certs )
{
    certificates = certs;
    const GpgME::Key cmsKey = certs[GpgME::CMS];
    cmsLabel->setText( !cmsKey.isNull() ? Formatting::formatForComboBox( cmsKey ) : i18n( "No certificate selected" ) );
    const GpgME::Key pgpKey = certs[GpgME::OpenPGP];
    pgpLabel->setText( !pgpKey.isNull() ? Formatting::formatForComboBox( pgpKey ) : i18n( "No certificate selected" )  );
    updateUi();
}

void SignerResolvePage::Private::updateUi()
{
    q->setExplanation( validator->explanation() );
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
        const QMap<Protocol, Key> certs = dlg->selectedCertificates(); 
        setCertificates( certs );
        if ( signingPreferences ) {
            signingPreferences->setPreferredCertificate( OpenPGP, certs.value( OpenPGP ) );
            signingPreferences->setPreferredCertificate( CMS, certs.value( CMS ) );    
        }
    }

    delete dlg;
    updateUi();
}

void SignerResolvePage::Private::setOperation( int mode_ )
{
    operation = static_cast<SignerResolvePage::Operation>( mode_ );
    signingBox->setEnabled( operation != EncryptOnly );
    encryptBox->setEnabled( operation != SignOnly );
    updateUi();
}


SignerResolvePage::Operation SignerResolvePage::operation() const
{
    return d->operation;
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
    d->updateUi();
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

std::vector<GpgME::Key> SignerResolvePage::signingCertificates( GpgME::Protocol protocol ) const
{
    std::vector<GpgME::Key> result;
    if ( protocol != GpgME::CMS && !d->certificates[GpgME::OpenPGP].isNull() )
            result.push_back( d->certificates[GpgME::OpenPGP] );
    if ( protocol != GpgME::OpenPGP && !d->certificates[GpgME::CMS].isNull() )
            result.push_back( d->certificates[GpgME::CMS] );
    return result;
}

std::vector<GpgME::Key> SignerResolvePage::resolvedSigners() const
{
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
    assert( d->validator );
    return d->validator->isComplete();
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

void SignerResolvePage::setSigningPreferences( const boost::shared_ptr<SigningPreferences>& prefs )
{
    d->signingPreferences = prefs; 
    QMap<Protocol,Key> map;
    map[OpenPGP] = prefs ? prefs->preferredCertificate( OpenPGP ) : Key();
    map[CMS] = prefs ? prefs->preferredCertificate( CMS ) : Key();
    d->setCertificates( map );
}

shared_ptr<SigningPreferences> SignerResolvePage::signingPreferences() const
{
    return d->signingPreferences;
}

void SignerResolvePage::onNext()
{
}

#include "moc_signerresolvepage.cpp"

