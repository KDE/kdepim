/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
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

// Qt includes
#include <qstring.h>

// KDE includes
#include <kdebug.h>

#ifndef NDEBUG
#define empathDebug(a) kdebug(KDEBUG_INFO, 0,  "%s: %s\n", className(), QString((a)).data())
// Enable Qt debuggers
# define CHECK_STATE
# define CHECK_RANGE
# define CHECK_NULL
# undef NO_DEBUG
# undef NO_CHECK
#else
# undef DEBUG
# undef CHECK_STATE
# undef CHECK_RANGE
# undef CHECK_NULL
# define NO_CHECK
# define empathDebug(a)
#endif

typedef Q_UINT32 uID;

#endif // included this file

