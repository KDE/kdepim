/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This is a C++ wrapper for the todo-list entry structures.
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
#include "options.h"

#include <stdlib.h>

#include <qtextcodec.h>
#include <qdatetime.h>
#include <kdebug.h>


#include "pilotTodoEntry.h"

static const char *pilotTodoEntry_id = "$Id$";


PilotTodoEntry::PilotTodoEntry(struct ToDoAppInfo &appInfo):PilotAppCategory(), fAppInfo(appInfo)
{
	FUNCTIONSETUP;
	::memset(&fTodoInfo, 0, sizeof(struct ToDo));
}

/* initialize the entry from another one. If rec==NULL, this constructor does the same as PilotTodoEntry()
*/
PilotTodoEntry::PilotTodoEntry(struct ToDoAppInfo &appInfo, PilotRecord * rec):PilotAppCategory(rec), fAppInfo(appInfo)
{
	::memset(&fTodoInfo, 0, sizeof(struct ToDo));
	if (rec)
	{
		unpack_ToDo(&fTodoInfo, (unsigned char *) rec->getData(),
			rec->getLen());
	}
	(void) pilotTodoEntry_id;
}


PilotTodoEntry::PilotTodoEntry(const PilotTodoEntry & e):PilotAppCategory(e), fAppInfo(e.fAppInfo)
{
	FUNCTIONSETUP;
	::memcpy(&fTodoInfo, &e.fTodoInfo, sizeof(fTodoInfo));
	// See PilotDateEntry::operator = for details
	fTodoInfo.description = 0L;
	fTodoInfo.note = 0L;

	setDescriptionP(e.getDescriptionP());
	setNoteP(e.getNoteP());

}				// end of copy constructor


PilotTodoEntry & PilotTodoEntry::operator = (const PilotTodoEntry & e)
{
	if (this != &e)
	{
		KPILOT_FREE(fTodoInfo.description);
		KPILOT_FREE(fTodoInfo.note);

		::memcpy(&fTodoInfo, &e.fTodoInfo, sizeof(fTodoInfo));
		// See PilotDateEntry::operator = for details
		fTodoInfo.description = 0L;
		fTodoInfo.note = 0L;

		setDescriptionP(e.getDescriptionP());
		setNoteP(e.getNoteP());

	}

	return *this;
}				// end of assignment operator

QString PilotTodoEntry::getTextRepresentation(bool richText)
{
	QString text, tmp;
	QString par = richText?CSL1("<p>"):CSL1("");
	QString ps = richText?CSL1("</p>"):CSL1("\n");
	QString br = richText?CSL1("<br/>"):CSL1("\n");

	// title + name
	text += par;
	tmp=richText?CSL1("<b><big>%1</big></b>"):CSL1("%1");
	text += tmp.arg(rtExpand(getDescription(), richText));
	text += ps;

	text += par;
	if (getComplete())
		text += i18n("Completed");
	else
		text += i18n("Not completed");
	text += ps;

	if (!getIndefinite())
	{
		QDate dt(readTm(getDueDate()).date());
		QString dueDate(dt.toString(Qt::LocalDate));
		text+=par;
		text+=i18n("Due date: %1").arg(dueDate);
		text+=ps;
	}

	text+=par;
	text+=ps;

	text+=par;
	text+=i18n("Priority: %1").arg(getPriority());
	text+=ps;

	if (!getNote().isEmpty())
	{
		text += richText?CSL1("<hr/>"):CSL1("-------------------------\n");
		text+=par;
		text+=richText?i18n("<b><em>Note:</em></b><br>"):i18n("Note:\n");
		text+=rtExpand(getNote(), richText);
		text+=ps;
	}

	return text;
}

bool PilotTodoEntry::setCategory(const QString &label)
{
	PILOTAPPCATEGORY_SETCATEGORY(fAppInfo,label)
}

QString PilotTodoEntry::getCategoryLabel() const
{
	return codec()->toUnicode(fAppInfo.category.name[getCat()]);
}

void *PilotTodoEntry::pack(void *buf, int *len)
{
	int i;

	i = pack_ToDo(&fTodoInfo, (unsigned char *) buf, *len);
	*len = i;
	return buf;
}

void PilotTodoEntry::setDescription(const QString &desc)
{
	setDescriptionP(codec()->fromUnicode(desc),desc.length());
}

void PilotTodoEntry::setDescriptionP(const char *desc, int len)
{
	KPILOT_FREE(fTodoInfo.description);
	if (desc && *desc)
	{
		if (-1 == len) len=::strlen(desc);

		fTodoInfo.description = (char *)::malloc(len + 1);
		if (fTodoInfo.description)
		{
			strlcpy(fTodoInfo.description, desc, len+1);
		}
		else
		{
			kdError(LIBPILOTDB_AREA) << __FUNCTION__
				<< ": malloc() failed, description not set"
				<< endl;
		}
	}
	else
	{
		fTodoInfo.description = 0L;
	}
}

QString PilotTodoEntry::getDescription() const
{
	return codec()->toUnicode(getDescriptionP());
}

void PilotTodoEntry::setNote(const QString &note)
{
	setNoteP(codec()->fromUnicode(note),note.length());
}

void PilotTodoEntry::setNoteP(const char *note, int len)
{
	KPILOT_FREE(fTodoInfo.note);
	if (note && *note)
	  {
	    if (-1 == len) len=::strlen(note);
		fTodoInfo.note = (char *)::malloc(len + 1);
		if (fTodoInfo.note)
		{
		    strlcpy(fTodoInfo.note, note, len+1);
		}
		else
		{
			kdError(LIBPILOTDB_AREA) << __FUNCTION__
				<< ": malloc() failed, note not set" << endl;
		}
	}
	else
	{
		fTodoInfo.note = 0L;
	}
}

QString PilotTodoEntry::getNote() const
{
	return codec()->toUnicode(getNoteP());
}

