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
#include "hierarchyanalyser.h"
#include "storedtransferjob.h"
#include "conf/configuredialog.h"

// libkleopatra
#include <kleo/cryptobackendfactory.h>
#include <kleo/downloadjob.h>
#include <kleo/importjob.h>
#include <kleo/exportjob.h>
#include <kleo/multideletejob.h>
#include <kleo/deletejob.h>
#include <kleo/keylistjob.h>
#include <kleo/dn.h>
#include <kleo/keyfilter.h>
#include <kleo/keyfiltermanager.h>
#include <kleo/hierarchicalkeylistjob.h>
#include <kleo/refreshkeysjob.h>

#include <ui/progressdialog.h>
#include <ui/progressbar.h>
#include <ui/keyselectiondialog.h>
#include <ui/cryptoconfigdialog.h>

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
#include <kstdaccel.h>

// Qt
#include <qfontmetrics.h>
#include <qpopupmenu.h>

// other
#include <algorithm>
#include <assert.h>

static const bool startWithHierarchicalKeyListing = false;

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
			  QWidget* parent, const char* name, WFlags f )
  : KMainWindow( parent, name, f|WDestructiveClose ),
    mCrlView( 0 ),
    mDirmngrProc( 0 ),
    mHierarchyAnalyser( 0 ),
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
  mKeyListView->setHierarchical( startWithHierarchicalKeyListing );
  mKeyListView->setRootIsDecorated( startWithHierarchicalKeyListing );
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
    slotSearch();

  if ( !import.isEmpty() )
    slotImportCertFromFile( KURL( import ) );

  updateStatusBarLabels();
  slotSelectionChanged(); // initial state for selection-dependent actions
}

