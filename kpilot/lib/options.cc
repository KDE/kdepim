/* KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot <groot@kde.org>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include "options.h"


#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdebug.h>

#include <config-kpilot.h>

#include <QtCore/QDateTime>
#include <QtCore/QIODevice>

int debug_level = 0;

// The daemon also has a debug level; debug_spaces is 60 spaces,
// to align FUNCTIONSETUP output. The one byte extra is for the NUL.
//
//
static const char debug_spaces[61] =
	"                    "
	"                    "
	"                    ";


QString rtExpand(const QString &s, Qt::TextFormat richText)
{
	if (richText == Qt::RichText)
	{
		QString t(s);
		return t.replace(CSL1("\n"), CSL1("<br/>\n"));
	}
	else
	{
		return s;
	}

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
#ifdef HAVE_STRUCT_TM_TM_ZONE
	t.tm_zone = 0; // unimplemented
#endif

	t.tm_year = dt.date().year() - 1900;
	t.tm_mon = dt.date().month() - 1;
	t.tm_mday = dt.date().day();
	t.tm_hour = dt.time().hour();
	t.tm_min = dt.time().minute();
	t.tm_sec = dt.time().second();

	return t;
}



struct tm writeTm(const QDate &d)
{
	QDateTime dt(d);
	return writeTm(dt);
}

KPilotDepthCount::KPilotDepthCount(int level, const char *s) :
	fDepth(depth),
	fLevel(level),
	fName(s)
{
	depth++;
}

KPilotDepthCount::~KPilotDepthCount()
{
	depth--;
}

const char *KPilotDepthCount::indent() const
{
	if (fDepth < 30)
	{
		return debug_spaces + 60-fDepth*2;
	}
	else
	{
		return debug_spaces;
	}
}

int KPilotDepthCount::depth = 0;

QDebug KPilotDebugStream(const KPilotDepthCount &d)
{
	if  (debug_level >= d.level())
	{
		return QDebug(QtDebugMsg);
	}
	else
	{
		return kDebugDevNull();
	}
}

QDebug KPilotDebugStream()
{
	return QDebug(QtDebugMsg);
}
