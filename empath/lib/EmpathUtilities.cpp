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

// System includes
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>

// Qt includes
#include <qregexp.h>

// KDE includes
#include <kapp.h>

// Local includes
#include "EmpathUtilities.h"

	QString
token(const char * str, const char * delim, Q_UINT32 pos)
{
	if (!delim || !str || strlen(delim) == 0 || strlen(str) == 0) return "";

	char * s = new char[strlen(str)];
	CHECK_PTR(s);
	
	strcpy(s, str);

	QString ret = strtok(s, delim);

	Q_UINT32 i = 0;
	while (i < pos) {
		ret = strtok(0, delim);
		++i;
	}

	delete [] s;
	
	return ret ? ret : QString("");
}

QString baseName(const QString & filename)
{
	return filename.right(filename.length() - filename.findRev('/') - 1);
}

	bool
EmpathLockMailFile(QFile & f, LockType ltype)
{
	int locked = 0;
#ifdef HAVE_FLOCK
	return (flock(
				f.handle(),
				(ltype == LockWrite ? LOCK_EX : LOCK_SH) | LOCK_NB) != -1);
#else
	struct flock lock;
	memset(&lock, 0, sizeof(struct flock));
	lock.l_type = ltype == LockWrite ? F_WRLCK : F_RDLCK;
	lock.l_whence = SEEK_SET;
	return (fcntl(f.handle(), F_SETLK, &lock) != -1);
#endif
}

	QString
QColorToHTML(const QColor & col)
{
	int r = col.red();
	int g = col.green();
	int b = col.blue();
	
	char colourAsString[6];
	
	sprintf(colourAsString, "%02X%02X%02X",
			col.red(), col.green(), col.blue()); 
	
	return colourAsString;
}

	QString
quoteSeparators(const QString & s, char separator)
{
	QString tempString(s);
	
	QString quoted = "\\";
	quoted += separator;
	
	tempString.replace(QRegExp(&separator), quoted);
	return tempString;
}

	QString
empathDir()
{
	return kapp->kde_datadir() + QString("/empath/");
}


	void
empathInvokeHelp(const QString & a, const QString & b)
{
	kapp->invokeHTMLHelp(a, b);
}

	QString
className()
{
	return "Empath General Debugging:";
}

