/*
    certificateinfowidgetimpl.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2002,2004 Klarälvdalens Datakonsult AB

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

#include "certificateinfowidgetimpl.h"

// libkleopatra
#include "libkleo/kleo/keylistjob.h"
#include "libkleo/kleo/dn.h"
#include "libkleo/kleo/cryptobackendfactory.h"
#include "libkleo/ui/progressdialog.h"

// gpgme++
#include <gpgme++/keylistresult.h>

// KDE
#include <klocale.h>
#include <kdialog.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kglobalsettings.h>

// Qt
#include <QTreeWidget>
#include <QTextEdit>
#include <QHeaderView>
#include <QPushButton>
#include <QCursor>
#include <QApplication>
#include <QDateTime>

// other
#include <assert.h>
#include <QTextCodec>

CertificateInfoWidgetImpl::CertificateInfoWidgetImpl( const GpgME::Key & key, bool external,
						      QWidget * parent )
  : CertificateInfoWidget( parent ),
    mExternal( external ),
    mFoundIssuer( true ),
    mHaveKeyLocally( false ),
    mProc(0L)
{
  importButton->setEnabled( false );

  listView->header()->setResizeMode( 0, QHeaderView::ResizeToContents );
  listView->header()->setResizeMode( 1, QHeaderView::Stretch );

  QFontMetrics fm = fontMetrics();
  listView->setColumnWidth( 1, fm.width( i18n("Information") ) * 5 );

  listView->header()->setClickable( false );
  listView->header()->setSortIndicatorShown( false );

  connect( listView, SIGNAL(itemSelectionChanged()),
	   this, SLOT(slotShowInfo()) );
  pathView->header()->setResizeMode( 0, QHeaderView::Stretch );
  pathView->header()->hide();

  connect( pathView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
	   this, SLOT(slotShowCertPathDetails(QTreeWidgetItem*)) );
  connect( importButton, SIGNAL( clicked() ),
	   this, SLOT( slotImportCertificate() ) );

  dumpView->setFont( KGlobalSettings::fixedFont() );

  if ( !key.isNull() )
    setKey( key );
}

static QString time_t2string( time_t t ) {
  QDateTime dt;
  dt.setTime_t( t );
  return dt.toString();
}

static QTreeWidgetItem * makeItem( QTreeWidget * p, const QString & text0, const QString & text1 ) {
    return new QTreeWidgetItem( p, QStringList() << text0 << text1 );
}

static QTreeWidgetItem * makeItem( QTreeWidget * p, const QString & text ) {
    return new QTreeWidgetItem( p, QStringList() << text );
}

static QTreeWidgetItem * makeItem( QTreeWidgetItem * p, const QString & text ) {
    return new QTreeWidgetItem( p, QStringList() << text );
}

void CertificateInfoWidgetImpl::setKey( const GpgME::Key & key  ) {
  mChain.clear();
  mFoundIssuer = true;
  mHaveKeyLocally = false;

  listView->clear();
  pathView->clear();
  importButton->setEnabled( false );

  if ( key.isNull() )
    return;

  mChain.push_front( key );
  startKeyExistanceCheck(); // starts a local keylisting to enable the
			    // importButton if needed

  makeItem( listView, i18n("Valid"), i18n("From %1 to %2",
                                          time_t2string( key.subkey(0).creationTime() ),
                                          time_t2string( key.subkey(0).expirationTime() ) ) );
  makeItem( listView, i18n("Can be used for signing"),
            key.canSign() ? i18n("Yes") : i18n("No") );
  makeItem( listView, i18n("Can be used for encryption"),
            key.canEncrypt() ? i18n("Yes") : i18n("No") );
  makeItem( listView, i18n("Can be used for certification"),
            key.canCertify() ? i18n("Yes") : i18n("No") );
  makeItem( listView, i18n("Can be used for authentication"),
            key.canAuthenticate() ? i18n("Yes") : i18n("No" ) );
  makeItem( listView, i18n("Fingerprint"), key.primaryFingerprint() );
  makeItem( listView, i18n("Issuer"), Kleo::DN( key.issuerName() ).prettyDN() );
  makeItem( listView, i18n("Serial Number"), key.issuerSerial() );
  
  const Kleo::DN dn( key.userID(0).id() );

  // FIXME: use the attributeLabelMap from certificatewizardimpl.cpp:
  static QMap<QString,QString> dnComponentNames;
  if ( dnComponentNames.isEmpty() ) {
	dnComponentNames["C"] = i18n("Country");
	dnComponentNames["OU"] = i18n("Organizational Unit");
	dnComponentNames["O"] = i18n("Organization");
	dnComponentNames["L"] = i18n("Location");
	dnComponentNames["CN"] = i18n("Common Name");
	dnComponentNames["EMAIL"] = i18n("Email");
  }

  for ( Kleo::DN::const_iterator dnit = dn.begin() ; dnit != dn.end() ; ++dnit ) {
	QString displayName = (*dnit).name();
	if( dnComponentNames.contains(displayName) ) displayName = dnComponentNames[displayName];
	makeItem( listView, displayName, (*dnit).value() );
  }

  const std::vector<GpgME::UserID> uids = key.userIDs();
  if ( !uids.empty() ) {
    makeItem( listView, i18n("Subject"),
              Kleo::DN( uids.front().id() ).prettyDN() );
    for ( std::vector<GpgME::UserID>::const_iterator it = uids.begin() + 1 ; it != uids.end() ; ++it ) {
      if ( !(*it).id() )
	continue;
      const QString email = QString::fromUtf8( (*it).id() ).trimmed();
      if ( email.isEmpty() )
	continue;
      if ( email.startsWith( "<" ) )
	makeItem( listView, i18n("Email"),
                  email.mid( 1, email.length()-2 ) );
      else
	makeItem( listView, i18n("A.k.a."), email );
    }
  }

  updateChainView();
  startCertificateChainListing();
  startCertificateDump();
}

static void showChainListError( QWidget * parent, const GpgME::Error & err, const char * subject ) {
  assert( err );
  const QString msg = i18n("<qt><p>An error occurred while fetching "
			   "the certificate <b>%1</b> from the backend:</p>"
			   "<p><b>%2</b></p></qt>",
    subject ? QString::fromUtf8( subject ) : QString(),
    QString::fromLocal8Bit( err.asString() ) );
  KMessageBox::error( parent, msg, i18n("Certificate Listing Failed" ) );
}

void CertificateInfoWidgetImpl::startCertificateChainListing() {
  kDebug() <<"CertificateInfoWidgetImpl::startCertificateChainListing()";

  if ( mChain.empty() ) {
    // we need a seed...
    kWarning() <<"CertificateInfoWidgetImpl::startCertificateChainListing(): mChain is empty!";
    return;
  }
  const char * chainID = mChain.front().chainID();
  if ( !chainID || !*chainID ) {
    // cert not found:
    kDebug() <<"CertificateInfoWidgetImpl::startCertificateChainListing(): empty chain ID - root not found";
    return;
  }
  const char * fpr = mChain.front().primaryFingerprint();
  if ( qstricmp( fpr, chainID ) == 0 ) {
    kDebug() <<"CertificateInfoWidgetImpl::startCertificateChainListing(): chain_id equals fingerprint -> found root";
    return;
  }
  if ( mChain.size() > 100 ) {
    // safe guard against certificate loops (paranoia factor 8 out of 10)...
    kWarning() <<"CertificateInfoWidgetImpl::startCertificateChainListing(): maximum chain length of 100 exceeded!";
    return;
  }
  if ( !mFoundIssuer ) {
    // key listing failed. Don't end up in endless loop
    kDebug() <<"CertificateInfoWidgetImpl::startCertificateChainListing(): issuer not found - giving up";
    return;
  }

  mFoundIssuer = false;

  // gpgsm / dirmngr / LDAP / whoever doesn't support looking up
  // external keys by fingerprint. Furthermore, since we actually got
  // a chain-id set on the key, we know that we have the issuer's cert
  // in the local keyring, so just use local keylisting.
  Kleo::KeyListJob * job =
    Kleo::CryptoBackendFactory::instance()->smime()->keyListJob( false );
  assert( job );

  connect( job, SIGNAL(result(const GpgME::KeyListResult&)),
	   SLOT(slotCertificateChainListingResult(const GpgME::KeyListResult&)) );
  connect( job, SIGNAL(nextKey(const GpgME::Key&)),
	   SLOT(slotNextKey(const GpgME::Key&)) );

  kDebug() <<"Going to fetch" << endl
	    << "  issuer  : \"" << mChain.front().issuerName() << "\"" << endl
	    << "  chain id:" << mChain.front().chainID() << endl
	    << "for" << endl
	    << "  subject : \"" << mChain.front().userID(0).id() << "\"" << endl
	    << "  subj.fpr:" << mChain.front().primaryFingerprint();

  const GpgME::Error err = job->start( QStringList( mChain.front().chainID() ) );

  if ( err )
    showChainListError( this, err, mChain.front().issuerName() );
  else
    (void)new Kleo::ProgressDialog( job, i18n("Fetching Certificate Chain"), this );
}

void CertificateInfoWidgetImpl::startCertificateDump() {
  delete mProc;
  mProc = new KProcess( this );
  (*mProc) << "gpgsm"; // must be in the PATH
  (*mProc) << "--dump-keys";
  (*mProc) << mChain.front().primaryFingerprint();

  connect( mProc, SIGNAL(readyReadStandardOutput ()),
           this, SLOT( slotCollectStdout()));
  connect( mProc, SIGNAL(readyReadStandardError()),
           this, SLOT(slotCollectStderr()));
  connect( mProc, SIGNAL(finished (int, QProcess::ExitStatus)),
           this,SLOT(slotDumpProcessExited(int, QProcess::ExitStatus)));

  mProc->setOutputChannelMode(KProcess::SeparateChannels);
  mProc->start();
  if ( !mProc->waitForStarted()) {
    QString wmsg = i18n("Failed to execute gpgsm:\n%1", i18n( "program not found" ) );
    dumpView->setText( wmsg );
    delete mProc;
    mProc = 0;
  }
}

void CertificateInfoWidgetImpl::slotCollectStdout()
{
  mDumpOutput += mProc->readAllStandardOutput ();
}

void CertificateInfoWidgetImpl::slotCollectStderr()
{
   mDumpOutput += mProc->readAllStandardError ();
}

void CertificateInfoWidgetImpl::slotDumpProcessExited(int, QProcess::ExitStatus) {
  int rc = ( mProc->exitStatus() == QProcess::NormalExit  ) ? mProc->exitStatus() : -1 ;

  if ( rc == 0 ) {
    dumpView->setText( QString::fromUtf8( mDumpOutput ) );
  } else {
    if ( !mDumpError.isEmpty() ) {
      dumpView->setText( QString::fromUtf8( mDumpError ) );
    } else
    {
      QString reason;
      if ( rc == -1 )
        reason = i18n( "program cannot be executed" );
      else
        reason = strerror(rc);
      dumpView->setText( i18n("Failed to execute gpgsm:\n%1", reason) );
    }
  }
}

void CertificateInfoWidgetImpl::slotNextKey( const GpgME::Key & key ) {
  kDebug() <<"CertificateInfoWidgetImpl::slotNextKey( \""
	    << key.userID(0).id() << "\" )";
  if ( key.isNull() )
    return;

  mFoundIssuer = true;
  mChain.push_front( key );
  updateChainView();
  // FIXME: cancel the keylisting. We're only interested in _one_ key.
}

void CertificateInfoWidgetImpl::updateChainView() {
  pathView->clear();
  if ( mChain.empty() )
    return;
  QTreeWidgetItem * item = 0;

  QList<GpgME::Key>::const_iterator it = mChain.begin();
  // root item:
  if ( (*it).chainID() && qstrcmp( (*it).chainID(), (*it).primaryFingerprint() ) == 0 )
    item = makeItem( pathView, Kleo::DN( (*it++).userID(0).id() ).prettyDN() );
  else {
    item = makeItem( pathView, i18n("Issuer certificate not found (%1)",
                                    Kleo::DN( (*it).issuerName() ).prettyDN() ) );
    item->setExpanded( true ); // Qt bug: doesn't open after setEnabled( false ) :/
    item->setDisabled( true );
  }
  item->setExpanded( true );

  // subsequent items:
  while ( it != mChain.end() ) {
    item = makeItem( item, Kleo::DN( (*it++).userID(0).id() ).prettyDN() );
    item->setExpanded( true );
  }
}

void CertificateInfoWidgetImpl::slotCertificateChainListingResult( const GpgME::KeyListResult & res ) {
  if ( res.error() )
    showChainListError( this, res.error(), mChain.front().issuerName() );
  else
    startCertificateChainListing();
}

void CertificateInfoWidgetImpl::slotShowInfo() {
    const QList<QTreeWidgetItem*> items = listView->selectedItems();
    textView->setText( items.empty() ? QString() : items.front()->text(1) );
}

void CertificateInfoWidgetImpl::slotShowCertPathDetails( QTreeWidgetItem * item ) {
  if ( !item )
    return;

  // find the key corresponding to "item". This hack would not be
  // necessary if pathView was a Kleo::KeyListView, but it's
  // Qt-Designer-generated and I don't feel like creating a custom
  // widget spec for Kleo::KeyListView.
  int totalCount = 0;
  int itemIndex = -1;
  for ( const QTreeWidgetItem * i = pathView->topLevelItem( 0 ) ; i ; i = i->child( 0 ) ) {
    if ( i == item )
      itemIndex = totalCount;
    ++totalCount;
  }

  assert( totalCount == mChain.size() || totalCount == mChain.size() + 1 );

  // skip pseudo root item with "not found message":
  if ( totalCount == mChain.size() + 1 )
    --itemIndex;

  assert( itemIndex >= 0 );

  CertificateInfoWidgetImpl * const top = new CertificateInfoWidgetImpl( mChain[itemIndex], mExternal );
  // proxy the signal to our receiver:
  connect( top, SIGNAL(requestCertificateDownload(QString, QString)),
  SIGNAL(requestCertificateDownload(QString, QString)) );
  KDialog * const dialog = createDialog( top, this );
  dialog->show();
}

KDialog * CertificateInfoWidgetImpl::createDialog( const GpgME::Key & key, QWidget * parent )
{
  return createDialog( new CertificateInfoWidgetImpl( key, /*external=*/false ), parent );  
}

