/*  -*- mode: C++; c-file-style: "gnu" -*-
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

#include "certificatewizardimpl.h"
#include "certificateinfowidgetimpl.h"
#include "crlview.h"
#include "customactions.h"

// libkleopatra
#include <cryptplugwrapper.h>
#include <cryptplugfactory.h>
#include <kleo/downloadjob.h>
#include <kleo/importjob.h>
#include <kleo/deletejob.h>
#include <kleo/keylister.h>
#include <kleo/dn.h>

#include <ui/progressdialog.h>
#include <ui/progressbar.h>
#include <ui/keylistview.h>

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

// Qt
#include <qfontmetrics.h>

// other
#include <assert.h>

namespace {
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
    mRemote( remote )
{
  createStatusBar();
  createActions();

  createGUI();
  setAutoSaveSettings();

  // Main Window --------------------------------------------------
  mKeyListView = new Kleo::KeyListView( new ColumnStrategy(), this, "mKeyListView" );
  setCentralWidget( mKeyListView );

  connect( mKeyListView, SIGNAL(doubleClicked(Kleo::KeyListViewItem*,const QPoint&,int)),
	   SLOT(slotListViewItemActivated(Kleo::KeyListViewItem*)) );
  connect( mKeyListView, SIGNAL(returnPressed(Kleo::KeyListViewItem*)),
	   SLOT(slotListViewItemActivated(Kleo::KeyListViewItem*)) );

  mLineEditAction->setText(query);
  if ( !mRemote || !query.isEmpty() )
    slotStartCertificateListing();

  if ( !import.isEmpty() )
    slotImportCertFromFile( import );

  updateStatusBarLabels();
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


void CertManager::createActions() {
  (void)KStdAction::redisplay( this, SLOT(slotStartCertificateListing()),
			       actionCollection() );
  (void)KStdAction::quit( this, SLOT( quit() ), actionCollection());

  // New Certificate
  (void)new KAction( i18n("New Certificate"), QIconSet(), 0, this, SLOT( newCertificate() ),
		     actionCollection(), "newCert" );
  // Revoke Certificate
  KAction* revokeCert = new KAction( i18n("Revoke Certificate"), QIconSet(), 0, this, SLOT( revokeCertificate() ),
                                     actionCollection(), "revokeCert" );
  revokeCert->setEnabled( false );

  // Extend Certificate
  KAction* extendCert = new KAction( i18n("Extend Certificate"), QIconSet(), 0, this, SLOT( extendCertificate() ),
                                     actionCollection(), "extendCert" );
  extendCert->setEnabled( false );

  // Delete Certificate
  (void)new KAction( i18n("Delete Certificate"), "editdelete", Key_Delete, this, 
                     SLOT(slotDeleteCertificate()), actionCollection(), "delCert" );

  // Import Certificates
  // Import from file
  (void)new KAction( i18n("Certificate..."), QIconSet(),
                                             0, this,
                                             SLOT(slotImportCertFromFile()),
                                             actionCollection(),
                                             "importCertFromFile" );
  // CRLs
  // Import from file
  KAction* importCRLFromFile = new KAction( i18n("CRL..."), QIconSet(), 0, this, SLOT( importCRLFromFile() ),
                                            actionCollection(), "importCRLFromFile" );

  QString dirmngr = KStandardDirs::findExe( "gpgsm" );
  bool dirMngrFound = !dirmngr.isEmpty();
  importCRLFromFile->setEnabled( dirMngrFound );

  // View CRLs
  KAction* viewCRLs = new KAction( i18n("CRL Cache..."), QIconSet(), 0, this, SLOT( slotViewCRLs() ),
				   actionCollection(), "viewCRLs");
  viewCRLs->setEnabled( dirMngrFound ); // we also need dirmngr for this

  // Toolbar
  KToolBar * _toolbar = toolBar( "mainToolBar" );

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
  createStandardStatusBarAction();
}

void CertManager::slotEditKeybindings() {
  KKeyDialog::configure( actionCollection(), true );
}

void CertManager::slotToggleRemote( int idx ) {
  mRemote = idx != 0;
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
  mLineEditAction->setEnabled( false );
  mComboAction->setEnabled( false );
  mFindAction->setEnabled( false );

  // Clear display
  mKeyListView->clear();

  const QString query = mLineEditAction->text();

  Kleo::KeyListJob * job =
    Kleo::CryptPlugFactory::instance()->smime()->keyListJob( mRemote );
  assert( job );

  statusBar()->message( i18n("Fetching keys...") );

  connect( job, SIGNAL(nextKey(const GpgME::Key&)),
	   mKeyListView, SLOT(slotAddKey(const GpgME::Key&)) );
  connect( job, SIGNAL(progress(const QString&,int,int,int)),
	   mProgressBar, SLOT(slotProgress(const QString&,int,int,int)) );
  connect( job, SIGNAL(done()), mProgressBar, SLOT(reset()) );
  connect( job, SIGNAL(result(const GpgME::KeyListResult&)),
	   this, SLOT(slotKeyListResult(const GpgME::KeyListResult&)) );

  const GpgME::Error err = job->start( query );
  if ( err )
    showKeyListError( this, err );

  mProgressBar->setProgress( 0, 0 ); // enable busy indicator
}


void CertManager::slotKeyListResult( const GpgME::KeyListResult & res ) {  
  if ( res.error() )
    showKeyListError( this, res.error() );
  else if ( res.isTruncated() )
    KMessageBox::information( this,
			      i18n("The server returned truncated output.\n"
				   "Please use a more specific search string "
				   "to get all results.") );

  mLineEditAction->setEnabled( true );
  mComboAction->setEnabled( true );
  mFindAction->setEnabled( true );

  mLineEditAction->focusAll();
  updateStatusBarLabels();
  statusBar()->message( i18n("Done."), 4000 );
}

void CertManager::updateStatusBarLabels() {
  mStatusLabel->setText( i18n( "%n Key.",
			       "%n Keys.", mKeyListView->childCount() ) );
}

/**
  This slot is invoked when the user selects "New certificate"
 */
