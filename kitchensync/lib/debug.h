/* debug.h                      KitchenSync
**
** Copyright (C) 2001 by Adriaan de Groot
**
** This program is part of KitchenSync, the KDE handheld-device
** synchronisation framework.
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


#ifndef _DEBUG_H
#define _DEBUG_H

#include "config.h"

// Define TEST_DEBUG to check whether all the
// calls to kdWarning and kdError use __FUNCTION__
// as they should, instead of the lazier (and incorrect)
// fname. (See below).
//
//
#undef TEST_DEBUG

#ifndef QSTRING_H
#include <qstring.h>
#endif

#ifndef _KLOCALE_H
#include <klocale.h>
#endif

#ifndef _KDEBUG_H
#include <kdebug.h>
#endif


// These are the debug areas reserved for KitchenSync
//
//
// #define KPILOT_AREA	5510
#define DAEMON_AREA	5511
// #define CONDUIT_AREA	5512
// #define LIBPILOTDB_AREA	5513
#define MANAGER_AREA    5514

#define DEBUGDAEMON     kdDebug(DAEMON_AREA)
#define DEBUGMANAGER    kdDebug(MANAGER_AREA)


#ifndef NDEBUG
class KCmdLineOptions;
extern int debug_level;
extern const char *debug_spaces;
extern KCmdLineOptions *debug_options;

// Debugging style suggests that
// every function be started with the macro FUNCTIONSETUP,
// which outputs function and line information on every call.
//
//
#define FUNCTIONSETUP	static const char *fname=__FUNCTION__; \
			if (debug_level) { kdDebug() << \
			fname << debug_spaces+(strlen(fname)) \
				<< "(" << __FILE__ << ":" << \
				__LINE__ << ")\n"; }

class kdbgstream;
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
// is still visible (it isn't all bracketed by #ifdef DEBUG
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
class debugName {
public:
	debugName(int i) : j(i) { };
	int j;
};
extern const debugName fname;
kndbgstream operator << (kndbgstream s, const debugName &);
#else
extern const int fname;
#endif
#endif


#endif

// $Log$
// Revision 1.1.1.1  2001/06/21 19:49:55  adridg
// KitchenSync is the next-gen KDE-PIM Handheld Device Synchronization
// Framework, which aims to integrate all the Handheld sync tools in 
// KDE, such as KPilot and Kandy. (This is the *real* import).
//
