// options.h
//
// Copyright (C) 1998,1999 Dan Pilone
// Copyright (C) 2000 Adriaan de Groot
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING.
//
//
// This file defines some global constants and macros for KPilot.
// In particular, KDE2 is defined when KDE2 seems to be the environment
// (is there a better way to do this?). Use of KDE2 to #ifdef sections
// of code is deprecated though.
//
// Many debug functions are defined as well.
//
//
#ifndef _KPILOT_OPTIONS_H_
#define _KPILOT_OPTIONS_H_ 1

#include "config.h"

#ifndef QT_VERSION
#include <qglobal.h>
#endif

#if (QT_VERSION > 199)
#define KDE2
#else
#error "This is KPilot for KDE2 and won't compile with Qt < 2.2.3"
#endif

// Turn OFF as much debugging as possible
//
//
#undef DEBUG

// Define TEST_DEBUG to check whether all the
// calls to kdWarning and kdError use __FUNCTION__
// as they should, instead of the lazier (and incorrect)
// fname. (See below).
//
//
#undef TEST_DEBUG

#include <klocale.h>
#include <kdebug.h>


// KPilot will move towards the *standard* way of doing
// debug messages soon. This means that we need some
// debug areas.
//
//
#define KPILOT_AREA	5510
#define DAEMON_AREA	5511
#define CONDUIT_AREA	5512

#define DEBUGKPILOT	kdDebug(KPILOT_AREA)
#define DEBUGDAEMON	kdDebug(DAEMON_AREA)
#define DEBUGCONDUIT	kdDebug(CONDUIT_AREA)


#ifdef DEBUG
// KPilot contains two kinds of debugging messages:
//
// * Old-style, with constructions like
//
// #ifdef DEBUG
//	if (debug_level & UI_MINOR)
//	{
//		kdDebug() << fname
//			<< ": Creating dialog window." << endl;
//	}
// #endif
//
// These debugging messages are controlled by the debug_level,
// which can be set by the user with --debug. The number of 
// debug levels is confusing, and practice teaches that you
// either want debug_level 0xffffffff or 0. So this is deprecated
// but not yet removed from all the code.
//
//
// * New-style, which looks like
//
//	DEBUGKPILOT << fname << ": Creating dialog window." << endl;
//
// This uses KDE's debug areas (accessible through kdebugdialog)
// to keep track of what to print. No extra #if or if().




// (Old-style) These are three-bit fields, basically we're defining
// 1<<n; 3<<n; 7<<n for some n.
//
// This allows better selection of what you want debugged.
//
//
#define UI_MAJOR	(2+4+8)
#define UI_MINOR	(4+8)
#define UI_TEDIOUS	(8)

// (Old-style) Debug level is set to some bit pattern; if any 
// bit in one of the debug masks (SYNC_MAJOR, for
// instance) is set print the messages corresponding
// to that debug level.
//
//
extern int debug_level;

#define DB_MAJOR	(16+32+64)
#define DB_MINOR	(32+64)
#define DB_TEDIOUS	(64)

#define SYNC_MAJOR	(128+256+512)
#define SYNC_MINOR	(256+512)
#define SYNC_TEDIOUS	(512)

// Both old and new-style debugging suggest (insist?) that
// every function be started with the macro FUNCTIONSETUP,
// which outputs function and line information on every call.
//
//
#define DEBUG_FUNCTIONS	(1)
#define FUNCTIONSETUP	static const char *fname=__FUNCTION__; \
			if (debug_level & DEBUG_FUNCTIONS) { kdDebug() << \
			fname << tabs+(strlen(fname)>>3) \
				<< "(" << __FILE__ << ":" << \
				__LINE__ << ")\n"; } 

extern const char *tabs;		// for indentation purposes in debug

class kdbgstream;
class KConfig;

// Next all kinds of specialty debugging functions,
// added in an ad-hoc fashion.
//
//
void listConfig(kdbgstream&,KConfig&);
void listStrList(kdbgstream&,const QStringList&);
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
// With TEST_DEBUG turned on, fname gets a weird type that is
// incompatible with kdWarning() and kdError(), leading to warnings
// if you mix them. Use __FUNCTION__ instead.
//
// With TEST_DEBUG turned off, fname is an int and you won't get
// warnings. If you use fname by accident in a warning message
// you will get strange results though, since instead of a readable
// function name you'll get some nasty number.
//
//
#define FUNCTIONSETUP
#ifdef TEST_DEBUG
class debugName { public: debugName(int i) : j(i) {} ; int j; } ;
extern const debugName fname;
kndbgstream operator << (kndbgstream s, const debugName&);
#else
extern const int fname;
#endif
#endif


// Some layout macros
//
// SPACING is a generic distance between visual elements;
// 10 seems reasonably good even at high resolutions.
//
//
#define SPACING		(10)
#endif
