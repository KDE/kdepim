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
#error "This is KPilot for KDE2"
#endif

// Turn OFF as much debugging as possible
//
//
#ifdef NDEBUG
#undef DEBUG
#endif

// Define TEST_DEBUG to check whether all the
// calls to kdWarning and kdError use __FUNCTION__
// as they should, instead of the lazier (and incorrect)
// fname.
//
//
#define TEST_DEBUG

#include <klocale.h>
#include <kdebug.h>


// KPilot will move towards the *standard* way of doing
// debug messages soon. This means that we need some
// debug areas.
//
//
#define DEBUGKPILOT	kdDebug(5510)
#define DEBUGDAEMON	kdDebug(5511)
#define DEBUGCONDUIT	kdDebug(5512)


#ifdef DEBUG
// These are three-bit fields, basically we're defining
// 1<<n; 3<<n; 7<<n for some n.
//
// This allows better selection of what you want debugged.
//
//
#define UI_MAJOR	(2+4+8)
#define UI_MINOR	(4+8)
#define UI_TEDIOUS	(8)

// Debug level is set to some bit pattern; if any 
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

#define DEBUG_FUNCTIONS	(1)
#define EFUNCTIONSETUP	static const char *fname=__FUNCTION__; \
			if (debug_level & DEBUG_FUNCTIONS) { kdDebug() << \
			fname << tabs+(strlen(fname)>>3) \
				<< "(" << __FILE__ << ":" << \
				__LINE__ << ")\n"; } 
#define FUNCTIONSETUP	EFUNCTIONSETUP

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
// This is used to define fname in functions that actually
// produce error messages and that need the name of the function.
// For debugging purposes it's also used by FUNCTIONSETUP.
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
