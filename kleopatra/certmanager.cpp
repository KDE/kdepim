#include "certmanager.h"

#include "certbox.h"
#include "certitem.h"
#include "agent.h"
#include "certificatewizardimpl.h"

// kdenetwork
#include <cryptplugwrapper.h>

// KDE
#include <kmenubar.h>
#include <kurlrequester.h>
#include <kfiledialog.h>
#include <kprocess.h>
#include <kaction.h>
#include <kapplication.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <dcopclient.h>
#include <ktoolbar.h>
#include <klineedit.h>
#include <kstatusbar.h>

// Qt
#include <qtextedit.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qwizard.h>
#include <qgrid.h>
#include <qcursor.h>

extern CryptPlugWrapper* pWrapper;

static const int ID_LINEEDIT = 1;
static const int ID_BUTTON   = 2;


CertManager::CertManager( bool remote, const QString& query, QWidget* parent, const char* name ) :
    KMainWindow( parent, name ),
    gpgsmProc( 0 ), _remote( remote )
{
  KMenuBar* bar = menuBar();

  // File Menu
  QPopupMenu* fileMenu = new QPopupMenu( bar, "fileMenu" );
  bar->insertItem( i18n("&File"), fileMenu );

  KAction* update = KStdAction::redisplay( this, SLOT( loadCertificates() ), actionCollection());
  update->plug( fileMenu );

  KToggleAction* remoteaction = new KToggleAction( i18n("Remote lookup"), KShortcut(), this);
  connect( remoteaction, SIGNAL( toggled(bool) ), this, SLOT( slotToggleRemote( bool ) ) );
  remoteaction->setChecked( _remote );
  remoteaction->plug( fileMenu );
  
  fileMenu->insertSeparator();

  KAction* quit = KStdAction::quit( this, SLOT( quit() ), actionCollection());
  quit->plug( fileMenu );


  // Certificate Menu --------------------------------------------------
  QPopupMenu* certMenu = new QPopupMenu( bar, "certMenu" );
  bar->insertItem( i18n("Certificates"), certMenu );

  // New Certificate
  KAction* newCert = new KAction( i18n("New Certificate"), QIconSet(), 0, this, SLOT( newCertificate() ),
                                  actionCollection(), "newCert" );
  newCert->plug( certMenu );

  // Revoke Certificate
  KAction* revokeCert = new KAction( i18n("Revoke Certificate"), QIconSet(), 0, this, SLOT( revokeCertificate() ),
                                     actionCollection(), "revokeCert" );
  revokeCert->plug( certMenu );
  revokeCert->setEnabled( false );

  // Extend Certificate
  KAction* extendCert = new KAction( i18n("Extend Certificate"), QIconSet(), 0, this, SLOT( extendCertificate() ),
                                     actionCollection(), "extendCert" );
  extendCert->plug( certMenu );
  extendCert->setEnabled( false );

  // Import Certificates
  QPopupMenu* certImportMenu = new QPopupMenu( certMenu, "certImportMenu" );
  certMenu->insertItem( i18n("&Import" ), certImportMenu );

  // Import from file
  KAction* importCertFromFile = new KAction( i18n("From &File..."), QIconSet(),
                                             0, this,
                                             SLOT( importCertFromFile() ),
                                             actionCollection(),
                                             "importCertFromFile" );
  importCertFromFile->plug( certImportMenu );
  importCertFromFile->setEnabled( true );


  // CRL menu --------------------------------------------------
  QPopupMenu* crlMenu = new QPopupMenu( bar, "crlMenu" );
  bar->insertItem( i18n( "CRL" ), crlMenu );

  // Import CRLs
  QPopupMenu* crlImportMenu = new QPopupMenu( crlMenu, "crlImportMenu" );
  crlMenu->insertItem( i18n("&Import" ), crlImportMenu );

  // Import from file
  KAction* importCRLFromFile = new KAction( i18n("From &File..."), QIconSet(), 0, this, SLOT( importCRLFromFile() ),
                                            actionCollection(), "importCRLFromFile" );
  importCRLFromFile->plug( crlImportMenu );
  importCRLFromFile->setEnabled( false );

  // Import from file
  KAction* importCRLFromLDAP = new KAction( i18n("From &LDAP"), QIconSet(), 0, this, SLOT( importCRLFromLDAP() ),
                                            actionCollection(), "importCRLFromLDAP" );
  importCRLFromLDAP->plug( crlImportMenu );
  importCRLFromLDAP->setEnabled( false );

  // Toolbar
  _toolbar = toolBar( "mainToolBar" );

  _toolbar->insertLined( query, ID_LINEEDIT, SIGNAL( returnPressed() ), this, 
			 SLOT( loadCertificates() ) );
  _toolbar->setItemAutoSized( ID_LINEEDIT, true );

  KAction* find = KStdAction::find( this, SLOT( loadCertificates() ), actionCollection());
  _toolbar->insertButton( find->icon(), ID_BUTTON, SIGNAL( clicked() ), this, 
			  SLOT( loadCertificates() ), 
			  true, i18n("Search") );
  _toolbar->alignItemRight( ID_BUTTON, true );

  // Main Window --------------------------------------------------
  _certBox = new CertBox( this, "certBox" );
  setCentralWidget( _certBox );

  if( !query.isEmpty() ) loadCertificates();
}

