/*
    certmanager.cpp

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

#include "certmanager.h"

#include "certlistview.h"
#include "certificatewizardimpl.h"
#include "certificateinfowidgetimpl.h"
#include "crlview.h"
#include "customactions.h"
#include "storedtransferjob.h"
#include "conf/configuredialog.h"

// libkleopatra
#include <cryptplugwrapper.h>
#include <cryptplugfactory.h>
#include <kleo/downloadjob.h>
#include <kleo/importjob.h>
#include <kleo/exportjob.h>
#include <kleo/multideletejob.h>
#include <kleo/deletejob.h>
#include <kleo/keylistjob.h>
#include <kleo/dn.h>
#include <kleo/keyfilter.h>
#include <kleo/keyfiltermanager.h>

#include <ui/progressdialog.h>
#include <ui/progressbar.h>
#include <ui/keyselectiondialog.h>

// GPGME++
#include <gpgmepp/importresult.h>
#include <gpgmepp/keylistresult.h>
#include <gpgmepp/key.h>

// KDE
#include <kfiledialog.h>
#include <kprocess.h>
#include <kaction.h>
#include <kapplication.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <dcopclient.h>
#include <ktoolbar.h>
#include <kstatusbar.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kkeydialog.h>
#include <ktempfile.h>
#include <kio/job.h>
#include <kio/netaccess.h>

// Qt
#include <qfontmetrics.h>
#include <qpopupmenu.h>

// other
#include <assert.h>

namespace {

  class DisplayStrategy : public Kleo::KeyListView::DisplayStrategy{
  public:
    ~DisplayStrategy() {}

    virtual QFont keyFont( const GpgME::Key& key, const QFont& font ) const {
      const Kleo::KeyFilter* filter = Kleo::KeyFilterManager::instance()->filterMatching( key );
      return filter ? filter->font( font ) : font;
    }
    virtual QColor keyForeground( const GpgME::Key& key, const QColor& c ) const {
      const Kleo::KeyFilter* filter = Kleo::KeyFilterManager::instance()->filterMatching( key );
      if ( filter && filter->fgColor().isValid() )
        return filter->fgColor();
      return c;
    }
    virtual QColor keyBackground( const GpgME::Key& key, const QColor& c  ) const {
      const Kleo::KeyFilter* filter = Kleo::KeyFilterManager::instance()->filterMatching( key );
      if ( filter && filter->bgColor().isValid() )
        return filter->bgColor();
      return c;
    }
  };

  class ColumnStrategy : public Kleo::KeyListView::ColumnStrategy {
  public:
    ~ColumnStrategy() {}

    QString title( int col ) const;
    QString text( const GpgME::Key & key, int col ) const;
    int width( int col, const QFontMetrics & fm ) const;
  };

  QString ColumnStrategy::title( int col ) const {
    switch ( col ) {
    case 0: return i18n("Subject");
    case 1: return i18n("Issuer");
    case 2: return i18n("Serial");
    default: return QString::null;
    }
  }

  QString ColumnStrategy::text( const GpgME::Key & key, int col ) const {
    switch ( col ) {
    case 0: return Kleo::DN( key.userID(0).id() ).prettyDN();
    case 1: return Kleo::DN( key.issuerName() ).prettyDN();
    case 2: return key.issuerSerial() ? QString::fromUtf8( key.issuerSerial() ) : QString::null ;
    default: return QString::null;
    }
  }

  int ColumnStrategy::width( int col, const QFontMetrics & fm ) const {
    int factor = -1;
    switch ( col ) {
    case 0: factor = 6; break;
    case 1: factor = 4; break;
    default: return -1;
    }
    return fm.width( title( col ) ) * factor;
  }
} // anon namespace

CertManager::CertManager( bool remote, const QString& query, const QString & import,
			  QWidget* parent, const char* name )
  : KMainWindow( parent, name ),
    mCrlView( 0 ),
    mDirmngrProc( 0 ),
    mLineEditAction( 0 ),
    mComboAction( 0 ),
    mFindAction( 0 ),
    mImportCertFromFileAction( 0 ),
    mImportCRLFromFileAction( 0 ),
    mNextFindRemote( false ),
    mRemote( remote ),
    mDirMngrFound( false )
{
  createStatusBar();
  createActions();

  createGUI();
  setAutoSaveSettings();

  // Main Window --------------------------------------------------
  mKeyListView = new CertKeyListView( new ColumnStrategy(), new DisplayStrategy(), this, "mKeyListView" );
  mKeyListView->setSelectionMode( QListView::Extended );
  setCentralWidget( mKeyListView );

  connect( mKeyListView, SIGNAL(doubleClicked(Kleo::KeyListViewItem*,const QPoint&,int)),
	   SLOT(slotViewDetails(Kleo::KeyListViewItem*)) );
  connect( mKeyListView, SIGNAL(returnPressed(Kleo::KeyListViewItem*)),
	   SLOT(slotViewDetails(Kleo::KeyListViewItem*)) );
  connect( mKeyListView, SIGNAL(selectionChanged()),
	   SLOT(slotSelectionChanged()) );
  connect( mKeyListView, SIGNAL(contextMenu(Kleo::KeyListViewItem*, const QPoint&)),
           SLOT(slotContextMenu(Kleo::KeyListViewItem*, const QPoint&)) );

  connect( mKeyListView, SIGNAL(dropped(const KURL::List&) ),
           SLOT( slotDropped(const KURL::List&) ) );

  mLineEditAction->setText(query);
  if ( !mRemote || !query.isEmpty() )
    slotStartCertificateListing();

  if ( !import.isEmpty() )
    slotImportCertFromFile( KURL( import ) );

  updateStatusBarLabels();
  slotSelectionChanged(); // initial state for selection-dependent actions
}

void CertManager::createStatusBar() {
  KStatusBar * bar = statusBar();
  mProgressBar = new Kleo::ProgressBar( bar, "mProgressBar" );
  mProgressBar->reset();
  mProgressBar->setFixedSize( QSize( 100, mProgressBar->height() * 3 / 5 ) );
  bar->addWidget( mProgressBar, 0, true );
  mStatusLabel = new QLabel( bar, "mStatusLabel" );
  bar->addWidget( mStatusLabel, 1, false );
}

static inline void connectEnableOperationSignal( QObject * s, QObject * d ) {
  QObject::connect( s, SIGNAL(enableOperations(bool)),
		    d, SLOT(setEnabled(bool)) );
}


void CertManager::createActions() {
  KAction * action = 0;

  (void)KStdAction::quit( this, SLOT(close()), actionCollection() );

  action = KStdAction::redisplay( this, SLOT(slotStartCertificateListing()),
				  actionCollection() );
  connectEnableOperationSignal( this, action );

  action = new KAction( i18n("Stop Operation"), "stop", 0,
			this, SIGNAL(stopOperations()),
			actionCollection(), "view_stop_operations" );
  action->setEnabled( false );

  (void)   new KAction( i18n("New Key Pair..."), "filenew", 0,
			this, SLOT(newCertificate()),
			actionCollection(), "file_new_certificate" );

#ifdef NOT_IMPLEMENTED_ANYWAY
  mRevokeCertificateAction = new KAction( i18n("Revoke Certificate"), 0,
                                          this, SLOT(revokeCertificate()),
                                          actionCollection(), "edit_revoke_certificate" );
  connectEnableOperationSignal( this, mRevokeCertificateAction );

  mExtendCertificateAction = new KAction( i18n("Extend Certificate"), 0,
                                          this, SLOT(extendCertificate()),
                                          actionCollection(), "edit_extend_certificate" );
  connectEnableOperationSignal( this, mExtendCertificateAction );
#endif

  mDeleteCertificateAction = new KAction( i18n("Delete Certificate"), "editdelete", Key_Delete,
                                    this, SLOT(slotDeleteCertificate()),
                                    actionCollection(), "edit_delete_certificate" );
  connectEnableOperationSignal( this, mDeleteCertificateAction );

  mImportCertFromFileAction = new KAction( i18n("Import Certificates..."), 0,
					   this, SLOT(slotImportCertFromFile()),
					   actionCollection(), "file_import_certificates" );
  connectEnableOperationSignal( this, mImportCertFromFileAction );

  mImportCRLFromFileAction = new KAction( i18n("Import CRLs..."), 0,
					  this, SLOT(importCRLFromFile()),
					  actionCollection(), "file_import_crls" );
  connectEnableOperationSignal( this, mImportCRLFromFileAction );

  mExportCertificateAction = new KAction( i18n("Export Certificates..."), "export", 0,
					  this, SLOT(slotExportCertificate()),
					  actionCollection(), "file_export_certificate" );

  mExportSecretKeyAction = new KAction( i18n("Export Secret Key..."), "export", 0,
                                        this, SLOT(slotExportSecretKey()),
                                        actionCollection(), "file_export_secret_keys" );
  connectEnableOperationSignal( this, mExportSecretKeyAction );

  mViewCertDetailsAction = new KAction( i18n("View Certificate Details..."), 0, 0,
                                        this, SLOT(slotViewDetails()), actionCollection(),
                                        "view_certificate_details" );
  mDownloadCertificateAction = new KAction( i18n( "Download Certificate"), 0, 0,
                                        this, SLOT(slotDownloadCertificate()), actionCollection(),
                                        "download_certificate" );

  const QString dirmngr = KStandardDirs::findExe( "gpgsm" );
  mDirMngrFound = !dirmngr.isEmpty();

  action = new KAction( i18n("Dump CRL Cache..."), 0,
			this, SLOT(slotViewCRLs()),
			actionCollection(), "view_dump_crls" );
  action->setEnabled( mDirMngrFound ); // we also need dirmngr for this

  // Toolbar
  KToolBar * _toolbar = toolBar( "searchToolBar" );

  (new LabelAction( i18n("Search:"), actionCollection(), "label_action"))->plug( _toolbar );
  mLineEditAction = new LineEditAction( QString::null, actionCollection(), this,
					SLOT(slotStartCertificateListing()),
					"query_lineedit_action");
  mLineEditAction->plug( _toolbar );

  QStringList lst;
  lst << i18n("in local certificates") << i18n("in external certificates");
  mComboAction = new ComboAction( lst, actionCollection(), this, SLOT( slotToggleRemote(int) ),
                                  "location_combo_action");
  mComboAction->plug( _toolbar );

  mFindAction = new KAction( i18n("Find"), "find", 0, this, SLOT(slotStartCertificateListing()),
			     actionCollection(), "find" );
  mFindAction->plug( _toolbar );

  KStdAction::keyBindings( this, SLOT(slotEditKeybindings()), actionCollection() );
  KStdAction::preferences( this, SLOT(slotShowConfigurationDialog()), actionCollection() );

  createStandardStatusBarAction();
  updateImportActions( true );
}

void CertManager::updateImportActions( bool enable ) {
  mImportCRLFromFileAction->setEnabled( mDirMngrFound && enable );
  mImportCertFromFileAction->setEnabled( enable );
}

void CertManager::slotEditKeybindings() {
  KKeyDialog::configure( actionCollection(), true );
}

void CertManager::slotShowConfigurationDialog() {
  ConfigureDialog dlg( this );
  dlg.exec();
}

void CertManager::slotToggleRemote( int idx ) {
  mNextFindRemote = idx != 0;
}


void CertManager::connectJobToStatusBarProgress( Kleo::Job * job, const QString & initialText ) {
  assert( mProgressBar );
  if ( !job )
    return;
  if ( !initialText.isEmpty() )
    statusBar()->message( initialText );
  connect( job, SIGNAL(progress(const QString&,int,int,int)),
	   mProgressBar, SLOT(slotProgress(const QString&,int,int,int)) );
  connect( job, SIGNAL(done()), mProgressBar, SLOT(reset()) );
  connect( this, SIGNAL(stopOperations()), job, SLOT(slotCancel()) );

  action("view_stop_operations")->setEnabled( true );
  emit enableOperations( false );
}

void CertManager::disconnectJobFromStatusBarProgress( const GpgME::Error & err ) {
  updateStatusBarLabels();
  const QString msg = err.isCanceled() ? i18n("Canceled.")
    : err ? i18n("Failed.")
    : i18n("Done.") ;
  statusBar()->message( msg, 4000 );

  action("view_stop_operations")->setEnabled( false );
  emit enableOperations( true );
  slotSelectionChanged();
}

void CertManager::updateStatusBarLabels() {
  mKeyListView->flushKeys();
  mStatusLabel->setText( i18n( "%n Key.",
			       "%n Keys.", mKeyListView->childCount() ) );
}

//
//
// Key Listing:
//
//


static void showKeyListError( QWidget * parent, const GpgME::Error & err ) {
  assert( err );
  const QString msg = i18n( "<qt><p>An error occurred while fetching "
			    "the certificates from the backend:</p>"
			    "<p><b>%1</b></p></qt>" )
    .arg( QString::fromLocal8Bit( err.asString() ) );

  KMessageBox::error( parent, msg, i18n( "Certificate Listing Failed" ) );
}

void CertManager::slotStartCertificateListing()
{
  mRemote = mNextFindRemote;
  mLineEditAction->setEnabled( false );
  mComboAction->setEnabled( false );
  mFindAction->setEnabled( false );

  // Clear display
  mKeyListView->clear();

  const QString query = mLineEditAction->text();

  Kleo::KeyListJob * job =
    Kleo::CryptPlugFactory::instance()->smime()->keyListJob( mRemote );
  assert( job );

  connect( job, SIGNAL(nextKey(const GpgME::Key&)),
	   mKeyListView, SLOT(slotAddKey(const GpgME::Key&)) );
  connect( job, SIGNAL(result(const GpgME::KeyListResult&)),
	   this, SLOT(slotKeyListResult(const GpgME::KeyListResult&)) );

  connectJobToStatusBarProgress( job, i18n("Fetching keys...") );

  const GpgME::Error err = job->start( query );
  if ( err ) {
    showKeyListError( this, err );
    return;
  }
  mProgressBar->setProgress( 0, 0 ); // enable busy indicator
}

void CertManager::slotKeyListResult( const GpgME::KeyListResult & res ) {
  if ( res.error() )
    showKeyListError( this, res.error() );
  else if ( res.isTruncated() )
    KMessageBox::information( this,
			      i18n("The server returned truncated output;\n"
				   "please use a more-specific search string "
				   "to get all results.") );

  mLineEditAction->setEnabled( true );
  mComboAction->setEnabled( true );
  mFindAction->setEnabled( true );

  mLineEditAction->focusAll();
  disconnectJobFromStatusBarProgress( res.error() );
}

void CertManager::slotContextMenu(Kleo::KeyListViewItem* item, const QPoint& point) {
  if ( !item )
    return;
  QPopupMenu *popup = static_cast<QPopupMenu*>(factory()->container("listview_popup",this));
  if ( popup ) {
    popup->exec( point );
  }
}

/**
  This slot is invoked when the user selects "New certificate"
*/
void CertManager::newCertificate()
{
  CertificateWizardImpl wizard( this );
  wizard.exec();
}

