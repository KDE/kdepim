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
** Bug reports and questions can be sent to kde-pim@kde.org
*/


static const char *options_id =
	"$Id$";

#ifdef NDEBUG
#undef NDEBUG
#endif
#ifndef DEBUG
#define DEBUG
#endif

#include "options.h"

#ifdef DEBUG

#include <iostream.h>

#include <qsize.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kcmdlineargs.h>

// The daemon also has a debug level; debug_spaces is 60 spaces,
// to align FUNCTIONSETUP output.
//
//
int debug_level = 0;
const char *debug_spaces =
	"                                                    ";

void listConfig(DEBUGSTREAM & s, KConfig & c)
{
	FUNCTIONSETUP;

	QMap < QString, QString > m;
	QStringList l = c.groupList();

	QStringList::Iterator i;

	s << fname << ": Listing groups in config file" << endl;
	for (i = l.begin(); i != l.end(); ++i)
	{
		s << fname << ": " << *i << endl;

		m = c.entryMap(*i);
		QMap < QString, QString >::ConstIterator j;

		for (j = m.begin(); j != m.end(); ++j)
		{
			s << fname
				<< ": " << j.key() << "=" << j.data() << endl;
		}
	}
	/* NOTREACHED */
	(void) options_id;
}

void listStrList(DEBUGSTREAM & s, const QStringList & l)
{
	FUNCTIONSETUP;

	QStringList::ConstIterator i;
	s << fname << ": Elements of string list:" << endl;

	for (i = l.begin(); i != l.end(); ++i)
	{
		s << fname << ": " << *i << endl;
	}
}

void listStrList(DEBUGSTREAM & s, QStrList & l)
{
	FUNCTIONSETUP;

	s << fname << ": Elements of string list:" << endl;

	for (char *p = l.first(); p; p = l.next())
	{
		s << fname << ":\t" << p << endl;
	}
}


QString qstringExpansion(const QString & s)
{
	FUNCTIONSETUP;
	QString t;

	for (unsigned i = 0; i < s.length(); i++)
	{
		t += s[i];
		t += ' ';
		t += QString::number((int) s[i].unicode());
		t += ' ';
	}

	return t;
}

QString charExpansion(const char *s)
{
	FUNCTIONSETUP;
	QString t;

	while (*s)
	{
		t += QChar(*s);
		t += ' ';
		t += QString::number(*s);
		t += ' ';
		s++;
	}

	return t;
}

ostream & operator << (ostream & o, const QSize & s)
{
	o << s.width() << "x" << s.height();
	return o;
}

kdbgstream & operator << (kdbgstream & o, const QSize & s)
{
	o << s.width() << "x" << s.height();
	return o;
}

static KCmdLineOptions debug_options_[] = {
	{"debug <level>", I18N_NOOP("Set debugging level"), "0"},
	{0, 0, 0}
};

KCmdLineOptions *debug_options = debug_options_;


#else
int const fname = ((int) options_id);
#endif


// $Log$
// Revision 1.2  2002/01/16 22:24:16  adridg
// Avoid lib incompatibility crashes
//
// Revision 1.1  2001/10/08 21:56:02  adridg
// Start of making a separate KPilot lib
//
// Revision 1.18  2001/09/29 16:26:18  adridg
// The big layout change
//
// Revision 1.17  2001/09/24 22:23:28  adridg
// More generalized debugging handling, even on broken platforms
//
// Revision 1.16  2001/09/23 21:42:35  adridg
// Factored out debugging options
//
// Revision 1.15  2001/09/05 21:53:51  adridg
// Major cleanup and architectural changes. New applications kpilotTest
// and kpilotConfig are not installed by default but can be used to test
// the codebase. Note that nothing else will actually compile right now.
//
// Revision 1.14  2001/05/25 16:06:52  adridg
// DEBUG breakage
//
// Revision 1.13  2001/04/16 13:54:17  adridg
// --enable-final file inclusion fixups
//
// Revision 1.12  2001/03/27 11:10:39  leitner
// ported to Tru64 unix: changed all stream.h to iostream.h, needed some
// #ifdef DEBUG because qstringExpand etc. were not defined.
//
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
