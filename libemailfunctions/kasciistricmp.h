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

#include <kdeversion.h>

#if KDE_IS_VERSION(3,3,89)
// get kasciistricmp from kglobal.h
#include <kglobal.h>
#else
// define kasciistricmp to this kdepim symbol (renamed to avoid problems when upgrading kdelibs later)
int kdepim_kasciistricmp( const char *str1, const char *str2 );
#define kasciistricmp kdepim_kasciistricmp
#endif
