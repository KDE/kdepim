/*
    Copyright (c) 2009 Kevin Ottens <ervin@kde.org>

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

#ifndef MESSAGECORE_CORELIST_EXPORT_H
#define MESSAGECORE_CORELIST_EXPORT_H

#include <kdemacros.h>

#ifndef MESSAGECORE_EXPORT
# if defined(KDEPIM_STATIC_LIBS)
   /* No export/import for static libraries */
#  define MESSAGECORE_EXPORT
# elif defined(MAKE_MESSAGECORE_LIB)
   /* We are building this library */
#  define MESSAGECORE_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define MESSAGECORE_EXPORT KDE_IMPORT
# endif
#endif

# ifndef MESSAGECORE_EXPORT_DEPRECATED
#   if defined(MAKE_MESSAGECORE_KDE_LIB) || defined MESSAGECORE_IGNORE_DEPRECATED_WARNINGS
#     define MESSAGECORE_EXPORT_DEPRECATED MESSAGECORE_EXPORT
#   else
#     define MESSAGECORE_EXPORT_DEPRECATED KDE_DEPRECATED MESSAGECORE_EXPORT
#   endif
# endif

#endif
