#include "certmanager.h"

#include "certbox.h"
#include "certitem.h"
#include "agent.h"
#include "certificatewizardimpl.h"
#include "crlview.h"

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
#include <kcombobox.h>

// Qt
#include <qtextedit.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qwizard.h>
#include <qgrid.h>
#include <qcursor.h>

extern CryptPlugWrapper* pWrapper;

/*
static const int ID_LINEEDIT = 1;
static const int ID_BUTTON   = 2;
static const int ID_COMBO    = 3;
static const int ID_LABEL    = 10;
*/

class LabelAction : public KAction {
public:
  LabelAction( const QString& text,  KActionCollection* parent, const char* name )
    : KAction( text, QIconSet(), KShortcut(), 0, 0, parent, name ) {}
  virtual int plug( QWidget *widget, int index = -1 ) {
    if (kapp && !kapp->authorizeKAction(name()))
      return -1;
    if ( widget->inherits( "KToolBar" ) ) {
      KToolBar *bar = (KToolBar *)widget;      
      int id_ = getToolButtonID();      
      QLabel* label = new QLabel( text(), bar, "kde toolbar widget" );
      bar->insertWidget( id_, label->width(), label, index );      
      addContainer( bar, id_ );      
      connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );      
      return containerCount() - 1;
    }
    
    int containerId = KAction::plug( widget, index );
    
    return containerId;
  }
};

class LineEditAction : public KAction {
public:
  LineEditAction( const QString& text,  KActionCollection* parent, QObject* receiver, 
		  const char* member, const char* name )
    : KAction( text, QIconSet(), KShortcut(), 0, 0, parent, name ), 
      _le(0), _receiver(receiver),_member(member) {}
  virtual int plug( QWidget *widget, int index = -1 ) {
    if (kapp && !kapp->authorizeKAction(name()))
      return -1;
    if ( widget->inherits( "KToolBar" ) ) {
      KToolBar *bar = (KToolBar *)widget;      
      int id_ = getToolButtonID();      
      // The toolbar trick doesn't seem to work for lineedits
      //_le = new QLineEdit( bar, "kde toolbar widget" );
      _le = new QLineEdit( bar );
      bar->insertWidget( id_, _le->width(), _le, index );      
      bar->setStretchableWidget( _le );
      addContainer( bar, id_ );      
      connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );      
      connect( _le, SIGNAL( returnPressed() ), _receiver, _member );      
      return containerCount() - 1;
    }
    
    int containerId = KAction::plug( widget, index );
    
    return containerId;
  }

  void clear() { _le->setText(""); }  
  void focusAll() { _le->selectAll(); _le->setFocus(); }
  QString text() { return _le->text(); }
  void setText( const QString& txt ) { _le->setText(txt); }

private:
  QLineEdit* _le;
  QObject* _receiver;
  const char* _member;
};

class ComboAction : public KAction {
public:
  ComboAction( const QStringList& lst,  KActionCollection* parent, QObject* receiver, 
		  const char* member, const char* name )
    : KAction( QString::null, QIconSet(), KShortcut(), 0, 0, parent, name ), 
      _lst(lst), _receiver(receiver), _member(member) {}
  virtual int plug( QWidget *widget, int index = -1 ) {
    if (kapp && !kapp->authorizeKAction(name()))
      return -1;
    if ( widget->inherits( "KToolBar" ) ) {
      KToolBar *bar = (KToolBar *)widget;      
      int id_ = getToolButtonID();   
      bar->insertCombo( _lst, id_, false, SIGNAL( highlighted(int) ), _receiver, _member ); 
      addContainer( bar, id_ );      
      connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );      
      return containerCount() - 1;
    }
    
    int containerId = KAction::plug( widget, index );
    
    return containerId;
  }
private:
  QStringList _lst;
  QObject* _receiver;
  const char* _member;
};


