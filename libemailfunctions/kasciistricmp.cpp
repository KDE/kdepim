/* This file is part of the KDE libraries
   Copyright (C) 2004 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kasciistricmp.h"

#if ! KDE_IS_VERSION(3,3,89)

static unsigned char kdepim_ASCIIToLower( unsigned char ch )
{
  if ( ch >= 'A' && ch <= 'Z' )
    return ch - 'A' + 'a';
  else
    return ch;
}

int kdepim_kasciistricmp( const char *str1, const char *str2 )
{
  const unsigned char *s1 = (const unsigned char *) str1;
  const unsigned char *s2 = (const unsigned char *) str2;
  int res;
  unsigned char c;

  if ( !s1 || !s2 )
    return s1 ? 1 : ( s2 ? -1 : 0 );
  for ( ; !( res = ( c = kdepim_ASCIIToLower( *s1 ) ) - kdepim_ASCIIToLower( *s2 ) );
        ++s1, ++s2 )
    if ( !c ) // strings are equal
      break;
  return res;
}

#endif
