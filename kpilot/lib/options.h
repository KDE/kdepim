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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

// the hex edit widget is in cvs now, so we can enable it globally.
// I still leave this flag here so one can always easily disable
// the generic DB viewer, which uses the widget.
#define USE_KHEXEDIT

// #define QT_NO_ASCII_CAST		(1)
// #define QT_NO_CAST_ASCII		(1)
// #define DEBUG			(1)
// #define DEBUG_CERR			(1)

#include "config.h"
#include <unistd.h>     /* For size_t for pilot-link */
#include <qglobal.h>
#include <pi-version.h>

#if (QT_VERSION < 0x030200)
#error "This is KPilot for KDE3.2 and won't compile with Qt < 3.2.0"
#endif

#ifndef KDE_VERSION
#include <kdeversion.h>
#endif

#if !(KDE_IS_VERSION(3,2,0))
#error "This is KPilot for KDE 3.2 and won't compile with KDE < 3.2.0"
#endif


#ifndef PILOT_LINK_VERSION
#error "You need at least pilot-link version 0.9.5"
#endif


#define PILOT_LINK_NUMBER	((10000*PILOT_LINK_VERSION) + \
				(100*PILOT_LINK_MAJOR)+PILOT_LINK_MINOR)
#define PILOT_LINK_0_10_0	(1000)
#define PILOT_LINK_0_11_0	(1100)
#define PILOT_LINK_0_11_8	(1108)
#define PILOT_LINK_0_12_0	(1200)

#if PILOT_LINK_NUMBER < PILOT_LINK_0_11_8
#warning "You need at least pilot-link version 0.11.8 for modern devices"
#endif

#if PILOT_LINK_NUMBER < PILOT_LINK_0_12_0
#define PI_SIZE_T int
#else
#define PI_SIZE_T size_t
#endif


// Turn ON as much debugging as possible with -DDEBUG -DDEBUG_CERR
// Some systems have changed kdWarning() and kdDebug() into nops,
// so DEBUG_CERR changes them into cerr again. Odd and disturbing.
//

#ifdef DEBUG_CERR
using namespace std;
#define DEBUGFUNC	std::cerr
#else
#define DEBUGFUNC	kdDebug()
#endif

// For QString, and everything else needs it anyway.
#include <qstring.h>
// Dunno, really. Probably because everything needs it.
#include <klocale.h>
// For the debug stuff.
#include <kdebug.h>

// KPilot will move towards the *standard* way of doing
// debug messages soon. This means that we need some
// debug areas.
//
//
#define KPILOT_AREA	5510
#define DAEMON_AREA	5511
#define CONDUIT_AREA	5512
#define LIBPILOTDB_AREA	5513

#ifdef DEBUG_CERR
// For ostream
#include <iostream>
#define DEBUGSTREAM	std::ostream
#define DEBUGKPILOT	std::cerr << fname_.string()
#define DEBUGDAEMON	std::cerr << fname_.string()
#define DEBUGCONDUIT	std::cerr << fname_.string()
#define DEBUGDB		std::cerr << fname_.string()

inline std::ostream& operator <<(std::ostream &o, const QString &s) { if (s.isEmpty()) return o<<"<empty>"; else return o<<s.latin1(); }
inline std::ostream& operator <<(std::ostream &o, const QCString &s) { if (s.isEmpty()) return o<<"<empty>"; else return o << *s; }

#else
#define DEBUGSTREAM	kdbgstream
#define DEBUGKPILOT	kdDebug(KPILOT_AREA) << fname_.string()
#define DEBUGDAEMON	kdDebug(DAEMON_AREA) << fname_.string()
#define DEBUGCONDUIT	kdDebug(CONDUIT_AREA) << fname_.string()
#define DEBUGDB         kdDebug(LIBPILOTDB_AREA) << fname_.string()
#endif

#define KPILOT_VERSION	"4.4.7 (baby)"

// * KPilot debugging code looks like:
//
//      DEBUGKPILOT << fname << ": Creating dialog window." << endl;
//
// This uses KDE's debug areas (accessible through kdebugdialog)
// to keep track of what to print. No extra #if or if(), since the
// global NDEBUG flag changes all the kdDebug() calls into nops and
// the compiler optimizes them away. There are four DEBUG* macros,
// defined above. Use the areas *_AREA in calls to kdWarning() or
// kdError() to make sure the right output is generated.


extern int debug_level;
extern const char *debug_spaces;

// Function to expand newlins in rich text to <br>\n
QString rtExpand(const QString &s, bool richText=true);


#ifdef DEBUG
// Both old and new-style debugging suggest (insist?) that
// every function be started with the macro FUNCTIONSETUP,
// which outputs function and line information on every call.
//
//
class KPilotDepthCount { public: KPilotDepthCount(); ~KPilotDepthCount(); QString string() const; static int depth; protected: int fDepth; } ;

#ifdef __GNUC__
#define KPILOT_FNAMEDEF	KPilotDepthCount fname_; static const char *fname=__FUNCTION__
// #define KPILOT_LOCNDEF __FILE__ << ":" << __LINE__
#define KPILOT_LOCNDEF ""
#else
#define	KPILOT_FNAMEDEF	KPilotDepthCount fname_; static const char *fname=__FILE__ ":" "__LINE__"
#define KPILOT_LOCNDEF	""
#endif

#define FUNCTIONSETUP	KPILOT_FNAMEDEF; \
			if (debug_level) { DEBUGFUNC << fname_.string() << KPILOT_LOCNDEF << ":" << fname << endl; }
#define FUNCTIONSETUPL(l)	KPILOT_FNAMEDEF; \
				if (debug_level>l) { DEBUGFUNC << fname_.string() << KPILOT_LOCNDEF << fname << endl; }


#else
// With debugging turned off, FUNCTIONSETUP doesn't do anything.
// In particular it doesn't give functions a local variable fname,
// like FUNCTIONSETUP does in the debugging case.
//
//
#define FUNCTIONSETUP
#define FUNCTIONSETUPL(a)
#endif

/**
 * Convert a struct tm from the pilot-link package to a QDateTime
 */
QDateTime readTm(const struct tm &t);
/**
 * Convert a QDateTime to a struct tm for use with the pilot-link package
 */
struct tm writeTm(const QDateTime &dt);
struct tm writeTm(const QDate &dt);


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
// but cannot be done now due to message freeze.
//
//
#define TODO_I18N(a)	QString::fromLatin1(a)

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
