#ifndef _KPILOT_PILOTTODOENTRY_H
#define _KPILOT_PILOTTODOENTRY_H
/* pilotTodoEntry.h	-*- C++ -*-		KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This is a wrapper around the pilot-link Memo structure. It is
** the interpreted form of a Pilot database record.
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

#include <time.h>

#include <pi-macros.h>
#include <pi-todo.h>

#include <qstring.h>

#include "pilotAppCategory.h"



class KDE_EXPORT PilotTodoEntry : public PilotAppCategory
{
public:
	PilotTodoEntry(struct ToDoAppInfo &appInfo);
	PilotTodoEntry(struct ToDoAppInfo &appInfo, PilotRecord * rec);
	PilotTodoEntry(const PilotTodoEntry &e);

	~PilotTodoEntry() { free_ToDo(&fTodoInfo); }

	virtual QString getTextRepresentation(bool richText=false);

	PilotTodoEntry& operator=(const PilotTodoEntry &e);

	struct tm getDueDate() const { return fTodoInfo.due; }
	void setDueDate(struct tm& d) { fTodoInfo.due = d; }
	const struct tm *getDueDate_p() const { return &fTodoInfo.due; }

	int getIndefinite() const { return fTodoInfo.indefinite; }
	void setIndefinite(int i) { fTodoInfo.indefinite = i; }

	int getPriority() const { return fTodoInfo.priority; }
	void setPriority(int p) { fTodoInfo.priority = p; }

	int getComplete() const { return fTodoInfo.complete; }
	void setComplete(int c) { fTodoInfo.complete = c; }

	void  setDescription(const QString &);
	QString getDescription() const;

	void  setNote(const QString &note);
	QString getNote() const;

	QString getCategoryLabel() const;


	/** If the label already exists, uses the id; if not, adds the label
	*  to the category list
	*  @return false if category labels are full
	*/
	inline bool setCategory(const QString &label) { return setCat(fAppInfo.category,label);  };

	// static const int APP_BUFFER_SIZE;

protected:
	void *pack_(void *, int *);
	void unpack(const void *, int = 0) { } ;

	const char *getDescriptionP() const { return fTodoInfo.description; } ;
	void setDescriptionP(const char *, int len=-1) ;
	const char *getNoteP() const { return fTodoInfo.note; } ;
	void setNoteP(const char *, int len=-1) ;

private:
	struct ToDo fTodoInfo;
	struct ToDoAppInfo &fAppInfo;
};



#else
#ifdef DEBUG
#warning "File doubly included"
#endif
#endif
