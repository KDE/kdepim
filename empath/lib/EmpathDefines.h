/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef EMPATHDEFINES_H
#define EMPATHDEFINES_H

#include <qstring.h>

static const QString    EMPATH_MAINTAINER       = "Rik Hemsley";
static const QString    EMPATH_MAINTAINER_EMAIL = "rik@kde.org";

static const QString    EMPATH_VERSION_STRING   = "1.0 Pre-alpha";
static const int        EMPATH_VERSION_MAJOR    = 0;
static const int        EMPATH_VERSION_MINOR    = 8;
static const int        EMPATH_VERSION_RELEASE  = 1;

#ifndef NDEBUG
#include <iostream>
# ifdef __GNUG__
#  define empathDebug(a) cerr << className() << ":"   << __FUNCTION__ << " (" \
                              << __LINE__    << "): " << QString((a)) << endl;
# else
#  define empathDebug(a) cerr << className() << ": " << QString((a)) << endl;
# endif
#else
# define empathDebug(a)
#endif

#endif // included this file

// vim:ts=4:sw=4:tw=78