/**
   This slot is invoked when the user selects revoke certificate.
   The slot will revoke the selected certificates
*/
void CertManager::revokeCertificate()
{
  qDebug("Not Yet Implemented");
}

/**
   This slot is invoked when the user selects extend certificate.
   It will send an extension request for the selected certificates
*/
void CertManager::extendCertificate()
{
  qDebug("Not Yet Implemented");
}


//
//
// Downloading / Importing Certificates
//
//


/**
   This slot is invoked when the user selects Certificates/Import/From File.
*/
void CertManager::slotImportCertFromFile()
{
  const QString filter = QString("*.pem *.der *.p7c *.p12|") + i18n("Certificates (*.pem *.der *.p7c *.p12)");
  slotImportCertFromFile( KFileDialog::getOpenURL( QString::null, filter, this,
                                                   i18n( "Select Certificate File" ) ) );
}

void CertManager::slotImportCertFromFile( const KURL & certURL )
{
  if ( !certURL.isValid() ) // empty or malformed
    return;

  // Prevent two simultaneous imports
  updateImportActions( false );

  // Download the cert
  KIOext::StoredTransferJob* importJob = KIOext::storedGet( certURL );
  importJob->setWindow( this );
  connect( importJob, SIGNAL(result(KIO::Job*)), SLOT(slotImportResult(KIO::Job*)) );
}

