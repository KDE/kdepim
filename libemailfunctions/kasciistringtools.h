/*  -*- c++ -*-
    kasciistringtools.h

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KPIM_KASCIISTRINGTOOLS_H
#define KPIM_KASCIISTRINGTOOLS_H

namespace KPIM {

/**
  Locale-independent function to convert ASCII strings to lower case ASCII
  strings. This means that it affects @em only the ASCII characters A-Z.

  @param str  pointer to the string which should be converted to lower case
  @return     pointer to the converted string (same as @a str)
*/
char * kAsciiToLower( char *str );

/**
  Locale-independent function to convert ASCII strings to upper case ASCII
  strings. This means that it affects @em only the ASCII characters a-z.

  @param str  pointer to the string which should be converted to upper case
  @return     pointer to the converted string (same as @a str)
*/
char * kAsciiToUpper( char *str );

} // namespace KPIM

#endif
