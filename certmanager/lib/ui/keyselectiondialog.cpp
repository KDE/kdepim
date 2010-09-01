/*  -*- c++ -*-
    keyselectiondialog.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarävdalens Datakonsult AB

    Based on kpgpui.cpp
    Copyright (C) 2001,2002 the KPGP authors
    See file libkdenetwork/AUTHORS.kpgp for details

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
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "keyselectiondialog.h"

#include "keylistview.h"
#include "progressdialog.h"

#include <kleo/dn.h>
#include <kleo/keylistjob.h>
#include <kleo/cryptobackendfactory.h>

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
#include <kprocess.h>
#include <kactivelabel.h>
#include <kurl.h>

// Qt
#include <tqcheckbox.h>
#include <tqtoolbutton.h>
#include <tqlabel.h>
#include <tqpixmap.h>
#include <tqtimer.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqwhatsthis.h>
#include <tqpopupmenu.h>
#include <tqregexp.h>
#include <tqpushbutton.h>

#include <algorithm>
#include <iterator>

#include <string.h>
#include <assert.h>

static bool checkKeyUsage( const GpgME::Key & key, unsigned int keyUsage ) {

  if ( keyUsage & Kleo::KeySelectionDialog::ValidKeys ) {
    if ( key.isInvalid() )
        if ( key.keyListMode() & GpgME::Context::Validate ) {
            kdDebug() << "key is invalid" << endl;
            return false;
        } else {
            kdDebug() << "key is invalid - ignoring" << endl;
        }
    if ( key.isExpired() ) {
      kdDebug() << "key is expired" << endl;
      return false;
    } else if ( key.isRevoked() ) {
      kdDebug() << "key is revoked" << endl;
      return false;
    } else if ( key.isDisabled() ) {
      kdDebug() << "key is disabled" << endl;
      return false;
    }
  }

  if ( keyUsage & Kleo::KeySelectionDialog::EncryptionKeys &&
       !key.canEncrypt() ) {
    kdDebug() << "key can't encrypt" << endl;
    return false;
  }
  if ( keyUsage & Kleo::KeySelectionDialog::SigningKeys &&
       !key.canSign() ) {
    kdDebug() << "key can't sign" << endl;
    return false;
  }
  if ( keyUsage & Kleo::KeySelectionDialog::CertificationKeys &&
       !key.canCertify() ) {
    kdDebug() << "key can't certify" << endl;
    return false;
  }
  if ( keyUsage & Kleo::KeySelectionDialog::AuthenticationKeys &&
       !key.canAuthenticate() ) {
    kdDebug() << "key can't authenticate" << endl;
    return false;
  }

  if ( keyUsage & Kleo::KeySelectionDialog::SecretKeys &&
       !( keyUsage & Kleo::KeySelectionDialog::PublicKeys ) &&
       !key.isSecret() ) {
    kdDebug() << "key isn't secret" << endl;
    return false;
  }

  if ( keyUsage & Kleo::KeySelectionDialog::TrustedKeys &&
       key.protocol() == GpgME::Context::OpenPGP &&
       // only check this for secret keys for now.
       // Seems validity isn't checked for secret keylistings...
       !key.isSecret() ) {
    std::vector<GpgME::UserID> uids = key.userIDs();
    for ( std::vector<GpgME::UserID>::const_iterator it = uids.begin() ; it != uids.end() ; ++it )
      if ( !it->isRevoked() && it->validity() >= GpgME::UserID::Marginal )
	return true;
    kdDebug() << "key has no UIDs with validity >= Marginal" << endl;
    return false;
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

static inline TQString time_t2string( time_t t ) {
  TQDateTime dt;
  dt.setTime_t( t );
  return dt.toString();
}

namespace {

  class ColumnStrategy : public Kleo::KeyListView::ColumnStrategy {
  public:
    ColumnStrategy( unsigned int keyUsage );

    TQString title( int col ) const;
    int width( int col, const TQFontMetrics & fm ) const;

    TQString text( const GpgME::Key & key, int col ) const;
    TQString toolTip( const GpgME::Key & key, int col ) const;
    const TQPixmap * pixmap( const GpgME::Key & key, int col ) const;

  private:
    const TQPixmap mKeyGoodPix, mKeyBadPix, mKeyUnknownPix, mKeyValidPix;
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

  TQString ColumnStrategy::title( int col ) const {
    switch ( col ) {
    case 0: return i18n("Key ID");
    case 1: return i18n("User ID");
    default: return TQString::null;
    }
  }

  int ColumnStrategy::width( int col, const TQFontMetrics & fm ) const {
    if ( col == 0 ) {
      static const char hexchars[] = "0123456789ABCDEF";
      int maxWidth = 0;
      for ( unsigned int i = 0 ; i < 16 ; ++i )
	maxWidth = kMax( fm.width( TQChar( hexchars[i] ) ), maxWidth );
      return 8 * maxWidth + 2 * mKeyGoodPix.width();
    }
    return Kleo::KeyListView::ColumnStrategy::width( col, fm );
  }

  TQString ColumnStrategy::text( const GpgME::Key & key, int col ) const {
    switch ( col ) {
    case 0:
      {
	if ( key.shortKeyID() )
	  return TQString::fromUtf8( key.shortKeyID() );
	else
	  return i18n("<unknown>");
      }
      break;
    case 1:
      {
	const char * uid = key.userID(0).id();
	if ( key.protocol() == GpgME::Context::OpenPGP )
	  return uid && *uid ? TQString::fromUtf8( uid ) : TQString::null ;
	else // CMS
	  return Kleo::DN( uid ).prettyDN();
      }
      break;
    default: return TQString::null;
    }
  }

  TQString ColumnStrategy::toolTip( const GpgME::Key & key, int ) const {
    const char * uid = key.userID(0).id();
    const char * fpr = key.primaryFingerprint();
    const char * issuer = key.issuerName();
    const GpgME::Subkey subkey = key.subkey(0);
    const TQString expiry = subkey.neverExpires() ? i18n("never") : time_t2string( subkey.expirationTime() ) ;
    const TQString creation = time_t2string( subkey.creationTime() );
    if ( key.protocol() == GpgME::Context::OpenPGP )
      return i18n( "OpenPGP key for %1\n"
		   "Created: %2\n"
		   "Expiry: %3\n"
		   "Fingerprint: %4" )
	.arg( uid ? TQString::fromUtf8( uid ) : i18n("unknown"),
	      creation, expiry,
	      fpr ? TQString::fromLatin1( fpr ) : i18n("unknown") );
    else
      return i18n( "S/MIME key for %1\n"
		   "Created: %2\n"
		   "Expiry: %3\n"
		   "Fingerprint: %4\n"
		   "Issuer: %5" )
	.arg( uid ? Kleo::DN( uid ).prettyDN() : i18n("unknown"),
	      creation, expiry,
	      fpr ? TQString::fromLatin1( fpr ) : i18n("unknown") )
	.arg( issuer ? Kleo::DN( issuer ).prettyDN() : i18n("unknown") );
  }

  const TQPixmap * ColumnStrategy::pixmap( const GpgME::Key & key, int col ) const {
    if ( col != 0 )
      return 0;
    // this key did not undergo a validating keylisting yet:
    if ( !( key.keyListMode() & GpgME::Context::Validate ) )
      return &mKeyUnknownPix;

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

Kleo::KeySelectionDialog::KeySelectionDialog( const TQString & title,
					      const TQString & text,
					      const std::vector<GpgME::Key> & selectedKeys,
					      unsigned int keyUsage,
					      bool extendedSelection,
					      bool rememberChoice,
					      TQWidget * parent, const char * name,
					      bool modal )
  : KDialogBase( parent, name, modal, title, Default|Ok|Cancel|Help, Ok ),
    mOpenPGPBackend( 0 ),
    mSMIMEBackend( 0 ),
    mRememberCB( 0 ),
    mSelectedKeys( selectedKeys ),
    mKeyUsage( keyUsage ),
    mCurrentContextMenuItem( 0 )
{
  init( rememberChoice, extendedSelection, text, TQString::null );
}

Kleo::KeySelectionDialog::KeySelectionDialog( const TQString & title,
					      const TQString & text,
                                              const TQString & initialQuery,
					      const std::vector<GpgME::Key> & selectedKeys,
					      unsigned int keyUsage,
					      bool extendedSelection,
					      bool rememberChoice,
					      TQWidget * parent, const char * name,
					      bool modal )
  : KDialogBase( parent, name, modal, title, Default|Ok|Cancel|Help, Ok ),
    mOpenPGPBackend( 0 ),
    mSMIMEBackend( 0 ),
    mRememberCB( 0 ),
    mSelectedKeys( selectedKeys ),
    mKeyUsage( keyUsage ),
    mSearchText( initialQuery ),
    mInitialQuery( initialQuery ),
    mCurrentContextMenuItem( 0 )
{
  init( rememberChoice, extendedSelection, text, initialQuery );
}

Kleo::KeySelectionDialog::KeySelectionDialog( const TQString & title,
					      const TQString & text,
					      const TQString & initialQuery,
					      unsigned int keyUsage,
					      bool extendedSelection,
					      bool rememberChoice,
					      TQWidget * parent, const char * name,
					      bool modal )
  : KDialogBase( parent, name, modal, title, Default|Ok|Cancel|Help, Ok ),
    mOpenPGPBackend( 0 ),
    mSMIMEBackend( 0 ),
    mRememberCB( 0 ),
    mKeyUsage( keyUsage ),
    mSearchText( initialQuery ),
    mInitialQuery( initialQuery ),
    mCurrentContextMenuItem( 0 )
{
  init( rememberChoice, extendedSelection, text, initialQuery );
}

void Kleo::KeySelectionDialog::init( bool rememberChoice, bool extendedSelection,
				     const TQString & text, const TQString & initialQuery ) {
  if ( mKeyUsage & OpenPGPKeys )
    mOpenPGPBackend = Kleo::CryptoBackendFactory::instance()->openpgp();
  if ( mKeyUsage & SMIMEKeys )
    mSMIMEBackend = Kleo::CryptoBackendFactory::instance()->smime();

  mCheckSelectionTimer = new TQTimer( this );
  mStartSearchTimer = new TQTimer( this );

  TQFrame *page = makeMainWidget();
  mTopLayout = new TQVBoxLayout( page, 0, spacingHint() );

  if ( !text.isEmpty() ) {
    if ( text.startsWith( "<qt>" ) ) {
      KActiveLabel *textLabel = new KActiveLabel( text, page );
      disconnect( textLabel, TQT_SIGNAL(linkClicked(const TQString&)), textLabel, TQT_SLOT(openLink(const TQString&)) );
      connect( textLabel, TQT_SIGNAL(linkClicked(const TQString&)), TQT_SLOT(slotStartCertificateManager(const TQString&)) );
      textLabel->setAlignment( textLabel->alignment() | TQt::WordBreak );
      mTopLayout->addWidget( textLabel );
    } else {
      KActiveLabel *textLabel = new KActiveLabel( text, page );
      textLabel->setAlignment( textLabel->alignment() | TQt::WordBreak );
      mTopLayout->addWidget( textLabel );
    }
  }

  TQPushButton * const searchExternalPB
      = new TQPushButton( i18n("Search for &External Certificates"), page );
  mTopLayout->addWidget( searchExternalPB, 0, TQt::AlignLeft );
  connect( searchExternalPB, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotStartSearchForExternalCertificates()) );
  if ( initialQuery.isEmpty() )
      searchExternalPB->hide();

  TQHBoxLayout * hlay = new TQHBoxLayout( mTopLayout ); // inherits spacing
  TQLineEdit * le = new TQLineEdit( page );
  le->setText( initialQuery );
  TQToolButton *clearButton = new TQToolButton( page );
  clearButton->setIconSet( KGlobal::iconLoader()->loadIconSet(
              KApplication::reverseLayout() ? "clear_left":"locationbar_erase", KIcon::Small, 0 ) );
  hlay->addWidget( clearButton );
  hlay->addWidget( new TQLabel( le, i18n("&Search for:"), page ) );
  hlay->addWidget( le, 1 );
  le->setFocus();

  connect( clearButton, TQT_SIGNAL( clicked() ), le, TQT_SLOT( clear() ) );
  connect( le, TQT_SIGNAL(textChanged(const TQString&)),
	   this, TQT_SLOT(slotSearch(const TQString&)) );
  connect( mStartSearchTimer, TQT_SIGNAL(timeout()), TQT_SLOT(slotFilter()) );

  mKeyListView = new KeyListView( new ColumnStrategy( mKeyUsage ), 0, page, "mKeyListView" );
  mKeyListView->setResizeMode( TQListView::LastColumn );
  mKeyListView->setRootIsDecorated( true );
  mKeyListView->setShowSortIndicator( true );
  mKeyListView->setSorting( 1, true ); // sort by User ID
  mKeyListView->setShowToolTips( true );
  if ( extendedSelection )
    mKeyListView->setSelectionMode( TQListView::Extended );
  mTopLayout->addWidget( mKeyListView, 10 );

  if ( rememberChoice ) {
    mRememberCB = new TQCheckBox( i18n("&Remember choice"), page );
    mTopLayout->addWidget( mRememberCB );
    TQWhatsThis::add( mRememberCB,
		     i18n("<qt><p>If you check this box your choice will "
			  "be stored and you will not be asked again."
			  "</p></qt>") );
  }

  connect( mCheckSelectionTimer, TQT_SIGNAL(timeout()),
	   TQT_SLOT(slotCheckSelection()) );
  connectSignals();

  connect( mKeyListView,
	   TQT_SIGNAL(doubleClicked(Kleo::KeyListViewItem*,const TQPoint&,int)),
	   TQT_SLOT(slotTryOk()) );
  connect( mKeyListView,
	   TQT_SIGNAL(contextMenu(Kleo::KeyListViewItem*,const TQPoint&)),
           TQT_SLOT(slotRMB(Kleo::KeyListViewItem*,const TQPoint&)) );

  setButtonText( KDialogBase::Default, i18n("&Reread Keys") );
  setButtonGuiItem( KDialogBase::Help, i18n("&Start Certificate Manager") );
  connect( this, TQT_SIGNAL(defaultClicked()), this, TQT_SLOT(slotRereadKeys()) );
  connect( this, TQT_SIGNAL(helpClicked()), this, TQT_SLOT(slotStartCertificateManager()) );

  slotRereadKeys();
  mTopLayout->activate();

  if ( kapp ) {
    KWin::setIcons( winId(), kapp->icon(), kapp->miniIcon() );
    TQSize dialogSize( 500, 400 );

    KConfigGroup dialogConfig( KGlobal::config(), "Key Selection Dialog" );
    dialogSize = dialogConfig.readSizeEntry( "Dialog size", &dialogSize );
    resize( dialogSize );
  }
}

Kleo::KeySelectionDialog::~KeySelectionDialog() {
  KConfigGroup dialogConfig( KGlobal::config(), "Key Selection Dialog" );
  dialogConfig.writeEntry( "Dialog size", size() );
  dialogConfig.sync();
}


void Kleo::KeySelectionDialog::connectSignals() {
  if ( mKeyListView->isMultiSelection() )
    connect( mKeyListView, TQT_SIGNAL(selectionChanged()),
             TQT_SLOT(slotSelectionChanged()) );
  else
    connect( mKeyListView, TQT_SIGNAL(selectionChanged(Kleo::KeyListViewItem*)),
             TQT_SLOT(slotCheckSelection(Kleo::KeyListViewItem*)) );
}

void Kleo::KeySelectionDialog::disconnectSignals() {
  if ( mKeyListView->isMultiSelection() )
    disconnect( mKeyListView, TQT_SIGNAL(selectionChanged()),
		this, TQT_SLOT(slotSelectionChanged()) );
  else
    disconnect( mKeyListView, TQT_SIGNAL(selectionChanged(Kleo::KeyListViewItem*)),
		this, TQT_SLOT(slotCheckSelection(Kleo::KeyListViewItem*)) );
}

const GpgME::Key & Kleo::KeySelectionDialog::selectedKey() const {
  if ( mKeyListView->isMultiSelection() || !mKeyListView->selectedItem() )
    return GpgME::Key::null;
  return mKeyListView->selectedItem()->key();
}

TQString Kleo::KeySelectionDialog::fingerprint() const {
  return selectedKey().primaryFingerprint();
}

TQStringList Kleo::KeySelectionDialog::fingerprints() const {
  TQStringList result;
  for ( std::vector<GpgME::Key>::const_iterator it = mSelectedKeys.begin() ; it != mSelectedKeys.end() ; ++it )
    if ( const char * fpr = it->primaryFingerprint() )
      result.push_back( fpr );
  return result;
}

TQStringList Kleo::KeySelectionDialog::pgpKeyFingerprints() const {
  TQStringList result;
  for ( std::vector<GpgME::Key>::const_iterator it = mSelectedKeys.begin() ; it != mSelectedKeys.end() ; ++it )
    if ( it->protocol() == GpgME::Context::OpenPGP )
      if ( const char * fpr = it->primaryFingerprint() )
        result.push_back( fpr );
  return result;
}

TQStringList Kleo::KeySelectionDialog::smimeFingerprints() const {
  TQStringList result;
  for ( std::vector<GpgME::Key>::const_iterator it = mSelectedKeys.begin() ; it != mSelectedKeys.end() ; ++it )
    if ( it->protocol() == GpgME::Context::CMS )
      if ( const char * fpr = it->primaryFingerprint() )
        result.push_back( fpr );
  return result;
}

void Kleo::KeySelectionDialog::slotRereadKeys() {
  mKeyListView->clear();
  mListJobCount = 0;
  mTruncated = 0;
  mSavedOffsetY = mKeyListView->contentsY();

  disconnectSignals();
  mKeyListView->setEnabled( false );

  // FIXME: save current selection
  if ( mOpenPGPBackend )
    startKeyListJobForBackend( mOpenPGPBackend, std::vector<GpgME::Key>(), false /*non-validating*/ );
  if ( mSMIMEBackend )
    startKeyListJobForBackend( mSMIMEBackend, std::vector<GpgME::Key>(), false /*non-validating*/ );

  if ( mListJobCount == 0 ) {
    mKeyListView->setEnabled( true );
    KMessageBox::information( this,
			      i18n("No backends found for listing keys. "
				   "Check your installation."),
			      i18n("Key Listing Failed") );
    connectSignals();
  }
}

