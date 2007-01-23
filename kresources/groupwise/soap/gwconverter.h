/*
    This file is part of kdepim.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KABC_GW_CONVERTER_H
#define KABC_GW_CONVERTER_H

#include <qdatetime.h>
#include <qstring.h>

#include "soapH.h"

class GWConverter
{
  public:
    GWConverter( struct soap* );

    struct soap* soap() const;

    static QString stringToQString( const std::string& );
    static QString stringToQString( std::string* );

    std::string* qStringToString( const QString& );
    char* qStringToChar( const QString& );

    char* qDateToChar( const QDate& );
    QDate charToQDate( const char * );

    std::string* qDateToString( const QDate &string );
    QDate stringToQDate( std::string* );

    char *qDateTimeToChar( const QDateTime &dt, const QString &timezone );
    char *qDateTimeToChar( const QDateTime &dt );

    QDateTime charToQDateTime( const char *str );
    QDateTime charToQDateTime( const char *str, const QString &timezone );

    std::string* qDateTimeToString( const QDateTime &string, const QString &timezone );
    std::string* qDateTimeToString( const QDateTime &string );
    
    QDateTime stringToQDateTime( const std::string* );

    bool emailsMatch( const QString & email1, const QString & email2 );
  private:
    struct soap* mSoap;
};

#endif