CertItem* CertManager::fillInOneItem( CertBox* lv, CertItem* parent, 
				      const CryptPlugWrapper::CertificateInfo& info )
{
  if( parent ) {
    //qDebug("New with parent");
    return new CertItem( /*info.userid[0].stripWhiteSpace(),
			 info.serial.stripWhiteSpace(), 
			 info.issuer.stripWhiteSpace(),
			 info.dn["CN"], 
			 info.dn["L"], 
			 info.dn["O"], 
			 info.dn["OU"], 
			 info.dn["C"],
			 info.dn["1.2.840.113549.1.9.1"], 
			 info.created,info.expire,
			 info.sign, info.encrypt, info.certify,*/
			 info,
			 0, this, parent );  
  } else {
    //qDebug("New root");
    return new CertItem( /*info.userid[0].stripWhiteSpace(), 
			 info.serial.stripWhiteSpace(),
			 info.issuer.stripWhiteSpace(),
			 info.dn["CN"], 
			 info.dn["L"], 
			 info.dn["O"], 
			 info.dn["OU"], 
			 info.dn["C"],
			 info.dn["1.2.840.113549.1.9.1"], 
			 info.created,info.expire,
			 info.sign, info.encrypt, info.certify,*/
			 info,			
			 0, this, lv );
  }
}

void CertManager::slotToggleRemote(bool b)
{
  _remote = b;
  // Clear display
  _certBox->clear();
}

/**
   This is an internal function, which loads the certificates that 
   match the current query, local or remote.
*/
void CertManager::loadCertificates()
{
  // These are just some demonstration data
  /*
  Agent* root = new Agent( "Root Agent", 0, this );
  Agent* sub = new Agent( "Sub Agent", root, this );
  Agent* subsub = new Agent( "SubSub Agent", sub, this );
  */

  QApplication::setOverrideCursor( QCursor::WaitCursor );
  _toolbar->setItemEnabled( ID_LINEEDIT, false );
  _toolbar->setItemEnabled( ID_BUTTON, false );

  // Clear display
  _certBox->clear();

  QString text = _toolbar->getLinedText( ID_LINEEDIT ).stripWhiteSpace();

  qDebug("About to query plugin");
  bool truncated;
  if( text.isEmpty() ) {
    _certList = pWrapper->listKeys(QString::null, _remote, &truncated );
  } else {
    _certList = pWrapper->listKeys(text, _remote, &truncated );
  }
  qDebug("Done");
  
  if( truncated ) {
    //statusBar()->message();
    KMessageBox::information( this, i18n("The server returned truncated output.\nPlease use a more specific search string to get all results.") );
  } else {
    //statusBar()->message( i18n("Query OK") );
  }

  //lst = fillInListView( _certBox, 0, lst );
  
  for( CryptPlugWrapper::CertificateInfoList::Iterator it = _certList.begin(); 
       it != _certList.end(); ++it ) {
    //qDebug("New CertItem %s", (*it).userid.latin1() );
    fillInOneItem( _certBox, 0, *it );
  }
  _toolbar->setItemEnabled( ID_LINEEDIT, true );
  _toolbar->setItemEnabled( ID_BUTTON, true );
  KLineEdit* le = _toolbar->getLined( ID_LINEEDIT );
  le->selectAll();
  le->setFocus();
  QApplication::restoreOverrideCursor();
}