void CertManager::slotImportResult( KIO::Job* job )
{
  if ( job->error() ) {
    job->showErrorDialog();
  } else {
    KIOext::StoredTransferJob* trJob = static_cast<KIOext::StoredTransferJob *>( job );
    startCertificateImport( trJob->data(), trJob->url().fileName() );
  }

  updateImportActions( true );
}

static void showCertificateDownloadError( QWidget * parent, const GpgME::Error & err, const QString& certDisplayName ) {
  assert( err );
  const QString msg = i18n( "<qt><p>An error occurred while trying "
			    "to download the certificate %1:</p>"
			    "<p><b>%2</b></p></qt>" )
                      .arg( certDisplayName )
                      .arg( QString::fromLocal8Bit( err.asString() ) );

  KMessageBox::error( parent, msg, i18n( "Certificate Download Failed" ) );
}

void CertManager::slotDownloadCertificate() {
  QPtrList<Kleo::KeyListViewItem> items = mKeyListView->selectedItems();
  for ( QPtrListIterator<Kleo::KeyListViewItem> it( items ) ; it.current() ; ++it )
    if ( !it.current()->key().isNull() )
      if ( const char * fpr = it.current()->key().subkey(0).fingerprint() )
        slotStartCertificateDownload( fpr, it.current()->text(0) );
}

