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

#include "config.h" // ??
#include "certmanager/lib/ui/keyrequester.h"
#include "certmanager/lib/cryptplugfactory.h"
#include "certmanager/lib/cryptplugwrapper.h"

#include "gpgmepp/data.h"
#include "gpgmepp/key.h"

#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qvgroupbox.h>
#include <qhbox.h>

#include "cryptowidget.h"

extern "C" {
  void *init_libkaddrbk_crypto()
  {
	qDebug("init_libkaddrbk_crypto()");
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

  QLabel* l = new QLabel( i18n("Preferred protocol"), this );
  topLayout->addWidget( l,0,0 );
  mProtocol = new QComboBox( false, this );
  mProtocol->insertItem( i18n("Inline OpenPGP") );
  mProtocol->insertItem( i18n("OpenPGP/MIME") );
  mProtocol->insertItem( i18n("S/MIME") );
  mProtocol->insertItem( i18n("S/MIME (opaque)") );
  topLayout->addWidget( mProtocol,0,1 );

  l = new QLabel( i18n("Preferred OpenPGP encryption key"), this );
  topLayout->addWidget( l,1,0 );
  
  mPgpKey = 
	new Kleo::EncryptionKeyRequester( Kleo::CryptPlugFactory::instance()->openpgp(), false, this );
  topLayout->addWidget( mPgpKey,1,1 );

  l = new QLabel( i18n("Preferred S/MIME encryption certificate"), this );  
  topLayout->addWidget( l,2,0 );

  mSmimeCert = 
	new Kleo::EncryptionKeyRequester( Kleo::CryptPlugFactory::instance()->smime(), false, this );
  topLayout->addWidget( mSmimeCert,2,1 );
  
  QGroupBox* box = new QVGroupBox( i18n("Message preference"), this );
  topLayout->addMultiCellWidget( box, 3,3,0,1 );
  

  //send preferences/sign: always sign, ask, never sign
  QHBox* hbox = new QHBox(box);

  l = new QLabel( i18n("Sign"), hbox );

  mSignPref = new QComboBox( false, hbox );
  mSignPref->insertItem( i18n("Ask") );
  mSignPref->insertItem( i18n("Always sign") );
  mSignPref->insertItem( i18n("Never sign") );

  //send preferences/encrypt: always encrypt, ask, never encrypt
  hbox = new QHBox(box);

  l = new QLabel( i18n("Encrypt"), hbox );

  mCryptPref = new QComboBox( false, hbox );
  mCryptPref->insertItem( i18n("Ask") );
  mCryptPref->insertItem( i18n("Always encrypt") );
  mCryptPref->insertItem( i18n("Never encrypt") );

  // Emit "changed()" signal
  connect( mProtocol, SIGNAL( activated(int) ), this, SLOT( setModified() ) );
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

static int proto_string_to_int( const QString& _str )
{
  QString str = _str.lower();
  if( str == "openpgp(inline)" ) return 0;
  if( str == "openpgp/mime" )    return 1;
  if( str == "s/mime" )          return 2;
  if( str == "s/mime(opaque)" )  return 3;
  return 0; // default
}

static QString proto_int_to_string( int i )
{
  static QString str[4] = { "openpgp(inline)", "openpgp/mime", "s/mime", "s/mime(opaque)" };
  if( i >= 0 && i < 4 ) return str[i];
  else return QString::null;
}

static int pref_string_to_int( const QString& _str )
{
  QString str = _str.lower();
  if( str == "ask" ) return 0;
  if( str == "always" ) return 1;
  if( str == "never" ) return 2;
  else return 0; // default
}

static QString pref_int_to_string( int i )
{
  static QString str[3] = { "ask", "always", "never" };
  if( i >= 0 && i < 3 ) return str[i];
  else return QString::null;
}

void CryptoWidget::loadContact( KABC::Addressee *addr )
{
  bool blocked = signalsBlocked();
  blockSignals( true );

  mProtocol->setCurrentItem( proto_string_to_int(addr->custom( "KADDRESSBOOK", 
															   "CRYPTOPROTOPREF" )) );
  mSignPref->setCurrentItem( pref_string_to_int(addr->custom( "KADDRESSBOOK", 
															  "CRYPTOSIGNPREF" )) );
  mCryptPref->setCurrentItem( pref_string_to_int(addr->custom( "KADDRESSBOOK", 
															   "CRYPTOENCRYPTPREF" )) );

  // We dont use the contents of addr->key(...) because we want just a ref.
  // to the key/cert. stored elsewhere.

  mPgpKey->setFingerprint( addr->custom( "KADDRESSBOOK", "OPENPGPFP" ) );
  mSmimeCert->setFingerprint( addr->custom( "KADDRESSBOOK", "SMIMEFP" ) );

  blockSignals( blocked );
}

void CryptoWidget::storeContact( KABC::Addressee *addr )
{
  addr->insertCustom( "KADDRESSBOOK", "CRYPTOPROTOPREF", 
					  proto_int_to_string(mProtocol->currentItem()) );
  addr->insertCustom( "KADDRESSBOOK", "CRYPTOSIGNPREF", 
					  pref_int_to_string(mSignPref->currentItem()) );
  addr->insertCustom( "KADDRESSBOOK", "CRYPTOENCRYPTPREF", 
					  pref_int_to_string(mCryptPref->currentItem()) );

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
  mProtocol->setEnabled( !readOnly );
  mSignPref->setEnabled( !readOnly );
  mCryptPref->setEnabled( !readOnly );
  mPgpKey->setEnabled( !readOnly );
  mSmimeCert->setEnabled( !readOnly );
}

#include "cryptowidget.moc"
