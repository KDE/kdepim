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
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "certificateinfowidgetimpl.h"

// libkleopatra
#include <kleo/keylistjob.h>
#include <kleo/dn.h>
#include <kleo/cryptobackendfactory.h>

#include <ui/progressdialog.h>

// gpgme++
#include <gpgmepp/keylistresult.h>

// KDE
#include <klocale.h>
#include <kdialogbase.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kprocio.h>
#include <kglobalsettings.h>

// Qt
#include <qlistview.h>
#include <qtextedit.h>
#include <qheader.h>
#include <qpushbutton.h>
#include <qcursor.h>
#include <qapplication.h>
#include <qdatetime.h>

// other
#include <assert.h>
#include <qtextcodec.h>

CertificateInfoWidgetImpl::CertificateInfoWidgetImpl( const GpgME::Key & key, bool external,
						      QWidget * parent, const char * name )
  : CertificateInfoWidget( parent, name ),
    mExternal( external ),
    mFoundIssuer( true ),
    mHaveKeyLocally( false )
{
  importButton->setEnabled( false );

  listView->setColumnWidthMode( 1, QListView::Maximum );
  QFontMetrics fm = fontMetrics();
  listView->setColumnWidth( 1, fm.width( i18n("Information") ) * 5 );

  listView->header()->setClickEnabled( false );
  listView->setSorting( -1 );

  connect( listView, SIGNAL( selectionChanged( QListViewItem* ) ),
	   this, SLOT( slotShowInfo( QListViewItem* ) ) );
  pathView->setColumnWidthMode( 0, QListView::Maximum );
  pathView->header()->hide();

  connect( pathView, SIGNAL( doubleClicked( QListViewItem* ) ),
	   this, SLOT( slotShowCertPathDetails( QListViewItem* ) ) );
  connect( pathView, SIGNAL( returnPressed( QListViewItem* ) ),
	   this, SLOT( slotShowCertPathDetails( QListViewItem* ) ) );
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

  QListViewItem * item = 0;
  item = new QListViewItem( listView, item, i18n("Valid"), QString("From %1 to %2")
			    .arg( time_t2string( key.subkey(0).creationTime() ),
				  time_t2string( key.subkey(0).expirationTime() ) ) );
  item = new QListViewItem( listView, item, i18n("Can be used for signing"),
			    key.canSign() ? i18n("Yes") : i18n("No") );
  item = new QListViewItem( listView, item, i18n("Can be used for encryption"),
			    key.canEncrypt() ? i18n("Yes") : i18n("No") );
  item = new QListViewItem( listView, item, i18n("Can be used for certification"),
			    key.canCertify() ? i18n("Yes") : i18n("No") );
  item = new QListViewItem( listView, item, i18n("Can be used for authentication"),
			    key.canAuthenticate() ? i18n("Yes") : i18n("No" ) );
  item = new QListViewItem( listView, item, i18n("Fingerprint"), key.primaryFingerprint() );
  item = new QListViewItem( listView, item, i18n("Issuer"), Kleo::DN( key.issuerName() ).prettyDN() );
  item = new QListViewItem( listView, item, i18n("Serial Number"), key.issuerSerial() );

  const Kleo::DN dn = key.userID(0).id();

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
	item = new QListViewItem( listView, item, displayName, (*dnit).value() );
  }

  const std::vector<GpgME::UserID> uids = key.userIDs();
  if ( !uids.empty() ) {
    item = new QListViewItem( listView, item, i18n("Subject"),
			      Kleo::DN( uids.front().id() ).prettyDN() );
    for ( std::vector<GpgME::UserID>::const_iterator it = uids.begin() + 1 ; it != uids.end() ; ++it ) {
      if ( !(*it).id() )
	continue;
      const QString email = QString::fromUtf8( (*it).id() ).stripWhiteSpace();
      if ( email.isEmpty() )
	continue;
      if ( email.startsWith( "<" ) )
	item = new QListViewItem( listView, item, i18n("Email"),
				  email.mid( 1, email.length()-2 ) );
      else
	item = new QListViewItem( listView, item, i18n("A.k.a."), email );
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
			   "<p><b>%2</b></p></qt>")
    .arg( subject ? QString::fromUtf8( subject ) : QString::null,
	  QString::fromLocal8Bit( err.asString() ) );
  KMessageBox::error( parent, msg, i18n("Certificate Listing Failed" ) );
}