// Called from slotDownloadCertificate and from the certificate-details widget
void CertManager::slotStartCertificateDownload( const QString& fingerprint, const QString& displayName ) {
  if ( fingerprint.isEmpty() )
    return;

  Kleo::DownloadJob * job =
    Kleo::CryptPlugFactory::instance()->smime()->downloadJob( false /* no armor */ );
  assert( job );

  connect( job, SIGNAL(result(const GpgME::Error&,const QByteArray&)),
	   SLOT(slotCertificateDownloadResult(const GpgME::Error&,const QByteArray&)) );

  connectJobToStatusBarProgress( job, i18n("Fetching certificate from server...") );

  const GpgME::Error err = job->start( fingerprint );
  if ( err )
    showCertificateDownloadError( this, err, displayName );
  else {
    mProgressBar->setProgress( 0, 0 );
    mJobsDisplayNameMap.insert( job, displayName );
  }
}

QString CertManager::displayNameForJob( const Kleo::Job *job )
{
  JobsDisplayNameMap::iterator it = mJobsDisplayNameMap.find( job );
  QString displayName;
  if ( it != mJobsDisplayNameMap.end() ) {
    displayName = *it;
    mJobsDisplayNameMap.remove( it );
  } else {
    kdWarning() << "Job not found in map: " << job << endl;
  }
  return displayName;
}

// Don't call directly!
void CertManager::slotCertificateDownloadResult( const GpgME::Error & err, const QByteArray & keyData ) {

  QString displayName = displayNameForJob( static_cast<const Kleo::Job *>( sender() ) );

  if ( err )
    showCertificateDownloadError( this, err, displayName );
  else
    startCertificateImport( keyData, displayName );
  disconnectJobFromStatusBarProgress( err );
}

