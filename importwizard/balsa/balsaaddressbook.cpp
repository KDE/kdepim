/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>
  
  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.
  
  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.
  
  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "balsaaddressbook.h"
#include "importwizardutil.h"

#include <KABC/Addressee>
#include <kabc/contactgroup.h>
#include <KABC/LDIFConverter>

#include <KConfig>
#include <KConfigGroup>

#include <QDebug>
#include <QFile>
#include <QFileInfo>

BalsaAddressBook::BalsaAddressBook(const QString &filename, ImportWizard *parent)
  : AbstractAddressBook( parent )
{
    KConfig config(filename);
    const QStringList addressBookList = config.groupList().filter( QRegExp( "address-book-\\+d" ) );
    Q_FOREACH(const QString& addressbook,addressBookList) {
      KConfigGroup grp = config.group(addressbook);
      readAddressBook(grp);
    }

}

BalsaAddressBook::~BalsaAddressBook()
{

}

void BalsaAddressBook::readAddressBook(const KConfigGroup& grp)
{
  const QString type = grp.readEntry(QLatin1String("Type"));
  if(type.isEmpty()) {
    return;
  }
  const QString name = grp.readEntry(QLatin1String("Name"));

  if(type == QLatin1String("LibBalsaAddressBookLdap")) {
    ldapStruct ldap;
    ldap.dn = grp.readEntry(QLatin1String("BaseDN"));
    ldap.useTLS = (grp.readEntry(QLatin1String("EnableTLS")) == QLatin1String("true"));
    ldap.ldapUrl = KUrl(grp.readEntry(QLatin1String("Host")));
    ldap.port = ldap.ldapUrl.port();
    //TODO: verify
    const QString bookDN  = grp.readEntry(QLatin1String("BookDN"));
    ImportWizardUtil::mergeLdap(ldap);
  } else if(type == QLatin1String("LibBalsaAddressBookGpe")) {
      qDebug()<<" Import it !";
  } else if(type == QLatin1String("LibBalsaAddressBookLdif")) {
    const QString path = grp.readEntry(QLatin1String("Path"));
    if(!path.isEmpty()) {
      KABC::Addressee::List contacts;
      QFile file( path );
      QTextStream stream( &file );
      stream.setCodec( "ISO 8859-1" );

      const QString wholeFile = stream.readAll();
      const QDateTime dtDefault = QFileInfo( file ).lastModified();
      file.close();

      KABC::LDIFConverter::LDIFToAddressee( wholeFile, contacts, dtDefault );
      Q_FOREACH(const KABC::Addressee&contact, contacts) {
        createContact( contact );
      }
    }
  } else if(type == QLatin1String("LibBalsaAddressBookVcard")) {
    const QString path = grp.readEntry(QLatin1String("Path"));
    if(!path.isEmpty()) {
      QMap<QString, QVariant> settings;
      settings.insert(QLatin1String("Path"),path);
      settings.insert(QLatin1String("DisplayName"),name);
      createResource(QLatin1String("akonadi_vcard_resource") ,name, settings);
    }
  } else {
      qDebug()<<" unknown addressbook type :"<<type;
  }
}