void Kleo::KeySelectionDialog::slotHelp()
{
    emit helpClicked();
}

void Kleo::KeySelectionDialog::slotStartCertificateManager( const TQString &query )
{
  KProcess certManagerProc;
  certManagerProc << "kleopatra";
  if ( !query.isEmpty() )
    certManagerProc << "--external" << "--query" << KURL::decode_string( query );

  if( !certManagerProc.start( KProcess::DontCare ) )
    KMessageBox::error( this, i18n( "Could not start certificate manager; "
                                    "please check your installation." ),
                                    i18n( "Certificate Manager Error" ) );
  else
    kdDebug(5006) << "\nslotStartCertManager(): certificate manager started.\n" << endl;
}

#ifndef __KLEO_UI_SHOW_KEY_LIST_ERROR_H__
#define __KLEO_UI_SHOW_KEY_LIST_ERROR_H__
static void showKeyListError( TQWidget * parent, const GpgME::Error & err ) {
  assert( err );
  const TQString msg = i18n( "<qt><p>An error occurred while fetching "
			    "the keys from the backend:</p>"
			    "<p><b>%1</b></p></qt>" )
    .arg( TQString::fromLocal8Bit( err.asString() ) );

  KMessageBox::error( parent, msg, i18n( "Key Listing Failed" ) );
}
#endif // __KLEO_UI_SHOW_KEY_LIST_ERROR_H__