static void showCertificateImportError( QWidget * parent, const GpgME::Error & err, const QString& certDisplayName ) {
  assert( err );
  const QString msg = i18n( "<qt><p>An error occurred while trying "
			    "to import the certificate %1:</p>"
			    "<p><b>%2</b></p></qt>" )
                      .arg( certDisplayName )
                      .arg( QString::fromLocal8Bit( err.asString() ) );
  KMessageBox::error( parent, msg, i18n( "Certificate Import Failed" ) );
}

void CertManager::startCertificateImport( const QByteArray & keyData, const QString& certDisplayName ) {
  Kleo::ImportJob * job = Kleo::CryptPlugFactory::instance()->smime()->importJob();
  assert( job );

  connect( job, SIGNAL(result(const GpgME::ImportResult&)),
	   SLOT(slotCertificateImportResult(const GpgME::ImportResult&)) );

  connectJobToStatusBarProgress( job, i18n("Importing certificates...") );

  kdDebug() << "Importing certificate. keyData size:" << keyData.size() << endl;
  const GpgME::Error err = job->start( keyData );
  if ( err )
    showCertificateImportError( this, err, certDisplayName );
  else {
    mProgressBar->setProgress( 0, 0 );
    mJobsDisplayNameMap.insert( job, certDisplayName );
  }
}

void CertManager::slotCertificateImportResult( const GpgME::ImportResult & res ) {
  QString displayName = displayNameForJob( static_cast<const Kleo::Job *>( sender() ) );

  if ( res.error() ) {
    showCertificateImportError( this, res.error(), displayName );
  } else {

  KMessageBox::information( this,
    QString( "<qt><p>%1</p>"
    "<p>%1</p>"
    "<table>"
    "<tr><td align=right>%1</td><td>%1</td></tr>"
    "<tr><td align=right>%1</td><td>%1 (%1 %1)</td></tr>"
    "<tr><td align=right>%1</td><td>%1</td></tr>"
    "<tr><td align=right>%1</td><td>%1</td></tr>"
    "<tr><td align=right>%1</td><td>%1</td></tr>"
    "<tr><td align=right>%1</td><td>%1</td></tr>"
    "<tr><td align=right>%1</td><td>%1</td></tr>"
    "<tr><td align=right>%1</td><td>%1</td></tr>"
    "<tr><td align=right>%1</td><td>%1 <br>&nbsp;</td></tr>"
    "<tr><td align=right>%1</td><td>%1</td></tr>"
    "<tr><td align=right>%1</td><td>%1</td></tr>"
    "<tr><td align=right>%1</td><td>%1</td></tr>"
    "</table></qt>" )
    .arg(i18n("Certificate %1 imported successfully.").arg( displayName ))
    .arg(i18n("Additional info:"))
    .arg(i18n("Total number processed:")).arg(res.numConsidered())
    .arg(i18n("imported:")).arg(res.numImported()).arg(i18n("RSA:")).arg(res.numRSAImported())
    .arg(i18n("new signatures:")).arg(res.newSignatures())
    .arg(i18n("new user IDs:")).arg(res.newUserIDs())
    .arg(i18n("keys without user ID:")).arg(res.numKeysWithoutUserID())
    .arg(i18n("new subkeys:")).arg(res.newSubkeys())
    .arg(i18n("new revocations:")).arg(res.newRevocations())
    .arg(i18n("not imported:")).arg(res.notImported())
    .arg(i18n("unchanged:")).arg(res.numUnchanged())
    .arg(i18n("number of secret keys:")).arg(res.numSecretKeysConsidered())
    .arg(i18n("secret keys imported:")).arg(res.numSecretKeysImported())
    .arg(i18n("secret keys unchanged:")).arg(res.numSecretKeysUnchanged())
    ,
    i18n( "Certificate Imported" ) );
  if ( !isRemote() )
    slotStartCertificateListing();
  else
    disconnectJobFromStatusBarProgress( res.error() );
  }
  if ( !mURLsToImport.isEmpty() )
    importNextURL();
}



/**
   This slot is called when the dirmngr process that imports a
   certificate file exists.
*/
void CertManager::slotDirmngrExited() {
    if ( !mDirmngrProc->normalExit() )
        KMessageBox::error( this, i18n( "The GpgSM process that tried to import the CRL file ended prematurely because of an unexpected error." ), i18n( "Certificate Manager Error" ) );
    else if ( mDirmngrProc->exitStatus() )
      KMessageBox::error( this, i18n( "An error occurred when trying to import the CRL file. The output from GpgSM was: ") + mErrorbuffer, i18n( "Certificate Manager Error" ) );
    else
      KMessageBox::information( this, i18n( "CRL file imported successfully." ), i18n( "Certificate Manager Error" ) );

    delete mDirmngrProc; mDirmngrProc = 0;
    if ( !mImportCRLTempFile.isEmpty() )
      QFile::remove( mImportCRLTempFile );
    updateImportActions( true );
}