void CertManager::newCertificate()
{
  CertificateWizardImpl* wizard = new CertificateWizardImpl( this );
  if( wizard->exec() == QDialog::Accepted ) {
    if( wizard->sendToCA() ) {
          // Ask KMail to send this key to the CA.
          DCOPClient* dcopClient = kapp->dcopClient();
          QByteArray data;
          QDataStream arg( data, IO_WriteOnly );
          arg << wizard->caEMailAddress();
          arg << wizard->keyData();
          if( !dcopClient->send( "kmail*", "KMailIface",
                                 "sendCertificate(QString,QByteArray)", data ) ) {
              KMessageBox::error( this,
                                  i18n( "DCOP Communication Error, unable to send certificate using KMail" ) );
              return;
          }
      } else {
          // Store in file
          QFile file( wizard->saveFileUrl() );
          if( file.open( IO_WriteOnly ) ) {
              file.writeBlock( wizard->keyData().data(),
                               wizard->keyData().count() );
              file.close();
          } else {
              KMessageBox::error( this,
                                  i18n( "Could not open output file for writing" ) );
              return;
          }

      }
  }
}


/**
   This slot is invoked when the user chooses File->Quit
*/
void CertManager::quit()
{
  qApp->quit();
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
   This slot is invoke dwhen the user selects Certificates/Import/From File.
*/
void CertManager::slotImportCertFromFile()
{
  slotImportCertFromFile( KFileDialog::getOpenFileName( QString::null, QString::null, this,
							i18n( "Select Certificate File" ) ) );
}

void CertManager::slotImportCertFromFile( const QString & certFilename )
{
  if ( certFilename.isEmpty() )
    return;

  QFile f( certFilename );
  if( !f.open( IO_ReadOnly ) ) {
    KMessageBox::error( this,
			i18n( "<qt><p>An error occurred while trying to import "
			      "certificate file <b>%1</b>:</p>"
			      "<p><b>%1</b></p></qt>" ).arg( certFilename ).arg( i18n("File does not exist or can't be read") ),
			i18n( "Certificate Manager Error" ) );
    return;
  }

  startCertificateImport( f.readAll() );
}

static void showCertificateDownloadError( QWidget * parent, const GpgME::Error & err ) {
  assert( err );
  const QString msg = i18n( "<qt><p>An error occurred while trying "
			    "to download the certificate:</p>"
			    "<p><b>%1</b></p></qt>" )
    .arg( QString::fromLocal8Bit( err.asString() ) );

  KMessageBox::error( parent, msg, i18n( "Certificate Download Failed" ) );
}

void CertManager::slotStartCertificateDownload( const QString & fingerprint ) {
  if ( fingerprint.isEmpty() )
    return;

  Kleo::DownloadJob * job =
    Kleo::CryptPlugFactory::instance()->smime()->downloadJob( false /* no armor */ );
  assert( job );

  connect( job, SIGNAL(result(const GpgME::Error&,const QByteArray&)),
	   SLOT(slotCertificateDownloadResult(const GpgME::Error&,const QByteArray&)) );

  const GpgME::Error err = job->start( fingerprint );
  if ( err )
    showCertificateDownloadError( this, err );
  else
    (void)new Kleo::ProgressDialog( job, i18n("Fetching certificate from server"), this );
}

void CertManager::slotCertificateDownloadResult( const GpgME::Error & err, const QByteArray & keyData ) {
  if ( err )
    showCertificateDownloadError( this, err );
  else
    startCertificateImport( keyData );
}

static void showCertificateImportError( QWidget * parent, const GpgME::Error & err ) {
  assert( err );
  const QString msg = i18n( "<qt><p>An error occurred while trying "
			    "to import the certificate:</p>"
			    "<p><b>%2</b></p></qt>" )
    .arg( QString::fromLocal8Bit( err.asString() ) );
  KMessageBox::error( parent, msg, i18n( "Certificate Import Failed" ) );
}

void CertManager::startCertificateImport( const QByteArray & keyData ) {
  Kleo::ImportJob * job = Kleo::CryptPlugFactory::instance()->smime()->importJob();
  assert( job );

  connect( job, SIGNAL(result(const GpgME::ImportResult&)),
	   SLOT(slotCertificateImportResult(const GpgME::ImportResult&)) );

  const GpgME::Error err = job->start( keyData );
  if ( err )
    showCertificateImportError( this, err );
  else
    (void)new Kleo::ProgressDialog( job, i18n("Importing Certificate"), this );
}

void CertManager::slotCertificateImportResult( const GpgME::ImportResult & res ) {
  if ( res.error() ) {
    showCertificateImportError( this, res.error() );
    return;
  }

  KMessageBox::information( this,
			    i18n( "<qt><p>Certificate imported successfully.</p>"
				  "<p>Additional info: %1</p></qt>" ).arg("FIXME"),
			    i18n( "Certificate Imported" ) );
  if ( !isRemote() )
    slotStartCertificateListing();
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
}

/**
   This slot will import CRLs from a file.
*/
void CertManager::importCRLFromFile() {
  QString filename = KFileDialog::getOpenFileName( QString::null,
						       QString::null,
						       this,
						       i18n( "Select CRL File" ) );
  
  if ( !filename.isEmpty() ) {
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
    }
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
  const QString msg = i18n("<qt><p>An error occured while trying to delete "
			   "the certificate:</p>"
			   "<p><b>%1</b></p></qt>")
    .arg( QString::fromLocal8Bit( err.asString() ) );
  KMessageBox::error( parent, msg, i18n("Certificate Deletion Failed") );
}

void CertManager::slotDeleteCertificate() {
  const Kleo::KeyListViewItem * item = mKeyListView->selectedItem();
  if ( !item || item->key().isNull() )
    return;

  Kleo::DeleteJob * job = Kleo::CryptPlugFactory::instance()->smime()->deleteJob();
  assert( job );

  connect( job, SIGNAL(result(const GpgME::Error&)),
	   SLOT(slotDeleteResult(const GpgME::Error&)) );

  const GpgME::Error err = job->start( item->key(), true );
  if ( err )
    showDeleteError( this, err );
  else
    (void)new Kleo::ProgressDialog( job, i18n("Deleting key"), this );
}

void CertManager::slotDeleteResult( const GpgME::Error & err ) {
  if ( err )
    return showDeleteError( this, err );

  // We should make sure that the selectedItem doesn't change between
  // slotDeleteCertificate() and this slot:
  delete mKeyListView->selectedItem();
  updateStatusBarLabels();
}


void CertManager::slotListViewItemActivated( Kleo::KeyListViewItem * item ) {
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


#include "certmanager.moc"
