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

#ifndef EGROUPWARE_EXPORT_H
#define EGROUPWARE_EXPORT_H

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

#ifndef KABC_XMLRPC_EXPORT
# if defined(MAKE_KABC_XMLRPC_LIB)
   /* We are building this library */ 
#  define KABC_XMLRPC_EXPORT KDE_EXPORT
# else
   /* We are using this library */ 
#  define KABC_XMLRPC_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KCAL_XMLRPC_EXPORT
# if defined(MAKE_KCAL_XMLRPC_LIB)
   /* We are building this library */
#  define KCAL_XMLRPC_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define KCAL_XMLRPC_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KNOTES_XMLRPC_EXPORT
# if defined(MAKE_KNOTES_XMLRPC_LIB)
   /* We are building this library */
#  define KNOTES_XMLRPC_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define KNOTES_XMLRPC_EXPORT KDE_IMPORT
# endif
#endif


# ifndef KABC_XMLRPC_EXPORT_DEPRECATED
#  define KABC_XMLRPC_EXPORT_DEPRECATED KDE_DEPRECATED KABC_XMLRPC_EXPORT
# endif

# ifndef KCAL_XMLRPC_EXPORT_DEPRECATED
#  define KCAL_XMLRPC_EXPORT_DEPRECATED KDE_DEPRECATED KCAL_XMLRPC_EXPORT
# endif

# ifndef KNOTES_XMLRPC_EXPORT_DEPRECATED
#  define KNOTES_XMLRPC_EXPORT_DEPRECATED KDE_DEPRECATED KNOTES_XMLRPC_EXPORT
# endif


#endif