/**
   This slot will import CRLs from a file.
*/
void CertManager::importCRLFromFile() {
  QString filter = QString("*.crl *.arl *-crl.der *-arl.der|") + i18n("Certificate Revocation List (*.crl *.arl *-crl.der *-arl.der)");
  KURL url = KFileDialog::getOpenURL( QString::null,
                                      filter,
                                      this,
                                      i18n( "Select CRL File" ) );
  if ( url.isValid() ) {
    updateImportActions( false );
    if ( url.isLocalFile() ) {
      startImportCRL( url.path(), false );
      updateImportActions( true );
    } else {
      KTempFile tempFile;
      KURL destURL;
      destURL.setPath( tempFile.name() );
      KIO::Job* copyJob = KIO::file_copy( url, destURL, 0600, true, false );
      copyJob->setWindow( this );
      connect( copyJob, SIGNAL( result( KIO::Job * ) ),
               SLOT( slotImportCRLJobFinished( KIO::Job * ) ) );
    }
  }
}

void CertManager::slotImportCRLJobFinished( KIO::Job *job )
{
  KIO::FileCopyJob* fcjob = static_cast<KIO::FileCopyJob*>( job );
  QString tempFilePath = fcjob->destURL().path();
  if ( job->error() ) {
    job->showErrorDialog();
    QFile::remove( tempFilePath ); // unlink tempfile
    updateImportActions( true );
    return;
  }
  startImportCRL( tempFilePath, true );
}

void CertManager::startImportCRL( const QString& filename, bool isTempFile )
{
  mImportCRLTempFile = isTempFile ? filename : QString::null;
  mDirmngrProc = new KProcess();
  *mDirmngrProc << "gpgsm" << "--call-dirmngr" << "loadcrl" << filename;
  mErrorbuffer = QString::null;
  connect( mDirmngrProc, SIGNAL(processExited(KProcess*)),
           this, SLOT(slotDirmngrExited()) );
  connect( mDirmngrProc, SIGNAL(receivedStderr(KProcess*,char*,int) ),
           this, SLOT(slotStderr(KProcess*,char*,int)) );
  if( !mDirmngrProc->start( KProcess::NotifyOnExit, KProcess::Stderr ) ) {
    KMessageBox::error( this, i18n( "Unable to start gpgsm process. Please check your installation." ), i18n( "Certificate Manager Error" ) );
    delete mDirmngrProc; mDirmngrProc = 0;
    updateImportActions( true );
    if ( isTempFile )
      QFile::remove( mImportCRLTempFile ); // unlink tempfile
  }
}

void CertManager::slotStderr( KProcess*, char* buf, int len ) {
  mErrorbuffer += QString::fromLocal8Bit( buf, len );
}

/**
   This slot will import CRLs from an LDAP server.
*/
void CertManager::importCRLFromLDAP()
{
  qDebug("Not Yet Implemented");
}

void CertManager::slotViewCRLs() {
  if ( !mCrlView )
    mCrlView = new CRLView( this );

  mCrlView->show();
  mCrlView->slotUpdateView();
}


static void showDeleteError( QWidget * parent, const GpgME::Error & err ) {
  assert( err );
  const QString msg = i18n("<qt><p>An error occurred while trying to delete "
			   "the certificates:</p>"
			   "<p><b>%1</b></p></qt>")
    .arg( QString::fromLocal8Bit( err.asString() ) );
  KMessageBox::error( parent, msg, i18n("Certificate Deletion Failed") );
}

void CertManager::slotDeleteCertificate() {
  mItemsToDelete = mKeyListView->selectedItems();
  if ( mItemsToDelete.isEmpty() )
    return;
  std::vector<GpgME::Key> keys;
  keys.reserve( mItemsToDelete.count() );
  QStringList keyDisplayNames;
  for ( QPtrListIterator<Kleo::KeyListViewItem> it( mItemsToDelete ) ; it.current() ; ++it )
    if ( !it.current()->key().isNull() ) {
      keys.push_back( it.current()->key() );
      keyDisplayNames.push_back( it.current()->text( 0 ) );
    }
  if ( keys.empty() )
    return;

  if ( KMessageBox::warningContinueCancelList(
         this,
         i18n( "Do you really want to delete this certificate?", "Do you really want to delete these %n certificates?", keyDisplayNames.count()),
         keyDisplayNames,
         i18n( "Delete Certificates" ),
         KGuiItem( i18n( "Delete" ), "editdelete" ),
         "ConfirmDeleteCert", KMessageBox::Dangerous ) != KMessageBox::Continue )
    return;

  if ( Kleo::DeleteJob * job = Kleo::CryptPlugFactory::instance()->smime()->deleteJob() )
    job->slotCancel();
  else {
    QString str = keys.size() == 1
                  ? i18n("<qt><p>An error occurred while trying to delete "
                         "the certificate:</p>"
                         "<p><b>%1</b><p></qt>" )
                  : i18n( "<qt><p>An error occurred while trying to delete "
                          "the certificates:</p>"
                          "<p><b>%1</b><p></qt>" );
    KMessageBox::error( this,
			str.arg( i18n("Operation not supported by the backend.") ),
			i18n("Certificate Deletion Failed") );
  }
  Kleo::MultiDeleteJob * job = new Kleo::MultiDeleteJob( Kleo::CryptPlugFactory::instance()->smime() );
  assert( job );

  connect( job, SIGNAL(result(const GpgME::Error&,const GpgME::Key&)),
	   SLOT(slotDeleteResult(const GpgME::Error&,const GpgME::Key&)) );

  connectJobToStatusBarProgress( job, i18n("Deleting keys...") );

  const GpgME::Error err = job->start( keys, true );
  if ( err )
    showDeleteError( this, err );
  else
    mProgressBar->setProgress( 0, 0 );
}

