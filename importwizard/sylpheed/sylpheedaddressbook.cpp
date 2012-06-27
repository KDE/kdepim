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
#include <KABC/Addressee>

#include <KDebug>

#include <QDir>
#include <QDebug>
#include <QDomDocument>

SylpheedAddressBook::SylpheedAddressBook(const QDir& dir, ImportWizard *parent)
  : AbstractAddressBook( parent )
{
  //qDebug()<<" dir :"<<dir;
  const QStringList files = dir.entryList(QStringList("addrbook-[0-9]*.xml" ), QDir::Files, QDir::Name);
  Q_FOREACH( const QString& file, files ) {
    readAddressBook( dir.path() + QLatin1Char( '/' ) + file );
  }
  cleanUp();
}

SylpheedAddressBook::~SylpheedAddressBook()
{
}

void SylpheedAddressBook::readAddressBook( const QString& filename )
{
  QFile file(filename);
  //kDebug()<<" import filename :"<<filename;
  if ( !file.open( QIODevice::ReadOnly ) ) {
    kDebug()<<" We can't open file"<<filename;
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
      //qDebug()<<" name :"<<name;
    }
      
    const QString tag = e.tagName();
    if ( tag == QLatin1String( "person" ) ) {
      KABC::Addressee address;
//uid="333304265" first-name="dd" last-name="ccc" nick-name="" cn="laurent"
      if ( e.hasAttribute( QLatin1String( "uid" ) ) ) {
        //Nothing
      }
      if ( e.hasAttribute( QLatin1String( "first-name" ) ) ) {
        address.setName( e.attribute( QLatin1String( "first-name" ) ) );
      }
      if ( e.hasAttribute( QLatin1String( "last-name" ) ) ) {
        address.setFamilyName( e.attribute( QLatin1String( "last-name" ) ) );
        
      }
      if ( e.hasAttribute( QLatin1String( "nick-name" ) ) ) {
        address.setNickName( e.attribute(QLatin1String( "nick-name" )) );
      }
      if ( e.hasAttribute( QLatin1String( "cn" ) ) ) {
        address.setFormattedName(e.attribute(QLatin1String( "cn" )));
      }
      for ( QDomElement addressElement = e.firstChildElement(); !addressElement.isNull(); addressElement = addressElement.nextSiblingElement() ) {
        const QString addressTag = addressElement.tagName();
        if ( addressTag == QLatin1String( "address-list" ) ) {
          QStringList emails;
          for ( QDomElement addresslist = addressElement.firstChildElement(); !addresslist.isNull(); addresslist = addresslist.nextSiblingElement() ) {
            const QString tagAddressList = addresslist.tagName();
            if ( tagAddressList == QLatin1String( "address" ) ) {
              if ( addresslist.hasAttribute( QLatin1String( "email" ) ) ) {
                emails<<addresslist.attribute( QLatin1String( "email" ) );
              } else if(addresslist.hasAttribute(QLatin1String("alias"))) {
                //TODO:
              }
            } else {
             kDebug()<<" tagAddressList unknown :"<<tagAddressList;
            }
          }
          if ( !emails.isEmpty() ) {
            address.setEmails( emails );
          }
            
        } else if ( addressTag == QLatin1String( "attribute-list" ) ) {
          for ( QDomElement attributelist = addressElement.firstChildElement(); !attributelist.isNull(); attributelist = attributelist.nextSiblingElement() ) {
            const QString tagAttributeList = attributelist.tagName();
            if ( tagAttributeList == QLatin1String( "attribute" ) ) {
              //TODO
            } else {
              //TODO
            }
          }
          
        } else {
          kDebug()<<" addressTag unknown :"<<addressTag;
        }
      }
      createContact( address );
    } else {
      kDebug()<<" SylpheedAddressBook::readAddressBook  tag unknown :"<<tag;
    }
      
    //qDebug()<<" tag :"<<tag;
  }
}
