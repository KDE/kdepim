/*
    This file is part of KAddressBook.
    Copyright (c) 2004 Klar�vdalens Datakonsult AB

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <config.h>
#include "certmanager/lib/ui/keyrequester.h"
#include "certmanager/lib/cryptplugfactory.h"
#include "certmanager/lib/cryptplugwrapper.h"
#include "certmanager/lib/kleo/enum.h"

#include "gpgmepp/data.h"
#include "gpgmepp/key.h"

#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qvgroupbox.h>
#include <qhbox.h>

#include "cryptowidget.h"

extern "C" {
  void *init_libkaddrbk_cryptosettings()
  {
    return ( new CryptoWidgetFactory );
  }
}

CryptoWidgetFactory::CryptoWidgetFactory()
{
  KGlobal::locale()->insertCatalogue( "libkleopatra" );
  KGlobal::iconLoader()->addAppDir( "libkleopatra" );
}

QString CryptoWidgetFactory::pageTitle() const
{
  return i18n( "Crypto Settings" );
}

QString CryptoWidgetFactory::pageIdentifier() const
{
  return "crypto";
}

CryptoWidget::CryptoWidget( KABC::AddressBook *ab, QWidget *parent, const char *name )
  : KAB::ContactEditorWidget( ab, parent, name ), mReadOnly( false )
{
  QGridLayout *topLayout = new QGridLayout( this, 2, 5, KDialog::marginHint(),
                                            KDialog::spacingHint() );
  topLayout->setColStretch( 1, 1 );
  topLayout->setRowStretch( 4, 1 );

  QVGroupBox* protGB = new QVGroupBox( i18n( "Allowed Protocols" ), this );
  topLayout->addMultiCellWidget( protGB, 0, 0, 0, 1 );

  uint msgFormat = 1;
  for ( uint i = 0 ; i < NumberOfProtocols ; ++i ) {
    Kleo::CryptoMessageFormat f = static_cast<Kleo::CryptoMessageFormat>( msgFormat );
    mProtocolCB[ i ] = new QCheckBox( Kleo::cryptoMessageFormatToLabel( f ), protGB );
    connect( mProtocolCB[i], SIGNAL( clicked() ), this, SLOT( setModified() ) );

    // Iterating over a bitfield means *2 every time
    msgFormat *= 2;
  }

  QLabel* l = new QLabel( i18n( "Preferred OpenPGP encryption key:" ), this );
  topLayout->addWidget( l, 1, 0 );

  mPgpKey = new Kleo::EncryptionKeyRequester( true, Kleo::EncryptionKeyRequester::OpenPGP, this );
  topLayout->addWidget( mPgpKey, 1, 1 );

  l = new QLabel( i18n( "Preferred S/MIME encryption certificate:" ), this );
  topLayout->addWidget( l, 2, 0 );

  mSmimeCert = new Kleo::EncryptionKeyRequester( true, Kleo::EncryptionKeyRequester::SMIME, this );
  topLayout->addWidget( mSmimeCert, 2, 1 );

  QGroupBox* box = new QVGroupBox( i18n( "Message Preference" ), this );
  topLayout->addMultiCellWidget( box, 3, 3, 0, 1 );


  // Send preferences/sign (see certmanager/lib/kleo/enum.h)
  QHBox* hbox = new QHBox( box );

  l = new QLabel( i18n( "Sign:" ), hbox );

  mSignPref = new QComboBox( false, hbox );
  for ( unsigned int i = Kleo::UnknownSigningPreference; i < Kleo::MaxSigningPreference ; ++i )
    mSignPref->insertItem( Kleo::signingPreferenceToLabel(
                           static_cast<Kleo::SigningPreference>( i ) ) );

  // Send preferences/encrypt (see certmanager/lib/kleo/enum.h)
  hbox = new QHBox( box );

  l = new QLabel( i18n("Encrypt:"), hbox );

  mCryptPref = new QComboBox( false, hbox );
  for ( unsigned int i = Kleo::UnknownPreference; i < Kleo::MaxEncryptionPreference ; ++i )
    mCryptPref->insertItem( Kleo::encryptionPreferenceToLabel(
                            static_cast<Kleo::EncryptionPreference>( i ) ) );

  // Emit "changed()" signal
  connect( mSignPref, SIGNAL( activated(int) ), this, SLOT( setModified() ) );
  connect( mCryptPref, SIGNAL( activated(int) ), this, SLOT( setModified() ) );
  // Not optimal, but KeyRequester doesn't emit any signals when the key changes
  connect( mPgpKey->eraseButton(), SIGNAL( clicked() ), this, SLOT( setModified() ) );
  connect( mPgpKey->dialogButton(), SIGNAL( clicked() ), this, SLOT( setModified() ) );
  connect( mSmimeCert->eraseButton(), SIGNAL( clicked() ), this, SLOT( setModified() ) );
  connect( mSmimeCert->dialogButton(), SIGNAL( clicked() ), this, SLOT( setModified() ) );
}

CryptoWidget::~CryptoWidget()
{
}

void CryptoWidget::loadContact( KABC::Addressee *addr )
{
  bool blocked = signalsBlocked();
  blockSignals( true );

  QStringList lst = QStringList::split( ',', addr->custom( "KADDRESSBOOK",
                                                           "CRYPTOPROTOPREF" ) );
  uint cryptoFormats = Kleo::stringListToCryptoMessageFormats( lst );

  uint msgFormat = 1;
  for ( uint i = 0 ; i < NumberOfProtocols ; ++i, msgFormat *= 2 ) {
    mProtocolCB[i]->setChecked( cryptoFormats & msgFormat );
  }

  mSignPref->setCurrentItem( Kleo::stringToSigningPreference( addr->custom( "KADDRESSBOOK",
                                                                            "CRYPTOSIGNPREF" ) ) );
  mCryptPref->setCurrentItem( Kleo::stringToEncryptionPreference( addr->custom( "KADDRESSBOOK",
                                                                                "CRYPTOENCRYPTPREF" ) ) );

  // We dont use the contents of addr->key(...) because we want just a ref.
  // to the key/cert. stored elsewhere.

  mPgpKey->setFingerprints( QStringList::split( ",", addr->custom( "KADDRESSBOOK", "OPENPGPFP" ) ) );
  mSmimeCert->setFingerprints( QStringList::split( ",", addr->custom( "KADDRESSBOOK", "SMIMEFP" ) ) );

  blockSignals( blocked );
}

void CryptoWidget::storeContact( KABC::Addressee *addr )
{
  uint cryptoFormats = 0;
  uint msgFormat = 1;
  for ( uint i = 0 ; i < NumberOfProtocols ; ++i, msgFormat *= 2 ) {
    if ( mProtocolCB[ i ]->isChecked() )
      cryptoFormats |= msgFormat;
  }

  QStringList lst = Kleo::cryptoMessageFormatsToStringList( cryptoFormats );
  if ( !lst.isEmpty() )
    addr->insertCustom( "KADDRESSBOOK", "CRYPTOPROTOPREF", lst.join( "," ) );
  else
    addr->removeCustom( "KADDRESSBOOK", "CRYPTOPROTOPREF" );

  Kleo::SigningPreference signPref =
      static_cast<Kleo::SigningPreference>( mSignPref->currentItem() );
  if ( signPref != Kleo::UnknownSigningPreference )
    addr->insertCustom( "KADDRESSBOOK", "CRYPTOSIGNPREF",
                        Kleo::signingPreferenceToString( signPref ) );
  else
    addr->removeCustom( "KADDRESSBOOK", "CRYPTOSIGNPREF" );

  Kleo::EncryptionPreference encryptPref =
      static_cast<Kleo::EncryptionPreference>( mCryptPref->currentItem() );
  if ( encryptPref != Kleo::UnknownPreference )
    addr->insertCustom( "KADDRESSBOOK", "CRYPTOENCRYPTPREF",
                        Kleo::encryptionPreferenceToString( encryptPref ) );
  else
    addr->removeCustom( "KADDRESSBOOK", "CRYPTOENCRYPTPREF" );

  QStringList pfp = mPgpKey->fingerprints();
  QStringList sfp = mSmimeCert->fingerprints();

  if ( !pfp.isEmpty() )
    addr->insertCustom( "KADDRESSBOOK", "OPENPGPFP", pfp.join( "," ) );
  else
    addr->removeCustom( "KADDRESSBOOK", "OPENPGPFP" );

  if ( !sfp.isEmpty() )
    addr->insertCustom( "KADDRESSBOOK", "SMIMEFP", sfp.join( "," ) );
  else
    addr->removeCustom( "KADDRESSBOOK", "SMIMEFP" );
}

void CryptoWidget::setReadOnly( bool readOnly )
{
  mReadOnly = readOnly;
  for ( uint i = 0 ; i < NumberOfProtocols ; ++i )
    mProtocolCB[ i ]->setEnabled( !readOnly );

  mSignPref->setEnabled( !readOnly );
  mCryptPref->setEnabled( !readOnly );
  mPgpKey->setEnabled( !readOnly );
  mSmimeCert->setEnabled( !readOnly );
}

#include "cryptowidget.moc"
