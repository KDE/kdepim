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

#include "options.h"


#include <iostream.h>

#include <qsize.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kcmdlineargs.h>

// The daemon also has a debug level; debug_spaces is 60 spaces,
// to align FUNCTIONSETUP output.
//
//
#ifdef DEBUG
int debug_level = 1;
#else
int debug_level = 0;
#endif
const char *debug_spaces =
	"                                                    ";

QString qstringExpansion(const QString & s)
{
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

kndbgstream & operator << (kndbgstream & o, const QSize & s)
{
	return o;
}

static KCmdLineOptions debug_options_[] = {
	{"debug <level>", I18N_NOOP("Set debugging level"), "0"},
	{0, 0, 0}
};

KCmdLineOptions *debug_options = debug_options_;


int const fname = ((int) options_id);


// $Log$
// Revision 1.4  2002/02/02 11:46:03  adridg
// Abstracting away pilot-link stuff
//
// Revision 1.3  2002/01/18 10:08:00  adridg
// CVS_SILENT: Fixing my compile fixes again
//
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
