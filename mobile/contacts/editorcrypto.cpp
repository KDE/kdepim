/*
    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "editorcrypto.h"

#include "ui_editorcrypto.h"

#include "libkleo/kleo/enum.h"

#include <KContacts/Addressee>

class EditorCrypto::Private
{
  EditorCrypto *const q;

  public:
    explicit Private( EditorCrypto *parent ) : q( parent )
    {
      mUi.setupUi( parent );

      mProtocolCB[ 0 ] = mUi.inlineOpenPGPCheckBox;
      mProtocolCB[ 1 ] = mUi.openPGPCheckBox;
      mProtocolCB[ 2 ] = mUi.smimeCheckBox;
      mProtocolCB[ 3 ] = mUi.smimeOpaqueCheckBox;

      mUi.openPGPKeyRequester->setMultipleKeysEnabled( true );
      mUi.openPGPKeyRequester->setAllowedKeys( Kleo::EncryptionKeyRequester::OpenPGP );

      mUi.smimeCertificateRequester->setMultipleKeysEnabled( true );
      mUi.smimeCertificateRequester->setAllowedKeys( Kleo::EncryptionKeyRequester::SMIME );

      for ( unsigned int i = Kleo::UnknownSigningPreference; i < Kleo::MaxSigningPreference ; ++i )
        mUi.signComboBox->addItem( Kleo::signingPreferenceToLabel( static_cast<Kleo::SigningPreference>( i ) ) );

      for ( unsigned int i = Kleo::UnknownPreference; i < Kleo::MaxEncryptionPreference ; ++i )
        mUi.encryptComboBox->addItem( Kleo::encryptionPreferenceToLabel( static_cast<Kleo::EncryptionPreference>( i ) ) );
    }

  public:
    Ui::EditorCrypto mUi;

    enum { NumberOfProtocols = 4 };
    QCheckBox* mProtocolCB[NumberOfProtocols];
};

static QString loadCustom( const KContacts::Addressee &contact, const QString &key )
{
  return contact.custom( QLatin1String( "KADDRESSBOOK" ), key );
}

static void storeCustom( KContacts::Addressee &contact, const QString &key, const QString &value )
{
  if ( value.isEmpty() )
    contact.removeCustom( QLatin1String( "KADDRESSBOOK" ), key );
  else
    contact.insertCustom( QLatin1String( "KADDRESSBOOK" ), key, value );
}


EditorCrypto::EditorCrypto( QWidget *parent )
  : EditorBase( parent ), d( new Private( this ) )
{
}

EditorCrypto::~EditorCrypto()
{
  delete d;
}

void EditorCrypto::loadContact( const KContacts::Addressee &contact, const Akonadi::ContactMetaData& )
{
  const QStringList protocolPrefs = loadCustom( contact, QLatin1String( "CRYPTOPROTOPREF" ) ).split( QLatin1Char( ',' ), QString::SkipEmptyParts );
  const uint cryptoFormats = Kleo::stringListToCryptoMessageFormats( protocolPrefs );

  uint msgFormat = 1;
  for ( uint i = 0 ; i < Private::NumberOfProtocols ; ++i, msgFormat *= 2 )
    d->mProtocolCB[i]->setChecked( cryptoFormats & msgFormat );

  d->mUi.signComboBox->setCurrentIndex( Kleo::stringToSigningPreference( loadCustom( contact, QLatin1String( "CRYPTOSIGNPREF" ) ) ) );
  d->mUi.encryptComboBox->setCurrentIndex( Kleo::stringToEncryptionPreference( loadCustom( contact, QLatin1String( "CRYPTOENCRYPTPREF" ) ) ) );

  d->mUi.openPGPKeyRequester->setFingerprints( loadCustom( contact, QLatin1String( "OPENPGPFP" ) ).split( QLatin1Char( ',' ), QString::SkipEmptyParts ) );
  d->mUi.smimeCertificateRequester->setFingerprints( loadCustom( contact, QLatin1String( "SMIMEFP" ) ).split( QLatin1Char( ',' ), QString::SkipEmptyParts ) );
}

void EditorCrypto::saveContact( KContacts::Addressee &contact, Akonadi::ContactMetaData& ) const
{
  uint cryptoFormats = 0;
  uint msgFormat = 1;
  for ( uint i = 0 ; i < Private::NumberOfProtocols ; ++i, msgFormat *= 2 ) {
    if ( d->mProtocolCB[ i ]->isChecked() )
      cryptoFormats |= msgFormat;
  }

  const QStringList protocolPref = Kleo::cryptoMessageFormatsToStringList( cryptoFormats );
  storeCustom( contact, QLatin1String( "CRYPTOPROTOPREF" ), protocolPref.join( QLatin1String( "," ) ) );

  const Kleo::SigningPreference signPref = static_cast<Kleo::SigningPreference>( d->mUi.signComboBox->currentIndex() );
  if ( signPref != Kleo::UnknownSigningPreference )
    contact.insertCustom( QLatin1String("KADDRESSBOOK"), QLatin1String("CRYPTOSIGNPREF"), QLatin1String(Kleo::signingPreferenceToString( signPref )) );
  else
    contact.removeCustom(QLatin1String( "KADDRESSBOOK"), QLatin1String("CRYPTOSIGNPREF") );

  const Kleo::EncryptionPreference encryptPref = static_cast<Kleo::EncryptionPreference>( d->mUi.encryptComboBox->currentIndex() );
  if ( encryptPref != Kleo::UnknownPreference )
    contact.insertCustom( QLatin1String("KADDRESSBOOK"), QLatin1String("CRYPTOENCRYPTPREF"), QLatin1String(Kleo::encryptionPreferenceToString( encryptPref )) );
  else
    contact.removeCustom( QLatin1String("KADDRESSBOOK"), QLatin1String("CRYPTOENCRYPTPREF") );

  const QStringList pfp = d->mUi.openPGPKeyRequester->fingerprints();
  storeCustom( contact, QLatin1String( "OPENPGPFP" ), pfp.join( QLatin1String( "," ) ) );

  const QStringList sfp = d->mUi.smimeCertificateRequester->fingerprints();
  storeCustom( contact, QLatin1String( "SMIMEFP" ), sfp.join( QLatin1String( "," ) ) );
}