void CertManager::slotDeleteResult( const GpgME::Error & err, const GpgME::Key & ) {
  if ( err )
    showDeleteError( this, err );
  else {
    mItemsToDelete.setAutoDelete( true );
    mItemsToDelete.clear();
    mItemsToDelete.setAutoDelete( false );
  }
  disconnectJobFromStatusBarProgress( err );
}

void CertManager::slotViewDetails( Kleo::KeyListViewItem * item ) {
  if ( !item || item->key().isNull() )
    return;

  // <UGH>
  KDialogBase * dialog = new KDialogBase( this, "dialog", false, i18n("Additional Information for Key"), KDialogBase::Close, KDialogBase::Close );

  CertificateInfoWidgetImpl * top = new CertificateInfoWidgetImpl( item->key(), isRemote(), dialog );
  dialog->setMainWidget( top );
  // </UGH>
  connect( top, SIGNAL(requestCertificateDownload(const QString&)),
	   SLOT(slotStartCertificateDownload(const QString&)) );
  dialog->show();
}

void CertManager::slotViewDetails()
{
  QPtrList<Kleo::KeyListViewItem> items = mKeyListView->selectedItems();
  if ( items.isEmpty() )
    return;

  // selectedItem() doesn't work in Extended mode.
  // But we only want to show the details of one item...
  slotViewDetails( items.first() );
}

void CertManager::slotSelectionChanged()
{
  mKeyListView->flushKeys();
  bool b = mKeyListView->hasSelection();
  mExportCertificateAction->setEnabled( b );
  mViewCertDetailsAction->setEnabled( b );
  mDeleteCertificateAction->setEnabled( b );
#ifdef NOT_IMPLEMENTED_ANYWAY
  mRevokeCertificateAction->setEnabled( b );
  mExtendCertificateAction->setEnabled( b );
#endif
  mDownloadCertificateAction->setEnabled( b && mRemote );
}

void CertManager::slotExportCertificate() {
  QPtrList<Kleo::KeyListViewItem> items = mKeyListView->selectedItems();
  if ( items.isEmpty() )
    return;

  QStringList fingerprints;
  for ( QPtrListIterator<Kleo::KeyListViewItem> it( items ) ; it.current() ; ++it )
    if ( !it.current()->key().isNull() )
      if ( const char * fpr = it.current()->key().subkey(0).fingerprint() )
	fingerprints.push_back( fpr );

  startCertificateExport( fingerprints );
}

static void showCertificateExportError( QWidget * parent, const GpgME::Error & err ) {
  assert( err );
  const QString msg = i18n("<qt><p>An error occurred while trying to export "
			   "the certificate:</p>"
			   "<p><b>%1</b></p></qt>")
    .arg( QString::fromLocal8Bit( err.asString() ) );
  KMessageBox::error( parent, msg, i18n("Certificate Export Failed") );
}

void CertManager::startCertificateExport( const QStringList & fingerprints ) {
  if ( fingerprints.empty() )
    return;

  // we need to use PEM (ascii armoured) format, since DER (binary)
  // can't transport more than one certificate *sigh* this is madness :/
  Kleo::ExportJob * job = Kleo::CryptPlugFactory::instance()->smime()->publicKeyExportJob( true );
  assert( job );

  connect( job, SIGNAL(result(const GpgME::Error&,const QByteArray&)),
	   SLOT(slotCertificateExportResult(const GpgME::Error&,const QByteArray&)) );

  connectJobToStatusBarProgress( job, i18n("Exporting certificate...") );

  const GpgME::Error err = job->start( fingerprints );
  if ( err )
    showCertificateExportError( this, err );
  else
    mProgressBar->setProgress( 0, 0 );
}

