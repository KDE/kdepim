/*  -*- c++ -*-
    keyselectiondialog.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "keyselectiondialog.h"

#include "keylistview.h"
#include "progressdialog.h"

#include <kleo/dn.h>
#include <kleo/cryptobackend.h>
#include <kleo/keylistjob.h>

#include <cryptplugwrapper.h>
#include <cryptplugwrapperlist.h>
#include <cryptplugfactory.h>

// gpgme++
#include <gpgmepp/key.h>
#include <gpgmepp/keylistresult.h>

// KDE
#include <klocale.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kwin.h>
#include <kconfig.h>
#include <kmessagebox.h>

// Qt
#include <qcheckbox.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qwhatsthis.h>
//#include <qdatetime.h>
#include <qpopupmenu.h>
#include <qregexp.h>

#include <string.h>
#include <assert.h>

static bool checkKeyUsage( const GpgME::Key & key, unsigned int keyUsage ) {
  

  if ( keyUsage & Kleo::KeySelectionDialog::ValidKeys &&
       ( key.isInvalid() || key.isExpired() || key.isRevoked() || key.isDisabled() ) )
    return false;

  if ( keyUsage & Kleo::KeySelectionDialog::EncryptionKeys &&
       !key.canEncrypt() )
    return false;
  if ( keyUsage & Kleo::KeySelectionDialog::SigningKeys &&
       !key.canSign() )
    return false;
  if ( keyUsage & Kleo::KeySelectionDialog::CertificationKeys &&
       !key.canCertify() )
    return false;
  if ( keyUsage & Kleo::KeySelectionDialog::AuthenticationKeys &&
       !key.canAuthenticate() )
    return false;

  if ( keyUsage & Kleo::KeySelectionDialog::SecretKeys &&
       !( keyUsage & Kleo::KeySelectionDialog::PublicKeys ) &&
       !key.isSecret() )
    return false;

  if ( keyUsage & Kleo::KeySelectionDialog::PublicKeys &&
       !( keyUsage & Kleo::KeySelectionDialog::SecretKeys ) &&
       key.isSecret() )
    return false;

  if ( keyUsage & Kleo::KeySelectionDialog::TrustedKeys &&
       key.protocol() == GpgME::Context::OpenPGP )
    switch ( key.userID(0).validity() ) {
    case GpgME::UserID::Unknown:
    case GpgME::UserID::Undefined:
    case GpgME::UserID::Never:
      return false;
    case GpgME::UserID::Marginal:
    case GpgME::UserID::Full:
    case GpgME::UserID::Ultimate:
      ;
    }
  // X.509 keys are always trusted, else they won't be the keybox.
  // PENDING(marc) check that this ^ is correct

  return true;
}

static bool checkKeyUsage( const std::vector<GpgME::Key> & keys, unsigned int keyUsage ) {
  for ( std::vector<GpgME::Key>::const_iterator it = keys.begin() ; it != keys.end() ; ++it )
    if ( !checkKeyUsage( *it, keyUsage ) )
      return false;
  return true;
}

namespace {

  class ColumnStrategy : public Kleo::KeyListView::ColumnStrategy {
  public:
    ColumnStrategy( unsigned int keyUsage );

    QString title( int col ) const;
    QString text( const GpgME::Key & key, int col ) const;
    const QPixmap * pixmap( const GpgME::Key & key, int col ) const;

  private:
    const QPixmap mKeyGoodPix, mKeyBadPix, mKeyUnknownPix, mKeyValidPix;
    const unsigned int mKeyUsage;
  };

  ColumnStrategy::ColumnStrategy( unsigned int keyUsage )
    : Kleo::KeyListView::ColumnStrategy(),
      mKeyGoodPix( UserIcon( "key_ok" ) ),
      mKeyBadPix( UserIcon( "key_bad" ) ),
      mKeyUnknownPix( UserIcon( "key_unknown" ) ),
      mKeyValidPix( UserIcon( "key" ) ),
      mKeyUsage( keyUsage )
  {
    kdWarning( keyUsage == 0, 5150 )
      << "KeySelectionDialog: keyUsage == 0. You want to use AllKeys instead." << endl;
  }

  QString ColumnStrategy::title( int col ) const {
    switch ( col ) {
    case 0: return i18n("Key ID");
    case 1: return i18n("User ID");
    default: return QString::null;
    }
  }

  QString ColumnStrategy::text( const GpgME::Key & key, int col ) const {
    switch ( col ) {
    case 0:
      {
	const GpgME::Subkey subkey = key.subkey(0);
	if ( subkey.fingerprint() )
	  return QString::fromUtf8( subkey.fingerprint() ).right( 8 );
	else
	  return i18n("<unknown>");
      }
      break;
    case 1:
      {
	const char * uid = key.userID(0).id();
	if ( key.protocol() == GpgME::Context::OpenPGP )
	  return uid && *uid ? QString::fromUtf8( uid ) : QString::null ;
	else // CMS
	  return Kleo::DN( uid ).prettyDN();
      }
      break;
    default: return QString::null;
    }
  }

  const QPixmap * ColumnStrategy::pixmap( const GpgME::Key & key, int col ) const {
    if ( col != 0 )
      return 0;
    if ( !checkKeyUsage( key, mKeyUsage ) )
      return &mKeyBadPix;

    if ( key.protocol() == GpgME::Context::CMS )
      return &mKeyGoodPix;

    switch ( key.userID(0).validity() ) {
    default:
    case GpgME::UserID::Unknown:
    case GpgME::UserID::Undefined:
      return &mKeyUnknownPix;
    case GpgME::UserID::Never:
      return &mKeyValidPix;
    case GpgME::UserID::Marginal:
    case GpgME::UserID::Full:
    case GpgME::UserID::Ultimate:
      return &mKeyGoodPix;
    }
  }

}


static const int sCheckSelectionDelay = 250;

Kleo::KeySelectionDialog::KeySelectionDialog( const QString & title,
					      const QString & text,
					      const CryptoBackend * backend,
					      const std::vector<GpgME::Key> & selectedKeys,
					      unsigned int keyUsage,
					      bool extendedSelection,
					      bool rememberChoice,
					      QWidget * parent, const char * name,
					      bool modal )
  : KDialogBase( parent, name, modal, title, Default|Ok|Cancel, Ok ),
    mBackend( backend ),
    mRememberCB( 0 ),
    mSelectedKeys( selectedKeys ),
    mKeyUsage( keyUsage ),
    mCurrentContextMenuItem( 0 )
{
  QSize dialogSize( 580, 400 );
  if ( kapp ) {
    KWin::setIcons( winId(), kapp->icon(), kapp->miniIcon() );

    KConfigGroup dialogConfig( KGlobal::config(), "Key Selection Dialog" );
    dialogSize = dialogConfig.readSizeEntry( "Dialog size", &dialogSize );
  }
  resize( dialogSize );

  mCheckSelectionTimer = new QTimer( this );
  mStartSearchTimer = new QTimer( this );

  QFrame *page = makeMainWidget();
  QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );

  if( !text.isEmpty() )
    topLayout->addWidget( new QLabel( text, page ) );

  QHBoxLayout * hlay = new QHBoxLayout( topLayout ); // inherits spacing
  QLineEdit * le = new QLineEdit( page );
  hlay->addWidget( new QLabel( le, i18n("&Search for:"), page ) );
  hlay->addWidget( le, 1 );
  le->setFocus();

  connect( le, SIGNAL(textChanged(const QString&)),
	   this, SLOT(slotSearch(const QString&)) );
  connect( mStartSearchTimer, SIGNAL(timeout()), SLOT(slotFilter()) );

  mKeyListView = new KeyListView( new ColumnStrategy( keyUsage ), page, "mKeyListView" );
  mKeyListView->setResizeMode( QListView::LastColumn );
  mKeyListView->setRootIsDecorated( true );
  mKeyListView->setShowSortIndicator( true );
  mKeyListView->setSorting( 1, true ); // sort by User ID
  mKeyListView->setShowToolTips( true );
  if ( extendedSelection )
    mKeyListView->setSelectionMode( QListView::Extended );
  topLayout->addWidget( mKeyListView, 10 );

  if ( rememberChoice ) {
    mRememberCB = new QCheckBox( i18n("Remember choice"), page );
    topLayout->addWidget( mRememberCB );
    QWhatsThis::add( mRememberCB,
		     i18n("<qt><p>If you check this box your choice will "
			  "be stored and you will not be asked again."
			  "</p></qt>") );
  }

  if ( extendedSelection )
    connect( mCheckSelectionTimer, SIGNAL(timeout()),
             SLOT(slotCheckSelection()) );
  connectSignals();

  connect( mKeyListView,
	   SIGNAL(doubleClicked(Kleo::KeyListViewItem*,const QPoint&,int)),
	   SLOT(accept()) );
  connect( mKeyListView,
	   SIGNAL(contextMenuRequested(Kleo::KeyListViewItem*,const QPoint&,int)),
           SLOT(slotRMB(Kleo::KeyListViewItem*,const QPoint&,int)) );

  setButtonText( KDialogBase::Default, i18n("&Reread Keys") );
  connect( this, SIGNAL(defaultClicked()),
           this, SLOT(slotRereadKeys()) );

  slotRereadKeys();
}

Kleo::KeySelectionDialog::~KeySelectionDialog() {
  KConfigGroup dialogConfig( KGlobal::config(), "Key Selection Dialog" );
  dialogConfig.writeEntry( "Dialog size", size() );
  dialogConfig.sync();
}


void Kleo::KeySelectionDialog::connectSignals() {
  if ( mKeyListView->isMultiSelection() )
    connect( mKeyListView, SIGNAL(selectionChanged()),
             SLOT(slotSelectionChanged()) );
  else
    connect( mKeyListView, SIGNAL(selectionChanged(Kleo::KeyListViewItem*)),
             SLOT(slotCheckSelection(Kleo::KeyListViewItem*)) );
}

void Kleo::KeySelectionDialog::disconnectSignals() {
  if ( mKeyListView->isMultiSelection() )
    disconnect( mKeyListView, SIGNAL(selectionChanged()),
		this, SLOT(slotSelectionChanged()) );
  else
    disconnect( mKeyListView, SIGNAL(selectionChanged(Kleo::KeyListViewItem*)),
		this, SLOT(slotCheckSelection(Kleo::KeyListViewItem*)) );
}

const GpgME::Key & Kleo::KeySelectionDialog::selectedKey() const {
  if ( mKeyListView->isMultiSelection() || !mKeyListView->selectedItem() )
    return GpgME::Key::null;
  return mKeyListView->selectedItem()->key();
}

#ifdef TEMPORARILY_REMOVED
QString KeySelectionDialog::keyInfo( const Kpgp::Key *key ) const
{
  QString status, remark;
  if( key->revoked() ) {
    status = i18n("Revoked");
  }
  else if( key->expired() ) {
    status = i18n("Expired");
  }
  else if( key->disabled() ) {
    status = i18n("Disabled");
  }
  else if( key->invalid() ) {
    status = i18n("Invalid");
  }
  else {
    Validity keyTrust = key->keyTrust();
    switch( keyTrust ) {
    case KPGP_VALIDITY_UNDEFINED:
      status = i18n("Undefined trust");
      break;
    case KPGP_VALIDITY_NEVER:
      status = i18n("Untrusted");
      break;
    case KPGP_VALIDITY_MARGINAL:
      status = i18n("Marginally trusted");
      break;
    case KPGP_VALIDITY_FULL:
      status = i18n("Fully trusted");
      break;
    case KPGP_VALIDITY_ULTIMATE:
      status = i18n("Ultimately trusted");
      break;
    case KPGP_VALIDITY_UNKNOWN:
    default:
      status = i18n("Unknown");
    }
    if( key->secret() ) {
      remark = i18n("Secret key available");
    }
    else if( !key->canEncrypt() ) {
      remark = i18n("Sign only key");
    }
    else if( !key->canSign() ) {
      remark = i18n("Encryption only key");
    }
  }

  QDateTime dt;
  dt.setTime_t( key->creationDate() );
  if( remark.isEmpty() ) {
    return " " + i18n("creation date and status of an OpenPGP key",
                      "Creation date: %1, Status: %2")
                     .arg( KGlobal::locale()->formatDate( dt.date(), true ) )
                     .arg( status );
  }
  else {
    return " " + i18n("creation date, status and remark of an OpenPGP key",
                      "Creation date: %1, Status: %2 (%3)")
                     .arg( KGlobal::locale()->formatDate( dt.date(), true ) )
                     .arg( status )
                     .arg( remark );
  }
}
#endif

#ifdef TEMPORARILY_REMOVED
QString KeySelectionDialog::beautifyFingerprint( const QCString& fpr ) const
{
  QCString result;

  if( 40 == fpr.length() ) {
    // convert to this format:
    // 0000 1111 2222 3333 4444  5555 6666 7777 8888 9999
    result.fill( ' ', 50 );
    memcpy( result.data()     , fpr.data()     , 4 );
    memcpy( result.data() +  5, fpr.data() +  4, 4 );
    memcpy( result.data() + 10, fpr.data() +  8, 4 );
    memcpy( result.data() + 15, fpr.data() + 12, 4 );
    memcpy( result.data() + 20, fpr.data() + 16, 4 );
    memcpy( result.data() + 26, fpr.data() + 20, 4 );
    memcpy( result.data() + 31, fpr.data() + 24, 4 );
    memcpy( result.data() + 36, fpr.data() + 28, 4 );
    memcpy( result.data() + 41, fpr.data() + 32, 4 );
    memcpy( result.data() + 46, fpr.data() + 36, 4 );
  }
  else if( 32 == fpr.length() ) {
    // convert to this format:
    // 00 11 22 33 44 55 66 77  88 99 AA BB CC DD EE FF
    result.fill( ' ', 48 );
    memcpy( result.data()     , fpr.data()     , 2 );
    memcpy( result.data() +  3, fpr.data() +  2, 2 );
    memcpy( result.data() +  6, fpr.data() +  4, 2 );
    memcpy( result.data() +  9, fpr.data() +  6, 2 );
    memcpy( result.data() + 12, fpr.data() +  8, 2 );
    memcpy( result.data() + 15, fpr.data() + 10, 2 );
    memcpy( result.data() + 18, fpr.data() + 12, 2 );
    memcpy( result.data() + 21, fpr.data() + 14, 2 );
    memcpy( result.data() + 25, fpr.data() + 16, 2 );
    memcpy( result.data() + 28, fpr.data() + 18, 2 );
    memcpy( result.data() + 31, fpr.data() + 20, 2 );
    memcpy( result.data() + 34, fpr.data() + 22, 2 );
    memcpy( result.data() + 37, fpr.data() + 24, 2 );
    memcpy( result.data() + 40, fpr.data() + 26, 2 );
    memcpy( result.data() + 43, fpr.data() + 28, 2 );
    memcpy( result.data() + 46, fpr.data() + 30, 2 );
  }
  else { // unknown length of fingerprint
    result = fpr;
  }

  return result;
}
#endif

#ifdef TEMPORARILY_REMOVED
int KeySelectionDialog::keyValidity( const Kpgp::Key *key ) const
{
  if( 0 == key ) {
    return -1;
  }

  if( ( mAllowedKeys & EncrSignKeys ) == EncryptionKeys ) {
    // only encryption keys are allowed
    if( ( mAllowedKeys & ValidKeys ) && !key->isValidEncryptionKey() ) {
      // only valid encryption keys are allowed
      return -1;
    }
    else if( !key->canEncrypt() ) {
      return -1;
    }
  }
  else if( ( mAllowedKeys & EncrSignKeys ) == SigningKeys ) {
    // only signing keys are allowed
    if( ( mAllowedKeys & ValidKeys ) && !key->isValidSigningKey() ) {
      // only valid signing keys are allowed
      return -1;
    }
    else if( !key->canSign() ) {
      return -1;
    }
  }
  else if( ( mAllowedKeys & ValidKeys ) && !key->isValid() ) {
    // only valid keys are allowed
    return -1;
  }

  // check the key's trust
  int val = 0;
  Validity keyTrust = key->keyTrust();
  switch( keyTrust ) {
  case KPGP_VALIDITY_NEVER:
    val = -1;
    break;
  case KPGP_VALIDITY_MARGINAL:
  case KPGP_VALIDITY_FULL:
  case KPGP_VALIDITY_ULTIMATE:
    val = 2;
    break;
  case KPGP_VALIDITY_UNDEFINED:
    if( mAllowedKeys & TrustedKeys ) {
      // only trusted keys are allowed
      val = -1;
    }
    else {
      val = 1;
    }
    break;
  case KPGP_VALIDITY_UNKNOWN:
  default:
    val = 0;
  }

  return val;
}
#endif

#ifdef TEMPORARILY_REMOVED
void KeySelectionDialog::updateKeyInfo( const Kpgp::Key* key,
                                        QListViewItem* lvi ) const
{
  if( 0 == lvi ) {
    return;
  }

  if( lvi->parent() != 0 ) {
    lvi = lvi->parent();
  }

  if( 0 == key ) {
    // the key doesn't exist anymore -> delete it from the list view
    while( lvi->firstChild() ) {
      kdDebug(5150) << "Deleting '" << lvi->firstChild()->text( 1 ) << "'\n";
      delete lvi->firstChild();
    }
    kdDebug(5150) << "Deleting key 0x" << lvi->text( 0 ) << " ("
                  << lvi->text( 1 ) << ")\n";
    delete lvi;
    lvi = 0;
    return;
  }

  // update the icon for this key
  switch( keyValidity( key ) ) {
  case 0: // the key's validity can't be determined
    lvi->setPixmap( 0, *mKeyUnknownPix );
    break;
  case 1: // key is valid but not trusted
    lvi->setPixmap( 0, *mKeyValidPix );
    break;
  case 2: // key is valid and trusted
    lvi->setPixmap( 0, *mKeyGoodPix );
    break;
  case -1: // key is invalid
    lvi->setPixmap( 0, *mKeyBadPix );
    break;
  }

  // update the key info for this key
  // the key info is identified by a leading space; this shouldn't be
  // a problem because User Ids shouldn't start with a space
  for( lvi = lvi->firstChild(); lvi; lvi = lvi->nextSibling() ) {
    if( lvi->text( 1 ).at(0) == ' ' ) {
      lvi->setText( 1, keyInfo( key ) );
      break;
    }
  }
}
#endif

#ifdef TEMPORARILY_REMOVED
int
KeySelectionDialog::keyAdmissibility( QListViewItem* lvi,
                                      TrustCheckMode trustCheckMode ) const
{
  // Return:
  //  -1 = key must not be chosen,
  //   0 = not enough information to decide whether the give key is allowed
  //       or not,
  //   1 = key can be chosen

  if( mAllowedKeys == AllKeys ) {
    return 1;
  }

  Kpgp::Module *pgp = Kpgp::Module::getKpgp();

  if( 0 == pgp ) {
    return 0;
  }

  KeyID keyId = getKeyId( lvi );
  Kpgp::Key* key = pgp->publicKey( keyId );

  if( 0 == key ) {
    return 0;
  }

  int val = 0;
  if( trustCheckMode == ForceTrustCheck ) {
    key = pgp->rereadKey( keyId, true );
    updateKeyInfo( key, lvi );
    val = keyValidity( key );
  }
  else {
    val = keyValidity( key );
    if( ( trustCheckMode == AllowExpensiveTrustCheck ) && ( 0 == val ) ) {
      key = pgp->rereadKey( keyId, true );
      updateKeyInfo( key, lvi );
      val = keyValidity( key );
    }
  }

  switch( val ) {
  case -1: // key is not usable
    return -1;
    break;
  case 0: // key status unknown
    return 0;
    break;
  case 1: // key is valid, but untrusted
    if( mAllowedKeys & TrustedKeys ) {
      // only trusted keys are allowed
      return -1;
    }
    return 1;
    break;
  case 2: // key is trusted
    return 1;
    break;
  default:
    kdDebug( 5150 ) << "Error: Invalid key status value.\n";
  }

  return 0;
}
#endif

void Kleo::KeySelectionDialog::slotRereadKeys() {
  mKeyListView->clear();
  mListJobCount = 0;
  mTruncated = 0;
  mSavedOffsetY = mKeyListView->contentsY();

  disconnectSignals();
  this->setEnabled( false );

  // FIXME: save current selection
  if ( mBackend )
    startKeyListJobForBackend( mBackend );
  else
    for ( CryptPlugWrapperListIterator it( CryptPlugFactory::instance()->list() ) ; it.current() ; ++it )
      startKeyListJobForBackend( it.current() );

  if ( mListJobCount == 0 ) {
    this->setEnabled( true );
    KMessageBox::information( this,
			      i18n("No backends found for listing keys. Check your installation."),
			      i18n("Key Listing Failed") );
    connectSignals();
  }
}

static void showKeyListError( QWidget * parent, const GpgME::Error & err ) {
  assert( err );
  const QString msg = i18n( "<qt><p>An error occurred while fetching "
			    "the keys from the backend:</p>"
			    "<p><b>%1</b></p></qt>" )
    .arg( QString::fromLocal8Bit( err.asString() ) );

  KMessageBox::error( parent, msg, i18n( "Key Listing Failed" ) );
}

void Kleo::KeySelectionDialog::startKeyListJobForBackend( const CryptoBackend * backend ) {
  assert( backend );
  KeyListJob * job = backend->keyListJob( false, true ); // local, with sigs
  if ( !job )
    return;

  connect( job, SIGNAL(result(const GpgME::KeyListResult&)),
	   SLOT(slotKeyListResult(const GpgME::KeyListResult&)) );
  connect( job, SIGNAL(nextKey(const GpgME::Key&)),
	   mKeyListView, SLOT(slotAddKey(const GpgME::Key&)) );

  const GpgME::Error err = job->start( QStringList(), mKeyUsage & SecretKeys && !( mKeyUsage & PublicKeys ) );

  if ( err )
    return showKeyListError( this, err );

  // FIXME: create a MultiProgressDialog:
  (void)new ProgressDialog( job, i18n( "Fetching keys for %1" ).arg( backend->protocol() ), this );
  ++mListJobCount;
}

static void selectKeys( Kleo::KeyListView * klv, const std::vector<GpgME::Key> & selectedKeys ) {
  if ( selectedKeys.empty() )
    return;
  int selectedKeysCount = selectedKeys.size();
  for ( Kleo::KeyListViewItem * item = klv->firstChild() ; item ; item = item->nextSibling() ) {
    const char * fpr = item->key().subkey(0).fingerprint();
    if ( !fpr || !*fpr )
      continue;
    for ( std::vector<GpgME::Key>::const_iterator it = selectedKeys.begin() ; it != selectedKeys.end() ; ++it )
      if ( qstrcmp( fpr, it->subkey(0).fingerprint() ) == 0 ) {
	item->setSelected( true );
	if ( --selectedKeysCount <= 0 )
	  return;
	else
	  break;
      }
  }
}

void Kleo::KeySelectionDialog::slotKeyListResult( const GpgME::KeyListResult & res ) {
  if ( res.error() )
    showKeyListError( this, res.error() );
  else if ( res.isTruncated() )
    ++mTruncated;

  if ( --mListJobCount > 0 )
    return; // not yet finished...

  if ( mTruncated > 0 )
    KMessageBox::information( this,
			      i18n("%n backends returned truncated output.\n"
				   "Not all available keys are shown"),
			      i18n("Key List Result") );
  this->setEnabled( true );
  mListJobCount = mTruncated = 0;

  selectKeys( mKeyListView, mSelectedKeys );

  slotFilter();

  connectSignals();

  slotSelectionChanged();

  // restore the saved position of the contents
  mKeyListView->setContentsPos( 0, mSavedOffsetY ); mSavedOffsetY = 0;
}

void Kleo::KeySelectionDialog::slotSelectionChanged() {
  kdDebug(5150) << "KeySelectionDialog::slotSelectionChanged()" << endl;

  // (re)start the check selection timer. Checking the selection is delayed
  // because else drag-selection doesn't work very good (checking key trust
  // is slow).
  mCheckSelectionTimer->start( sCheckSelectionDelay );
}

void Kleo::KeySelectionDialog::slotCheckSelection( KeyListViewItem * item ) {
  kdDebug(5150) << "KeySelectionDialog::slotCheckSelection()\n";

  mSelectedKeys.clear();

  if ( !mKeyListView->isMultiSelection() ) {
    if ( item )
      mSelectedKeys.push_back( item->key() );
    enableButtonOK( item && checkKeyUsage( item->key(), mKeyUsage ) );
    return;
  }

  mCheckSelectionTimer->stop();

  for ( KeyListViewItem * it = mKeyListView->firstChild() ; it ; it = it->nextSibling() )
    if ( it->isSelected() )
      mSelectedKeys.push_back( it->key() );

  enableButtonOK( !mSelectedKeys.empty() &&
		  checkKeyUsage( mSelectedKeys, mKeyUsage ) );
}

#ifdef TEMPORARILY_REMOVED
  // As we might change the selection, we have to disconnect the slot
  // to prevent recursion
  disconnectSignals();

  KeyIDList newKeyIdList;
  std::vector<QListViewItem*> keysToBeChecked;

    bool keysAllowed = true;
    enum { UNKNOWN, SELECTED, DESELECTED } userAction = UNKNOWN;
    // Iterate over the tree to find selected keys.
    for( QListViewItem *lvi = mListView->firstChild();
         0 != lvi;
         lvi = lvi->nextSibling() ) {
      // We make sure that either all items belonging to a key are selected
      // or unselected. As it's possible to select/deselect multiple keys at
      // once in extended selection mode we have to figure out whether the user
      // selected or deselected keys.

      // First count the selected items of this key
      int itemCount = 1 + lvi->childCount();
      int selectedCount = lvi->isSelected() ? 1 : 0;
      for( QListViewItem *clvi = lvi->firstChild();
           0 != clvi;
           clvi = clvi->nextSibling() ) {
        if( clvi->isSelected() ) {
          ++selectedCount;
        }
      }

      if( userAction == UNKNOWN ) {
        // Figure out whether the user selected or deselected this key
        // Remark: A selected count of 0 doesn't mean anything since in
        //         extended selection mode a normal left click deselects
        //         the not clicked items.
        if( 0 < selectedCount ) {
          if( -1 == mKeyIds.findIndex( lvi->text(0).local8Bit() ) ) {
            // some items of this key are selected and the key wasn't selected
            // before => the user selected something
            kdDebug(5150) << "selectedCount: "<<selectedCount<<"/"<<itemCount
                          <<" --- User selected key "<<lvi->text(0)<<endl;
            userAction = SELECTED;
          }
          else if( ( itemCount > selectedCount ) &&
                   ( -1 != mKeyIds.findIndex( lvi->text(0).local8Bit() ) ) ) {
            // some items of this key are unselected and the key was selected
            // before => the user deselected something
            kdDebug(5150) << "selectedCount: "<<selectedCount<<"/"<<itemCount
                          <<" --- User deselected key "<<lvi->text(0)<<endl;
            userAction = DESELECTED;
          }
        }
      }
      if( itemCount == selectedCount ) {
        // add key to the list of selected keys
        KeyID keyId = lvi->text(0).local8Bit();
        newKeyIdList.append( keyId );
        int admissibility = keyAdmissibility( lvi, NoExpensiveTrustCheck );
        if( -1 == admissibility ) {
          keysAllowed = false;
        }
        else if ( 0 == admissibility ) {
          keysToBeChecked.append( lvi );
        }
      }
      else if( 0 < selectedCount ) {
        // not all items of this key are selected or unselected. change this
        // according to the user's action
        if( userAction == SELECTED ) {
          // select all items of this key
          mListView->setSelected( lvi, true );
          for( QListViewItem *clvi = lvi->firstChild();
               0 != clvi;
               clvi = clvi->nextSibling() ) {
            mListView->setSelected( clvi, true );
          }
          // add key to the list of selected keys
          KeyID keyId = lvi->text(0).local8Bit();
          newKeyIdList.append( keyId );
          int admissibility = keyAdmissibility( lvi, NoExpensiveTrustCheck );
          if( -1 == admissibility ) {
            keysAllowed = false;
          }
          else if ( 0 == admissibility ) {
            keysToBeChecked.append( lvi );
          }
        }
        else { // userAction == DESELECTED
          // deselect all items of this key
          mListView->setSelected( lvi, false );
          for( QListViewItem *clvi = lvi->firstChild();
               0 != clvi;
               clvi = clvi->nextSibling() ) {
            mListView->setSelected( clvi, false );
          }
        }
      }
    }
    kdDebug(5150) << "Selected keys: " << newKeyIdList.toStringList().join(", ") << endl;
    mKeyIds = newKeyIdList;
    if( !keysToBeChecked.isEmpty() ) {
      keysAllowed &= checkKeys( keysToBeChecked );
    }
    enableButtonOK( keysAllowed );

    connect( mListView, SIGNAL( selectionChanged() ),
             this,      SLOT( slotSelectionChanged() ) );
  }
#endif


bool Kleo::KeySelectionDialog::rememberSelection() const {
  return mRememberCB && mRememberCB->isChecked() ;
}

void Kleo::KeySelectionDialog::slotRMB( Kleo::KeyListViewItem * item, const QPoint & p, int ) {
  if ( !item ) return;

  mCurrentContextMenuItem = item;

  QPopupMenu menu;
  menu.insertItem( i18n( "Recheck Key" ), this, SLOT(slotRecheckKey()) );
  menu.exec( p );
}

void Kleo::KeySelectionDialog::slotRecheckKey() {
  if ( !mCurrentContextMenuItem )
    return;

#ifdef TEMPORARILY_REMOVED
  // force rereading the key
  keyAdmissibility( mCurrentContextMenuItem, ForceTrustCheck );
  // recheck the selection
  slotCheckSelection( mCurrentContextMenuItem );
#endif
}

void Kleo::KeySelectionDialog::slotOk() {
  if ( mCheckSelectionTimer->isActive() )
    slotCheckSelection();
  mStartSearchTimer->stop();
  accept();
}


void Kleo::KeySelectionDialog::slotCancel() {
  mCheckSelectionTimer->stop();
  mStartSearchTimer->stop();
  reject();
}

void Kleo::KeySelectionDialog::slotSearch( const QString & text ) {
  mSearchText = text.stripWhiteSpace().upper();
  mStartSearchTimer->start( sCheckSelectionDelay, true /*single-shot*/ );
}