/**
   This slot is invoked when the user selects "New certificate"
*/
void CertManager::newCertificate()
{
  CertificateWizardImpl* wizard = new CertificateWizardImpl( this );
  if( wizard->exec() == QDialog::Accepted ) {
      if( wizard->sendToCARB->isChecked() ) {
          // Ask KMail to send this key to the CA.
          DCOPClient* dcopClient = kapp->dcopClient();
          QByteArray data;
          QDataStream arg( data, IO_WriteOnly );
          arg << wizard->caEmailED->text();
          arg << wizard->keyData();
          if( !dcopClient->send( "kmail*", "KMailIface",
                                 "sendCertificate(QString,QByteArray)", data ) ) {
              KMessageBox::error( this,
                                  i18n( "DCOP Communication Error, unable to send certificate using KMail" ) );
              return;
          }
      } else {
          // Store in file
          QFile file( wizard->storeUR->url() );
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


/**
   This slot is invoke dwhen the user selects Certificates/Import/From File.
*/
void CertManager::importCertFromFile()
{
    QString certFilename = KFileDialog::getOpenFileName( QString::null,
                                                         QString::null,
                                                         this,
                                                         i18n( "Select Certificate File" ) );

    if( !certFilename.isEmpty() ) {
        gpgsmProc = new KProcess();
        *gpgsmProc << "gpgsm";
        *gpgsmProc << "--import" << certFilename;
        connect( gpgsmProc, SIGNAL( processExited( KProcess* ) ),
                 this, SLOT( slotGPGSMExited() ) );
        if( !gpgsmProc->start() ) { // NotifyOnExit, NoCommunication
                                    // are defaults
            KMessageBox::error( this, i18n( "Couldn't start gpgsm process. Please check your installation." ), i18n( "Certificate Manager Error" ) );
            delete gpgsmProc;
            gpgsmProc = 0;
        }
    }
}


/**
   This slot is called when the gpgsm process that imports a
   certificate file exists.
*/
void CertManager::slotGPGSMExited()
{
    if( !gpgsmProc->normalExit() )
        KMessageBox::error( this, i18n( "The GPGSM process that tried to import the certificate file ended prematurely because of an unexpected error." ), i18n( "Certificate Manager Error" ) );
    else
        if( gpgsmProc->exitStatus() )
            KMessageBox::error( this, i18n( "An error occurred when trying to import the certificate file." ), i18n( "Certificate Manager Error" ) );
        else
            KMessageBox::information( this, i18n( "Certificate file imported successfully." ), i18n( "Certificate Manager Error" ) );

    if( gpgsmProc )
        delete gpgsmProc;
}


/**
   This slot will import CRLs from a file.
*/
void CertManager::importCRLFromFile()
{
  qDebug("Not Yet Implemented");
}

/**
   This slot will import CRLs from an LDAP server.
*/
void CertManager::importCRLFromLDAP()
{
  qDebug("Not Yet Implemented");
}

#include "certmanager.moc"
