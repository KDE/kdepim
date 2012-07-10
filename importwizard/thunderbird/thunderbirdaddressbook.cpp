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
  const QStringList files = dir.entryList(QStringList("impab-[0-9]*.map" ), QDir::Files, QDir::Name);
  Q_FOREACH( const QString& file, files ) {
    readAddressBook( dir.path() + QLatin1Char( '/' ) + file );
  }
  readAddressBook(dir.path() + QLatin1String( "/abook.mab" ) );
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
                KABC::Addressee address;
                MorkCells cells = rowIter.value();
                qDebug()<<" cells "<<cells;
                MorkCells::iterator endCellIter = cells.end();
                for ( MorkCells::iterator cellsIter = cells.begin();cellsIter != endCellIter; ++cellsIter ) {
                    //qDebug()<<" cellsIter.value()"<<cellsIter.value()<< "cellsIter.key()"<<cellsIter.key();
                    QString value = mork.getValue(cellsIter.value());
                    QString column = mork.getValue(cellsIter.key());
                    qDebug()<<" value :"<<value<<" column"<<column;
                }
                //TODO
            }
          }
        }
      }
    }
  }
}