namespace {
  struct ExtractFingerprint {
    TQString operator()( const GpgME::Key & key ) {
      return key.primaryFingerprint();
    }
  };
}

void Kleo::KeySelectionDialog::startKeyListJobForBackend( const CryptoBackend::Protocol * backend, const std::vector<GpgME::Key> & keys, bool validate ) {
  assert( backend );
  KeyListJob * job = backend->keyListJob( false, false, validate ); // local, w/o sigs, validation as givem
  if ( !job )
    return;

  connect( job, TQT_SIGNAL(result(const GpgME::KeyListResult&)),
	   TQT_SLOT(slotKeyListResult(const GpgME::KeyListResult&)) );
  connect( job, TQT_SIGNAL(nextKey(const GpgME::Key&)),
	   mKeyListView, validate ?
	   TQT_SLOT(slotRefreshKey(const GpgME::Key&)) :
	   TQT_SLOT(slotAddKey(const GpgME::Key&)) );

  TQStringList fprs;
  std::transform( keys.begin(), keys.end(), std::back_inserter( fprs ), ExtractFingerprint() );
  const GpgME::Error err = job->start( fprs, mKeyUsage & SecretKeys && !( mKeyUsage & PublicKeys ) );

  if ( err )
    return showKeyListError( this, err );

  // FIXME: create a MultiProgressDialog:
  (void)new ProgressDialog( job, validate ? i18n( "Checking selected keys..." ) : i18n( "Fetching keys..." ), this );
  ++mListJobCount;
}

