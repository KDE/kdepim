/* KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


static const char *options_id =
	"$Id$";

#include "options.h"


#include <iostream>

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

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
QString rtExpand(const QString &s, bool richText)
{
	if (richText)
	{
		QString t(s);
		return t.replace(CSL1("\n"), CSL1("<br>\n"));
	}
	else
		return s;

	Q_UNUSED(options_id);
}

QDateTime readTm(const struct tm &t)
{
  QDateTime dt;
  dt.setDate(QDate(1900 + t.tm_year, t.tm_mon + 1, t.tm_mday));
  dt.setTime(QTime(t.tm_hour, t.tm_min, t.tm_sec));
  return dt;
}



struct tm writeTm(const QDateTime &dt)
{
  struct tm t;

  t.tm_wday = 0; // unimplemented
  t.tm_yday = 0; // unimplemented
  t.tm_isdst = 0; // unimplemented

  t.tm_year = dt.date().year() - 1900;
  t.tm_mon = dt.date().month() - 1;
  t.tm_mday = dt.date().day();
  t.tm_hour = dt.time().hour();
  t.tm_min = dt.time().minute();
  t.tm_sec = dt.time().second();

  return t;
}



struct tm writeTm(const QDate &dt)
{
  struct tm t;

  t.tm_wday = 0; // unimplemented
  t.tm_yday = 0; // unimplemented
  t.tm_isdst = 0; // unimplemented

  t.tm_year = dt.year() - 1900;
  t.tm_mon = dt.month() - 1;
  t.tm_mday = dt.day();
  t.tm_hour = 0;
  t.tm_min = 0;
  t.tm_sec = 0;

  return t;
}

#ifdef DEBUG
KPilotDepthCount::KPilotDepthCount(int level, const char *s) :
	fDepth(depth),
	fLevel(level),
	fName(s)
{
#ifdef DEBUG
	if (debug_level>fLevel) { DEBUGKPILOT << indent() << ">" << name() << endl; }
#endif
	depth++;
}

KPilotDepthCount::~KPilotDepthCount()
{
#ifdef DEBUG
	if (debug_level>fLevel+1) { DEBUGKPILOT << indent() << "<" << name() << endl; }
#endif
	depth--;
}

QString KPilotDepthCount::indent() const
{
	QString s;
	s.fill(' ',fDepth);
	return s+s+' ';
}

int KPilotDepthCount::depth = 0;
#endif
