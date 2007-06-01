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

#ifndef KGROUPWARE_EXPORT_H
#define KGROUPWARE_EXPORT_H

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

#ifndef KGROUPWAREBASE_EXPORT
# if defined(MAKE_KGROUPWAREBASE_LIB)
   /* We are building this library */
#  define KGROUPWAREBASE_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define KGROUPWAREBASE_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KGROUPWAREDAV_EXPORT
# if defined(MAKE_KGROUPWAREDAV_LIB)
   /* We are building this library */
#  define KGROUPWAREDAV_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define KGROUPWAREDAV_EXPORT KDE_IMPORT
# endif
#endif

# ifndef KGROUPWAREBASE_EXPORT_DEPRECATED
#  define KGROUPWAREBASE_EXPORT_DEPRECATED KDE_DEPRECATED KGROUPWAREBASE_EXPORT
# endif
# ifndef KGROUPWAREDAV_EXPORT_DEPRECATED
#  define KGROUPWAREDAV_EXPORT_DEPRECATED KDE_DEPRECATED KGROUPWAREDAV_EXPORT
# endif

#endif