static void selectKeys( Kleo::KeyListView * klv, const std::vector<GpgME::Key> & selectedKeys ) {
  klv->clearSelection();
  if ( selectedKeys.empty() )
    return;
  for ( std::vector<GpgME::Key>::const_iterator it = selectedKeys.begin() ; it != selectedKeys.end() ; ++it )
    if ( Kleo::KeyListViewItem * item = klv->itemByFingerprint( it->primaryFingerprint() ) )
      item->setSelected( true );
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
			      i18n("<qt>One backend returned truncated output.<br>"
				   "Not all available keys are shown</qt>",
			           "<qt>%n backends returned truncated output.<br>"
				   "Not all available keys are shown</qt>",
				   mTruncated),
			      i18n("Key List Result") );

  mKeyListView->flushKeys();

  mKeyListView->setEnabled( true );
  mListJobCount = mTruncated = 0;
  mKeysToCheck.clear();

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

namespace {
  struct AlreadyChecked {
    bool operator()( const GpgME::Key & key ) const {
      return key.keyListMode() & GpgME::Context::Validate ;
    }
  };
}

void Kleo::KeySelectionDialog::slotCheckSelection( KeyListViewItem * item ) {
  kdDebug(5150) << "KeySelectionDialog::slotCheckSelection()\n";

  mCheckSelectionTimer->stop();

  mSelectedKeys.clear();

  if ( !mKeyListView->isMultiSelection() ) {
    if ( item )
      mSelectedKeys.push_back( item->key() );
  }

  for ( KeyListViewItem * it = mKeyListView->firstChild() ; it ; it = it->nextSibling() )
    if ( it->isSelected() )
      mSelectedKeys.push_back( it->key() );

  mKeysToCheck.clear();
  std::remove_copy_if( mSelectedKeys.begin(), mSelectedKeys.end(),
		       std::back_inserter( mKeysToCheck ),
		       AlreadyChecked() );
  if ( mKeysToCheck.empty() ) {
    enableButtonOK( !mSelectedKeys.empty() &&
		    checkKeyUsage( mSelectedKeys, mKeyUsage ) );
    return;
  }

  // performed all fast checks - now for validating key listing:
  startValidatingKeyListing();
}

