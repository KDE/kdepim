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

#include "operaaddressbook.h"

#include <KABC/Addressee>
#include <kabc/contactgroup.h>


#include <QDebug>
#include <QFile>

OperaAddressBook::OperaAddressBook(const QString &filename, ImportWizard *parent)
  : AbstractAddressBook( parent )
{
  QFile file(filename);
  if ( !file.open( QIODevice::ReadOnly ) ) {
    kDebug()<<" We can't open file"<<filename;
    return;
  }

  QTextStream stream(&file);
  while ( !stream.atEnd() ) {
    const QString line = stream.readLine();
    if(line == QLatin1String("#CONTACT")) {
        readContact(stream);
    } else if(line == QLatin1String("#FOLDER")) {
        //TODO
    } else {
        qDebug()<<" line :"<<line;
    }
  }
}

OperaAddressBook::~OperaAddressBook()
{

}

void OperaAddressBook::readContact(QTextStream &stream)
{
  KABC::Addressee contact;
  while ( !stream.atEnd() ) {
      QString line = stream.readLine().trimmed();
      if(line.startsWith(QLatin1String("ID"))) {
          //Nothing
      } else if(line.startsWith(QLatin1String("NAME"))) {

      } else if(line.startsWith(QLatin1String("URL"))) {

      } else if(line.startsWith(QLatin1String("DESCRIPTION"))) {

      } else if(line.startsWith(QLatin1String("PHONE"))) {

      } else if(line.startsWith(QLatin1String("FAX"))) {

      } else if(line.startsWith(QLatin1String("POSTALADDRESS"))) {

      } else if(line.startsWith(QLatin1String("PICTUREURL"))) {

      } else if(line.startsWith(QLatin1String("ICON"))) {

      } else {
          qDebug() <<" unknown line :"<<line;
      }
  }
  createContact( contact );
}
