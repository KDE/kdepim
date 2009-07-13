/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "util.h"

using namespace KMime;
using namespace MessageComposer;

QString MessageComposer::nameForEncoding( Headers::contentEncoding enc )
{
  switch( enc ) {
    case Headers::CE7Bit: return QString::fromLatin1( "7bit" );
    case Headers::CE8Bit: return QString::fromLatin1( "8bit" );
    case Headers::CEquPr: return QString::fromLatin1( "quoted-printable" );
    case Headers::CEbase64: return QString::fromLatin1( "base64" );
    case Headers::CEuuenc: return QString::fromLatin1( "uuencode" );
    case Headers::CEbinary: return QString::fromLatin1( "binary" );
    default: return QString::fromLatin1( "unknown" );
  }
}