CertManager::~CertManager() {
  delete mDirmngrProc; mDirmngrProc = 0;
  delete mHierarchyAnalyser; mHierarchyAnalyser = 0;
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

  action = KStdAction::redisplay( this, SLOT(slotRedisplay()), actionCollection() );
  // work around the fact that the stdaction has no shortcut
  KShortcut reloadShortcut = KStdAccel::shortcut(KStdAccel::Reload);
  reloadShortcut.append(KKey(CTRL + Key_R));
  action->setShortcut( reloadShortcut );

  connectEnableOperationSignal( this, action );

  action = new KAction( i18n("Stop Operation"), "stop", Key_Escape,
			this, SIGNAL(stopOperations()),
			actionCollection(), "view_stop_operations" );
  action->setEnabled( false );

  (void)   new KAction( i18n("New Key Pair..."), "filenew", 0,
			this, SLOT(newCertificate()),
			actionCollection(), "file_new_certificate" );

  connect( new KToggleAction( i18n("Hierarchical Key List"), 0,
			      actionCollection(), "view_hierarchical" ),
	   SIGNAL(toggled(bool)), SLOT(slotToggleHierarchicalView(bool)) );

  action = new KAction( i18n("Expand All"), 0, CTRL+Key_Period,
			this, SLOT(slotExpandAll()),
			actionCollection(), "view_expandall" );
  action->setEnabled( startWithHierarchicalKeyListing );
  action = new KAction( i18n("Collapse All"), 0, CTRL+Key_Comma,
			this, SLOT(slotCollapseAll()),
			actionCollection(), "view_collapseall" );
  action->setEnabled( startWithHierarchicalKeyListing );

  (void)   new KAction( i18n("Refresh CRLs"), 0, 0,
			this, SLOT(slotRefreshKeys()),
			actionCollection(), "certificates_refresh_clr" );

#ifdef NOT_IMPLEMENTED_ANYWAY
  mRevokeCertificateAction = new KAction( i18n("Revoke"), 0,
                                          this, SLOT(revokeCertificate()),
                                          actionCollection(), "edit_revoke_certificate" );
  connectEnableOperationSignal( this, mRevokeCertificateAction );

  mExtendCertificateAction = new KAction( i18n("Extend"), 0,
                                          this, SLOT(extendCertificate()),
                                          actionCollection(), "edit_extend_certificate" );
  connectEnableOperationSignal( this, mExtendCertificateAction );
#endif

  mDeleteCertificateAction = new KAction( i18n("Delete"), "editdelete", Key_Delete,
                                    this, SLOT(slotDeleteCertificate()),
                                    actionCollection(), "edit_delete_certificate" );
  connectEnableOperationSignal( this, mDeleteCertificateAction );

  mValidateCertificateAction = new KAction( i18n("Validate"), "reload", SHIFT + Key_F5,
					    this, SLOT(slotValidate()),
					    actionCollection(), "certificates_validate" );
  connectEnableOperationSignal( this, mValidateCertificateAction );

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

  mViewCertDetailsAction = new KAction( i18n("Certificate Details..."), 0, 0,
                                        this, SLOT(slotViewDetails()), actionCollection(),
                                        "view_certificate_details" );
  mDownloadCertificateAction = new KAction( i18n( "Download"), 0, 0,
                                        this, SLOT(slotDownloadCertificate()), actionCollection(),
                                        "download_certificate" );

  const QString dirmngr = KStandardDirs::findExe( "gpgsm" );
  mDirMngrFound = !dirmngr.isEmpty();

  action = new KAction( i18n("Dump CRL Cache..."), 0,
			this, SLOT(slotViewCRLs()),
			actionCollection(), "crl_dump_crl_cache" );
  action->setEnabled( mDirMngrFound ); // we also need dirmngr for this

  action = new KAction( i18n("Clear CRL Cache..."), 0,
			this, SLOT(slotClearCRLs()),
			actionCollection(), "crl_clear_crl_cache" );
  action->setEnabled( mDirMngrFound ); // we also need dirmngr for this

  action = new KAction( i18n("GnuPG Log Viewer..."), "pgp-keys", 0, this,
                        SLOT(slotStartWatchGnuPG()), actionCollection(), "tools_start_kwatchgnupg");
  // disable action if no kwatchgnupg binary is around
  if (KStandardDirs::findExe("kwatchgnupg").isEmpty()) action->setEnabled(false);

  (void)new LabelAction( i18n("Search:"), actionCollection(), "label_action" );

  mLineEditAction = new LineEditAction( QString::null, actionCollection(), this,
					SLOT(slotSearch()),
					"query_lineedit_action");

  QStringList lst;
  lst << i18n("In Local Certificates") << i18n("In External Certificates");
  mComboAction = new ComboAction( lst, actionCollection(), this, SLOT( slotToggleRemote(int) ),
                                  "location_combo_action");

  mFindAction = new KAction( i18n("Find"), "find", 0, this, SLOT(slotSearch()),
			     actionCollection(), "find" );

  KStdAction::keyBindings( this, SLOT(slotEditKeybindings()), actionCollection() );
  KStdAction::preferences( this, SLOT(slotShowConfigurationDialog()), actionCollection() );

  new KAction( i18n( "Configure &GpgME Backend" ), 0, 0, this, SLOT(slotConfigureGpgME()),
               actionCollection(), "configure_gpgme" );

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
  connect( &dlg, SIGNAL( configCommitted() ), SLOT( slotRepaint() ) );
  dlg.exec();
}

void CertManager::slotConfigureGpgME() {
  Kleo::CryptoConfig* config = Kleo::CryptoBackendFactory::instance()->config();
  if ( config ) {
    Kleo::CryptoConfigDialog dlg( config );
    dlg.exec();
  }
}

void CertManager::slotRepaint()
{
  mKeyListView->repaintContents();
}

void CertManager::slotToggleRemote( int idx ) {
  mNextFindRemote = idx != 0;
}

void CertManager::slotToggleHierarchicalView( bool hier ) {
  mKeyListView->setHierarchical( hier );
  mKeyListView->setRootIsDecorated( hier );
  if ( KAction * act = action("view_expandall") )
    act->setEnabled( hier );
  if ( KAction * act = action("view_collapseall" ) )
    act->setEnabled( hier );
  if ( hier && !mCurrentQuery.isEmpty() )
    startRedisplay( false );
}

void CertManager::slotExpandAll() {
  for ( QListViewItemIterator it( mKeyListView ) ; it.current() ; ++it )
    it.current()->setOpen( true );
}

void CertManager::slotCollapseAll() {
  for ( QListViewItemIterator it( mKeyListView ) ; it.current() ; ++it )
    it.current()->setOpen( false );
}

void CertManager::connectJobToStatusBarProgress( Kleo::Job * job, const QString & initialText ) {
  assert( mProgressBar );
  if ( !job )
    return;
  if ( !initialText.isEmpty() )
    statusBar()->message( initialText );
  connect( job, SIGNAL(progress(const QString&,int,int)),
	   mProgressBar, SLOT(slotProgress(const QString&,int,int)) );
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
  int total = 0;
  for ( QListViewItemIterator it( mKeyListView ) ; it.current() ; ++it )
    ++total;
  mStatusLabel->setText( i18n( "%n Key.","%n Keys.", total ) );
}

