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
// (is there a better way to do this?) and many debug functions
// are defined as well.
//
//

#ifndef QT_VERSION
#include <qglobal.h>
#endif

#if (QT_VERSION > 199)
#define KDE2
#else
#undef KDE2
#endif

#ifdef KDE2
#include <klocale.h>
#endif

extern int debug_level;
extern const char *tabs;

void usage(const char *banner, struct option *longOptions);


// These are three-bit fields, basically we're defining
// 1<<n; 3<<n; 7<<n for some n.
//
// This allows better selection of what you want debugged.
//
//
#define DEBUG_FUNCTIONS	(1)
#define UI_MAJOR	(2)
#define UI_MINOR	(2+4)
#define UI_TEDIOUS	(2+4+8)

#define DB_MAJOR	(16)
#define DB_MINOR	(16+32)
#define DB_TEDIOUS	(16+32+64)

#define SYNC_MAJOR	(128)
#define SYNC_MINOR	(128+256)
#define SYNC_TEDIOUS	(128+256+512)

#define FUNCTIONSETUP	static char *fname=__FUNCTION__; \
			if (debug_level & DEBUG_FUNCTIONS) { cerr << \
			fname << tabs+(strlen(fname)>>3) \
				<< "(" << __FILE__ << ':' << \
				__LINE__ << ")\n"; } 

// Some layout macros
//
// SPACING is a generic distance between visual elements;
// 10 seems reasonably good even at high resolutions.
//
// Give RIGHT and BELOW a QWidget. In all likelihood
// these will disappear soon with a new layout style in
// KPilot 3.2.
//
#define SPACING		(10)
#define BELOW(a)	a->y()+a->height()+SPACING
#define RIGHT(a)	a->x()+a->width()+SPACING



