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

#include <qstring.h>

static const QString	EMPATH_VERSION_STRING	=
	QString::fromLatin1("1.0 Alpha");

static const int		EMPATH_VERSION_MAJOR	= 0;
static const int		EMPATH_VERSION_MINOR	= 8;
static const int		EMPATH_VERSION_RELEASE	= 1;

#ifdef DEBUG
#include <iostream>
#	define empathDebug(a)	std::cerr << className() << ": " << \
							QString((a)).ascii() << std::endl;
#else
#	define empathDebug(a)
#endif

#endif // included this file