//
//
// Key Listing:
//
//


static std::set<std::string> extractKeyFingerprints( const QPtrList<Kleo::KeyListViewItem> & items ) {
  std::set<std::string> result;
  for ( QPtrListIterator<Kleo::KeyListViewItem> it( items ) ; it.current() ; ++it )
    if ( const char * fpr = it.current()->key().primaryFingerprint() )
      result.insert( fpr );
  return result;
}

static QStringList stringlistFromSet( const std::set<std::string> & set ) {
  // ARGH. This is madness. Shitty Qt containers don't support QStringList( patterns.begin(), patterns.end() ) :/
  QStringList sl;
  for ( std::set<std::string>::const_iterator it = set.begin() ; it != set.end() ; ++it )
    // let's make extra sure, maybe someone tries to make Qt not support std::string->QString conversion
    sl.push_back( QString::fromLatin1( it->c_str() ) );
  return sl;
}

void CertManager::slotRefreshKeys() {
  const QStringList keys = stringlistFromSet( extractKeyFingerprints( mKeyListView->selectedItems() ) );
  Kleo::RefreshKeysJob * job = Kleo::CryptoBackendFactory::instance()->smime()->refreshKeysJob();
  assert( job );

  connect( job, SIGNAL(result(const GpgME::Error&)),
	   this, SLOT(slotRefreshKeysResult(const GpgME::Error&)) );

  connectJobToStatusBarProgress( job, i18n("Refreshing keys...") );
  if ( const GpgME::Error err = job->start( keys ) )
    slotRefreshKeysResult( err );
}

void CertManager::slotRefreshKeysResult( const GpgME::Error & err ) {
  disconnectJobFromStatusBarProgress( err );
  if ( err.isCanceled() )
    return;
  if ( err )
    KMessageBox::error( this, i18n("An error occurred while trying to refresh "
				   "keys:\n%1").arg( QString::fromLocal8Bit( err.asString() ) ),
			i18n("Refreshing Keys Failed") );
}

static void showKeyListError( QWidget * parent, const GpgME::Error & err ) {
  assert( err );
  const QString msg = i18n( "<qt><p>An error occurred while fetching "
			    "the certificates from the backend:</p>"
			    "<p><b>%1</b></p></qt>" )
    .arg( QString::fromLocal8Bit( err.asString() ) );

  KMessageBox::error( parent, msg, i18n( "Certificate Listing Failed" ) );
}

void CertManager::slotSearch() {
  mPreviouslySelectedFingerprints.clear();
  // Clear display
  mKeyListView->clear();
  mCurrentQuery = mLineEditAction->text();
  startKeyListing( false, false, mCurrentQuery );
}

void CertManager::startRedisplay( bool validate ) {
  mPreviouslySelectedFingerprints = extractKeyFingerprints( mKeyListView->selectedItems() );
  if ( mPreviouslySelectedFingerprints.empty() )
    startKeyListing( validate, true, mCurrentQuery );
  else
    startKeyListing( validate, true, mPreviouslySelectedFingerprints );
}

void CertManager::startKeyListing( bool validating, bool refresh, const std::set<std::string> & patterns ) {
  startKeyListing( validating, refresh, stringlistFromSet( patterns ) );
}

void CertManager::startKeyListing( bool validating, bool refresh, const QStringList & patterns ) {
  mRemote = mNextFindRemote;
  mLineEditAction->setEnabled( false );
  mComboAction->setEnabled( false );
  mFindAction->setEnabled( false );

  Kleo::KeyListJob * job = 0;
  if ( !validating && !refresh && mKeyListView->hierarchical() && !patterns.empty() )
    job = new Kleo::HierarchicalKeyListJob( Kleo::CryptoBackendFactory::instance()->smime(),
					    mRemote, false, validating );
  else
    job = Kleo::CryptoBackendFactory::instance()->smime()->keyListJob( mRemote, false, validating );
  assert( job );

  connect( job, SIGNAL(nextKey(const GpgME::Key&)),
	   mKeyListView, refresh ? SLOT(slotRefreshKey(const GpgME::Key&)) : SLOT(slotAddKey(const GpgME::Key&)) );
  connect( job, SIGNAL(result(const GpgME::KeyListResult&)),
	   this, SLOT(slotKeyListResult(const GpgME::KeyListResult&)) );

  connectJobToStatusBarProgress( job, i18n("Fetching keys...") );

  const GpgME::Error err = job->start( patterns ) ;
  if ( err ) {
    showKeyListError( this, err );
    return;
  }
  mProgressBar->setProgress( 0, 0 ); // enable busy indicator
}

