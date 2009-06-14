#ifndef KPILOT_PILOTTODOENTRY_H
#define KPILOT_PILOTTODOENTRY_H
/* pilotTodoEntry.h	-*- C++ -*-		KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone <dan@kpilot.org>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

// KPilot headers
#include "pilotAppInfo.h"
#include "pilotRecord.h"

// pilot-link headers
#include <pi-macros.h>
#include <pi-todo.h>

// Other headers
#include <time.h>

/** @file This file defines structures wrapped around the ToDo database
* on the Pilot, based on pilot-link's ToDo stuff.
*/

/** A decoded ToDo item. */
class KPILOT_EXPORT PilotTodoEntry : public PilotRecordBase
{
public:
	/** Create an empty ToDo item. All attributes are 0. */
	PilotTodoEntry();

	/**
	* Constructor. Create a ToDo item and fill it with data from the
	* uninterpreted record @p rec. The record may be NULL, in which
	* case the todo is empty and its category and ID are zero, as in
	* the constructor above.
	*/
	PilotTodoEntry(PilotRecord * rec);

	/** Copy an existing ToDo item. */
	PilotTodoEntry(const PilotTodoEntry &e);

	/** Delete a ToDo item. */
	~PilotTodoEntry()
	{
		free_ToDo(&fTodoInfo);
	}

	/** Return a string for the ToDo item. If @param richText is Qt::RichText, then
	* use qt style markup to make the string clearer when displayed.
	*/
	QString getTextRepresentation(Qt::TextFormat richText) const;

	/** Assign an existing ToDo item to this one. */
	PilotTodoEntry& operator=(const PilotTodoEntry &e);

	/** Accessor for the Due Date of the ToDo item. */
	struct tm getDueDate() const { return fTodoInfo.due; }

	/** Set the Due Date for the ToDo item. */
	void setDueDate(struct tm& d)
	{
		fTodoInfo.due = d;
	}

	/** Return the indefinite status of the ToDo (? that is, whether it
	* had a Due Date that is relevant or not). Return values are 0
	* (not indefinite) or non-0.
	*/
	int getIndefinite() const
	{
		return fTodoInfo.indefinite;
	}

	/** Set whether the ToDo is indefinite or not. */
	void setIndefinite(int i)
	{
		fTodoInfo.indefinite = i;
	}

	/** Return the priority of the ToDo item. The priority ranges
	* from 1-5 on the handheld, so this needs to be mapped (perhaps)
	* onto KOrganizer's priority levels.
	*/
	int getPriority() const
	{
		return fTodoInfo.priority;
	}

	/** Set the priority of the ToDo. */
	void setPriority(int p)
	{
		fTodoInfo.priority = p;
	}

	/** Return whether the ToDo is complete (done, finished) or not. */
	int getComplete() const
	{
		return fTodoInfo.complete;
	}

	/** Set whether the ToDo is done. */
	void setComplete(int c)
	{
		fTodoInfo.complete = c;
	}

	/** Get the ToDo item's description (which is the title shown on
	* the handheld, and the item's Title in KDE). This uses the default codec.
	*/
	QString getDescription() const;
	/** Set the ToDo item's description. */
	void  setDescription(const QString &);

	/** Get the ToDo item's note (the longer text, not immediately accessible
	* on the handheld). This uses the default codec.
	*/
	QString getNote() const;

	/** Set the ToDo item's note. */
	void  setNote(const QString &note);

	/** Returns the label for the category this ToDo item is in. */
	QString getCategoryLabel() const;

	PilotRecord *pack() const;

protected:
	const char *getDescriptionP() const { return fTodoInfo.description; } ;
	void setDescriptionP(const char *, int len=-1) ;
	const char *getNoteP() const { return fTodoInfo.note; } ;
	void setNoteP(const char *, int len=-1) ;

private:
	struct ToDo fTodoInfo;
	unsigned int fDescriptionSize, fNoteSize;
};

inline int _upTDAI(struct ToDoAppInfo *m, const unsigned char *b, size_t s)
{
	return unpack_ToDoAppInfo(m,b,s);
}

inline int _pTDAI(const struct ToDoAppInfo *m, unsigned char *b, size_t s)
{
	return pack_ToDoAppInfo(m,b,s);
}

typedef PilotAppInfo<ToDoAppInfo, _upTDAI,  _pTDAI> PilotToDoInfo;


#endif

