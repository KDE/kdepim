/* options.cc			KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot
**
** This is a file of odds and ends, with debugging functions and stuff.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/


static const char *options_id="$Id$";

#include <iostream.h>
#include <kconfig.h>
#include <kdebug.h>
#include "options.h"

#ifdef DEBUG
// The daemon also has a debug level
//
//
int debug_level=0;
const char *tabs="\t\t\t\t\t\t";

void listConfig(kdbgstream& s,KConfig &c)
{
	FUNCTIONSETUP;

	QMap<QString,QString> m ;
	QStringList l = c.groupList();
	QStringList::Iterator i;

	s << fname << ": Listing groups in config file" << endl ;
	for (i=l.begin(); i!=l.end(); ++i)
	{
		s << fname 
			<< ": "
			<< *i
			<< endl;

		m = c.entryMap(*i);
		QMap<QString,QString>::ConstIterator j;

		for (j=m.begin(); j!=m.end(); ++j)
		{
			s << fname
				<< ": "
				<< j.key()
				<< "="
				<< j.data()
				<< endl;
		}
	}
	/* NOTREACHED */
	(void) options_id;
}

void listStrList(kdbgstream& s,const QStringList& l)
{
	FUNCTIONSETUP;

	QStringList::ConstIterator i;
	s << fname << ": Elements of string list:" << endl;

	for (i=l.begin(); i!=l.end(); ++i)
	{
		s << fname << ": " << *i << endl;
	}
}

void listStrList(kdbgstream& s,QStrList &l)
{
	FUNCTIONSETUP;

	s << fname << ": Elements of string list:" << endl;

	for (char *p=l.first(); p; p=l.next())
	{
		s << fname << ":\t" << p << endl;
	}
}


QString qstringExpansion(const QString& s)
{
	QString t;

	for (unsigned i=0; i<s.length(); i++)
	{
		t+=s[i];
		t+=' ';
		t+=QString::number((int)s[i].unicode());
		t+=' ';
	}

	return t;
}

QString charExpansion(const char *s)
{
	QString t;

	while (*s)
	{
		t+=QChar(*s);
		t+=' ';
		t+=QString::number(*s);
		t+=' ';
		s++;
	}

	return t;
}


#else
#ifdef TEST_DEBUG
debugName const fname((int) options_id);
kndbgstream operator << (kndbgstream s, const debugName&)
{
	return s;
}
#else
const int fname = (int) options_id;
#endif
#endif


// $Log$
// Revision 1.11  2001/03/01 20:43:24  adridg
// Some new (and harmless) debug functions
//
// Revision 1.10  2001/02/09 12:56:29  brianj
// Fixed bug where variable "id" was renamed to "options_id" but
// a couple of occurences were missed.
//
// Revision 1.9  2001/02/08 08:13:44  habenich
// exchanged the common identifier "id" with source unique <sourcename>_id for --enable-final build
//
// Revision 1.8  2001/02/05 20:58:48  adridg
// Fixed copyright headers for source releases. No code changed
//
