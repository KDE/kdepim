#ifndef _KPILOT_OPTIONS_H
#define _KPILOT_OPTIONS_H
/* options.h			KPilot
**
** Copyright (C) 1998-2001,2002,2003 by Dan Pilone
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/



// #define QT_NO_ASCII_CAST		(1)
// #define QT_NO_CAST_ASCII		(1)


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef QT_VERSION
#include <qglobal.h>
#endif

#if (QT_VERSION < 223)
#error "This is KPilot for KDE2 and won't compile with Qt < 2.2.3"
#endif

#ifndef KDE_VERSION
#include <kdeversion.h>
#endif

#if KDE_VERSION > 289
#define KDE3
#undef KDE2
#else
#undef KDE3
#define KDE2
#endif

// Turn ON as much debugging as possible with -DDEBUG -DDEBUG_CERR
// Some systems have changed kdWarning() and kdDebug() into nops,
// so DEBUG_CERR changes them into cerr again. Odd and disturbing.
//


#ifdef DEBUG_CERR
#define DEBUGFUNC	cerr
#else
#define DEBUGFUNC	kdDebug()
#endif

// For ostream
#include <iostream>
// For QString, and everything else needs it anyway.
#include <qstring.h>
// Dunno, really. Probably because everything needs it.
#include <klocale.h>
// For the debug stuff.
#include <kdebug.h>

using namespace std;

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
#define DEBUGSTREAM	ostream
#define DEBUGKPILOT	cerr
#define DEBUGDAEMON	cerr
#define DEBUGCONDUIT	cerr
#define DEBUGDB		cerr
#else
#define DEBUGSTREAM	kdbgstream
#define DEBUGKPILOT	kdDebug(KPILOT_AREA)
#define DEBUGDAEMON	kdDebug(DAEMON_AREA)
#define DEBUGCONDUIT	kdDebug(CONDUIT_AREA)
#define DEBUGDB         kdDebug(LIBPILOTDB_AREA)
#endif

#define KPILOT_VERSION	"4.4.0 (HEAD)"

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

class KCmdLineOptions;
extern KCmdLineOptions *debug_options;

#ifdef DEBUG
// Both old and new-style debugging suggest (insist?) that
// every function be started with the macro FUNCTIONSETUP,
// which outputs function and line information on every call.
//
//
#ifdef __GNUC__
#define KPILOT_FNAMEDEF	static const char *fname=__FUNCTION__
#define KPILOT_LOCNDEF	debug_spaces+(::strlen(fname)) \
				<< "(" << __FILE__ << ":" << \
				__LINE__ << ")\n"
#else
#define	KPILOT_FNAMEDEF	static const char *fname=__FILE__ ":" "__LINE__"
#define KPILOT_LOCNDEF	"\n"
#endif

#define FUNCTIONSETUP	KPILOT_FNAMEDEF; \
			if (debug_level) { DEBUGFUNC << \
			fname << KPILOT_LOCNDEF ; }
#define FUNCTIONSETUPL(l)	KPILOT_FNAMEDEF; \
				if (debug_level>l) { DEBUGFUNC << \
				fname << KPILOT_LOCNDEF; }

class KConfig;

// Next all kinds of specialty debugging functions,
// added in an ad-hoc fashion.
//
//
QString qstringExpansion(const QString &);
QString charExpansion(const char *);

#else
// With debugging turned off, FUNCTIONSETUP doesn't do anything.
// In particular it doesn't give functions a local variable fname,
// like FUNCTIONSETUP does in the debugging case. Since code like
//
// DEBUGKPILOT << fname << ": Help! I'm descructing" << endl;
//
// is still visible in KPilot (it isn't all bracketed by #ifdef DEBUG
// and it doesn't *need* to be, that's the whole point of kdDebug())
// we still need *something* with the name fname. So we'll declare a
// single extern fname here.
//
// fname gets a weird type that is
// incompatible with kdWarning() and kdError(), leading to warnings
// if you mix them. Use k_funcinfo instead.
//
//
#define FUNCTIONSETUP
#define FUNCTIONSETUPL(a)
#endif

class KConfig;

// Next all kinds of specialty debugging functions,
// added in an ad-hoc fashion.
//
//
QString qstringExpansion(const QString &);
QString charExpansion(const char *);

// class QStringList;
// ostream& operator <<(ostream&,const QStringList &);
// kdbgstream& operator <<(kdbgstream&,const QStringList &);


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