static void selectKeys( Kleo::KeyListView * lv, const std::set<std::string> & fprs ) {
  if ( !lv || fprs.empty() )
    return;
  for  ( QListViewItemIterator it( lv ) ; it.current() ; ++it )
    if ( Kleo::KeyListViewItem * item = Kleo::lvi_cast<Kleo::KeyListViewItem>( it.current() ) ) {
      const char * fpr = item->key().primaryFingerprint();
      item->setSelected( fpr && fprs.find( fpr ) != fprs.end() );
    }
}

void CertManager::slotKeyListResult( const GpgME::KeyListResult & res ) {
  if ( res.error() )
    showKeyListError( this, res.error() );
  else if ( res.isTruncated() )
    KMessageBox::information( this,
			      i18n("The query result has been truncated.\n"
				   "Either the local or a remote limit on "
				   "the maximum number of returned hits has "
				   "been exceeded.\n"
				   "You can try to increase the local limit "
				   "in the configuration dialog, but if one "
				   "of the configured servers is the limiting "
				   "factor, you have to refine your search.") );

  mLineEditAction->setEnabled( true );
  mComboAction->setEnabled( true );
  mFindAction->setEnabled( true );

  mLineEditAction->focusAll();
  disconnectJobFromStatusBarProgress( res.error() );
  selectKeys( mKeyListView, mPreviouslySelectedFingerprints );
}

void CertManager::slotContextMenu(Kleo::KeyListViewItem* item, const QPoint& point) {
  if ( !item )
    return;
  if ( QPopupMenu * popup = static_cast<QPopupMenu*>(factory()->container("listview_popup",this)) )
    popup->exec( point );
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
  const QString filter = "application/x-x509-ca-cert application/x-pkcs12 application/pkcs7-mime";
  //const QString filter = QString("*.pem *.der *.p7c *.p12|") + i18n("Certificates (*.pem *.der *.p7c *.p12)");
  slotImportCertFromFile( KFileDialog::getOpenURL( QString::null, filter, this,
                                                   i18n( "Select Certificate File" ) ) );
}

void CertManager::slotImportCertFromFile( const KURL & certURL )
{
  if ( !certURL.isValid() ) // empty or malformed
    return;

  mPreviouslySelectedFingerprints.clear();

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
  mPreviouslySelectedFingerprints.clear();
  QPtrList<Kleo::KeyListViewItem> items = mKeyListView->selectedItems();
  for ( QPtrListIterator<Kleo::KeyListViewItem> it( items ) ; it.current() ; ++it )
    if ( !it.current()->key().isNull() )
      if ( const char * fpr = it.current()->key().primaryFingerprint() )
        slotStartCertificateDownload( fpr, it.current()->text(0) );
}

