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

#ifndef LIBKLEO_EXPORT_H
#define LIBKLEO_EXPORT_H

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

#ifdef Q_WS_WIN

#ifndef KLEO_EXPORT
# if defined(KDEPIM_STATIC_LIBS)
   /* No export/import for static libraries */
#  define KLEO_EXPORT
# elif defined(MAKE_KLEO_LIB)
#  define KLEO_EXPORT KDE_EXPORT
# else
#  define KLEO_EXPORT KDE_IMPORT
# endif
#endif

#else // not windows

#define KLEO_EXPORT KDE_EXPORT
#endif /* not windows */

# ifndef KLEO_EXPORT_DEPRECATED
#  define KLEO_EXPORT_DEPRECATED KDE_DEPRECATED KLEO_EXPORT
# endif

#endif
