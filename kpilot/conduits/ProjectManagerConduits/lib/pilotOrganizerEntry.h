/* pilotOrganizerEntry.h	-*- C++ -*-		KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
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
** Bug reports and questions can be sent to reinhold@kainhofer.com
*/
#ifndef _KPILOT_PILOTOrganizerENTRY_H
#define _KPILOT_PILOTOrganizerENTRY_H

#include <time.h>
#include <string.h>
#include <qdatetime.h>

#ifndef QBITARRAY_H
#include <qbitarray.h>
#endif

#include "options.h"
#ifndef _KPILOT_PILOTAPPCATEGORY_H
#include "pilotAppCategory.h"
#endif

#include <calendar.h>

using namespace KCal;


#define HAS_PREVIOUS	0x8000
#define HAS_NEXT		0x4000
#define HAS_CHILDREN	0x2000
#define IS_CHECKED	0x1000
#define IS_EXPANDED	0x0800
#define IS_VISIBLE	0x0400
#define FLAG_CUSTOM1	0x0200
#define FLAG_CUSTOM2	0x0100
#define FLAG_TYPE		0x00f0
#define FLAG_PRIORITY	0x000f

typedef enum {
	DATE_CREATED,
	DATE_STARTED,
	DATE_DUE,
	DATE_FINISHED
} po_date_type;

typedef struct entryFlags {
	int hasPrevious:1;
	int hasNext:1;
	int hasChildren:1;
	int checked:1;
	int expanded:1;
	int visible:1;
	int custom2:1;
	int custom1:1;
	
	int type:4;
	int priority:4;
};

typedef struct OrganizerEnty {
	union {
		struct entryFlags flags;
		int iFlags;
	};
	
	QDateTime dates[4];
	
	int progress;
	int level;
	int nr;
	long todolnk;
	
	char*descr;
	char*note;
	char*customstr;
	
} OrganizerEntry;


class PilotOrganizerEntry : public PilotAppCategory {
protected:
  OrganizerEntry fData;
public:
	PilotOrganizerEntry(void);
	PilotOrganizerEntry(const PilotOrganizerEntry &e);
	PilotOrganizerEntry(PilotRecord* rec);
	PilotOrganizerEntry(KCal::Todo*todo);
	~PilotOrganizerEntry() { free_OrganizerEntry(&fData); }

	PilotOrganizerEntry& operator=(const PilotOrganizerEntry &e);
	PilotOrganizerEntry& operator=(const KCal::Todo &todo);
	KCal::Todo* getTodo();

	PilotRecord* pack() { return PilotAppCategory::pack(); }  // ??? What's this for?

	bool getFlag(const int flag) const { return fData.iFlags|flag;}
	void setFlag(const int flag, const bool val) {fData.iFlags=val?(fData.iFlags|flag):(fData.iFlags&(!flag)); }
	
	int getLevel() const { return fData.level; }
	void setLevel(const int l) { fData.level=l; }
	
	int getType() const { return (fData.iFlags&FLAG_TYPE)>>4; }
	void setType(const int t) { fData.iFlags=(fData.iFlags & !FLAG_TYPE) | (t<<4 & FLAG_TYPE); }
	
	int getPriority() const { return (fData.iFlags&FLAG_PRIORITY); }
	void setPriority(const int t) { fData.iFlags=(fData.iFlags & !FLAG_PRIORITY) | (t & FLAG_PRIORITY); }
	
	bool hasDate(po_date_type tp) const {return fData.dates[tp].date().isValid(); }
	QDateTime getDate(po_date_type tp) const { return fData.dates[tp]; }
	void setDate(const po_date_type tp, QDateTime dt) { fData.dates[tp]=dt; }
	void setDate(const po_date_type tp, unsigned short int); // Assign a date directly from the packed value
	void deleteDate(po_date_type tp) {QDate dt; fData.dates[tp].setDate(dt); QTime tm; fData.dates[tp].setTime(tm);}
	
	int getProgress() const { return fData.progress; }
	void setProgress(const int pg) { fData.progress=pg; }
	
	int getNumber() const { return fData.nr; }
	void setNumber(const int n) { fData.nr=n; }
	
	char* getDescription() const { return fData.descr; }
	void setDescription(const char*dsc);// { fData.descr=dsc; }
	
	char* getNote() const { return fData.note; }
	void setNote(const char*nt);// { fData.note=nt; }
	
	char* getCustStr() const { return fData.customstr; }
	void setCustStr(const char*str);// { fData.customstr=str; }
	
	long getTodoLink() const {return fData.todolnk; }
	void setTodoLink(long tdlnk) { fData.todolnk=tdlnk; }

protected:
	virtual void *pack(void *, int *)=0;
	virtual void unpack(const void *, int = 0)=0;

private:
	void free_OrganizerEntry(OrganizerEntry*entry);
};


#else
#ifdef DEBUG
#warning "File doubly included"
#endif
#endif