// Called from slotDownloadCertificate and from the certificate-details widget
void CertManager::slotStartCertificateDownload( const QString& fingerprint, const QString& displayName ) {
  if ( fingerprint.isEmpty() )
    return;

  Kleo::DownloadJob * job =
    Kleo::CryptoBackendFactory::instance()->smime()->downloadJob( false /* no armor */ );
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
  Kleo::ImportJob * job = Kleo::CryptoBackendFactory::instance()->smime()->importJob();
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

  if ( res.error().isCanceled() ) {
    // do nothing
  } else if ( res.error() ) {
    showCertificateImportError( this, res.error(), displayName );
  } else {

    const QString normalLine = i18n("<tr><td align=\"right\">%1</td><td>%2</td></tr>");
    const QString boldLine = i18n("<tr><td align=\"right\"><b>%1</b></td><td>%2</td></tr>");

    QStringList lines;
    lines.push_back( normalLine.arg( i18n("Total number processed:"),
				     QString::number( res.numConsidered() ) ) );
    lines.push_back( normalLine.arg( i18n("Imported:"),
				     QString::number( res.numImported() ) ) );
    if ( res.newSignatures() )
      lines.push_back( normalLine.arg( i18n("New signatures:"),
				       QString::number( res.newSignatures() ) ) );
    if ( res.newUserIDs() )
      lines.push_back( normalLine.arg( i18n("New user IDs:"),
				       QString::number( res.newUserIDs() ) ) );
    if ( res.numKeysWithoutUserID() )
      lines.push_back( normalLine.arg( i18n("Keys without user IDs:"),
				       QString::number( res.numKeysWithoutUserID() ) ) );
    if ( res.newSubkeys() )
      lines.push_back( normalLine.arg( i18n("New subkeys:"),
				       QString::number( res.newSubkeys() ) ) );
    if ( res.newRevocations() )
      lines.push_back( boldLine.arg( i18n("Newly revoked:"),
				     QString::number( res.newRevocations() ) ) );
    if ( res.notImported() )
      lines.push_back( boldLine.arg( i18n("Not imported:"),
				     QString::number( res.notImported() ) ) );
    if ( res.numUnchanged() )
      lines.push_back( normalLine.arg( i18n("Unchanged:"),
				       QString::number( res.numUnchanged() ) ) );
    if ( res.numSecretKeysConsidered() )
      lines.push_back( normalLine.arg( i18n("Secret keys processed:"),
				       QString::number( res.numSecretKeysConsidered() ) ) );
    if ( res.numSecretKeysImported() )
      lines.push_back( normalLine.arg( i18n("Secret keys imported:"),
				       QString::number( res.numSecretKeysImported() ) ) );
    if ( res.numSecretKeysConsidered() - res.numSecretKeysImported() - res.numSecretKeysUnchanged() > 0 )
      lines.push_back( boldLine.arg( i18n("Secret keys <em>not</em> imported:"),
				     QString::number( res.numSecretKeysConsidered()
						      - res.numSecretKeysImported()
						      - res.numSecretKeysUnchanged() ) ) );
    if ( res.numSecretKeysUnchanged() )
      lines.push_back( normalLine.arg( i18n("Secret keys unchanged:"),
				       QString::number( res.numSecretKeysUnchanged() ) ) );

    KMessageBox::information( this,
			      i18n( "<qt><p>Detailed results of importing %1:</p>"
				    "<table>%2</table></qt>" )
			      .arg( displayName ).arg( lines.join( QString::null ) ),
			      i18n( "Certificate Import Result" ) );

    disconnectJobFromStatusBarProgress( res.error() );
    // save the fingerprints of imported certs for later selection:
    const std::vector<GpgME::Import> imports = res.imports();
    for ( std::vector<GpgME::Import>::const_iterator it = imports.begin() ; it != imports.end() ; ++it )
      mPreviouslySelectedFingerprints.insert( it->fingerprint() );
  }
  importNextURLOrRedisplay();
}