CertManager::CertManager( bool remote, const QString& query, 
			  QWidget* parent, const char* name )
  : KMainWindow( parent, name ),
    dirmngrProc(0),
    _crlView(0),
    _toolbar(0),
    _leAction(0),
    _comboAction(0),
    _findAction(0),
    _certBox(0),
    _remote( remote )
{
  (void)KStdAction::redisplay( this, SLOT( loadCertificates() ), actionCollection());
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

  // Import Certificates
  // Import from file
  (void)new KAction( i18n("Certificate..."), QIconSet(),
                                             0, this,
                                             SLOT( importCertFromFile() ),
                                             actionCollection(),
                                             "importCertFromFile" );
  // CRLs
  // Import from file
  KAction* importCRLFromFile = new KAction( i18n("CRL..."), QIconSet(), 0, this, SLOT( importCRLFromFile() ),
                                            actionCollection(), "importCRLFromFile" );
  QStringList lst;
  lst << "dirmngr" << "-h";
  importCRLFromFile->setEnabled( checkExec( lst ) );

  // View CRLs
  KAction* viewCRLs = new KAction( i18n("CRL Cache..."), QIconSet(), 0, this, SLOT( slotViewCRLs() ),
				   actionCollection(), "viewCRLs");
  viewCRLs->setEnabled( importCRLFromFile->isEnabled() ); // we also need dirmngr for this
  

  // Toolbar
  _toolbar = toolBar( "mainToolBar" );

  (new LabelAction( i18n("Look For"), actionCollection(), "label_action"))->plug( _toolbar );
  _leAction = new LineEditAction( QString::null, actionCollection(), this, 
				  SLOT( loadCertificates() ), 
				  "query_lineedit_action");
  _leAction->plug( _toolbar );

  lst.clear();
  lst << i18n("in local certificates") << i18n("in external certificates");
  _comboAction = new ComboAction( lst, actionCollection(), this, SLOT( slotToggleRemote(int) ), 
		       "location_combo_action");
  _comboAction->plug( _toolbar );

  _findAction = new KAction( i18n("Find"), "find", 0, this, SLOT( loadCertificates() ),
                                            actionCollection(), "find" );
  _findAction->plug( _toolbar );

  createGUI();

  // Main Window --------------------------------------------------
  _certBox = new CertBox( this, "certBox" );
  setCentralWidget( _certBox );

  _leAction->setText(query);
  if ( !_remote || !query.isEmpty() )
    loadCertificates();
}

bool CertManager::checkExec( const QStringList& args )
{
  KProcess testProc;
  testProc << args;
  return testProc.start( KProcess::DontCare );
}

CertItem* CertManager::fillInOneItem( CertBox* lv, CertItem* parent, 
				      const CryptPlugWrapper::CertificateInfo& info )
{
  if( parent ) {
    //qDebug("New with parent");
    return new CertItem( info,
			 0, this, parent );  
  } else {
    //qDebug("New root");
    return new CertItem( info,			
			 0, this, lv );
  }
}

void CertManager::slotToggleRemote( int idx )
{
  _remote = idx==0?false:true;
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
  _leAction->setEnabled( false );
  _comboAction->setEnabled( false );
  _findAction->setEnabled( false );

  // Clear display
  _certBox->clear();

  QString text = _leAction->text();

  //qDebug("About to query plugin");
  bool truncated;
  if( text.isEmpty() ) {
    _certList = pWrapper->listKeys(QString::null, _remote, &truncated );
  } else {
    _certList = pWrapper->listKeys(text, _remote, &truncated );
  }
  //qDebug("Done");
  
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
  _leAction->setEnabled( true );
  _comboAction->setEnabled( true );
  _findAction->setEnabled( true );

  _leAction->focusAll();
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
	QString info;
	int retval = importCertificateFromFile( certFilename, &info );
	if( retval ) {
	  KMessageBox::error( this, i18n( "An error occurred when trying to import the certificate file. The error code from CryptPlug was %1 and output was: %2" ).arg(retval).arg(info), i18n( "Certificate Manager Error" ) );	  
	} else {
	  KMessageBox::information( this, i18n( "Certificate file imported successfully. Additional info: %1" ).arg(info), i18n( "Certificate Imported" ) );	  
	}
    }
}


