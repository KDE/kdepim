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

#if (!defined NDEBUG) && (defined __GNUG__)

#   include <stdio.h>
#   include <iostream.h>
   
#   define empathDebug(a) \
        fprintf(stderr, "%s, line %d\n", __PRETTY_FUNCTION__, __LINE__); \
        cerr << QString((a)) << endl;

#   define eDebug(format, args) \
        fprintf(stderr, "%s, line %d\n", __PRETTY_FUNCTION__, __LINE__); \
        fprintf(stderr, format, ## args); \
        fprintf(stderr, "\n");
        
#else
        
#       define empathDebug(a)

#endif

#endif // included this file

// vim:ts=4:sw=4:tw=78