KDialog * CertificateInfoWidgetImpl::createDialog( CertificateInfoWidgetImpl * widget, QWidget * parent )
{
  KDialog * const dialog = new KDialog( parent );
  dialog->setObjectName( "dialog" );
  dialog->setButtons( KDialog::Close );
  dialog->setDefaultButton( KDialog::Close );
  dialog->setModal( false );
  dialog->setMainWidget( widget );
  dialog->setWindowTitle( i18n("Additional Information for Certificate") );
  dialog->setAttribute( Qt::WA_DeleteOnClose );
  return dialog;
}

void CertificateInfoWidgetImpl::slotImportCertificate()
{
  if ( mChain.empty() || mChain.back().isNull() )
    return;
  const Kleo::DN dn( mChain.back().userID( 0 ).id() );
  emit requestCertificateDownload( mChain.back().primaryFingerprint(), dn.prettyDN() );
  importButton->setEnabled( false );
}

void CertificateInfoWidgetImpl::startKeyExistanceCheck() {
  if ( !mExternal )
    // we already have it if it's from a local keylisting :)
    return;
  if ( mChain.empty() || mChain.back().isNull() )
    // need a key to look for
    return;
  const QString fingerprint = mChain.back().primaryFingerprint();
  if ( fingerprint.isEmpty() )
    // empty pattern means list all keys. We don't want that
    return;

  // start _local_ keylistjob (no progressdialog needed here):
  Kleo::KeyListJob * job =
    Kleo::CryptoBackendFactory::instance()->smime()->keyListJob( false );
  assert( job );

  connect( job, SIGNAL(nextKey(const GpgME::Key&)),
	   SLOT(slotKeyExistanceCheckNextCandidate(const GpgME::Key&)) );
  connect( job, SIGNAL(result(const GpgME::KeyListResult&)),
	   SLOT(slotKeyExistanceCheckFinished()) );
  // nor to check for errors:
  job->start( QStringList( fingerprint ) );
}

void CertificateInfoWidgetImpl::slotKeyExistanceCheckNextCandidate( const GpgME::Key & key ) {
  if ( key.isNull() || mChain.empty() || !key.primaryFingerprint() )
    return;

  if ( qstrcmp( key.primaryFingerprint(),
		mChain.back().primaryFingerprint() ) == 0 )
    mHaveKeyLocally = true;
}

void CertificateInfoWidgetImpl::slotKeyExistanceCheckFinished() {
  importButton->setEnabled( !mHaveKeyLocally );
}


#include "certificateinfowidgetimpl.moc"
