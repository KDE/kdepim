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

#include <tqdatetime.h>
#include <tqstring.h>

#include "soapH.h"

class GWConverter
{
  public:
    GWConverter( struct soap* );

    struct soap* soap() const;

    static TQString stringToQString( const std::string& );
    static TQString stringToQString( std::string* );

    std::string* qStringToString( const TQString& );
    char* qStringToChar( const TQString& );

    char* qDateToChar( const TQDate& );
    TQDate charToQDate( const char * );

    std::string* qDateToString( const TQDate &string );
    TQDate stringToQDate( std::string* );

    char *qDateTimeToChar( const TQDateTime &dt, const TQString &timezone );
    char *qDateTimeToChar( const TQDateTime &dt );

    TQDateTime charToQDateTime( const char *str );
    TQDateTime charToQDateTime( const char *str, const TQString &timezone );

    std::string* qDateTimeToString( const TQDateTime &string, const TQString &timezone );
    std::string* qDateTimeToString( const TQDateTime &string );
    
    TQDateTime stringToQDateTime( const std::string* );

    bool emailsMatch( const TQString & email1, const TQString & email2 );
  private:
    struct soap* mSoap;
};

#endif
