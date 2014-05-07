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

#ifndef LIBKLEOPATRACLIENT_KLEOPATRACLIENT_EXPORT_H
#define LIBKLEOPATRACLIENT_KLEOPATRACLIENT_EXPORT_H

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

#ifdef Q_WS_WIN

#ifndef KLEOPATRACLIENTCORE_EXPORT
# if defined(KDEPIM_STATIC_LIBS)
   /* No export/import for static libraries */
#  define KLEOPATRACLIENTCORE_EXPORT
# elif defined(MAKE_KLEOPATRACLIENTCORE_LIB)
#  define KLEOPATRACLIENTCORE_EXPORT KDE_EXPORT
# else
#  define KLEOPATRACLIENTCORE_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KLEOPATRACLIENTGUI_EXPORT
# if defined(KDEPIM_STATIC_LIBS)
   /* No export/import for static libraries */
#  define KLEOPATRACLIENTGUI_EXPORT
# elif defined(MAKE_KLEOPATRACLIENTGUI_LIB)
#  define KLEOPATRACLIENTGUI_EXPORT KDE_EXPORT
# else
#  define KLEOPATRACLIENTGUI_EXPORT KDE_IMPORT
# endif
#endif

#endif // Q_OS_WIN

#ifndef KLEOPATRACLIENTCORE_EXPORT
# define KLEOPATRACLIENTCORE_EXPORT KDE_EXPORT
#endif

#ifndef KLEOPATRACLIENTGUI_EXPORT
# define KLEOPATRACLIENTGUI_EXPORT KDE_EXPORT
#endif

#ifndef KleopatraClientCopy
# define KleopatraClientCopy KleopatraClient
#endif

#endif /* LIBKLEOPATRACLIENT_KLEOPATRACLIENT_EXPORT_H */