void CertificateInfoWidgetImpl::startCertificateChainListing() {
  kdDebug() << "CertificateInfoWidgetImpl::startCertificateChainListing()" << endl;

  if ( mChain.empty() ) {
    // we need a seed...
    kdWarning() << "CertificateInfoWidgetImpl::startCertificateChainListing(): mChain is empty!" << endl;
    return;
  }
  const char * chainID = mChain.front().chainID();
  if ( !chainID || !*chainID ) {
    // cert not found:
    kdDebug() << "CertificateInfoWidgetImpl::startCertificateChainListing(): empty chain ID - root not found" << endl;
    return;
  }
  const char * fpr = mChain.front().primaryFingerprint();
  if ( qstricmp( fpr, chainID ) == 0 ) {
    kdDebug() << "CertificateInfoWidgetImpl::startCertificateChainListing(): chain_id equals fingerprint -> found root" << endl;
    return;
  }
  if ( mChain.size() > 100 ) {
    // safe guard against certificate loops (paranoia factor 8 out of 10)...
    kdWarning() << "CertificateInfoWidgetImpl::startCertificateChainListing(): maximum chain length of 100 exceeded!" << endl;
    return;
  }
  if ( !mFoundIssuer ) {
    // key listing failed. Don't end up in endless loop
    kdDebug() << "CertificateInfoWidgetImpl::startCertificateChainListing(): issuer not found - giving up" << endl;
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

  kdDebug() << "Going to fetch" << endl
	    << "  issuer  : \"" << mChain.front().issuerName() << "\"" << endl
	    << "  chain id: " << mChain.front().chainID() << endl
	    << "for" << endl
	    << "  subject : \"" << mChain.front().userID(0).id() << "\"" << endl
	    << "  subj.fpr: " << mChain.front().primaryFingerprint() << endl;

  const GpgME::Error err = job->start( mChain.front().chainID() );

  if ( err )
    showChainListError( this, err, mChain.front().issuerName() );
  else
    (void)new Kleo::ProgressDialog( job, i18n("Fetching Certificate Chain"), this );
}

void CertificateInfoWidgetImpl::startCertificateDump() {
  KProcess* proc = new KProcess( this );
  (*proc) << "gpgsm"; // must be in the PATH
  (*proc) << "--dump-keys";
  (*proc) << mChain.front().primaryFingerprint();

  QObject::connect( proc, SIGNAL( receivedStdout(KProcess *, char *, int) ),
                    this, SLOT( slotCollectStdout(KProcess *, char *, int) ) );
  QObject::connect( proc, SIGNAL( receivedStderr(KProcess *, char *, int) ),
                    this, SLOT( slotCollectStderr(KProcess *, char *, int) ) );
  QObject::connect( proc, SIGNAL( processExited(KProcess*) ),
                    this, SLOT( slotDumpProcessExited(KProcess*) ) );

  if ( !proc->start( KProcess::NotifyOnExit, (KProcess::Communication)(KProcess::Stdout | KProcess::Stderr) ) ) {
    QString wmsg = i18n("Failed to execute gpgsm:\n%1").arg( i18n( "program not found" ) );
    dumpView->setText( wmsg );
  }
}

void CertificateInfoWidgetImpl::slotCollectStdout(KProcess *, char *buffer, int buflen)
{
  mDumpOutput += QCString(buffer, buflen+1); // like KProcIO does
}

void CertificateInfoWidgetImpl::slotCollectStderr(KProcess *, char *buffer, int buflen)
{
  mDumpError += QCString(buffer, buflen+1); // like KProcIO does
}

void CertificateInfoWidgetImpl::slotDumpProcessExited(KProcess* proc) {
  int rc = ( proc->normalExit() ) ? proc->exitStatus() : -1 ;

  if ( rc == 0 ) {
    dumpView->setText( QString::fromUtf8( mDumpOutput ) );
  } else {
    if ( !mDumpError.isEmpty() ) {
      dumpView->setText( QString::fromUtf8( mDumpError ) );
    } else
    {
      QString wmsg = i18n("Failed to execute gpgsm:\n%1");
      if ( rc == -1 )
        wmsg = wmsg.arg( i18n( "program cannot be executed" ) );
      else
        wmsg = wmsg.arg( strerror(rc) );
      dumpView->setText( wmsg );
    }
  }

  proc->deleteLater();
}

void CertificateInfoWidgetImpl::slotNextKey( const GpgME::Key & key ) {
  kdDebug() << "CertificateInfoWidgetImpl::slotNextKey( \""
	    << key.userID(0).id() << "\" )" << endl;
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
  QListViewItem * item = 0;

  QValueList<GpgME::Key>::const_iterator it = mChain.begin();
  // root item:
  if ( (*it).chainID() && qstrcmp( (*it).chainID(), (*it).primaryFingerprint() ) == 0 )
    item = new QListViewItem( pathView, Kleo::DN( (*it++).userID(0).id() ).prettyDN() );
  else {
    item = new QListViewItem( pathView, i18n("Issuer certificate not found ( %1)")
			      .arg( Kleo::DN( (*it).issuerName() ).prettyDN() ) );
    item->setOpen( true ); // Qt bug: doesn't open after setEnabled( false ) :/
    item->setEnabled( false );
  }
  item->setOpen( true );

  // subsequent items:
  while ( it != mChain.end() ) {
    item = new QListViewItem( item, Kleo::DN( (*it++).userID(0).id() ).prettyDN() );
    item->setOpen( true );
  }
}

void CertificateInfoWidgetImpl::slotCertificateChainListingResult( const GpgME::KeyListResult & res ) {
  if ( res.error() )
    return showChainListError( this, res.error(), mChain.front().issuerName() );
  else
    startCertificateChainListing();
}

void CertificateInfoWidgetImpl::slotShowInfo( QListViewItem * item ) {
  textView->setText( item->text(1) );
}

void CertificateInfoWidgetImpl::slotShowCertPathDetails( QListViewItem * item ) {
  if ( !item )
    return;

  // find the key corresponding to "item". This hack would not be
  // necessary if pathView was a Kleo::KeyListView, but it's
  // Qt-Designer-generated and I don't feel like creating a custom
  // widget spec for Kleo::KeyListView.
  unsigned int totalCount = 0;
  int itemIndex = -1;
  for ( const QListViewItem * i = pathView->firstChild() ; i ; i = i->firstChild() ) {
    if ( i == item )
      itemIndex = totalCount;
    ++totalCount;
  }

  assert( totalCount == mChain.size() || totalCount == mChain.size() + 1 );

  // skip pseudo root item with "not found message":
  if ( totalCount == mChain.size() + 1 )
    --itemIndex;

  assert( itemIndex >= 0 );

  KDialogBase * dialog =
    new KDialogBase( this, "dialog", false, i18n("Additional Information for Key"),
		     KDialogBase::Close, KDialogBase::Close );
  CertificateInfoWidgetImpl * top =
    new CertificateInfoWidgetImpl( mChain[itemIndex], mExternal, dialog );
  dialog->setMainWidget( top );
  // proxy the signal to our receiver:
  connect( top, SIGNAL(requestCertificateDownload(const QString&, const QString&)),
	   SIGNAL(requestCertificateDownload(const QString&, const QString&)) );
  dialog->show();
}


void CertificateInfoWidgetImpl::slotImportCertificate()
{
  if ( mChain.empty() || mChain.back().isNull() )
    return;
  const Kleo::DN dn = mChain.back().userID( 0 ).id();
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
  job->start( fingerprint );
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
