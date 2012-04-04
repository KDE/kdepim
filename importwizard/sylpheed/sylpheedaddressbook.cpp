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

#include "sylpheedaddressbook.h"

#include <KDebug>

#include <QDir>
#include <QDebug>
#include <QDomDocument>

SylpheedAddressBook::SylpheedAddressBook(const QDir& dir, ImportWizard *parent)
  : AbstractAddressBook( parent )
{
  qDebug()<<" dir :"<<dir;
  const QStringList files = dir.entryList(QStringList("addrbook-[0-9]*.xml" ), QDir::Files, QDir::Name);
  Q_FOREACH( const QString& file, files ) {
    readAddressBook( dir.path() + QLatin1Char( '/' ) + file );
  }
}

SylpheedAddressBook::~SylpheedAddressBook()
{
}

void SylpheedAddressBook::readAddressBook( const QString& filename )
{
  QFile file(filename);
  qDebug()<<" import filename :"<<filename;
  if ( !file.open( QIODevice::ReadOnly ) ) {
    qDebug()<<" We can't open file"<<filename;
    return;
  }
  QString errorMsg;
  int errorRow;
  int errorCol;
  QDomDocument doc;
  if ( !doc.setContent( &file, &errorMsg, &errorRow, &errorCol ) ) {
    kDebug() << "Unable to load document.Parse error in line " << errorRow
             << ", col " << errorCol << ": " << errorMsg;
    return;
  }
  QDomElement domElement = doc.documentElement();

  if ( domElement.isNull() ) {
    kDebug() << "addressbook not found";
    return;
  }

  for ( QDomElement e = domElement.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
    QString name;
    if ( e.hasAttribute( QLatin1String( "name" ) ) ) {
      name = e.attribute( QLatin1String( "name" ) );
      qDebug()<<" name :"<<name;
    }
      
    const QString tag = e.tagName();
    if ( tag == QLatin1String( "person" ) ) {
//uid="333304265" first-name="dd" last-name="ccc" nick-name="" cn="laurent"
      if ( e.hasAttribute( QLatin1String( "uid" ) ) ) {
        //Nothing
      }
      if ( e.hasAttribute( QLatin1String( "first-name" ) ) ) {
        
      }
      if ( e.hasAttribute( QLatin1String( "last-name" ) ) ) {
        
      }
      if ( e.hasAttribute( QLatin1String( "nick-name" ) ) ) {
        
      }
      if ( e.hasAttribute( QLatin1String( "cn" ) ) ) {
        
      }
      for ( QDomElement address = e.firstChildElement(); !address.isNull(); address = address.nextSiblingElement() ) {
        const QString addressTag = address.tagName();
        if ( addressTag == QLatin1String( "address-list" ) ) {
          
        } else if ( addressTag == QLatin1String( "attribute-list" ) ) {
          
        } else {
          qDebug()<<" addressTag unknown :"<<addressTag;
        }
      }

    } else {
      qDebug()<<" SylpheedAddressBook::readAddressBook  tag unknown :"<<tag;
    }
      
    qDebug()<<" tag :"<<tag;
  }
}
