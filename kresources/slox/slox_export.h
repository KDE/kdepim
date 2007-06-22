/*  This file is part of the KDE project
    Copyright (C) 2007 David Faure <faure@kde.org>

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

#ifndef SLOX_EXPORT_H
#define SLOX_EXPORT_H

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

#ifndef KSLOX_EXPORT
# if defined(MAKE_KSLOX_LIB)
   /* We are building this library */ 
#  define KSLOX_EXPORT KDE_EXPORT
# else
   /* We are using this library */ 
#  define KSLOX_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KABC_SLOX_EXPORT
# if defined(MAKE_KABC_SLOX_LIB)
   /* We are building this library */
#  define KABC_SLOX_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define KABC_SLOX_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KCAL_SLOX_EXPORT
# if defined(MAKE_KCAL_SLOX_LIB)
   /* We are building this library */
#  define KCAL_SLOX_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define KCAL_SLOX_EXPORT KDE_IMPORT
# endif
#endif


# ifndef KSLOX_EXPORT_DEPRECATED
#  define KSLOX_EXPORT_DEPRECATED KDE_DEPRECATED KSLOX_EXPORT
# endif

# ifndef KABC_SLOX_EXPORT_DEPRECATED
#  define KABC_SLOX_EXPORT_DEPRECATED KDE_DEPRECATED KABC_SLOX_EXPORT
# endif

# ifndef KCAL_SLOX_EXPORT_DEPRECATED
#  define KCAL_SLOX_EXPORT_DEPRECATED KDE_DEPRECATED KCAL_SLOX_EXPORT
# endif


#endif