// return true if we should proceed, false if we should abort
static bool checkOverwrite( const KURL& url, bool& overwrite, QWidget* w )
{
  if ( KIO::NetAccess::exists( url, false /*dest*/, w ) ) {
    if ( KMessageBox::Cancel ==
         KMessageBox::warningContinueCancel(
                                            w,
                                            i18n( "A file named \"%1\" already exists. "
                                                  "Are you sure you want to overwrite it?" ).arg( url.prettyURL() ),
                                            i18n( "Overwrite File?" ),
                                            i18n( "&Overwrite" ) ) )
      return false;
    overwrite = true;
  }
  return true;
}

void CertManager::slotCertificateExportResult( const GpgME::Error & err, const QByteArray & data ) {
  disconnectJobFromStatusBarProgress( err );
  if ( err ) {
    showCertificateExportError( this, err );
    return;
  }

  kdDebug() << "CertManager::slotCertificateExportResult(): got " << data.size() << " bytes" << endl;

  const QString filter = QString("*.pem|") + i18n("ASCII Armored Certificate Bundles (*.pem)");
  const KURL url = KFileDialog::getOpenURL( QString::null,
                                      filter,
                                      this,
                                      i18n( "Save Certificate" ) );
  if ( !url.isValid() )
    return;

  bool overwrite = false;
  if ( !checkOverwrite( url, overwrite, this ) )
    return;

  KIO::Job* uploadJob = KIOext::put( data, url, -1, overwrite, false /*resume*/ );
  uploadJob->setWindow( this );
  connect( uploadJob, SIGNAL( result( KIO::Job* ) ),
           this, SLOT( slotUploadResult( KIO::Job* ) ) );
}


void CertManager::slotExportSecretKey() {
  Kleo::KeySelectionDialog dlg( i18n("Secret Key Export"),
				i18n("Select the secret key to export "
				     "(<b>Warning: The PKCS#12 format is insecure; "
				     "exporting secret keys is discouraged</b>):"),
				std::vector<GpgME::Key>(),
				Kleo::KeySelectionDialog::SecretKeys|Kleo::KeySelectionDialog::SMIMEKeys,
				false /* no multiple selection */,
				false /* no remember choice box */,
				this, "secret key export key selection dialog" );
  //dlg.setHideInvalidKeys( false );

  if ( dlg.exec() != QDialog::Accepted )
    return;

  startSecretKeyExport( dlg.fingerprint() );
}

static void showSecretKeyExportError( QWidget * parent, const GpgME::Error & err ) {
  assert( err );
  const QString msg = i18n("<qt><p>An error occurred while trying to export "
			   "the secret key:</p>"
			   "<p><b>%1</b></p></qt>")
    .arg( QString::fromLocal8Bit( err.asString() ) );
  KMessageBox::error( parent, msg, i18n("Secret-Key Export Failed") );
}

void CertManager::startSecretKeyExport( const QString & fingerprint ) {
  if ( fingerprint.isEmpty() )
    return;

  // PENDING(marc): let user choose between binary and PEM format?
  Kleo::ExportJob * job = Kleo::CryptPlugFactory::instance()->smime()->secretKeyExportJob( false );
  assert( job );

  connect( job, SIGNAL(result(const GpgME::Error&,const QByteArray&)),
	   SLOT(slotSecretKeyExportResult(const GpgME::Error&,const QByteArray&)) );

  connectJobToStatusBarProgress( job, i18n("Exporting secret key...") );

  const GpgME::Error err = job->start( fingerprint );
  if ( err )
    showSecretKeyExportError( this, err );
  else
    mProgressBar->setProgress( 0, 0 );
}

void CertManager::slotSecretKeyExportResult( const GpgME::Error & err, const QByteArray & data ) {
  disconnectJobFromStatusBarProgress( err );
  if ( err ) {
    showSecretKeyExportError( this, err );
    return;
  }

  kdDebug() << "CertManager::slotSecretKeyExportResult(): got " << data.size() << " bytes" << endl;
  QString filter = QString("*.p12|") + i18n("PKCS#12 Key Bundle (*.p12)");
  KURL url = KFileDialog::getOpenURL( QString::null,
                                      filter,
                                      this,
                                      i18n( "Save Certificate" ) );
  if ( !url.isValid() )
    return;

  bool overwrite = false;
  if ( !checkOverwrite( url, overwrite, this ) )
    return;

  KIO::Job* uploadJob = KIOext::put( data, url, -1, overwrite, false /*resume*/ );
  uploadJob->setWindow( this );
  connect( uploadJob, SIGNAL( result( KIO::Job* ) ),
           this, SLOT( slotUploadResult( KIO::Job* ) ) );
}

void CertManager::slotUploadResult( KIO::Job* job )
{
  if ( job->error() )
    job->showErrorDialog();
}

void CertManager::slotDropped(const KURL::List& lst)
{
  mURLsToImport = lst;
  importNextURL();
}

void CertManager::importNextURL()
{
  if ( !mURLsToImport.isEmpty() ) {
    // We can only import them one by one, otherwise the jobs would run into each other
    KURL url = mURLsToImport.front();
    mURLsToImport.pop_front();
    slotImportCertFromFile( url );
  }
}

#include "certmanager.moc"
