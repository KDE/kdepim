/* options.cc			KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot
**
** This is a file of odds and ends, with debugging functions and stuff.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
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


#include <iostream>

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

#if KDE_VERSION < 319
kdbgstream & operator << (kdbgstream & o, const QSize & s)
{
	o << s.width() << "x" << s.height();
	return o;
}

kndbgstream & operator << (kndbgstream & o, const QSize &)
{
	return o;
}
#endif

static KCmdLineOptions debug_options_[] = {
	{"debug <level>", I18N_NOOP("Set debugging level"), "0"},
	{0, 0, 0}
};

KCmdLineOptions *debug_options = debug_options_;


int const fname = ((int) options_id);


