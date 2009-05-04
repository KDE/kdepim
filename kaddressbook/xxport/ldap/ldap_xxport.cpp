#include "ldap_xxport.h"
#include "ldapsearchdialog.h"

//#include <QtCore/QFile>
//#include <QtCore/QTextStream>

//#include <kabc/ldifconverter.h>
//#include <kcodecs.h>
//#include <kfiledialog.h>
//#include <kio/netaccess.h>
//#include <klocale.h>
//#include <kmessagebox.h>
//#include <ktemporaryfile.h>
//#include <kurl.h>
#include <kdebug.h>

LDAPXXPort::LDAPXXPort( QWidget *parentWidget )
  : XXPort( parentWidget )
{
}

KABC::Addressee::List LDAPXXPort::importContacts() const
{
  kDebug()<<"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";


  KABC::Addressee::List contacts;

  KABC::AddressBook *ab = 0;
  KABCore *core = 0;
  LDAPSearchDialog dlg(ab, core, parentWidget());
  dlg.exec();
  contacts = dlg.m_result;

/*
  const QString fileName = KFileDialog::getOpenFileName( QDir::homePath(), "text/x-ldif", 0 );
  if ( fileName.isEmpty() )
    return contacts;

  QFile file( fileName );
  if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
    const QString msg = i18n( "<qt>Unable to open <b>%1</b> for reading.</qt>", fileName );
    KMessageBox::error( parentWidget(), msg );
    return contacts;
  }

  QTextStream stream( &file );
  stream.setCodec( "ISO 8859-1" );

  const QString wholeFile = stream.readAll();
  const QDateTime dtDefault = QFileInfo( file ).lastModified();
  file.close();

  KABC::LDAPConverter::LDAPToAddressee( wholeFile, contacts, dtDefault );
*/
  return contacts;
}

bool LDAPXXPort::exportContacts( const KABC::Addressee::List &contacts ) const
{
    Q_UNUSED(contacts);
    return false;
}
