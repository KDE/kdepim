/*
    certmanager.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2002,2004 Klar�vdalens Datakonsult AB

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
#include <kleo/cryptoconfig.h>

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
#include <kdialog.h>
#include <kkeydialog.h>
#include <ktempfile.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kstdaccel.h>
#include <kstdaction.h>
#include <ktoggleaction.h>
#include <kxmlguifactory.h>

// Qt
#include <QFontMetrics>
//Added by qt3to4:
#include <Q3PtrList>
#include <QLabel>
#include <QMenu>
// other
#include <algorithm>
#include <assert.h>
#include <kdepimmacros.h>
namespace {

  class KDE_EXPORT DisplayStrategy : public Kleo::KeyListView::DisplayStrategy{
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

  class KDE_EXPORT ColumnStrategy : public Kleo::KeyListView::ColumnStrategy {
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
    default: return QString();
    }
  }

  QString ColumnStrategy::text( const GpgME::Key & key, int col ) const {
    switch ( col ) {
    case 0: return Kleo::DN( key.userID(0).id() ).prettyDN();
    case 1: return Kleo::DN( key.issuerName() ).prettyDN();
    case 2: return key.issuerSerial() ? QString::fromUtf8( key.issuerSerial() ) : QString() ;
    default: return QString();
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
			  QWidget* parent, const char* name, Qt::WFlags f )
  : KMainWindow( parent, name, f|Qt::WDestructiveClose ),
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
  mKeyListView = new CertKeyListView( new ColumnStrategy(), new DisplayStrategy(), this );
  mKeyListView->setObjectName( "mKeyListView" );
  mKeyListView->setSelectionMode( Q3ListView::Extended );
  setCentralWidget( mKeyListView );

  connect( mKeyListView, SIGNAL(doubleClicked(Kleo::KeyListViewItem*,const QPoint&,int)),
	   SLOT(slotViewDetails(Kleo::KeyListViewItem*)) );
  connect( mKeyListView, SIGNAL(returnPressed(Kleo::KeyListViewItem*)),
	   SLOT(slotViewDetails(Kleo::KeyListViewItem*)) );
  connect( mKeyListView, SIGNAL(selectionChanged()),
	   SLOT(slotSelectionChanged()) );
  connect( mKeyListView, SIGNAL(contextMenu(Kleo::KeyListViewItem*, const QPoint&)),
           SLOT(slotContextMenu(Kleo::KeyListViewItem*, const QPoint&)) );

  connect( mKeyListView, SIGNAL(dropped(const KUrl::List&) ),
           SLOT( slotDropped(const KUrl::List&) ) );

  mLineEditAction->setText(query);
  if ( !mRemote || !query.isEmpty() )
    slotSearch();

  if ( !import.isEmpty() )
    slotImportCertFromFile( KUrl( import ) );

  readConfig();
  updateStatusBarLabels();
  slotSelectionChanged(); // initial state for selection-dependent actions
}

CertManager::~CertManager() {
  writeConfig();
  delete mDirmngrProc; mDirmngrProc = 0;
  delete mHierarchyAnalyser; mHierarchyAnalyser = 0;
}

void CertManager::readConfig() {
  KConfig config( "kleopatrarc" );
  config.setGroup( "Display Options" );
  slotToggleHierarchicalView( config.readEntry( "hierarchicalView", false ) );
}

void CertManager::writeConfig() {
  KConfig config( "kleopatrarc" );
  config.setGroup( "Display Options" );
  config.writeEntry( "hierarchicalView", mKeyListView->hierarchical() );
}

void CertManager::createStatusBar() {
  KStatusBar * bar = statusBar();
  mProgressBar = new Kleo::ProgressBar( bar );
  mProgressBar->setObjectName( "mProgressBar" );
  mProgressBar->reset();
  mProgressBar->setFixedSize( QSize( 100, mProgressBar->height() * 3 / 5 ) );
  bar->addPermanentWidget( mProgressBar, 0 );
  mStatusLabel = new QLabel( bar );
  mStatusLabel->setObjectName( "mStatusLabel" );
  bar->addWidget( mStatusLabel, 1 );
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
  reloadShortcut.append(Qt::CTRL + Qt::Key_R);
  action->setShortcut( reloadShortcut );

  connectEnableOperationSignal( this, action );

  action = new KAction(KIcon("stop"),  i18n("Stop Operation"), actionCollection(), "view_stop_operations" );
  connect(action, SIGNAL(triggered(bool) ), SIGNAL(stopOperations()));
  action->setShortcut(Qt::Key_Escape);
  action->setEnabled( false );

  action = new KAction(KIcon("filenew"),  i18n("New Key Pair..."), actionCollection(), "file_new_certificate" );
  connect(action, SIGNAL(triggered(bool) ), SLOT(newCertificate()));

  connect( new KToggleAction( i18n("Hierarchical Key List"),
			      actionCollection(), "view_hierarchical" ),
	   SIGNAL(toggled(bool)), SLOT(slotToggleHierarchicalView(bool)) );

  action = new KAction( i18n("Expand All"), actionCollection(), "view_expandall" );
  connect(action, SIGNAL(triggered(bool) ), SLOT(slotExpandAll()));
  action->setShortcut(Qt::CTRL+Qt::Key_Period);
  action = new KAction( i18n("Collapse All"), actionCollection(), "view_collapseall" );
  connect(action, SIGNAL(triggered(bool) ), SLOT(slotCollapseAll()));
  action->setShortcut(Qt::CTRL+Qt::Key_Comma);

  action = new KAction( i18n("Refresh CRLs"), actionCollection(), "certificates_refresh_clr" );
  connect(action, SIGNAL(triggered(bool) ), SLOT(slotRefreshKeys()));

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

  mDeleteCertificateAction = new KAction(KIcon("editdelete"),  i18n("Delete"), actionCollection(), "edit_delete_certificate" );
  connect(mDeleteCertificateAction, SIGNAL(triggered(bool) ), SLOT(slotDeleteCertificate()));
  mDeleteCertificateAction->setShortcut(Qt::Key_Delete);
  connectEnableOperationSignal( this, mDeleteCertificateAction );

  mValidateCertificateAction = new KAction(KIcon("reload"),  i18n("Validate"), actionCollection(), "certificates_validate" );
  connect(mValidateCertificateAction, SIGNAL(triggered(bool) ), SLOT(slotValidate()));
  mValidateCertificateAction->setShortcut(Qt::SHIFT + Qt::Key_F5);
  connectEnableOperationSignal( this, mValidateCertificateAction );

  mImportCertFromFileAction = new KAction( i18n("Import Certificates..."), actionCollection(), "file_import_certificates" );
  connect(mImportCertFromFileAction, SIGNAL(triggered(bool) ), SLOT(slotImportCertFromFile()));
  connectEnableOperationSignal( this, mImportCertFromFileAction );

  mImportCRLFromFileAction = new KAction( i18n("Import CRLs..."), actionCollection(), "file_import_crls" );
  connect(mImportCRLFromFileAction, SIGNAL(triggered(bool) ), SLOT(importCRLFromFile()));
  connectEnableOperationSignal( this, mImportCRLFromFileAction );

  mExportCertificateAction = new KAction(KIcon("export"),  i18n("Export Certificates..."), actionCollection(), "file_export_certificate" );
  connect(mExportCertificateAction, SIGNAL(triggered(bool) ), SLOT(slotExportCertificate()));

  mExportSecretKeyAction = new KAction(KIcon("export"),  i18n("Export Secret Key..."), actionCollection(), "file_export_secret_keys" );
  connect(mExportSecretKeyAction, SIGNAL(triggered(bool) ), SLOT(slotExportSecretKey()));
  connectEnableOperationSignal( this, mExportSecretKeyAction );

  mViewCertDetailsAction = new KAction( i18n("Certificate Details..."), actionCollection(), "view_certificate_details" );
  connect(mViewCertDetailsAction, SIGNAL(triggered(bool) ), SLOT(slotViewDetails()));
  mDownloadCertificateAction = new KAction( i18n( "Download"), actionCollection(), "download_certificate" );
  connect(mDownloadCertificateAction, SIGNAL(triggered(bool) ), SLOT(slotDownloadCertificate()));

  const QString dirmngr = KStandardDirs::findExe( "gpgsm" );
  mDirMngrFound = !dirmngr.isEmpty();

  action = new KAction( i18n("Dump CRL Cache..."), actionCollection(), "crl_dump_crl_cache" );
  connect(action, SIGNAL(triggered(bool) ), SLOT(slotViewCRLs()));
  action->setEnabled( mDirMngrFound ); // we also need dirmngr for this

  action = new KAction( i18n("Clear CRL Cache..."), actionCollection(), "crl_clear_crl_cache" );
  connect(action, SIGNAL(triggered(bool) ), SLOT(slotClearCRLs()));
  action->setEnabled( mDirMngrFound ); // we also need dirmngr for this

  action = new KAction(KIcon("pgp-keys"),  i18n("GnuPG Log Viewer..."), actionCollection(), "tools_start_kwatchgnupg");
  connect(action, SIGNAL(triggered(bool)), SLOT(slotStartWatchGnuPG()));
  // disable action if no kwatchgnupg binary is around
  if (KStandardDirs::findExe("kwatchgnupg").isEmpty()) action->setEnabled(false);

  (void)new LabelAction( i18n("Search:"), actionCollection(), "label_action" );

  mLineEditAction = new LineEditAction( QString(), actionCollection(), this,
					SLOT(slotSearch()),
					"query_lineedit_action");

  QStringList lst;
  lst << i18n("In Local Certificates") << i18n("In External Certificates");
  mComboAction = new ComboAction( lst, actionCollection(), this, SLOT( slotToggleRemote(int) ),
                                  "location_combo_action");

  mFindAction = new KAction(KIcon("find"),  i18n("Find"), actionCollection(), "find" );
  connect(mFindAction, SIGNAL(triggered(bool)), SLOT(slotSearch()));

  KStdAction::keyBindings( this, SLOT(slotEditKeybindings()), actionCollection() );
  KStdAction::preferences( this, SLOT(slotShowConfigurationDialog()), actionCollection() );

  action = new KAction( i18n( "Configure &GpgME Backend" ), actionCollection(), "configure_gpgme" );
  connect(action, SIGNAL(triggered(bool)), SLOT(slotConfigureGpgME()));

  createStandardStatusBarAction();
  updateImportActions( true );
}

void CertManager::updateImportActions( bool enable ) {
  mImportCRLFromFileAction->setEnabled( mDirMngrFound && enable );
  mImportCertFromFileAction->setEnabled( enable );
}

void CertManager::slotEditKeybindings() {
  KKeyDialog::configure( actionCollection(), KKeyChooser::LetterShortcutsAllowed );
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

    int result = dlg.exec();

    // Forget all data parsed from gpgconf, so that we show updated information
    // when reopening the configuration dialog.
    config->clear();

    if ( result == QDialog::Accepted )
    {
      // Tell other apps (e.g. kmail) that the gpgconf data might have changed
      kapp->dcopClient()->emitDCOPSignal( "KPIM::CryptoConfig", "changed()", QByteArray() );
    }
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
  if ( KToggleAction * act =
      static_cast<KToggleAction*>( action("view_hierarchical") ) )
    act->setChecked( hier );

  if ( hier && !mCurrentQuery.isEmpty() )
    startRedisplay( false );
}

void CertManager::slotExpandAll() {
  for ( Q3ListViewItemIterator it( mKeyListView ) ; it.current() ; ++it )
    it.current()->setOpen( true );
}

void CertManager::slotCollapseAll() {
  for ( Q3ListViewItemIterator it( mKeyListView ) ; it.current() ; ++it )
    it.current()->setOpen( false );
}

void CertManager::connectJobToStatusBarProgress( Kleo::Job * job, const QString & initialText ) {
  assert( mProgressBar );
  if ( !job )
    return;
  if ( !initialText.isEmpty() )
    statusBar()->showMessage( initialText );
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
  statusBar()->showMessage( msg, 4000 );

  action("view_stop_operations")->setEnabled( false );
  emit enableOperations( true );
  slotSelectionChanged();
}

void CertManager::updateStatusBarLabels() {
  mKeyListView->flushKeys();
  int total = 0;
  for ( Q3ListViewItemIterator it( mKeyListView ) ; it.current() ; ++it )
    ++total;
  mStatusLabel->setText( i18np( "%n Key.","%n Keys.", total ) );
}

//
//
// Key Listing:
//
//


static std::set<std::string> extractKeyFingerprints( const Q3PtrList<Kleo::KeyListViewItem> & items ) {
  std::set<std::string> result;
  for ( Q3PtrListIterator<Kleo::KeyListViewItem> it( items ) ; it.current() ; ++it )
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
				   "keys:\n%1", QString::fromLocal8Bit( err.asString() ) ),
			i18n("Refreshing Keys Failed") );
}

static void showKeyListError( QWidget * parent, const GpgME::Error & err ) {
  assert( err );
  const QString msg = i18n( "<qt><p>An error occurred while fetching "
			    "the certificates from the backend:</p>"
			    "<p><b>%1</b></p></qt>" ,
      QString::fromLocal8Bit( err.asString() ) );

  KMessageBox::error( parent, msg, i18n( "Certificate Listing Failed" ) );
}

void CertManager::slotSearch() {
  mPreviouslySelectedFingerprints.clear();
  // Clear display
  mKeyListView->clear();
  mCurrentQuery = mLineEditAction->text();
  startKeyListing( false, false, QStringList( mCurrentQuery ) );
}

void CertManager::startRedisplay( bool validate ) {
  mPreviouslySelectedFingerprints = extractKeyFingerprints( mKeyListView->selectedItems() );
  if ( mPreviouslySelectedFingerprints.empty() )
    startKeyListing( validate, true, QStringList( mCurrentQuery ) );
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
  for  ( Q3ListViewItemIterator it( lv ) ; it.current() ; ++it )
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
  if ( QMenu * popup = static_cast<QMenu*>(factory()->container("listview_popup",this)) )
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
  slotImportCertFromFile( KFileDialog::getOpenURL( QString(), filter, this,
                                                   i18n( "Select Certificate File" ) ) );
}

void CertManager::slotImportCertFromFile( const KUrl & certURL )
{
  if ( !certURL.isValid() ) // empty or malformed
    return;

  mPreviouslySelectedFingerprints.clear();

  // Prevent two simultaneous imports
  updateImportActions( false );

  // Download the cert
  KIO::StoredTransferJob* importJob = KIO::storedGet( certURL );
  importJob->setWindow( this );
  connect( importJob, SIGNAL(result(KJob*)), SLOT(slotImportResult(KJob*)) );
}

void CertManager::slotImportResult( KJob* job )
{
  if ( job->error() ) {
    static_cast<KIO::Job*>(job)->showErrorDialog();
  } else {
    KIO::StoredTransferJob* trJob = static_cast<KIO::StoredTransferJob *>( job );
    startCertificateImport( trJob->data(), trJob->url().fileName() );
  }

  updateImportActions( true );
}

static void showCertificateDownloadError( QWidget * parent, const GpgME::Error & err, const QString& certDisplayName ) {
  assert( err );
  const QString msg = i18n( "<qt><p>An error occurred while trying "
			    "to download the certificate %1:</p>"
			    "<p><b>%2</b></p></qt>" ,
                        certDisplayName ,
                        QString::fromLocal8Bit( err.asString() ) );

  KMessageBox::error( parent, msg, i18n( "Certificate Download Failed" ) );
}

void CertManager::slotDownloadCertificate() {
  mPreviouslySelectedFingerprints.clear();
  Q3PtrList<Kleo::KeyListViewItem> items = mKeyListView->selectedItems();
  for ( Q3PtrListIterator<Kleo::KeyListViewItem> it( items ) ; it.current() ; ++it )
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

  const GpgME::Error err = job->start( QStringList( fingerprint ) );
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
    mJobsDisplayNameMap.erase( it );
  } else {
    kWarning() << "Job not found in map: " << job << endl;
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
			    "<p><b>%2</b></p></qt>" ,
                        certDisplayName ,
                        QString::fromLocal8Bit( err.asString() ) );
  KMessageBox::error( parent, msg, i18n( "Certificate Import Failed" ) );
}

void CertManager::startCertificateImport( const QByteArray & keyData, const QString& certDisplayName ) {
  Kleo::ImportJob * job = Kleo::CryptoBackendFactory::instance()->smime()->importJob();
  assert( job );

  connect( job, SIGNAL(result(const GpgME::ImportResult&)),
	   SLOT(slotCertificateImportResult(const GpgME::ImportResult&)) );

  connectJobToStatusBarProgress( job, i18n("Importing certificates...") );

  kDebug() << "Importing certificate. keyData size:" << keyData.size() << endl;
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

    const KLocalizedString normalLine = ki18n("<tr><td align=\"right\">%1</td><td>%2</td></tr>");
    const KLocalizedString boldLine = ki18n("<tr><td align=\"right\"><b>%1</b></td><td>%2</td></tr>");

    QStringList lines;
    lines.push_back( normalLine.subs( i18n("Total number processed:") )
			       .subs( res.numConsidered() ).toString() );
    lines.push_back( normalLine.subs( i18n("Imported:") )
			       .subs( res.numImported() ).toString() );
    if ( res.newSignatures() )
      lines.push_back( normalLine.subs( i18n("New signatures:") )
				 .subs( res.newSignatures() ).toString() );
    if ( res.newUserIDs() )
      lines.push_back( normalLine.subs( i18n("New user IDs:") )
				 .subs( res.newUserIDs() ).toString() );
    if ( res.numKeysWithoutUserID() )
      lines.push_back( normalLine.subs( i18n("Keys without user IDs:") )
				 .subs( res.numKeysWithoutUserID() ).toString() );
    if ( res.newSubkeys() )
      lines.push_back( normalLine.subs( i18n("New subkeys:") )
				 .subs( res.newSubkeys() ).toString() );
    if ( res.newRevocations() )
      lines.push_back( boldLine.subs( i18n("Newly revoked:") )
			       .subs( res.newRevocations() ).toString() );
    if ( res.notImported() )
      lines.push_back( boldLine.subs( i18n("Not imported:") )
			       .subs( res.notImported() ).toString() );
    if ( res.numUnchanged() )
      lines.push_back( normalLine.subs( i18n("Unchanged:") )
				 .subs( res.numUnchanged() ).toString() );
    if ( res.numSecretKeysConsidered() )
      lines.push_back( normalLine.subs( i18n("Secret keys processed:") )
				 .subs( res.numSecretKeysConsidered() ).toString() );
    if ( res.numSecretKeysImported() )
      lines.push_back( normalLine.subs( i18n("Secret keys imported:") )
				 .subs( res.numSecretKeysImported() ).toString() );
    if ( res.numSecretKeysConsidered() - res.numSecretKeysImported() - res.numSecretKeysUnchanged() > 0 )
      lines.push_back( boldLine.subs( i18n("Secret keys <em>not</em> imported:") )
			       .subs(  res.numSecretKeysConsidered()
				     - res.numSecretKeysImported()
				     - res.numSecretKeysUnchanged() ).toString() );
    if ( res.numSecretKeysUnchanged() )
      lines.push_back( normalLine.subs( i18n("Secret keys unchanged:") )
				 .subs( res.numSecretKeysUnchanged() ).toString() );

    KMessageBox::information( this,
			      i18n( "<qt><p>Detailed results of importing %1:</p>"
				    "<table>%2</table></qt>" ,
			        displayName, lines.join( QString() ) ),
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
      KMessageBox::error( this, i18n( "An error occurred when trying to import the CRL file. The output from GpgSM was:\n%1", mErrorbuffer ), i18n( "Certificate Manager Error" ) );
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
  KUrl url = KFileDialog::getOpenURL( QString(),
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
      KUrl destURL;
      destURL.setPath( tempFile.name() );
      KIO::Job* copyJob = KIO::file_copy( url, destURL, 0600, true, false );
      copyJob->setWindow( this );
      connect( copyJob, SIGNAL( result( KJob * ) ),
               SLOT( slotImportCRLJobFinished( KJob * ) ) );
    }
  }
}

void CertManager::slotImportCRLJobFinished( KJob *job )
{
  KIO::FileCopyJob* fcjob = static_cast<KIO::FileCopyJob*>( job );
  QString tempFilePath = fcjob->destURL().path();
  if ( job->error() ) {
    static_cast<KIO::Job*>(job)->showErrorDialog();
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
  mErrorbuffer.clear();
  connect( mDirmngrProc, SIGNAL(processExited(KProcess*)), slot );
  connect( mDirmngrProc, SIGNAL(receivedStderr(KProcess*,char*,int) ),
           this, SLOT(slotStderr(KProcess*,char*,int)) );
  if( !mDirmngrProc->start( KProcess::NotifyOnExit, KProcess::Stderr ) ) {
    delete mDirmngrProc; mDirmngrProc = 0;
    KMessageBox::error( this, i18n( "Unable to start %1 process. Please check your installation.", processname ), i18n( "Certificate Manager Error" ) );
    return false;
  }
  return true;
}

void CertManager::startImportCRL( const QString& filename, bool isTempFile )
{
  assert( !mDirmngrProc );
  mImportCRLTempFile = isTempFile ? filename : QString();
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
    KMessageBox::error( this, i18n( "An error occurred when trying to clear the CRL cache. The output from DirMngr was:\n%1", mErrorbuffer ), i18n( "Certificate Manager Error" ) );
  else
    KMessageBox::information( this, i18n( "CRL cache cleared successfully." ), i18n( "Certificate Manager Information" ) );
  delete mDirmngrProc; mDirmngrProc = 0;
}

static void showDeleteError( QWidget * parent, const GpgME::Error & err ) {
  assert( err );
  const QString msg = i18n("<qt><p>An error occurred while trying to delete "
			   "the certificates:</p>"
			   "<p><b>%1</b></p></qt>",
      QString::fromLocal8Bit( err.asString() ) );
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
  for ( Q3PtrListIterator<Kleo::KeyListViewItem> it( mItemsToDelete ) ; it.current() ; ++it )
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
    ? i18np("Do you really want to delete this certificate and the %1 certificates it certified?",
	   "Do you really want to delete these %n certificates and the %1 certificates they certified?",
	   keys.size(), keysToDelete.size() - keys.size() )
    : i18np("Do you really want to delete this certificate?",
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
    QString reason = i18n("Operation not supported by the backend.");
    QString str = keys.size() == 1
                  ? i18n("<qt><p>An error occurred while trying to delete "
                         "the certificate:</p>"
                         "<p><b>%1</b><p></qt>", reason )
                  : i18n( "<qt><p>An error occurred while trying to delete "
                          "the certificates:</p>"
                          "<p><b>%1</b><p></qt>", reason );
    KMessageBox::error( this, str, i18n("Certificate Deletion Failed") );
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
      Q3PtrListIterator<Kleo::KeyListViewItem> it( mItemsToDelete );
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
  KDialog * dialog = new KDialog( this, i18n("Additional Information for Key"), KDialog::Close );
  dialog->setObjectName( "dialog" );
  dialog->setModal( false );
  dialog->setDefaultButton( KDialog::Close );

  CertificateInfoWidgetImpl * top = new CertificateInfoWidgetImpl( item->key(), isRemote(), dialog );
  dialog->setMainWidget( top );
  // </UGH>
  connect( top, SIGNAL(requestCertificateDownload(const QString&, const QString&)),
	   SLOT(slotStartCertificateDownload(const QString&, const QString&)) );
  dialog->show();
}

void CertManager::slotViewDetails()
{
  Q3PtrList<Kleo::KeyListViewItem> items = mKeyListView->selectedItems();
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
  Q3PtrList<Kleo::KeyListViewItem> items = mKeyListView->selectedItems();
  if ( items.isEmpty() )
    return;

  QStringList fingerprints;
  for ( Q3PtrListIterator<Kleo::KeyListViewItem> it( items ) ; it.current() ; ++it )
    if ( !it.current()->key().isNull() )
      if ( const char * fpr = it.current()->key().primaryFingerprint() )
	fingerprints.push_back( fpr );

  startCertificateExport( fingerprints );
}

static void showCertificateExportError( QWidget * parent, const GpgME::Error & err ) {
  assert( err );
  const QString msg = i18n("<qt><p>An error occurred while trying to export "
			   "the certificate:</p>"
			   "<p><b>%1</b></p></qt>",
      QString::fromLocal8Bit( err.asString() ) );
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
static bool checkOverwrite( const KUrl& url, bool& overwrite, QWidget* w )
{
  if ( KIO::NetAccess::exists( url, false /*dest*/, w ) ) {
    if ( KMessageBox::Cancel ==
         KMessageBox::warningContinueCancel(
                                            w,
                                            i18n( "A file named \"%1\" already exists. "
                                                  "Are you sure you want to overwrite it?", url.prettyURL() ),
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

  kDebug() << "CertManager::slotCertificateExportResult(): got " << data.size() << " bytes" << endl;

  const QString filter = QString("*.pem|") + i18n("ASCII Armored Certificate Bundles (*.pem)");
  const KUrl url = KFileDialog::getOpenURL( QString(),
                                      filter,
                                      this,
                                      i18n( "Save Certificate" ) );
  if ( !url.isValid() )
    return;

  bool overwrite = false;
  if ( !checkOverwrite( url, overwrite, this ) )
    return;

  KIO::Job* uploadJob = KIO::storedPut( data, url, -1, overwrite, false /*resume*/ );
  uploadJob->setWindow( this );
  connect( uploadJob, SIGNAL( result( KJob* ) ),
           this, SLOT( slotUploadResult( KJob* ) ) );
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
			   "<p><b>%1</b></p></qt>",
      QString::fromLocal8Bit( err.asString() ) );
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

  const GpgME::Error err = job->start( QStringList( fingerprint ) );
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

  kDebug() << "CertManager::slotSecretKeyExportResult(): got " << data.size() << " bytes" << endl;
  QString filter = QString("*.p12|") + i18n("PKCS#12 Key Bundle (*.p12)");
  KUrl url = KFileDialog::getOpenURL( QString(),
                                      filter,
                                      this,
                                      i18n( "Save Certificate" ) );
  if ( !url.isValid() )
    return;

  bool overwrite = false;
  if ( !checkOverwrite( url, overwrite, this ) )
    return;

  KIO::Job* uploadJob = KIO::storedPut( data, url, -1, overwrite, false /*resume*/ );
  uploadJob->setWindow( this );
  connect( uploadJob, SIGNAL( result( KJob* ) ),
           this, SLOT( slotUploadResult( KJob* ) ) );
}

void CertManager::slotUploadResult( KJob* job )
{
  if ( job->error() )
    static_cast<KIO::Job*>(job)->showErrorDialog();
}

void CertManager::slotDropped(const KUrl::List& lst)
{
  mURLsToImport = lst;
  if ( !lst.empty() )
    importNextURLOrRedisplay();
}

void CertManager::importNextURLOrRedisplay()
{
  if ( !mURLsToImport.empty() ) {
    // We can only import them one by one, otherwise the jobs would run into each other
    KUrl url = mURLsToImport.front();
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