void Kleo::KeySelectionDialog::startValidatingKeyListing() {
  if ( mKeysToCheck.empty() )
    return;

  mListJobCount = 0;
  mTruncated = 0;
  mSavedOffsetY = mKeyListView->contentsY();

  disconnectSignals();
  mKeyListView->setEnabled( false );

  std::vector<GpgME::Key> smime, openpgp;
  for ( std::vector<GpgME::Key>::const_iterator it = mKeysToCheck.begin() ; it != mKeysToCheck.end() ; ++it )
    if ( it->protocol() == GpgME::Context::OpenPGP )
      openpgp.push_back( *it );
    else
      smime.push_back( *it );

  if ( !openpgp.empty() ) {
    assert( mOpenPGPBackend );
    startKeyListJobForBackend( mOpenPGPBackend, openpgp, true /*validate*/ );
  }
  if ( !smime.empty() ) {
    assert( mSMIMEBackend );
    startKeyListJobForBackend( mSMIMEBackend, smime, true /*validate*/ );
  }

  assert( mListJobCount > 0 );
}

bool Kleo::KeySelectionDialog::rememberSelection() const {
  return mRememberCB && mRememberCB->isChecked() ;
}

void Kleo::KeySelectionDialog::slotRMB( Kleo::KeyListViewItem * item, const TQPoint & p ) {
  if ( !item ) return;

  mCurrentContextMenuItem = item;

  TQPopupMenu menu;
  menu.insertItem( i18n( "Recheck Key" ), this, TQT_SLOT(slotRecheckKey()) );
  menu.exec( p );
}

