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

#include <KABC/Addressee>
#include <kabc/contactgroup.h>

#include <KConfig>
#include <KConfigGroup>

#include <QDebug>

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
//TODO
  const QString type = grp.readEntry(QLatin1String("Type"));
  if(type.isEmpty()) {
    return;
  }
  const QString name = grp.readEntry(QLatin1String("Name"));

  if(type == QLatin1String("LibBalsaAddressBookLdap")) {

      /*
      Host=ldap://ldap
      BaseDN=ss
      BookDN=ss
      EnableTLS=true
      Type=LibBalsaAddressBookLdap
      Name=tito
      ExpandAliases=true
      IsExpensive=true
      DistListMode=false
*/
  } else if(type == QLatin1String("LibBalsaAddressBookGpe")) {

  } else if(type == QLatin1String("LibBalsaAddressBookLdif")) {

  } else {
      qDebug()<<" unknown addressbook type :"<<type;
  }
}