/**
   This slot is called when the dirmngr process that imports a
   certificate file exists.
*/
void CertManager::slotDirmngrExited()
{
    if( !dirmngrProc->normalExit() )
        KMessageBox::error( this, i18n( "The Dirmngr process that tried to import the CRL file ended prematurely because of an unexpected error." ), i18n( "Certificate Manager Error" ) );
    else
        if( dirmngrProc->exitStatus() )
            KMessageBox::error( this, i18n( "An error occurred when trying to import the CRL file. The output from Dirmngr was: ") + errorbuffer, i18n( "Certificate Manager Error" ) );
        else
            KMessageBox::information( this, i18n( "CRL file imported successfully." ), i18n( "Certificate Manager Error" ) );

    if( dirmngrProc )
        delete dirmngrProc;
}

/**
   This slot will import CRLs from a file.
*/
void CertManager::importCRLFromFile()
{
  QString filename = KFileDialog::getOpenFileName( QString::null,
						       QString::null,
						       this,
						       i18n( "Select CRL File" ) );
  
  if( !filename.isEmpty() ) {
    dirmngrProc = new KProcess();
    *dirmngrProc << "dirmngr";
    *dirmngrProc << "--load-crl" << filename;
    errorbuffer = "";
    connect( dirmngrProc, SIGNAL( processExited( KProcess* ) ),
	     this, SLOT( slotDirmngrExited() ) );
    connect( dirmngrProc, SIGNAL( receivedStderr(KProcess*, char*, int)  ),
	     this, SLOT( slotStderr( KProcess*, char*, int ) ) );
    if( !dirmngrProc->start( KProcess::NotifyOnExit, KProcess::Stderr ) ) { 
      KMessageBox::error( this, i18n( "Unable to start dirmngr process. Please check your installation." ), i18n( "Certificate Manager Error" ) );
      delete dirmngrProc;
      dirmngrProc = 0;
    }
  }
}

void CertManager::slotStderr( KProcess*, char* buf, int len )
{
  errorbuffer += QString::fromLocal8Bit( buf, len );
}

/**
   This slot will import CRLs from an LDAP server.
*/
void CertManager::importCRLFromLDAP()
{
  qDebug("Not Yet Implemented");
}

void CertManager::slotViewCRLs()
{
  if( _crlView == 0 ) {
    _crlView = new CRLView( this );
  }
  _crlView->show();
  _crlView->slotUpdateView();
}

int CertManager::importCertificateWithFingerprint( const QString& fingerprint, QString* info )
{
  qDebug("Importing certificate with fpr %s", fingerprint.latin1() );
  int retval = pWrapper->importCertificate( fingerprint, info );

  qDebug("importCertificate() returned %d", retval );

  // values > 0 are "real" GPGME errors
  if( retval > 0 ) return retval;
  if( haveCertificate( fingerprint ) ) {
    // It seems everyting went OK!
  } else {
    // Everything went OK, but the certificate wasn't imported
    // retval was probably -1 here (= GPGME_EOF)
    retval = -42;
  }
  if( !isRemote() ) loadCertificates();
  return retval;
}

bool CertManager::haveCertificate( const QString& fingerprint ) 
{
  bool truncated;
  CryptPlugWrapper::CertificateInfoList lst = pWrapper->listKeys( fingerprint, false, &truncated );
  return !lst.isEmpty();
}

int CertManager::importCertificateFromFile( const QString& filename, QString* info )
{
  QFile f( filename );
  if( !f.open( IO_ReadOnly ) ) {
    if( info ) *info = i18n( "Error opening file %1" ).arg( filename );
    return -1;
  }
  QByteArray data = f.readAll();

  int retval = pWrapper->importCertificate( data.data(), data.size(), info );

  qDebug("importCertificate() returned %d", retval );

  return retval;
}

#include "certmanager.moc"
