/*  -*- c++ -*-
    keyselectiondialog.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klar�vdalens Datakonsult AB

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


#include "keyselectiondialog.h"

#include "keylistview.h"
#include "progressdialog.h"

#include "kleo/dn.h"
#include "kleo/keylistjob.h"
#include "kleo/cryptobackendfactory.h"

// gpgme++
#include <gpgme++/key.h>
#include <gpgme++/keylistresult.h>

// KDE
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kwindowsystem.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <qpushbutton.h>
#include <kconfiggroup.h>
#include <qmenu.h>
#include <klineedit.h>
#include <kurl.h>

// Qt
#include <QCheckBox>
#include <QLabel>
#include <QTimer>
#include <QDateTime>
#include <QProcess>

#include <QRegExp>
#include <QPushButton>
#include <QFrame>
#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <algorithm>
#include <iterator>

#include <string.h>
#include <assert.h>
#include <qscrollbar.h>
#include <KSharedConfig>

static bool checkKeyUsage( const GpgME::Key & key, unsigned int keyUsage ) {

  if ( keyUsage & Kleo::KeySelectionDialog::ValidKeys ) {
    if ( key.isInvalid() ) {
      if ( key.keyListMode() & GpgME::Validate ) {
        kDebug(5150) << "key is invalid";
        return false;
      } else {
        kDebug(5150) << "key is invalid - ignoring";
      }
    }
    if ( key.isExpired() ) {
      kDebug(5150) <<"key is expired";
      return false;
    } else if ( key.isRevoked() ) {
      kDebug(5150) <<"key is revoked";
      return false;
    } else if ( key.isDisabled() ) {
      kDebug(5150) <<"key is disabled";
      return false;
    }
  }

  if ( keyUsage & Kleo::KeySelectionDialog::EncryptionKeys &&
       !key.canEncrypt() ) {
    kDebug(5150) <<"key can't encrypt";
    return false;
  }
  if ( keyUsage & Kleo::KeySelectionDialog::SigningKeys &&
       !key.canSign() ) {
    kDebug(5150) <<"key can't sign";
    return false;
  }
  if ( keyUsage & Kleo::KeySelectionDialog::CertificationKeys &&
       !key.canCertify() ) {
    kDebug(5150) <<"key can't certify";
    return false;
  }
  if ( keyUsage & Kleo::KeySelectionDialog::AuthenticationKeys &&
       !key.canAuthenticate() ) {
    kDebug(5150) <<"key can't authenticate";
    return false;
  }

  if ( keyUsage & Kleo::KeySelectionDialog::SecretKeys &&
       !( keyUsage & Kleo::KeySelectionDialog::PublicKeys ) &&
       !key.hasSecret() ) {
    kDebug(5150) <<"key isn't secret";
    return false;
  }

  if ( keyUsage & Kleo::KeySelectionDialog::TrustedKeys &&
       key.protocol() == GpgME::OpenPGP &&
       // only check this for secret keys for now.
       // Seems validity isn't checked for secret keylistings...
       !key.hasSecret() ) {
    std::vector<GpgME::UserID> uids = key.userIDs();
    for ( std::vector<GpgME::UserID>::const_iterator it = uids.begin() ; it != uids.end() ; ++it )
      if ( !it->isRevoked() && it->validity() >= GpgME::UserID::Marginal )
        return true;
    kDebug(5150) <<"key has no UIDs with validity >= Marginal";
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

static inline QString time_t2string( time_t t ) {
  QDateTime dt;
  dt.setTime_t( t );
  return dt.toString();
}

namespace {

  class ColumnStrategy : public Kleo::KeyListView::ColumnStrategy {
  public:
    ColumnStrategy( unsigned int keyUsage );

    QString title( int col ) const;
    int width( int col, const QFontMetrics & fm ) const;

    QString text( const GpgME::Key & key, int col ) const;
    QString toolTip( const GpgME::Key & key, int col ) const;
    QIcon icon( const GpgME::Key & key, int col ) const;

  private:
    const QIcon mKeyGoodPix, mKeyBadPix, mKeyUnknownPix, mKeyValidPix;
    const unsigned int mKeyUsage;
  };

  ColumnStrategy::ColumnStrategy( unsigned int keyUsage )
    : Kleo::KeyListView::ColumnStrategy(),
      mKeyGoodPix( QLatin1String("key_ok") ),
      mKeyBadPix( QLatin1String("key_bad") ),
      mKeyUnknownPix( QLatin1String("key_unknown") ),
      mKeyValidPix( QLatin1String("key") ),
      mKeyUsage( keyUsage )
  {
    kWarning( keyUsage == 0, 5150 )
      << "KeySelectionDialog: keyUsage == 0. You want to use AllKeys instead.";
  }

  QString ColumnStrategy::title( int col ) const {
    switch ( col ) {
    case 0: return i18n("Key ID");
    case 1: return i18n("User ID");
    default: return QString();
    }
  }

  int ColumnStrategy::width( int col, const QFontMetrics & fm ) const {
    if ( col == 0 ) {
      static const char hexchars[] = "0123456789ABCDEF";
      int maxWidth = 0;
      for ( unsigned int i = 0 ; i < 16 ; ++i )
        maxWidth = qMax( fm.width( QLatin1Char( hexchars[i] ) ), maxWidth );
      return 8 * maxWidth + 2 * KIconLoader::SizeSmall;
    }
    return Kleo::KeyListView::ColumnStrategy::width( col, fm );
  }

  QString ColumnStrategy::text( const GpgME::Key & key, int col ) const {
    switch ( col ) {
    case 0:
      {
        if ( key.shortKeyID() )
          return QString::fromUtf8( key.shortKeyID() );
        else
          return i18n("<placeholder>unknown</placeholder>");
      }
      break;
    case 1:
      {
        const char * uid = key.userID(0).id();
        if ( key.protocol() == GpgME::OpenPGP )
          return uid && *uid ? QString::fromUtf8( uid ) : QString() ;
        else // CMS
          return Kleo::DN( uid ).prettyDN();
      }
      break;
    default: return QString();
    }
  }

  QString ColumnStrategy::toolTip( const GpgME::Key & key, int ) const {
    const char * uid = key.userID(0).id();
    const char * fpr = key.primaryFingerprint();
    const char * issuer = key.issuerName();
    const GpgME::Subkey subkey = key.subkey(0);
    const QString expiry = subkey.neverExpires() ? i18n("never") : time_t2string( subkey.expirationTime() ) ;
    const QString creation = time_t2string( subkey.creationTime() );
    if ( key.protocol() == GpgME::OpenPGP )
      return i18n( "OpenPGP key for %1\n"
                   "Created: %2\n"
                   "Expiry: %3\n"
                   "Fingerprint: %4",
              uid ? QString::fromUtf8( uid ) : i18n("unknown"),
              creation, expiry,
              fpr ? QString::fromLatin1( fpr ) : i18n("unknown") );
    else
      return i18n( "S/MIME key for %1\n"
                   "Created: %2\n"
                   "Expiry: %3\n"
                   "Fingerprint: %4\n"
                   "Issuer: %5",
              uid ? Kleo::DN( uid ).prettyDN() : i18n("unknown"),
              creation, expiry,
              fpr ? QString::fromLatin1( fpr ) : i18n("unknown"),
              issuer ? Kleo::DN( issuer ).prettyDN() : i18n("unknown") );
  }

  QIcon ColumnStrategy::icon( const GpgME::Key & key, int col ) const {
    if ( col != 0 )
      return QIcon();
    // this key did not undergo a validating keylisting yet:
    if ( !( key.keyListMode() & GpgME::Validate ) )
      return mKeyUnknownPix;

    if ( !checkKeyUsage( key, mKeyUsage ) )
      return mKeyBadPix;

    if ( key.protocol() == GpgME::CMS )
      return mKeyGoodPix;

    switch ( key.userID(0).validity() ) {
    default:
    case GpgME::UserID::Unknown:
    case GpgME::UserID::Undefined:
      return mKeyUnknownPix;
    case GpgME::UserID::Never:
      return mKeyValidPix;
    case GpgME::UserID::Marginal:
    case GpgME::UserID::Full:
    case GpgME::UserID::Ultimate:
      return mKeyGoodPix;
    }
  }

}


static const int sCheckSelectionDelay = 250;

Kleo::KeySelectionDialog::KeySelectionDialog( const QString & title,
                                              const QString & text,
                                              const std::vector<GpgME::Key> & selectedKeys,
                                              unsigned int keyUsage,
                                              bool extendedSelection,
                                              bool rememberChoice,
                                              QWidget * parent,
                                              bool modal )
  : KDialog( parent ),
    mOpenPGPBackend( 0 ),
    mSMIMEBackend( 0 ),
    mRememberCB( 0 ),
    mSelectedKeys( selectedKeys ),
    mKeyUsage( keyUsage ),
    mCurrentContextMenuItem( 0 )
{
  setCaption( title );
  setButtons( User1|User2|Ok|Cancel );
  setDefaultButton( Ok );
  setModal( modal );
  init( rememberChoice, extendedSelection, text, QString() );
}

Kleo::KeySelectionDialog::KeySelectionDialog( const QString & title,
                                              const QString & text,
                                              const QString & initialQuery,
                                              const std::vector<GpgME::Key> & selectedKeys,
                                              unsigned int keyUsage,
                                              bool extendedSelection,
                                              bool rememberChoice,
                                              QWidget * parent,
                                              bool modal )
  : KDialog( parent ),
    mOpenPGPBackend( 0 ),
    mSMIMEBackend( 0 ),
    mRememberCB( 0 ),
    mSelectedKeys( selectedKeys ),
    mKeyUsage( keyUsage ),
    mSearchText( initialQuery ),
    mInitialQuery( initialQuery ),
    mCurrentContextMenuItem( 0 )
{
  setCaption( title );
  setButtons( User1|User2|Ok|Cancel );
  setDefaultButton( Ok );
  setModal( modal );
  init( rememberChoice, extendedSelection, text, initialQuery );
}

Kleo::KeySelectionDialog::KeySelectionDialog( const QString & title,
                                              const QString & text,
                                              const QString & initialQuery,
                                              unsigned int keyUsage,
                                              bool extendedSelection,
                                              bool rememberChoice,
                                              QWidget * parent,
                                              bool modal )
  : KDialog( parent ),
    mOpenPGPBackend( 0 ),
    mSMIMEBackend( 0 ),
    mRememberCB( 0 ),
    mKeyUsage( keyUsage ),
    mSearchText( initialQuery ),
    mInitialQuery( initialQuery ),
    mCurrentContextMenuItem( 0 )
{
  setCaption( title );
  setButtons( User1|User2|Ok|Cancel );
  setDefaultButton( Ok );
  setModal( modal );
  init( rememberChoice, extendedSelection, text, initialQuery );
}

void Kleo::KeySelectionDialog::init( bool rememberChoice, bool extendedSelection,
                                     const QString & text, const QString & initialQuery ) {
  if ( mKeyUsage & OpenPGPKeys )
    mOpenPGPBackend = Kleo::CryptoBackendFactory::instance()->openpgp();
  if ( mKeyUsage & SMIMEKeys )
    mSMIMEBackend = Kleo::CryptoBackendFactory::instance()->smime();

  mCheckSelectionTimer = new QTimer( this );
  mStartSearchTimer = new QTimer( this );

  QFrame *page = new QFrame( this );
  setMainWidget( page );
  mTopLayout = new QVBoxLayout( page );
  mTopLayout->setMargin( 0 );
  mTopLayout->setSpacing( spacingHint() );

  if ( !text.isEmpty() ) {
#ifndef KDEPIM_MOBILE_UI
    QLabel* textLabel = new QLabel( text, page );
    textLabel->setWordWrap( true );

    // Setting the size policy is necessary as a workaround for https://issues.kolab.org/issue4429
    // and http://bugreports.qt.nokia.com/browse/QTBUG-8740
    textLabel->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
    connect( textLabel, SIGNAL(linkActivated(QString)), SLOT(slotStartCertificateManager(QString)) );
    mTopLayout->addWidget( textLabel );
#endif
  }

#ifndef KDEPIM_MOBILE_UI
  QPushButton * const searchExternalPB =
    new QPushButton( i18n( "Search for &External Certificates" ), page );
  mTopLayout->addWidget( searchExternalPB, 0, Qt::AlignLeft );
  connect( searchExternalPB, SIGNAL(clicked()),
           this, SLOT(slotStartSearchForExternalCertificates()) );
  if ( initialQuery.isEmpty() ) {
    searchExternalPB->hide();
  }
#endif

  QHBoxLayout * hlay = new QHBoxLayout();
  mTopLayout->addLayout( hlay );

  KLineEdit * le = new KLineEdit( page );
  le->setClearButtonShown(true);
  le->setText( initialQuery );

  QLabel* lbSearchFor =  new QLabel( i18n("&Search for:"), page ) ;
  lbSearchFor->setBuddy(le);

  hlay->addWidget(lbSearchFor);
  hlay->addWidget( le, 1 );
  le->setFocus();

  connect( le, SIGNAL(textChanged(QString)),
           this, SLOT(slotSearch(QString)) );
  connect( mStartSearchTimer, SIGNAL(timeout()), SLOT(slotFilter()) );

  mKeyListView = new KeyListView( new ColumnStrategy( mKeyUsage ), 0, page );
  mKeyListView->setObjectName( QLatin1String("mKeyListView") );
  mKeyListView->header()->stretchLastSection();
  mKeyListView->setRootIsDecorated( true );
  mKeyListView->setSortingEnabled( true );
  mKeyListView->header()->setSortIndicatorShown( true );
  mKeyListView->header()->setSortIndicator( 1, Qt::AscendingOrder ); // sort by User ID
  if ( extendedSelection )
    mKeyListView->setSelectionMode( QAbstractItemView::ExtendedSelection );
  mTopLayout->addWidget( mKeyListView, 10 );

  if ( rememberChoice ) {
#ifndef KDEPIM_MOBILE_UI
    mRememberCB = new QCheckBox( i18n("&Remember choice"), page );
    mTopLayout->addWidget( mRememberCB );
    mRememberCB->setWhatsThis(
                     i18n("<qt><p>If you check this box your choice will "
                          "be stored and you will not be asked again."
                          "</p></qt>") );
#endif
  }

  connect( mCheckSelectionTimer, SIGNAL(timeout()),
           SLOT(slotCheckSelection()) );
  connectSignals();

  connect( mKeyListView,
           SIGNAL(doubleClicked(Kleo::KeyListViewItem*,int)),
           SLOT(slotTryOk()) );
  connect( mKeyListView,
           SIGNAL(contextMenu(Kleo::KeyListViewItem*,QPoint)),
           SLOT(slotRMB(Kleo::KeyListViewItem*,QPoint)) );

  setButtonText( KDialog::User1, i18n("&Reread Keys") );
  setButtonText( KDialog::User2, i18n("&Start Certificate Manager") );
  connect( this, SIGNAL(user1Clicked()), this, SLOT(slotRereadKeys()) );
  connect( this, SIGNAL(user2Clicked()), this, SLOT(slotStartCertificateManager()) );
  connect( this, SIGNAL(okClicked()), this, SLOT(slotOk()));
  connect( this, SIGNAL(cancelClicked()),this,SLOT(slotCancel()));
  slotRereadKeys();
  mTopLayout->activate();

  if ( qApp ) {
    QSize dialogSize( sizeHint() );
    int iconSize = IconSize(KIconLoader::Desktop);
    int miniSize = IconSize(KIconLoader::Small);
    KWindowSystem::setIcons( winId(), qApp->windowIcon().pixmap(iconSize, iconSize),
                             qApp->windowIcon().pixmap(miniSize, miniSize) );

    KConfigGroup dialogConfig( KSharedConfig::openConfig(), "Key Selection Dialog" );
    dialogSize = dialogConfig.readEntry( "Dialog size", dialogSize );
    const QByteArray headerState = dialogConfig.readEntry( "header", QByteArray());
    if (!headerState.isEmpty())
        mKeyListView->header()->restoreState(headerState);
    resize( dialogSize );
  }
}

Kleo::KeySelectionDialog::~KeySelectionDialog() {
  KConfigGroup dialogConfig( KSharedConfig::openConfig(), "Key Selection Dialog" );
  dialogConfig.writeEntry( "Dialog size", size() );
  dialogConfig.writeEntry( "header", mKeyListView->header()->saveState());
  dialogConfig.sync();
}


void Kleo::KeySelectionDialog::connectSignals() {
  if ( mKeyListView->isMultiSelection() )
    connect( mKeyListView, SIGNAL(itemSelectionChanged()),
             SLOT(slotSelectionChanged()) );
  else
    connect( mKeyListView, SIGNAL(selectionChanged(Kleo::KeyListViewItem*)),
             SLOT(slotCheckSelection(Kleo::KeyListViewItem*)) );
}

void Kleo::KeySelectionDialog::disconnectSignals() {
  if ( mKeyListView->isMultiSelection() )
    disconnect( mKeyListView, SIGNAL(itemSelectionChanged()),
                this, SLOT(slotSelectionChanged()) );
  else
    disconnect( mKeyListView, SIGNAL(selectionChanged(Kleo::KeyListViewItem*)),
                this, SLOT(slotCheckSelection(Kleo::KeyListViewItem*)) );
}

const GpgME::Key & Kleo::KeySelectionDialog::selectedKey() const {
  static const GpgME::Key null = GpgME::Key::null;
  if ( mKeyListView->isMultiSelection() || !mKeyListView->selectedItem() )
    return null;
  return mKeyListView->selectedItem()->key();
}

QString Kleo::KeySelectionDialog::fingerprint() const {
  return QLatin1String(selectedKey().primaryFingerprint());
}

QStringList Kleo::KeySelectionDialog::fingerprints() const {
  QStringList result;
  for ( std::vector<GpgME::Key>::const_iterator it = mSelectedKeys.begin() ; it != mSelectedKeys.end() ; ++it )
    if ( const char * fpr = it->primaryFingerprint() )
      result.push_back( QLatin1String(fpr) );
  return result;
}

QStringList Kleo::KeySelectionDialog::pgpKeyFingerprints() const {
  QStringList result;
  for ( std::vector<GpgME::Key>::const_iterator it = mSelectedKeys.begin() ; it != mSelectedKeys.end() ; ++it )
    if ( it->protocol() == GpgME::OpenPGP )
      if ( const char * fpr = it->primaryFingerprint() )
        result.push_back( QLatin1String(fpr) );
  return result;
}

QStringList Kleo::KeySelectionDialog::smimeFingerprints() const {
  QStringList result;
  for ( std::vector<GpgME::Key>::const_iterator it = mSelectedKeys.begin() ; it != mSelectedKeys.end() ; ++it )
    if ( it->protocol() == GpgME::CMS )
      if ( const char * fpr = it->primaryFingerprint() )
        result.push_back( QLatin1String(fpr) );
  return result;
}

void Kleo::KeySelectionDialog::slotRereadKeys() {
  mKeyListView->clear();
  mListJobCount = 0;
  mTruncated = 0;
  mSavedOffsetY = mKeyListView->verticalScrollBar()->value();

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

void Kleo::KeySelectionDialog::slotStartCertificateManager( const QString &query )
{
  QStringList args;
  // ### waits for bug 175980 to be fixed, ie. those command line args to be added again
#if 0
  // ### port to libkleopatra
  if ( !query.isEmpty() )
    args << QLatin1String("--external") << QLatin1String("--query") << KUrl::decode_string( query );
#endif
  Q_UNUSED( query );
  if( !QProcess::startDetached( QLatin1String("kleopatra"), args ) )
    KMessageBox::error( this,
                        i18n( "Could not start certificate manager; "
                              "please check your installation." ),
                        i18n( "Certificate Manager Error" ) );
  else
    kDebug(5150) <<"\nslotStartCertManager(): certificate manager started.";
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

namespace {
  struct ExtractFingerprint {
    QString operator()( const GpgME::Key & key ) {
      return QLatin1String(key.primaryFingerprint());
    }
  };
}

void Kleo::KeySelectionDialog::startKeyListJobForBackend( const CryptoBackend::Protocol * backend, const std::vector<GpgME::Key> & keys, bool validate ) {
  assert( backend );
  KeyListJob * job = backend->keyListJob( false, false, validate ); // local, w/o sigs, validation as givem
  if ( !job )
    return;

  connect( job, SIGNAL(result(GpgME::KeyListResult)),
           SLOT(slotKeyListResult(GpgME::KeyListResult)) );
  if ( validate )
    connect( job, SIGNAL(nextKey(GpgME::Key)),
             mKeyListView, SLOT(slotRefreshKey(GpgME::Key)) );
  else
    connect( job, SIGNAL(nextKey(GpgME::Key)),
             mKeyListView, SLOT(slotAddKey(GpgME::Key)) );

  QStringList fprs;
  std::transform( keys.begin(), keys.end(), std::back_inserter( fprs ), ExtractFingerprint() );
  const GpgME::Error err = job->start( fprs, mKeyUsage & SecretKeys && !( mKeyUsage & PublicKeys ) );

  if ( err )
    return showKeyListError( this, err );

#ifndef LIBKLEO_NO_PROGRESSDIALOG
  // FIXME: create a MultiProgressDialog:
  (void)new ProgressDialog( job, validate ? i18n( "Checking selected keys..." ) : i18n( "Fetching keys..." ), this );
#endif
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
                              i18np("<qt>One backend returned truncated output.<p>"
                                   "Not all available keys are shown</p></qt>",
                                   "<qt>%1 backends returned truncated output.<p>"
                                   "Not all available keys are shown</p></qt>",
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
  mKeyListView->verticalScrollBar()->setValue( mSavedOffsetY ); mSavedOffsetY = 0;
}

void Kleo::KeySelectionDialog::slotSelectionChanged() {
  kDebug(5150) <<"KeySelectionDialog::slotSelectionChanged()";

  // (re)start the check selection timer. Checking the selection is delayed
  // because else drag-selection doesn't work very good (checking key trust
  // is slow).
  mCheckSelectionTimer->start( sCheckSelectionDelay );
}

namespace {
  struct AlreadyChecked {
    bool operator()( const GpgME::Key & key ) const {
      return key.keyListMode() & GpgME::Validate ;
    }
  };
}

void Kleo::KeySelectionDialog::slotCheckSelection( KeyListViewItem * item ) {
  kDebug(5150) <<"KeySelectionDialog::slotCheckSelection()";

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
    enableButton( Ok, !mSelectedKeys.empty() &&
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
  mSavedOffsetY = mKeyListView->verticalScrollBar()->value();

  disconnectSignals();
  mKeyListView->setEnabled( false );

  std::vector<GpgME::Key> smime, openpgp;
  for ( std::vector<GpgME::Key>::const_iterator it = mKeysToCheck.begin() ; it != mKeysToCheck.end() ; ++it )
    if ( it->protocol() == GpgME::OpenPGP )
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

void Kleo::KeySelectionDialog::slotRMB( Kleo::KeyListViewItem * item, const QPoint & p ) {
  if ( !item ) return;

  mCurrentContextMenuItem = item;

  QMenu menu;
  menu.addAction( i18n( "Recheck Key" ), this, SLOT(slotRecheckKey()) );
  menu.exec( p );
}

void Kleo::KeySelectionDialog::slotRecheckKey() {
  if ( !mCurrentContextMenuItem || mCurrentContextMenuItem->key().isNull() )
    return;

  mKeysToCheck.clear();
  mKeysToCheck.push_back( mCurrentContextMenuItem->key() );
}

void Kleo::KeySelectionDialog::slotTryOk() {
  if ( !mSelectedKeys.empty() && checkKeyUsage( mSelectedKeys, mKeyUsage ) )
    slotOk();
}

void Kleo::KeySelectionDialog::slotOk() {
  if ( mCheckSelectionTimer->isActive() )
    slotCheckSelection();

  // button could be disabled again after checking the selected key
  if ( !mSelectedKeys.empty() && checkKeyUsage( mSelectedKeys, mKeyUsage ) )
    return;

  mStartSearchTimer->stop();
  accept();
}


void Kleo::KeySelectionDialog::slotCancel() {
  mCheckSelectionTimer->stop();
  mStartSearchTimer->stop();
  reject();
}

void Kleo::KeySelectionDialog::slotSearch( const QString & text ) {
  mSearchText = text.trimmed().toUpper();
  slotSearch();
}

void Kleo::KeySelectionDialog::slotSearch() {
  mStartSearchTimer->setSingleShot( true );
  mStartSearchTimer->start( sCheckSelectionDelay );
}

void Kleo::KeySelectionDialog::slotFilter() {
  if ( mSearchText.isEmpty() ) {
    showAllItems();
    return;
  }

  // OK, so we need to filter:
  QRegExp keyIdRegExp( QLatin1String("(?:0x)?[A-F0-9]{1,8}"), Qt::CaseInsensitive );
  if ( keyIdRegExp.exactMatch( mSearchText ) ) {
    if ( mSearchText.startsWith( QLatin1String("0X") ) )
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
      item->setHidden( !item->text( 0 ).toUpper().startsWith( keyID ) );
}

static bool anyUIDMatches( const Kleo::KeyListViewItem * item, QRegExp & rx ) {
  if ( !item )
    return false;

  const std::vector<GpgME::UserID> uids = item->key().userIDs();
  for ( std::vector<GpgME::UserID>::const_iterator it = uids.begin() ; it != uids.end() ; ++it )
    if ( it->id() && rx.indexIn( QString::fromUtf8( it->id() ) ) >= 0 )
      return true;
  return false;
}

void Kleo::KeySelectionDialog::filterByKeyIDOrUID( const QString & str ) {
  assert( !str.isEmpty() );

  // match beginnings of words:
  QRegExp rx( QLatin1String("\\b") + QRegExp::escape( str ), Qt::CaseInsensitive );
  
  for ( KeyListViewItem * item = mKeyListView->firstChild() ; item ; item = item->nextSibling() )
    item->setHidden( !item->text( 0 ).toUpper().startsWith( str ) && !anyUIDMatches( item, rx ) );

}

void Kleo::KeySelectionDialog::filterByUID( const QString & str ) {
  assert( !str.isEmpty() );

  // match beginnings of words:
  QRegExp rx( QLatin1String("\\b") + QRegExp::escape( str ), Qt::CaseInsensitive );

  for ( KeyListViewItem * item = mKeyListView->firstChild() ; item ; item = item->nextSibling() )
    item->setHidden( !anyUIDMatches( item, rx ) );
}


void Kleo::KeySelectionDialog::showAllItems() {
  for ( KeyListViewItem * item = mKeyListView->firstChild() ; item ; item = item->nextSibling() )
    item->setHidden( false );
}

