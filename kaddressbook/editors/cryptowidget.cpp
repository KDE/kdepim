/*
    This file is part of KAddressBook.
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

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

QString CryptoWidgetFactory::pageTitle() const
{
  return "Crypto Settings";
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

  QVGroupBox* protGB = new QVGroupBox( i18n("Allowed protocols:"), this );
  topLayout->addMultiCellWidget( protGB,0,0,0,1 );

  uint msgFormat = 1;
  for ( uint i = 0 ; i < NumberOfProtocols ; ++i ) {
      Kleo::CryptoMessageFormat f = static_cast<Kleo::CryptoMessageFormat>( msgFormat );
      mProtocolCB[i] = new QCheckBox( Kleo::cryptoMessageFormatToLabel( f ), protGB );
      connect( mProtocolCB[i], SIGNAL( clicked() ), this, SLOT( setModified() ) );
      // Iterating over a bitfield means *2 every time
      msgFormat *= 2;
  }

  QLabel* l = new QLabel( i18n("Preferred OpenPGP encryption key:"), this );
  topLayout->addWidget( l,1,0 );

  mPgpKey =
	new Kleo::EncryptionKeyRequester( false, Kleo::EncryptionKeyRequester::OpenPGP, this );
  topLayout->addWidget( mPgpKey,1,1 );

  l = new QLabel( i18n("Preferred S/MIME encryption certificate:"), this );
  topLayout->addWidget( l,2,0 );

  mSmimeCert =
	new Kleo::EncryptionKeyRequester( false, Kleo::EncryptionKeyRequester::SMIME, this );
  topLayout->addWidget( mSmimeCert,2,1 );

  QGroupBox* box = new QVGroupBox( i18n("Message Preference"), this );
  topLayout->addMultiCellWidget( box, 3,3,0,1 );


  //send preferences/sign (see certmanager/lib/kleo/enum.h)
  QHBox* hbox = new QHBox(box);

  l = new QLabel( i18n("Sign:"), hbox );

  mSignPref = new QComboBox( false, hbox );
  mSignPref->insertItem( i18n("Ask") );
  //mSignPref->insertItem( i18n("Ask Whenever Possible") );
  mSignPref->insertItem( i18n("Always Sign") );
  //mSignPref->insertItem( i18n("Always Sign If Possible") );
  mSignPref->insertItem( i18n("Never Sign") );

  //send preferences/encrypt (see certmanager/lib/kleo/enum.h)
  hbox = new QHBox(box);

  l = new QLabel( i18n("Encrypt:"), hbox );

  mCryptPref = new QComboBox( false, hbox );
  mCryptPref->insertItem( i18n("Ask") );
  mCryptPref->insertItem( i18n("Ask Whenever Possible") );
  mCryptPref->insertItem( i18n("Always Encrypt") );
  mCryptPref->insertItem( i18n("Always Encrypt If Possible") );
  mCryptPref->insertItem( i18n("Never Encrypt") );

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

// Keep in sync with certmanager/lib/kleo/enum.cpp
static int encrypt_pref_string_to_int( const QString& str )
{
  if( str == "askAlways" ) return 0;
  if( str == "askWhenPossible" ) return 1;
  if( str == "always" ) return 2;
  if( str == "alwaysIfPossible" ) return 3;
  if( str == "never" ) return 4;
  else return 0; // default
}

static QString encrypt_pref_int_to_string( int i )
{
  static const char* str[5] = { "askAlways", "askWhenPossible", "always", "alwaysIfPossible", "never" };
  if( i >= 0 && i < 5 ) return QString::fromLatin1( str[i] );
  else return QString::null;
}

static int sign_pref_string_to_int( const QString& str )
{
  if( str == "askAlways" ) return 0;
  if( str == "always" ) return 1;
  if( str == "never" ) return 2;
  else return 0; // default
}

static QString sign_pref_int_to_string( int i )
{
  static const char* str[3] = { "askAlways", "always", "never" };
  if( i >= 0 && i < 3 ) return QString::fromLatin1( str[i] );
  else return QString::null;
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

  mSignPref->setCurrentItem( sign_pref_string_to_int(addr->custom( "KADDRESSBOOK",
                                                                   "CRYPTOSIGNPREF" )) );
  mCryptPref->setCurrentItem( encrypt_pref_string_to_int(addr->custom( "KADDRESSBOOK",
                                                                       "CRYPTOENCRYPTPREF" )) );

  // We dont use the contents of addr->key(...) because we want just a ref.
  // to the key/cert. stored elsewhere.

  mPgpKey->setFingerprint( addr->custom( "KADDRESSBOOK", "OPENPGPFP" ) );
  mSmimeCert->setFingerprint( addr->custom( "KADDRESSBOOK", "SMIMEFP" ) );

  blockSignals( blocked );
}

void CryptoWidget::storeContact( KABC::Addressee *addr )
{
  uint cryptoFormats = 0;
  uint msgFormat = 1;
  for ( uint i = 0 ; i < NumberOfProtocols ; ++i, msgFormat *= 2 ) {
      if ( mProtocolCB[i]->isChecked() )
          cryptoFormats |= msgFormat;
  }
  QStringList lst = Kleo::cryptoMessageFormatsToStringList(cryptoFormats);

  addr->insertCustom( "KADDRESSBOOK", "CRYPTOPROTOPREF", lst.join( "," ) );
  addr->insertCustom( "KADDRESSBOOK", "CRYPTOSIGNPREF",
                      sign_pref_int_to_string(mSignPref->currentItem()) );
  addr->insertCustom( "KADDRESSBOOK", "CRYPTOENCRYPTPREF",
                      encrypt_pref_int_to_string(mCryptPref->currentItem()) );

  QString pfp = mPgpKey->fingerprint();
  QString sfp = mSmimeCert->fingerprint();

  if( !pfp.isNull() ) {
	addr->insertCustom( "KADDRESSBOOK", "OPENPGPFP", pfp );
  } else {
	addr->removeCustom( "KADDRESSBOOK", "OPENPGPFP" );
  }

  if( !sfp.isNull() ) {
	addr->insertCustom( "KADDRESSBOOK", "SMIMEFP", sfp );
  } else {
	addr->removeCustom( "KADDRESSBOOK", "SMIMEFP" );
  }

}

void CryptoWidget::setReadOnly( bool readOnly )
{
  mReadOnly = readOnly;
  for ( uint i = 0 ; i < NumberOfProtocols ; ++i )
      mProtocolCB[i]->setEnabled( !readOnly );
  mSignPref->setEnabled( !readOnly );
  mCryptPref->setEnabled( !readOnly );
  mPgpKey->setEnabled( !readOnly );
  mSmimeCert->setEnabled( !readOnly );
}

#include "cryptowidget.moc"
