/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef ABOUTDATA_H
#define ABOUTDATA_H

#include <kaboutdata.h>
#include <config-kmobiletools.h>
#ifdef SVNREVISION
#define KMOBILETOOLS_VERSION "0.5 svn " SVNREVISION
#else
#define KMOBILETOOLS_VERSION "0.5 svn " __DATE__
#endif
#define kmteestring "\107\162\145\141\164\040\123\143\157\164\164\041"

// Uncomment here for releases
/*
#undef KMOBILETOOLS_VERSION
#define KMOBILETOOLS_VERSION "0.5.0-beta2"
*/
/**
@author Marco Gulino
*/
class KDE_EXPORT AboutData : public KAboutData
{
public:
    AboutData();
    ~AboutData();
};

#endif
