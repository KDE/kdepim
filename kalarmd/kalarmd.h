/*
 *  kalarmd.h  -  global header file
 *  Program:  kalarmd
 *  (C) 2001 by David Jarvie  software@astrojar.org.uk
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef KALARMD_H
#define KALARMD_H

// Define whether the daemon or the GUI is being built
#define KALARMD

// Compatibility for QT 3 libraries
#include <qglobal.h>
#if QT_VERSION < 300
	#define QPtrList         QList
	#define QPtrListIterator QListIterator
#endif

#include <kapp.h>
#if KDE_VERSION < 290
   #define Alarm KOAlarm
#endif

#endif // KALARMD_H

