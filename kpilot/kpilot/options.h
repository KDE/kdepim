#ifndef _KPILOT_OPTIONS_H
#define _KPILOT_OPTIONS_H
/* options.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
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





#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef QT_VERSION
#include <qglobal.h>
#endif

#if (QT_VERSION < 223)
#error "This is KPilot for KDE2 and won't compile with Qt < 2.2.3"
#endif

// Turn ON as much debugging as possible with -DDEBUG -DDEBUG_CERR
// Some systems have changed kdWarning() and kdDebug() into nops,
// so DEBUG_CERR changes them into cerr again. Odd and disturbing.
//
#ifdef DEBUG_CERR
#include <iostream.h>
#define DEBUGFUNC	cerr
#else
#define DEBUGFUNC	kdDebug()
#endif

#ifndef QSTRING_H
#include <qstring.h>
#endif

#ifndef QSTRLIST_H
#include <qstrlist.h>
#endif

#ifndef _KLOCALE_H
#include <klocale.h>
#endif

#ifndef _KDEBUG_H
#include <kdebug.h>
#endif


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
#define DEBUGKPILOT	cerr
#define DEBUGDAEMON	cerr
#define DEBUGCONDUIT	cerr
#define DEBUGDB		cerr
#else
#define DEBUGKPILOT	kdDebug(KPILOT_AREA)
#define DEBUGDAEMON	kdDebug(DAEMON_AREA)
#define DEBUGCONDUIT	kdDebug(CONDUIT_AREA)
#define DEBUGDB         kdDebug(LIBPILOTDB_AREA)
#endif

#define KPILOT_VERSION	"4.2.9"

#ifdef DEBUG
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

// Both old and new-style debugging suggest (insist?) that
// every function be started with the macro FUNCTIONSETUP,
// which outputs function and line information on every call.
//
//
#define FUNCTIONSETUP	static const char *fname=__FUNCTION__; \
			if (debug_level) { DEBUGFUNC << \
			fname << debug_spaces+(strlen(fname)) \
				<< "(" << __FILE__ << ":" << \
				__LINE__ << ")\n"; }

class KConfig;

// Next all kinds of specialty debugging functions,
// added in an ad-hoc fashion.
//
//
void listConfig(kdbgstream &, KConfig &);
void listStrList(kdbgstream &, const QStringList &);
void listStrList(kdbgstream &, QStrList &);
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
// if you mix them. Use __FUNCTION__ instead.
//
//
#define FUNCTIONSETUP

class debugName {
public:
	debugName(int i) : j(i) { };
	int j;
};
extern const debugName fname;
inline kndbgstream operator << (kndbgstream s, const debugName &d) { s << d.j; return s; } ;
#endif


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


// $Log$
// Revision 1.32  2001/09/16 13:37:48  adridg
// Large-scale restructuring
//
// Revision 1.31  2001/09/08 14:31:54  cschumac
// qt3 include fix
//
// Revision 1.30  2001/09/05 21:53:51  adridg
// Major cleanup and architectural changes. New applications kpilotTest
// and kpilotConfig are not installed by default but can be used to test
// the codebase. Note that nothing else will actually compile right now.
//
// Revision 1.29  2001/08/26 13:17:47  zander
// added includes to make it compile
//
// Revision 1.28  2001/06/13 22:51:38  cschumac
// Minor fixes reviewed on the mailing list.
//
// Revision 1.27  2001/05/24 10:31:38  adridg
// Philipp Hullmann's extensive memory-leak hunting patches
//
// Revision 1.26  2001/04/16 13:48:35  adridg
// --enable-final cleanup and #warning reduction
//
// Revision 1.25  2001/04/14 15:21:35  adridg
// XML GUI and ToolTips
//
// Revision 1.24  2001/04/11 21:33:06  adridg
// Make version number consistent across KPilot applications
//
// Revision 1.23  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.22  2001/03/05 23:57:53  adridg
// Added KPILOT_VERSION
//
// Revision 1.21  2001/03/01 20:43:24  adridg
// Some new (and harmless) debug functions
//
// Revision 1.20  2001/02/06 08:05:19  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
#endif
