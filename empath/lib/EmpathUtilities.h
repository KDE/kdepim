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

#ifndef EMPATHUTILITIES_H
#define EMPATHUTILITIES_H

// Qt includes
#include <qstring.h>
#include <qcolor.h>
#include <qfile.h>

// Local includes
#include "EmpathDefines.h"

QString token(const char * str, const char * delim, Q_UINT32 pos);
QString baseName(const QString & filename);
bool	EmpathLockMailFile(QFile & f, LockType ltype);
QString QColorToHTML(const QColor & col);
QString quoteSeparators(const QString & s, char separator);
QString empathDir();
void empathInvokeHelp(const QString & a, const QString & b);
QString className();

#endif
