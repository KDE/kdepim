#ifndef _PILOT_HINOTE_H_	/* -*- C++ -*- */
#define _PILOT_HINOTE_H_

#include "pi-args.h"
#include "pi-appinfo.h"

#ifdef __cplusplus
extern "C" {
#endif

	struct HiNoteNote {
		int flags;
		int level;
		char *text;
	};

	struct HiNoteAppInfo {
		struct CategoryAppInfo category;
		unsigned char reserved[48];
	};

	extern void free_HiNoteNote PI_ARGS((struct HiNoteNote *));
	extern int unpack_HiNoteNote
	    PI_ARGS((struct HiNoteNote *, unsigned char *record, int len));
	extern int pack_HiNoteNote
	    PI_ARGS((struct HiNoteNote *, unsigned char *record, int len));
	extern int unpack_HiNoteAppInfo
	    PI_ARGS((struct HiNoteAppInfo *, unsigned char *AppInfo,
		     int len));
	extern int pack_HiNoteAppInfo
	    PI_ARGS((struct HiNoteAppInfo *, unsigned char *AppInfo,
		     int len));

#ifdef __cplusplus
}
#include "pi-hinote.hxx"	//! not implemented

#endif				/*__cplusplus*/
#endif				/* _PILOT_HINOTE_H_ */
