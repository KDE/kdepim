/*  -*- c++ -*-
    kasciistringtools.cpp

    This file is part of libkdepim.

    Copyright (c) 2005 Ingo Kloecker <kloecker@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "kasciistringtools.h"

namespace KPIM {

static unsigned char ASCIIToLower( unsigned char ch )
{
  if ( ch >= 'A' && ch <= 'Z' )
    return ch - 'A' + 'a';
  else
    return ch;
}

char * kAsciiToLower( char *s )
{
  if ( !s )
    return 0;
  for ( unsigned char *p = (unsigned char *) s; *p; ++p )
    *p = ASCIIToLower( *p );
  return s;
}

static unsigned char ASCIIToUpper( unsigned char ch )
{
  if ( ch >= 'a' && ch <= 'z' )
    return ch - 'a' + 'A';
  else
    return ch;
}

char * kAsciiToUpper( char *s )
{
  if ( !s )
    return 0;
  for ( unsigned char *p = (unsigned char *) s; *p; ++p )
    *p = ASCIIToUpper( *p );
  return s;
}

} // namespace KPIM
