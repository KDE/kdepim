/* pilotMemo.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** See the .cc file for an explanation of what this file is for.
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
#ifndef _KPILOT_PILOTMEMO_H
#define _KPILOT_PILOTMEMO_H

#ifndef _PILOT_MACROS_H_
#include <pi-macros.h>
#endif

#ifndef QSTRING_H
#include <qstring.h>
#endif

#ifndef _KPILOT_PILOTAPPCATEGORY_H
#include "pilotAppCategory.h"
#endif

#ifndef _KPILOT_PILOTRECORD_H
#include "pilotRecord.h"
#endif


class PilotMemo : public PilotAppCategory
{
public:
  PilotMemo(void) : PilotAppCategory() { fText = NULL; fSize = 0; }
  PilotMemo(PilotRecord* rec);
  PilotMemo(void *buf) : PilotAppCategory() { unpack(buf, 1); }
  PilotMemo(void *buf, int attr, recordid_t id, int category)
    : PilotAppCategory(attr, id, category) { unpack(buf, 1); }
  ~PilotMemo() { if (fText) delete fText; if (fTitle) delete fTitle;}
  
  const char *text(void) const { return fText; }
  void setText(const char* text) { unpack(text, 0); }
  const char* getTitle(void) const { return fTitle; }
  PilotRecord* pack() { return PilotAppCategory::pack(); }
  
	typedef enum { MAX_MEMO_LEN=8192 } Constants ;

	/**
	* Return a "short but sensible" title. getTitle() returns the
	* first line of the memo, which may be very long
	* and inconvenient. sensibleTitle() returns about 30
	* characters.
	*/
	QString shortTitle() const;

	/**
	* Returns a (complete) title if there is one and [unknown]
	* otherwise.
	*/
	QString sensibleTitle() const;

protected:
  //     void *pack(int *i);
  void *pack(void *, int *);
  void unpack(const void *, int = 0);
  
private:
  char *fText;
  int fSize;
  char* fTitle;
  void *internalPack(unsigned char *);
};



#else
#warning "File doubly included"
#endif


// $Log$
// Revision 1.7  2001/02/06 08:05:20  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
