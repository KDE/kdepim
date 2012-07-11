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

#include "thunderbirdaddressbook.h"
#include "addressbook/MorkParser.h"

#include <KABC/Addressee>
#include <kabc/contactgroup.h>

ThunderBirdAddressBook::ThunderBirdAddressBook(const QDir& dir, ImportWizard *parent)
  : AbstractAddressBook( parent )
{
  readAddressBook(dir.path() + QLatin1String( "/impab.mab" ) );

  const QStringList filesimportab = dir.entryList(QStringList("impab-[0-9]*.map" ), QDir::Files, QDir::Name);
  Q_FOREACH( const QString& file, filesimportab ) {
    readAddressBook( dir.path() + QLatin1Char( '/' ) + file );
  }
  readAddressBook(dir.path() + QLatin1String( "/abook.mab" ) );

  const QStringList files = dir.entryList(QStringList("abook-[0-9]*.map" ), QDir::Files, QDir::Name);
  Q_FOREACH( const QString& file, files ) {
    readAddressBook( dir.path() + QLatin1Char( '/' ) + file );
  }
  readAddressBook(dir.path() + QLatin1String( "/history.mab" ) );

  cleanUp();
}

ThunderBirdAddressBook::~ThunderBirdAddressBook()
{

}

void ThunderBirdAddressBook::readAddressBook( const QString& filename )
{
  MorkParser mork;
  if ( !mork.open( filename ) )
  {
      qDebug()<<" error during read file "<<filename<<" Error type "<<mork.error();
      return;
  }
  MorkTableMap *tables = mork.getTables(0x80);
  MorkTableMap::iterator tableIterEnd(tables->end());
  MorkRowMap *rows = 0;
  if ( tables ) {
    for ( MorkTableMap::iterator tableIter = tables->begin(); tableIter != tableIterEnd; ++tableIter ) {
      if ( tableIter.key() != 0 ) {
        rows = mork.getRows( 0x80, &tableIter.value() );
        if(rows) {
          MorkRowMap::iterator endRow(rows->end());
          for ( MorkRowMap::iterator rowIter = rows->begin(); rowIter != endRow; ++rowIter ) {
            if(rowIter.key() != 0) {
              KABC::Addressee contact;
              MorkCells cells = rowIter.value();
              MorkCells::iterator endCellIter = cells.end();
              KABC::PhoneNumber phone;
              KABC::Address homeAddr = KABC::Address( KABC::Address::Home );
              KABC::Address workAddr = KABC::Address( KABC::Address::Work );
              int birthday = -1;
              int birthmonth = -1;
              int birthyear = -1;


              for ( MorkCells::iterator cellsIter = cells.begin();cellsIter != endCellIter; ++cellsIter ) {
                const QString value = mork.getValue(cellsIter.value());
                const QString column = mork.getColumn(cellsIter.key());
                qDebug()<<"column :"<<column<<" value :"<<value;
                if( column == QLatin1String("LastModifiedDate" ) ) {
                } else if( column == QLatin1String("RecordKey" ) ) {
                } else if( column == QLatin1String("AddrCharSet" ) ) {
                } else if( column == QLatin1String("LastRecordKey" ) ) {
                } else if( column == QLatin1String("ns:addrbk:db:table:kind:pab" ) ) {
                } else if( column == QLatin1String("ListName" ) ) {
                } else if( column == QLatin1String("ListNickName" ) ) {
                } else if( column == QLatin1String("ListDescription" ) ) {
                } else if( column == QLatin1String("ListTotalAddresses" ) ) {
                } else if( column == QLatin1String("LowercaseListName" ) ) {
                } else if( column == QLatin1String("ns:addrbk:db:table:kind:deleted" ) ) {
                } else if( column == QLatin1String("PhotoType" ) ) {
                } else if( column == QLatin1String("PreferDisplayName" ) ) {
                } else if( column == QLatin1String("PhotoURI" ) ) {
                } else if( column == QLatin1String("PhotoName" ) ) {
                } else if( column == QLatin1String("DbRowID" ) ) {
                } else if( column == QLatin1String("ns:addrbk:db:row:scope:card:all" ) ) {
                } else if( column == QLatin1String("ns:addrbk:db:row:scope:list:all" ) ) {
                } else if( column == QLatin1String("ns:addrbk:db:row:scope:data:all" ) ) {
                } else if( column == QLatin1String("FirstName" ) ) {
                  contact.setName(value);
                } else if( column == QLatin1String("LastName" ) ) {
                  contact.setFamilyName(value);
                } else if( column == QLatin1String("PhoneticFirstName" ) ) {
                } else if( column == QLatin1String("PhoneticLastName" ) ) {
                } else if( column == QLatin1String("DisplayName" ) ) {
                } else if( column == QLatin1String("NickName" ) ) {
                } else if( column == QLatin1String("PrimaryEmail" ) ) {
                  contact.setEmails(QStringList()<<value);
                } else if( column == QLatin1String("LowercasePrimaryEmail" ) ) {
                } else if( column == QLatin1String("SecondEmail" ) ) {
                } else if( column == QLatin1String("PreferMailFormat" ) ) {
                } else if( column == QLatin1String("PopularityIndex" ) ) {
                } else if( column == QLatin1String("AllowRemoteContent" ) ) {
                } else if( column == QLatin1String("WorkPhone" ) ) {
                } else if( column == QLatin1String("HomePhone" ) ) {
                } else if( column == QLatin1String("FaxNumber" ) ) {
                } else if( column == QLatin1String("PagerNumber" ) ) {
                } else if( column == QLatin1String("CellularNumber" ) ) {
                } else if( column == QLatin1String("WorkPhoneType" ) ) {
                } else if( column == QLatin1String("HomePhoneType" ) ) {
                } else if( column == QLatin1String("FaxNumberType" ) ) {
                } else if( column == QLatin1String("PagerNumberType" ) ) {
                } else if( column == QLatin1String("CellularNumberType" ) ) {
                } else if( column == QLatin1String("HomeAddress" ) ) {
                  homeAddr.setStreet(value);
                } else if( column == QLatin1String("HomeAddress2" ) ) {
                } else if( column == QLatin1String("HomeCity" ) ) {
                  homeAddr.setLocality(value);
                } else if( column == QLatin1String("HomeState" ) ) {
                } else if( column == QLatin1String("HomeZipCode" ) ) {
                  homeAddr.setPostalCode(value);
                } else if( column == QLatin1String("HomeCountry" ) ) {
                  homeAddr.setCountry(value);
                } else if( column == QLatin1String("WorkAddress" ) ) {
                  workAddr.setStreet(value);
                } else if( column == QLatin1String("WorkAddress2" ) ) {
                } else if( column == QLatin1String("WorkCity" ) ) {
                } else if( column == QLatin1String("WorkState" ) ) {
                } else if( column == QLatin1String("WorkZipCode" ) ) {
                  workAddr.setPostalCode(value);
                } else if( column == QLatin1String("WorkCountry") ) {
                  workAddr.setCountry(value);
                } else if( column == QLatin1String("JobTitle" ) ) {
                } else if( column == QLatin1String("Department" ) ) {
                } else if( column == QLatin1String("Company" ) ) {
                } else if( column == QLatin1String("_AimScreenName" ) ) {
                } else if( column == QLatin1String("AnniversaryYear" ) ) {
                } else if( column == QLatin1String("AnniversaryMonth" ) ) {
                } else if( column == QLatin1String("AnniversaryDay" ) ) {
                } else if( column == QLatin1String("SpouseName" ) ) {
                } else if( column == QLatin1String("FamilyName" ) ) {
                } else if( column == QLatin1String("WebPage1" ) ) {
                } else if( column == QLatin1String("WebPage2" ) ) {
                } else if( column == QLatin1String("BirthYear" ) ) {
                    birthyear = value.toInt();
                } else if( column == QLatin1String("BirthMonth" ) ) {
                    birthmonth = value.toInt();
                } else if( column == QLatin1String("BirthDay" ) ) {
                    birthday = value.toInt();
                } else if( column == QLatin1String("Custom1" ) ) {
                } else if( column == QLatin1String("Custom2" ) ) {
                } else if( column == QLatin1String("Custom3" ) ) {
                } else if( column == QLatin1String("Custom4" ) ) {
                } else if( column == QLatin1String("Notes" ) ) {
                  contact.setNote(value);
                } else {
                  kDebug()<<" Columnn not implemented "<<column;
                }

                //qDebug()<<" value :"<<value<<" column"<<column;
              }
              contact.insertPhoneNumber(phone);

              if(!homeAddr.isEmpty())
                contact.insertAddress(homeAddr);
              if(!workAddr.isEmpty())
                contact.insertAddress(workAddr);

              QDateTime birthDate( QDate( birthyear, birthmonth, birthday ) );
              if(birthDate.isValid()) {
                  contact.setBirthday( birthDate );
              }
              createContact( contact );
              qDebug()<<"-----------------------";
            }
          }
        }
      }
    }
  }
}