void Kleo::KeySelectionDialog::slotFilter() {
  if ( mSearchText.isEmpty() ) {
    showAllItems();
    return;
  }

  // OK, so we need to filter:
  QRegExp keyIdRegExp( "(?:0x)?[A-F0-9]{1,8}", false /*case-insens.*/ );
  if ( keyIdRegExp.exactMatch( mSearchText ) ) {
    if ( mSearchText.startsWith( "0X" ) )
      // search for keyID only:
      filterByKeyID( mSearchText.mid( 2 ) );
    else
      // search for UID and keyID:
      filterByKeyIDOrUID( mSearchText );
  } else {
    // search in UID:
    filterByUID( mSearchText );
  }
}

void Kleo::KeySelectionDialog::filterByKeyID( const QString & keyID ) {
  assert( keyID.length() <= 8 );
  assert( !keyID.isEmpty() ); // regexp in slotFilter should prevent these
  if ( keyID.isEmpty() )
    showAllItems();
  else
    for ( KeyListViewItem * item = mKeyListView->firstChild() ; item ; item = item->nextSibling() )
      item->setVisible( item->text( 0 ).upper().startsWith( keyID ) );
}

static bool anyUIDMatches( const Kleo::KeyListViewItem * item, QRegExp & rx ) {
  if ( !item )
    return false;

  const std::vector<GpgME::UserID> uids = item->key().userIDs();
  for ( std::vector<GpgME::UserID>::const_iterator it = uids.begin() ; it != uids.end() ; ++it )
    if ( it->id() && rx.search( QString::fromUtf8( it->id() ) ) >= 0 )
      return true;
  return false;
}

void Kleo::KeySelectionDialog::filterByKeyIDOrUID( const QString & str ) {
  assert( !str.isEmpty() );

  // match beginnings of words:
  QRegExp rx( "\\b" + QRegExp::escape( str ), false );

  for ( KeyListViewItem * item = mKeyListView->firstChild() ; item ; item = item->nextSibling() )
    item->setVisible( item->text( 0 ).upper().startsWith( str )
		      || anyUIDMatches( item, rx ) );

}

void Kleo::KeySelectionDialog::filterByUID( const QString & str ) {
  assert( !str.isEmpty() );

  // match beginnings of words:
  QRegExp rx( "\\b" + QRegExp::escape( str ), false );

  for ( KeyListViewItem * item = mKeyListView->firstChild() ; item ; item = item->nextSibling() )
    item->setVisible( anyUIDMatches( item, rx ) );
}


void Kleo::KeySelectionDialog::showAllItems() {
  for ( KeyListViewItem * item = mKeyListView->firstChild() ; item ; item = item->nextSibling() )
    item->setVisible( true );
}

#include "keyselectiondialog.moc"
