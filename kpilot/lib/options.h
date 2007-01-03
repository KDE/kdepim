#ifndef _KPILOT_OPTIONS_H
#define _KPILOT_OPTIONS_H
/* options.h			KPilot
**
** Copyright (C) 1998-2001,2002,2003 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This file defines some global constants and macros for KPilot.
** In particular, KDE2 is defined when KDE2 seems to be the environment
** (is there a better way to do this?). Use of KDE2 to #ifdef sections
** of code is deprecated though.
**
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

// the hex edit widget is in cvs now, so we can enable it globally.
// I still leave this flag here so one can always easily disable
// the generic DB viewer, which uses the widget.
#define USE_KHEXEDIT

// Want to be as careful as possible about casting QStrings back and
// forth, because of the potential for putting UTF-8 encoded data on the HH.
// KHexEdit headers are not safe, though, so don't use this in general.
#ifndef QT_NO_ASCII_CAST
// #define QT_NO_ASCII_CAST		(1)
#endif
#ifndef QT_NO_CAST_ASCII
// #define QT_NO_CAST_ASCII		(1)
#endif


#include "config.h"

#include <unistd.h>     /* For size_t for pilot-link */
#include <qglobal.h>

#if (QT_VERSION < 0x030300)
#error "This is KPilot for KDE3.5 and won't compile with Qt < 3.3.0"
#endif

#ifndef KDE_VERSION
#include <kdeversion.h>
#endif

#if !(KDE_IS_VERSION(3,4,0))
#error "This is KPilot for (really) KDE 3.5 and won't compile with KDE < 3.4.0"
#endif

#if !(KDE_IS_VERSION(3,5,0))
#warning "This is KPilot for KDE 3.5 and might not compile with KDE < 3.5.0"
#endif

#include "pilotLinkVersion.h"

// For QString, and everything else needs it anyway.
#include <qstring.h>
// Dunno, really. Probably because everything needs it.
#include <klocale.h>
// For the debug stuff.
#ifdef DEBUG
#undef NDEBUG
#undef NO_DEBUG
#ifndef DEBUG_CERR
#define DEBUG_CERR
#endif
#endif
#include <kdebug.h>

extern KDE_EXPORT int debug_level;

class KDE_EXPORT KPilotDepthCount
{
public:
	KPilotDepthCount(int area, int level, const char *s);
	~KPilotDepthCount();
	QString indent() const;
	const char *name() const { return fName; } ;
	// if DEBUG_CERR is defined, we can't return std::cerr (by value),
	// since the copy constructor is private!
#ifndef DEBUG_CERR
#ifdef NDEBUG
	inline kndbgstream debug(int area=0)
#else
	inline kdbgstream debug(int area=0)
#endif
	{ return kdDebug(debug_level >= fLevel, area); }
#endif

protected:
	static int depth;
	int fDepth;
	int fLevel;
	const char *fName;
} ;


#ifdef DEBUG
#ifdef __GNUC__
#define KPILOT_FNAMEDEF(l)	KPilotDepthCount fname(DEBUGAREA,l,__FUNCTION__)
#else
#define	KPILOT_FNAMEDEF(l)	KPilotDepthCount fname(DEBUGAREA,l,__FILE__ ":" "__LINE__")
#endif

#define FUNCTIONSETUP		KPILOT_FNAMEDEF(1)
#define FUNCTIONSETUPL(l)	KPILOT_FNAMEDEF(l)
#define DEBUGAREA 		0

#define DEBUGAREA_KPILOT	5510
#define DEBUGAREA_LIBRARY	5511
#define DEBUGAREA_CONDUIT	5512
#define DEBUGAREA_DB		5513

#ifdef DEBUG_CERR
#include <iostream>
#endif

// stderr / iostream-based debugging.
//
//
#ifdef DEBUG_CERR
#include <iostream>
#define DEBUGKPILOT	std::cerr
#define DEBUGLIBRARY	std::cerr
#define DEBUGCONDUIT	std::cerr
#define DEBUGDB		std::cerr
using namespace std;

inline std::ostream& operator <<(std::ostream &o, const QString &s)
	{ if (s.isEmpty()) return o<<"<empty>"; else return o<<s.latin1(); }
inline std::ostream& operator <<(std::ostream &o, const QCString &s)
	{ if (s.isEmpty()) return o<<"<empty>"; else return o << *s; }



inline std::ostream& operator <<(std::ostream &o, const KPilotDepthCount &d)
	{ return o << d.indent() << ' ' << d.name(); }

#else

#warning "kdDebug()-based debugging is deprecated"

// kddebug based debugging
//
//
#define DEBUGKPILOT	fname.debug(DEBUGAREA_KPILOT)
#define DEBUGLIBRARY	fname.debug(DEBUGAREA_LIBRARY)
#define DEBUGCONDUIT	fname.debug(DEBUGAREA_CONDUIT)
#define DEBUGDB         fname.debug(DEBUGAREA_DB)

inline kdbgstream& operator <<(kdbgstream o, const KPilotDepthCount &d)
	{ return o << d.indent() ; }

#endif


// no debugging at all
//
#else
#define DEBUGSTREAM	kndbgstream
#define DEBUGKPILOT	kndDebug()
#define DEBUGLIBRARY	kndDebug()
#define DEBUGCONDUIT	kndDebug()
#define DEBUGDB         kndDebug()

// With debugging turned off, FUNCTIONSETUP doesn't do anything.
//
//
#define FUNCTIONSETUP const int fname = 0; Q_UNUSED(fname);
#define FUNCTIONSETUPL(a) const int fname = a; Q_UNUSED(fname);
#endif

#define KPILOT_VERSION	"4.9.0 (deepsix)"


// Function to expand newlines in rich text to <br>\n
QString rtExpand(const QString &s, bool richText=true);



/**
 * Convert a struct tm from the pilot-link package to a QDateTime
 */
KDE_EXPORT QDateTime readTm(const struct tm &t);
/**
 * Convert a QDateTime to a struct tm for use with the pilot-link package
 */
KDE_EXPORT struct tm writeTm(const QDateTime &dt);
KDE_EXPORT struct tm writeTm(const QDate &dt);


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
