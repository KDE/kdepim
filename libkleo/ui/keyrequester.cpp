/*  -*- c++ -*-
    keyrequester.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klar�vdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
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


    Based on kpgpui.cpp
    Copyright (C) 2001,2002 the KPGP authors
    See file libkdenetwork/AUTHORS.kpgp for details

    This file is part of KPGP, the KDE PGP/GnuPG support library.

    KPGP is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "keyrequester.h"

#include "keyselectiondialog.h"

#include "kleo/keylistjob.h"
#include "kleo/dn.h"
#include "kleo/cryptobackendfactory.h"

// gpgme++
#include <gpgme++/key.h>
#include <gpgme++/keylistresult.h>

// KDE
#include <KLocalizedString>
#include <kiconloader.h>
#include <QDialog>
#include <kdebug.h>
#include <kmessagebox.h>
#include <qpushbutton.h>

// Qt
#include <QApplication>

#include <QString>
#include <QStringList>
#include <QLabel>

#include <QHBoxLayout>

#include <assert.h>

Kleo::KeyRequester::KeyRequester( unsigned int allowedKeys, bool multipleKeys,
                                  QWidget * parent )
  : QWidget( parent ),
    mOpenPGPBackend( 0 ),
    mSMIMEBackend( 0 ),
    mMulti( multipleKeys ),
    mKeyUsage( allowedKeys ),
    mJobs( 0 ),
    d( 0 )
{
  init();
}

Kleo::KeyRequester::KeyRequester( QWidget * parent )
  : QWidget( parent ),
    mOpenPGPBackend( 0 ),
    mSMIMEBackend( 0 ),
    mMulti( false ),
    mKeyUsage( 0 ),
    mJobs( 0 ),
    d( 0 )
{
  init();
}

void Kleo::KeyRequester::init()
{
  QHBoxLayout * hlay = new QHBoxLayout( this );
//TODO PORT QT5   hlay->setSpacing( QDialog::spacingHint() );
  hlay->setMargin( 0 );

  // the label where the key id is to be displayed:
  mLabel = new QLabel( this );
  mLabel->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );

  // the button to unset any key:
  mEraseButton = new QPushButton( this );
  mEraseButton->setAutoDefault( false );
  mEraseButton->setSizePolicy( QSizePolicy( QSizePolicy::Minimum,
                                            QSizePolicy::Minimum ) );
  mEraseButton->setIcon( QIcon::fromTheme( QApplication::isRightToLeft() ? QLatin1String("edit-clear-locationbar-ltr") : QLatin1String("edit-clear-locationbar-rtl") ) );
  mEraseButton->setToolTip( i18n("Clear") );

  // the button to call the KeySelectionDialog:
  mDialogButton = new QPushButton( i18n("Change..."), this );
  mDialogButton->setAutoDefault( false );

  hlay->addWidget( mLabel, 1 );
  hlay->addWidget( mEraseButton );
  hlay->addWidget( mDialogButton );

  connect( mEraseButton,  SIGNAL(clicked()), SLOT(slotEraseButtonClicked()) );
  connect( mDialogButton, SIGNAL(clicked()), SLOT(slotDialogButtonClicked()) );

  setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding,
                              QSizePolicy::Fixed ) );

  setAllowedKeys( mKeyUsage );
}

Kleo::KeyRequester::~KeyRequester() {

}

const std::vector<GpgME::Key> & Kleo::KeyRequester::keys() const {
  return mKeys;
}

const GpgME::Key & Kleo::KeyRequester::key() const {
  static const GpgME::Key null = GpgME::Key::null;
  if ( mKeys.empty() )
    return null;
  else
    return mKeys.front();
}

void Kleo::KeyRequester::setKeys( const std::vector<GpgME::Key> & keys ) {
  mKeys.clear();
  for ( std::vector<GpgME::Key>::const_iterator it = keys.begin() ; it != keys.end() ; ++it )
    if ( !it->isNull() )
      mKeys.push_back( *it );
  updateKeys();
}

void Kleo::KeyRequester::setKey( const GpgME::Key & key ) {
  mKeys.clear();
  if ( !key.isNull() )
    mKeys.push_back( key );
  updateKeys();
}

QString Kleo::KeyRequester::fingerprint() const {
  if ( mKeys.empty() )
    return QString();
  else
    return QLatin1String(mKeys.front().primaryFingerprint());
}

QStringList Kleo::KeyRequester::fingerprints() const {
  QStringList result;
  for ( std::vector<GpgME::Key>::const_iterator it = mKeys.begin() ; it != mKeys.end() ; ++it )
    if ( !it->isNull() )
      if ( const char * fpr = it->primaryFingerprint() )
        result.push_back( QLatin1String(fpr) );
  return result;
}

void Kleo::KeyRequester::setFingerprint( const QString & fingerprint ) {
  startKeyListJob( QStringList( fingerprint ) );
}

void Kleo::KeyRequester::setFingerprints( const QStringList & fingerprints ) {
  startKeyListJob( fingerprints );
}

void Kleo::KeyRequester::updateKeys() {
  if ( mKeys.empty() ) {
    mLabel->clear();
    return;
  }
  if ( mKeys.size() > 1 )
    setMultipleKeysEnabled( true );

  QStringList labelTexts;
  QString toolTipText;
  for ( std::vector<GpgME::Key>::const_iterator it = mKeys.begin() ; it != mKeys.end() ; ++it ) {
    if ( it->isNull() )
      continue;
    const QString fpr = QLatin1String(it->primaryFingerprint());
    labelTexts.push_back( fpr.right(8) );
    toolTipText += fpr.right(8) + QLatin1String(": ");
    if ( const char * uid = it->userID(0).id() )
      if ( it->protocol() == GpgME::OpenPGP )
        toolTipText += QString::fromUtf8( uid );
      else
        toolTipText += Kleo::DN( uid ).prettyDN();
    else
      toolTipText += xi18n("<placeholder>unknown</placeholder>");
    toolTipText += QLatin1Char('\n');
  }

  mLabel->setText( labelTexts.join(QLatin1String(", ")) );
  mLabel->setToolTip( toolTipText );
}

#ifndef __KLEO_UI_SHOW_KEY_LIST_ERROR_H__
#define __KLEO_UI_SHOW_KEY_LIST_ERROR_H__
static void showKeyListError( QWidget * parent, const GpgME::Error & err ) {
  assert( err );
  const QString msg = i18n( "<qt><p>An error occurred while fetching "
                            "the keys from the backend:</p>"
                            "<p><b>%1</b></p></qt>" ,
      QString::fromLocal8Bit( err.asString() ) );

  KMessageBox::error( parent, msg, i18n( "Key Listing Failed" ) );
}
#endif // __KLEO_UI_SHOW_KEY_LIST_ERROR_H__

void Kleo::KeyRequester::startKeyListJob( const QStringList & fingerprints ) {
  if ( !mSMIMEBackend && !mOpenPGPBackend )
    return;

  mTmpKeys.clear();
  mJobs = 0;

  unsigned int count = 0;
  for ( QStringList::const_iterator it = fingerprints.begin() ; it != fingerprints.end() ; ++it )
    if ( !(*it).trimmed().isEmpty() )
      ++count;

  if ( !count ) {
    // don't fall into the trap that an empty pattern means
    // "return all keys" :)
    setKey( GpgME::Key::null );
    return;
  }

  if ( mOpenPGPBackend ) {
    KeyListJob * job = mOpenPGPBackend->keyListJob( false ); // local, no sigs
    if ( !job ) {
      KMessageBox::error( this,
                          i18n("The OpenPGP backend does not support listing keys. "
                               "Check your installation."),
                          i18n("Key Listing Failed") );
    } else {
      connect( job, SIGNAL(result(GpgME::KeyListResult)),
               SLOT(slotKeyListResult(GpgME::KeyListResult)) );
      connect( job, SIGNAL(nextKey(GpgME::Key)),
               SLOT(slotNextKey(GpgME::Key)) );

      const GpgME::Error err = job->start( fingerprints,
        mKeyUsage & Kleo::KeySelectionDialog::SecretKeys &&
        !( mKeyUsage & Kleo::KeySelectionDialog::PublicKeys ) );

      if ( err )
        showKeyListError( this, err );
      else
        ++mJobs;
    }
  }

  if ( mSMIMEBackend ) {
    KeyListJob * job = mSMIMEBackend->keyListJob( false ); // local, no sigs
    if ( !job ) {
      KMessageBox::error( this,
                          i18n("The S/MIME backend does not support listing keys. "
                               "Check your installation."),
                          i18n("Key Listing Failed") );
    } else {
      connect( job, SIGNAL(result(GpgME::KeyListResult)),
               SLOT(slotKeyListResult(GpgME::KeyListResult)) );
      connect( job, SIGNAL(nextKey(GpgME::Key)),
               SLOT(slotNextKey(GpgME::Key)) );

      const GpgME::Error err = job->start( fingerprints,
        mKeyUsage & Kleo::KeySelectionDialog::SecretKeys &&
        !( mKeyUsage & Kleo::KeySelectionDialog::PublicKeys ) );

      if ( err )
        showKeyListError( this, err );
      else
        ++mJobs;
    }
  }

  if ( mJobs > 0 ) {
    mEraseButton->setEnabled( false );
    mDialogButton->setEnabled( false );
  }
}

void Kleo::KeyRequester::slotNextKey( const GpgME::Key & key ) {
  if ( !key.isNull() )
    mTmpKeys.push_back( key );
}

void Kleo::KeyRequester::slotKeyListResult( const GpgME::KeyListResult & res ) {
  if ( res.error() )
    showKeyListError( this, res.error() );

  if ( --mJobs <= 0 ) {
    mEraseButton->setEnabled( true );
    mDialogButton->setEnabled( true );

    setKeys( mTmpKeys );
    mTmpKeys.clear();
  }
}


void Kleo::KeyRequester::slotDialogButtonClicked() {
#ifndef KDEPIM_MOBILE_UI
  KeySelectionDialog * dlg = mKeys.empty()
    ? new KeySelectionDialog( mDialogCaption, mDialogMessage, mInitialQuery, mKeyUsage, mMulti, false, this )
    : new KeySelectionDialog( mDialogCaption, mDialogCaption, mKeys, mKeyUsage, mMulti, false, this ) ;
#else
  KeySelectionDialog * dlg = mKeys.empty()
    ? new KeySelectionDialog( mDialogCaption, mDialogMessage, mInitialQuery, mKeyUsage, mMulti, false, 0 )
    : new KeySelectionDialog( mDialogCaption, mDialogCaption, mKeys, mKeyUsage, mMulti, false, 0 ) ;
#endif

  if ( dlg->exec() == QDialog::Accepted ) {
    if ( mMulti )
      setKeys( dlg->selectedKeys() );
    else
      setKey( dlg->selectedKey() );
    emit changed();
  }

  delete dlg;
}

void Kleo::KeyRequester::slotEraseButtonClicked() {
  if ( !mKeys.empty() )
    emit changed();
  mKeys.clear();
  updateKeys();
}

void Kleo::KeyRequester::setDialogCaption( const QString & caption ) {
  mDialogCaption = caption;
}

void Kleo::KeyRequester::setDialogMessage( const QString & msg ) {
  mDialogMessage = msg;
}

bool Kleo::KeyRequester::isMultipleKeysEnabled() const {
  return mMulti;
}

void Kleo::KeyRequester::setMultipleKeysEnabled( bool multi ) {
  if ( multi == mMulti ) return;

  if ( !multi && !mKeys.empty() )
    mKeys.erase( mKeys.begin() + 1, mKeys.end() );

  mMulti = multi;
  updateKeys();
}

unsigned int Kleo::KeyRequester::allowedKeys() const {
  return mKeyUsage;
}

void Kleo::KeyRequester::setAllowedKeys( unsigned int keyUsage ) {
  mKeyUsage = keyUsage;
  mOpenPGPBackend = 0;
  mSMIMEBackend = 0;

  if ( mKeyUsage & KeySelectionDialog::OpenPGPKeys )
    mOpenPGPBackend = Kleo::CryptoBackendFactory::instance()->openpgp();
  if ( mKeyUsage & KeySelectionDialog::SMIMEKeys )
    mSMIMEBackend = Kleo::CryptoBackendFactory::instance()->smime();

  if ( mOpenPGPBackend && !mSMIMEBackend ) {
    mDialogCaption = i18n("OpenPGP Key Selection");
    mDialogMessage = i18n("Please select an OpenPGP key to use.");
  } else if ( !mOpenPGPBackend && mSMIMEBackend ) {
    mDialogCaption = i18n("S/MIME Key Selection");
    mDialogMessage = i18n("Please select an S/MIME key to use.");
  } else {
    mDialogCaption = i18n("Key Selection");
    mDialogMessage = i18n("Please select an (OpenPGP or S/MIME) key to use.");
  }
}

QPushButton * Kleo::KeyRequester::dialogButton() {
  return mDialogButton;
}

QPushButton * Kleo::KeyRequester::eraseButton() {
  return mEraseButton;
}

static inline unsigned int foo( bool openpgp, bool smime, bool trusted, bool valid ) {
  unsigned int result = 0;
  if ( openpgp )
    result |= Kleo::KeySelectionDialog::OpenPGPKeys;
  if ( smime )
    result |= Kleo::KeySelectionDialog::SMIMEKeys;
  if ( trusted )
    result |= Kleo::KeySelectionDialog::TrustedKeys;
  if ( valid )
    result |= Kleo::KeySelectionDialog::ValidKeys;
  return result;
}

static inline unsigned int encryptionKeyUsage( bool openpgp, bool smime, bool trusted, bool valid ) {
  return foo( openpgp, smime, trusted, valid ) | Kleo::KeySelectionDialog::EncryptionKeys | Kleo::KeySelectionDialog::PublicKeys;
}

static inline unsigned int signingKeyUsage( bool openpgp, bool smime, bool trusted, bool valid ) {
  return foo( openpgp, smime, trusted, valid ) | Kleo::KeySelectionDialog::SigningKeys | Kleo::KeySelectionDialog::SecretKeys;
}

Kleo::EncryptionKeyRequester::EncryptionKeyRequester( bool multi, unsigned int proto,
                                                      QWidget * parent,
                                                      bool onlyTrusted, bool onlyValid )
  : KeyRequester( encryptionKeyUsage( proto & OpenPGP, proto & SMIME, onlyTrusted, onlyValid ), multi,
                  parent ),d(0)
{
}

Kleo::EncryptionKeyRequester::EncryptionKeyRequester( QWidget * parent )
  : KeyRequester( 0, false, parent ),d(0)
{
}

Kleo::EncryptionKeyRequester::~EncryptionKeyRequester() {}


void Kleo::EncryptionKeyRequester::setAllowedKeys( unsigned int proto, bool onlyTrusted, bool onlyValid )
{
  KeyRequester::setAllowedKeys( encryptionKeyUsage( proto & OpenPGP, proto & SMIME, onlyTrusted, onlyValid ) );
}

Kleo::SigningKeyRequester::SigningKeyRequester( bool multi, unsigned int proto,
                                                QWidget * parent,
                                                bool onlyTrusted, bool onlyValid )
  : KeyRequester( signingKeyUsage( proto & OpenPGP, proto & SMIME, onlyTrusted, onlyValid ), multi,
                  parent ),d(0)
{
}

Kleo::SigningKeyRequester::SigningKeyRequester( QWidget * parent )
  : KeyRequester( 0, false, parent ),d(0)
{
}

Kleo::SigningKeyRequester::~SigningKeyRequester() {}

void Kleo::SigningKeyRequester::setAllowedKeys( unsigned int proto, bool onlyTrusted, bool onlyValid )
{
  KeyRequester::setAllowedKeys( signingKeyUsage( proto & OpenPGP, proto & SMIME, onlyTrusted, onlyValid ) );
}

void Kleo::KeyRequester::virtual_hook( int, void* ) {}
void Kleo::EncryptionKeyRequester::virtual_hook( int id, void * data ) {
  KeyRequester::virtual_hook( id, data );
}
void Kleo::SigningKeyRequester::virtual_hook( int id, void * data ) {
  KeyRequester::virtual_hook( id, data );
}

