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

#include "thunderbirdsettings.h"
#include <QTextStream>
#include <QStringList>
#include <QFile>
#include <QDebug>

ThunderbirdSettings::ThunderbirdSettings( const QString& filename, ImportWizard *parent )
    :AbstractSettings( parent )
{
  QFile file(filename);
  if ( !file.open( QIODevice::ReadOnly ) ) {
    qDebug()<<" We can't open file"<<filename;
    return;
  }
  QTextStream stream(&file);
  while ( !stream.atEnd() ) {
    const QString line = stream.readLine();
    if(line.startsWith(QLatin1String("user_pref"))) {
      if(line.contains(QLatin1String("mail.smtpserver.")) ||
         line.contains(QLatin1String("mail.server.") ) ||
         line.contains(QLatin1String("mail.identity.")) ||
         line.contains(QLatin1String("mail.account.")) ||
         line.contains( QLatin1String( "mail.accountmanager." ) ) ) {
        insertIntoMap( line );
      }
    }
  }
}

ThunderbirdSettings::~ThunderbirdSettings()
{
}

void ThunderbirdSettings::insertIntoMap( const QString& line )
{
  QString newLine = line;
  newLine.remove( QLatin1String( "user_pref(\"" ) );
  newLine.remove( QLatin1String( ");" ) );
  const int pos = newLine.indexOf( QLatin1Char( ',' ) );
  QString key = newLine.left( pos );
  key.remove( key.length() -1, 1 );
  QString valueStr = newLine.right( newLine.length() - pos -2);
  if ( valueStr.at( 0 ) == QLatin1Char( '"' ) ) {
    valueStr.remove( 0, 1 );
    if ( valueStr.at( valueStr.length()-1 ) == QLatin1Char( '"' ) )
      valueStr.remove( valueStr.length()-1, 1 );
    //Store as String
    mHashConfig.insert( key, valueStr );
  } else {
    //Store as integer
    const int value = valueStr.toInt();
    mHashConfig.insert( key, value );
  }
}
