#include "certmanager.h"
#include <qtextedit.h>
#include <kmenubar.h>
#include <kaction.h>
#include <qapplication.h>
#include <klocale.h>
#include "certbox.h"
#include "certitem.h"
#include "agent.h"
#include <qwizard.h>
#include <qgrid.h>
#include "certificatewizardimpl.h"
#include <cryptplugwrapper.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <dcopclient.h>
#include <qlineedit.h>

extern CryptPlugWrapper* pWrapper;

CertManager::CertManager( QWidget* parent, const char* name ) :KMainWindow( parent, name )
{
  KMenuBar* bar = menuBar();

  // File Menu
  QPopupMenu* fileMenu = new QPopupMenu( bar, "fileMenu" );
  bar->insertItem( i18n("&File"), fileMenu );
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

  // Extend Cerificate
  KAction* extendCert = new KAction( i18n("Extend Certificate"), QIconSet(), 0, this, SLOT( extendCertificate() ),
                                     actionCollection(), "extendCert" );
  extendCert->plug( certMenu );
  extendCert->setEnabled( false );

  // CRL menu --------------------------------------------------
  QPopupMenu* crlMenu = new QPopupMenu( bar, "crlMenu" );
  bar->insertItem( i18n( "CRL" ), crlMenu );

  // Import
  QPopupMenu* crlImportMenu = new QPopupMenu( crlMenu, "crlImportMenu" );
  crlMenu->insertItem( i18n("Import" ), crlImportMenu );

  // Import from file
  KAction* importCRLFromFile = new KAction( i18n("from file"), QIconSet(), 0, this, SLOT( importCRLFromFile() ),
                                            actionCollection(), "importCRLFromFile" );
  importCRLFromFile->plug( crlImportMenu );
  importCRLFromFile->setEnabled( false );

  // Import from file
  KAction* importCRLFromLDAP = new KAction( i18n("from LDAP"), QIconSet(), 0, this, SLOT( importCRLFromLDAP() ),
                                            actionCollection(), "importCRLFromLDAP" );
  importCRLFromLDAP->plug( crlImportMenu );
  importCRLFromLDAP->setEnabled( false );

  // Main Window --------------------------------------------------
  _certBox = new CertBox( this, "certBox" );
  setCentralWidget( _certBox );

  loadCertificates();
}

/**
   This is an internal function, which loads the users current certificates
*/
void CertManager::loadCertificates()
{
  // These are just some demonstration data
  Agent* root = new Agent( "Root Agent", 0, this );
  Agent* sub = new Agent( "Sub Agent", root, this );
  Agent* subsub = new Agent( "SubSub Agent", sub, this );
  CertItem* item = new CertItem( "JesperPedersenKlaralv", "Verein der Schornsteinfeger", "DK", subsub, _certBox );
  item->addKey( "email", "blackie@blackie.dk" );
  item->addKey( "email", "blackie@klabberdabberdalens-datakonsålt" );
}

/**
   This slot is invoked when the user selects "New certificate"
*/
void CertManager::newCertificate()
{
  CertificateWizardImpl* wizard = new CertificateWizardImpl( this );
  if( wizard->exec() == QDialog::Accepted ) {
      qDebug( "Cert Wizard was Accepted" );
      // Ask KMail to send this key to the CA.
      DCOPClient* dcopClient = kapp->dcopClient();
      QByteArray data;
      QDataStream arg( data, IO_WriteOnly );
      arg << wizard->caEmailED->text();
      arg << QString( "" ); // CC:
      arg << QString( "" ); // BCC:
      arg << i18n( "Certificate Request" ); // Subject:
      arg << i18n( "Please process this certificate request" ); // Body
      arg << 0; // not hidden
      arg << QString( "smime.p10" ); // attachment name
      arg << QCString( "base64" ); // content transfer encoding
      arg << QCString( wizard->keyData() );
      arg << QCString( "application" ); // attachment type
      arg << QCString( "pkcs10" ); // attachment subtype
      arg << QCString( "" ); // attachment param attr
      arg << QString( "" ); // attachment param value
      arg << QCString( "attachment" ); // attachment content disposition
      QCString replyType;
      QByteArray replyData;
      if( !dcopClient->send( "kmail*", "KMailIface",
                             "openComposer(QString,QString,QString,QString,QString,int,QString,QCString,QCString,QCString,QCString,QCString,QString,QCString)", data ) ) {
          KMessageBox::error( this,
                              i18n( "DCOP Communication Error, can't ask KMail to send certificate" ),
                              i18n( "Certificate Manager Error" ) );
          return;
      } else
          qDebug( "DCOP message sent" );
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
   It will send an extension request fir the selected certificates
*/
void CertManager::extendCertificate()
{
  qDebug("Not Yet Implemented");
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