void Kleo::KeySelectionDialog::slotRecheckKey() {
  if ( !mCurrentContextMenuItem || mCurrentContextMenuItem->key().isNull() )
    return;

  mKeysToCheck.clear();
  mKeysToCheck.push_back( mCurrentContextMenuItem->key() );
}

void Kleo::KeySelectionDialog::slotTryOk() {
  if ( actionButton( Ok )->isEnabled() )
    slotOk();
}

void Kleo::KeySelectionDialog::slotOk() {
  if ( mCheckSelectionTimer->isActive() )
    slotCheckSelection();
  // button could be disabled again after checking the selected key
  if ( !actionButton( Ok )->isEnabled() )
    return;
  mStartSearchTimer->stop();
  accept();
}


void Kleo::KeySelectionDialog::slotCancel() {
  mCheckSelectionTimer->stop();
  mStartSearchTimer->stop();
  reject();
}

void Kleo::KeySelectionDialog::slotSearch( const TQString & text ) {
  mSearchText = text.stripWhiteSpace().upper();
  slotSearch();
}

void Kleo::KeySelectionDialog::slotSearch() {
  mStartSearchTimer->start( sCheckSelectionDelay, true /*single-shot*/ );
}

void Kleo::KeySelectionDialog::slotFilter() {
  if ( mSearchText.isEmpty() ) {
    showAllItems();
    return;
  }

  // OK, so we need to filter:
  TQRegExp keyIdRegExp( "(?:0x)?[A-F0-9]{1,8}", false /*case-insens.*/ );
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

void Kleo::KeySelectionDialog::filterByKeyID( const TQString & keyID ) {
  assert( keyID.length() <= 8 );
  assert( !keyID.isEmpty() ); // regexp in slotFilter should prevent these
  if ( keyID.isEmpty() )
    showAllItems();
  else
    for ( KeyListViewItem * item = mKeyListView->firstChild() ; item ; item = item->nextSibling() )
      item->setVisible( item->text( 0 ).upper().startsWith( keyID ) );
}

static bool anyUIDMatches( const Kleo::KeyListViewItem * item, TQRegExp & rx ) {
  if ( !item )
    return false;

  const std::vector<GpgME::UserID> uids = item->key().userIDs();
  for ( std::vector<GpgME::UserID>::const_iterator it = uids.begin() ; it != uids.end() ; ++it )
    if ( it->id() && rx.search( TQString::fromUtf8( it->id() ) ) >= 0 )
      return true;
  return false;
}

void Kleo::KeySelectionDialog::filterByKeyIDOrUID( const TQString & str ) {
  assert( !str.isEmpty() );

  // match beginnings of words:
  TQRegExp rx( "\\b" + TQRegExp::escape( str ), false );

  for ( KeyListViewItem * item = mKeyListView->firstChild() ; item ; item = item->nextSibling() )
    item->setVisible( item->text( 0 ).upper().startsWith( str ) || anyUIDMatches( item, rx ) );

}

void Kleo::KeySelectionDialog::filterByUID( const TQString & str ) {
  assert( !str.isEmpty() );

  // match beginnings of words:
  TQRegExp rx( "\\b" + TQRegExp::escape( str ), false );

  for ( KeyListViewItem * item = mKeyListView->firstChild() ; item ; item = item->nextSibling() )
    item->setVisible( anyUIDMatches( item, rx ) );
}


void Kleo::KeySelectionDialog::showAllItems() {
  for ( KeyListViewItem * item = mKeyListView->firstChild() ; item ; item = item->nextSibling() )
    item->setVisible( true );
}

#include "keyselectiondialog.moc"