/**
   This slot is called when the dirmngr process that imports a
   certificate file exists.
*/
void CertManager::slotDirmngrExited() {
    if ( !mDirmngrProc->normalExit() )
        KMessageBox::error( this, i18n( "The GpgSM process that tried to import the CRL file ended prematurely because of an unexpected error." ), i18n( "Certificate Manager Error" ) );
    else if ( mDirmngrProc->exitStatus() )
      KMessageBox::error( this, i18n( "An error occurred when trying to import the CRL file. The output from GpgSM was:\n%1").arg( mErrorbuffer ), i18n( "Certificate Manager Error" ) );
    else
      KMessageBox::information( this, i18n( "CRL file imported successfully." ), i18n( "Certificate Manager Information" ) );

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

bool CertManager::connectAndStartDirmngr( const char * slot, const char * processname ) {
  assert( slot );
  assert( processname );
  assert( mDirmngrProc );
  mErrorbuffer = QString::null;
  connect( mDirmngrProc, SIGNAL(processExited(KProcess*)), slot );
  connect( mDirmngrProc, SIGNAL(receivedStderr(KProcess*,char*,int) ),
           this, SLOT(slotStderr(KProcess*,char*,int)) );
  if( !mDirmngrProc->start( KProcess::NotifyOnExit, KProcess::Stderr ) ) {
    delete mDirmngrProc; mDirmngrProc = 0;
    KMessageBox::error( this, i18n( "Unable to start %1 process. Please check your installation." ).arg( processname ), i18n( "Certificate Manager Error" ) );
    return false;
  }
  return true;
}

void CertManager::startImportCRL( const QString& filename, bool isTempFile )
{
  assert( !mDirmngrProc );
  mImportCRLTempFile = isTempFile ? filename : QString::null;
  mDirmngrProc = new KProcess();
  *mDirmngrProc << "gpgsm" << "--call-dirmngr" << "loadcrl" << filename;
  if ( !connectAndStartDirmngr( SLOT(slotDirmngrExited()), "gpgsm" ) ) {
    updateImportActions( true );
    if ( isTempFile )
      QFile::remove( mImportCRLTempFile ); // unlink tempfile
  }
}

void CertManager::startClearCRLs() {
  assert( !mDirmngrProc );
  mDirmngrProc = new KProcess();
  *mDirmngrProc << "dirmngr" << "--flush";
  //*mDirmngrProc << "gpgsm" << "--call-dimngr" << "flush"; // use this once it's implemented!
  connectAndStartDirmngr( SLOT(slotClearCRLsResult()), "dirmngr" );
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


void CertManager::slotClearCRLs() {
  startClearCRLs();
}

void CertManager::slotClearCRLsResult() {
  assert( mDirmngrProc );
  if ( !mDirmngrProc->normalExit() )
    KMessageBox::error( this, i18n( "The DirMngr process that tried to clear the CRL cache ended prematurely because of an unexpected error." ), i18n( "Certificate Manager Error" ) );
  else if ( mDirmngrProc->exitStatus() )
    KMessageBox::error( this, i18n( "An error occurred when trying to clear the CRL cache. The output from DirMngr was:\n%1").arg( mErrorbuffer ), i18n( "Certificate Manager Error" ) );
  else
    KMessageBox::information( this, i18n( "CRL cache cleared successfully." ), i18n( "Certificate Manager Information" ) );
  delete mDirmngrProc; mDirmngrProc = 0;
}

static void showDeleteError( QWidget * parent, const GpgME::Error & err ) {
  assert( err );
  const QString msg = i18n("<qt><p>An error occurred while trying to delete "
			   "the certificates:</p>"
			   "<p><b>%1</b></p></qt>")
    .arg( QString::fromLocal8Bit( err.asString() ) );
  KMessageBox::error( parent, msg, i18n("Certificate Deletion Failed") );
}

static bool ByFingerprint( const GpgME::Key & left, const GpgME::Key & right ) {
  return qstricmp( left.primaryFingerprint(), right.primaryFingerprint() ) < 0 ;
}

static bool WithRespectToFingerprints( const GpgME::Key & left, const GpgME::Key & right ) {
  return qstricmp( left.primaryFingerprint(), right.primaryFingerprint() ) == 0;
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

  if ( !mHierarchyAnalyser ) {
    mHierarchyAnalyser = new HierarchyAnalyser( this, "mHierarchyAnalyser" );
    Kleo::KeyListJob * job = Kleo::CryptoBackendFactory::instance()->smime()->keyListJob();
    assert( job );
    connect( job, SIGNAL(nextKey(const GpgME::Key&)),
	     mHierarchyAnalyser, SLOT(slotNextKey(const GpgME::Key&)) );
    connect( job, SIGNAL(result(const GpgME::KeyListResult&)),
	     this, SLOT(slotDeleteCertificate()) );
    connectJobToStatusBarProgress( job, i18n("Checking key dependencies...") );
    if ( const GpgME::Error error = job->start( QStringList() ) ) {
      showKeyListError( this, error );
      delete mHierarchyAnalyser; mHierarchyAnalyser = 0;
    }
    return;
  } else
    disconnectJobFromStatusBarProgress( 0 );

  std::vector<GpgME::Key> keysToDelete = keys;
  for ( std::vector<GpgME::Key>::const_iterator it = keys.begin() ; it != keys.end() ; ++it )
    if ( !it->isNull() ) {
      const std::vector<GpgME::Key> subjects
	= mHierarchyAnalyser->subjectsForIssuerRecursive( it->primaryFingerprint() );
      keysToDelete.insert( keysToDelete.end(), subjects.begin(), subjects.end() );
    }

  std::sort( keysToDelete.begin(), keysToDelete.end(), ByFingerprint );
  keysToDelete.erase( std::unique( keysToDelete.begin(), keysToDelete.end(),
				   WithRespectToFingerprints ),
		      keysToDelete.end() );

  delete mHierarchyAnalyser; mHierarchyAnalyser = 0;

  if ( keysToDelete.size() > keys.size() )
    if ( KMessageBox::warningContinueCancel( this,
					     i18n("Some or all of the selected "
						  "certificates are issuers (CA certificates) "
						  "for other, non-selected certificates.\n"
						  "Deleting a CA certificate will also delete "
						  "all certificates issued by it."),
					     i18n("Deleting CA Certificates") )
	 != KMessageBox::Continue )
      return;

  const QString msg = keysToDelete.size() > keys.size()
    ? i18n("Do you really want to delete this certificate and the %1 certificates it certified?",
	   "Do you really want to delete these %n certificates and the %1 certificates they certified?",
	   keys.size() ).arg( keysToDelete.size() - keys.size() )
    : i18n("Do you really want to delete this certificate?",
	   "Do you really want to delete these %n certificates?", keys.size() ) ;

  if ( KMessageBox::warningContinueCancelList( this, msg, keyDisplayNames,
					       i18n( "Delete Certificates" ),
					       KGuiItem( i18n( "Delete" ), "editdelete" ),
					       "ConfirmDeleteCert", KMessageBox::Dangerous )
       != KMessageBox::Continue )
    return;

  if ( Kleo::DeleteJob * job = Kleo::CryptoBackendFactory::instance()->smime()->deleteJob() )
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

  mItemsToDelete.clear(); // re-create according to the real selection
  for ( std::vector<GpgME::Key>::const_iterator it = keysToDelete.begin() ; it != keysToDelete.end() ; ++it )
    if ( Kleo::KeyListViewItem * item = mKeyListView->itemByFingerprint( it->primaryFingerprint() ) )
      mItemsToDelete.append( item );

  Kleo::MultiDeleteJob * job = new Kleo::MultiDeleteJob( Kleo::CryptoBackendFactory::instance()->smime() );
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
    const int infinity = 100; // infinite loop guard...
    mItemsToDelete.setAutoDelete( true );
    for ( int i = 0 ; i < infinity ; ++i ) {
      QPtrListIterator<Kleo::KeyListViewItem> it( mItemsToDelete );
      while ( Kleo::KeyListViewItem * cur = it.current() ) {
	++it;
	if ( cur->childCount() == 0 ) {
	  mItemsToDelete.remove( cur );
	}
      }
      if ( mItemsToDelete.isEmpty() )
	break;
    }
    mItemsToDelete.setAutoDelete( false );
    Q_ASSERT( mItemsToDelete.isEmpty() );
    mItemsToDelete.clear();
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
  connect( top, SIGNAL(requestCertificateDownload(const QString&, const QString&)),
	   SLOT(slotStartCertificateDownload(const QString&, const QString&)) );
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
  mValidateCertificateAction->setEnabled( !mRemote );
}

void CertManager::slotExportCertificate() {
  QPtrList<Kleo::KeyListViewItem> items = mKeyListView->selectedItems();
  if ( items.isEmpty() )
    return;

  QStringList fingerprints;
  for ( QPtrListIterator<Kleo::KeyListViewItem> it( items ) ; it.current() ; ++it )
    if ( !it.current()->key().isNull() )
      if ( const char * fpr = it.current()->key().primaryFingerprint() )
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
  Kleo::ExportJob * job = Kleo::CryptoBackendFactory::instance()->smime()->publicKeyExportJob( true );
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
  Kleo::ExportJob * job = Kleo::CryptoBackendFactory::instance()->smime()->secretKeyExportJob( false );
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
  if ( !lst.empty() )
    importNextURLOrRedisplay();
}

void CertManager::importNextURLOrRedisplay()
{
  if ( !mURLsToImport.empty() ) {
    // We can only import them one by one, otherwise the jobs would run into each other
    KURL url = mURLsToImport.front();
    mURLsToImport.pop_front();
    slotImportCertFromFile( url );
  } else {
    if ( isRemote() )
      return;
    startKeyListing( false, true, mPreviouslySelectedFingerprints );
  }
}

void CertManager::slotStartWatchGnuPG()
{
  KProcess certManagerProc;
  certManagerProc << "kwatchgnupg";

  if( !certManagerProc.start( KProcess::DontCare ) )
    KMessageBox::error( this, i18n( "Could not start GnuPG LogViewer (kwatchgnupg). "
                                    "Please check your installation!" ),
                                    i18n( "Kleopatra Error" ) );
}

#include "certmanager.moc"
