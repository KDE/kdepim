#ifndef _KPILOT_OPTIONS_H
#define _KPILOT_OPTIONS_H
/* options.h			KPilot
**
** Copyright (C) 1998-2001,2002,2003 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2004-2007 Adriaan de Groot <groot@kde.org>
**
** This file defines some global constants and macros for KPilot.
** Many debug functions are defined as well.
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

#include <QtCore/qglobal.h>
#include <QtCore/qnamespace.h>

#include <kdebug.h>
#include <kdeversion.h>
#include <klocale.h>

#include "pilotLinkVersion.h"
#include "kpilot_export.h"

#include <iostream>

#define KPILOT_VERSION	"5.0.0-pre1 (swedish chef)"

extern KDE_EXPORT int debug_level;

class KPILOT_EXPORT KPilotDepthCount
{
public:
	KPilotDepthCount(int level, const char *s);
	~KPilotDepthCount();
	const char *indent() const;
	inline const char *name() const { return fName; } ;
	inline int level() const { return fLevel; } ;

protected:
	static int depth;
	int fDepth;
	int fLevel;
	const char *fName;
} ;

inline std::ostream& operator <<(std::ostream &o, const KPilotDepthCount &d)
{
	return o << d.indent() << ' ' << d.name() << ": ";
}

inline std::ostream& operator <<(std::ostream &o, const QString &s)
{
	if (s.isEmpty())
	{
		return o << "<empty>";
	}
	else
	{
		return o << s.toLatin1().constData();
	}
}

class KPILOT_EXPORT KPilotDebugStream
{
public:
	KPilotDebugStream(int dummy) :
		enabled(false),
		has_printed(false)
	{ Q_UNUSED(dummy); }
	KPilotDebugStream(const KPilotDepthCount &d) :
		enabled(debug_level >= d.level()),
		has_printed(false)
	{
	}
	KPilotDebugStream() : // For warnings, needs no fname / depth
		enabled(true),
		has_printed(false)
	{
	}
	~KPilotDebugStream()
	{
		if (enabled && has_printed)
		{
			std::cerr << std::endl;
		}
	}

	template <typename T> KPilotDebugStream &operator <<(const T &v)
	{
		if (enabled)
		{
			has_printed = true;
			std::cerr << v;
		}
		return *this;
	}

private:
	bool enabled,has_printed;
} ;


#ifdef __GNUC__
#define KPILOT_FNAMEDEF(l)	KPilotDepthCount fname(l,__FUNCTION__)
#else
#define	KPILOT_FNAMEDEF(l)	KPilotDepthCount fname(l,__FILE__ ":" "__LINE__")
#endif

#define FUNCTIONSETUP		KPILOT_FNAMEDEF(1)
#define FUNCTIONSETUPL(l)	KPILOT_FNAMEDEF(l)

// stderr / iostream-based debugging.
//
//
#define DEBUGKPILOT   KPilotDebugStream(fname) << fname
#define WARNINGKPILOT KPilotDebugStream() \
	<< "! " << k_funcinfo << "\n!   "




#if 0
// No debugging at all
//
#define DEBUGSTREAM   kndbgstream
#define DEBUGKPILOT   kDebugDevNull()
#define WARNINGKPILOT kDebugDevNull()
#define FUNCTIONSETUP const int fname = 0; Q_UNUSED(fname);
#define FUNCTIONSETUPL(a) const int fname = a; Q_UNUSED(fname);
#endif


// Function to expand newlines in rich text to <br>\n
QString rtExpand(const QString &s, Qt::TextFormat richText);



/**
 * Convert a struct tm from the pilot-link package to a QDateTime
 */
KPILOT_EXPORT QDateTime readTm(const struct tm &t);
/**
 * Convert a QDateTime to a struct tm for use with the pilot-link package
 */
KPILOT_EXPORT struct tm writeTm(const QDateTime &dt);
KPILOT_EXPORT struct tm writeTm(const QDate &dt);


// Some layout macros
//
// SPACING is a generic distance between visual elements;
// 10 seems reasonably good even at high resolutions.
//
//
#define SPACING		(10)

// Semi-Standard safe-free expression. Argument a may be evaluated more
// than once though, so be careful.
//
//
#define KPILOT_FREE(a)	{ if (a) { ::free(a); a=0L; } }
#define KPILOT_DELETE(a) { if (a) { delete a; a=0L; } }


// This marks strings that need to be i18n()ed in future,
// but cannot be done now due to message freeze. The _P
// variant is to handle plurals and is wrong, but unavoidable.
//
//
#define TODO_I18N(a)	QString::fromLatin1(a)
#define TODO_I18N_P(a,b,c) ((c>1) ? a : b)

// Handle some cases for QT_NO_CAST_ASCII and NO_ASCII_CAST.
// Where possible in the source, known constant strings in
// latin1 encoding are marked with CSL1(), to avoid gobs
// of latin1() or fromlatin1() calls which might obscure
// those places where the code really is translating
// user data from latin1.
//
// The extra "" in CSL1 is to enforce that it's only called
// with constant strings.
//
//
#define CSL1(a)		QString::fromLatin1(a "")

#endif
