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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/
#include "options.h"


#include <stdlib.h>

#include <qdatetime.h>
#include <qnamespace.h>

#include <kglobal.h>
#include <kdebug.h>


#include "pilotTodoEntry.h"


PilotTodoEntry::PilotTodoEntry() :
	fDescriptionSize(0),
	fNoteSize(0)
{
	FUNCTIONSETUP;
	::memset(&fTodoInfo, 0, sizeof(struct ToDo));
}

PilotTodoEntry::PilotTodoEntry(PilotRecord * rec) :
	PilotRecordBase(rec),
	fDescriptionSize(0),
	fNoteSize(0)
{
	::memset(&fTodoInfo, 0, sizeof(struct ToDo));
	if (rec)
	{
		pi_buffer_t b;
		b.data = (unsigned char *) rec->data();
		b.allocated = b.used = rec->size();
		unpack_ToDo(&fTodoInfo, &b, todo_v1);
		if (fTodoInfo.description)
		{
			// Assume size of buffer allocated is just large enough;
			// count trailing NUL as well.
			fDescriptionSize = strlen(fTodoInfo.description)+1;
		}
		if (fTodoInfo.note)
		{
			// Same
			fNoteSize = strlen(fTodoInfo.note)+1;
		}
	}

}


PilotTodoEntry::PilotTodoEntry(const PilotTodoEntry & e) :
	PilotRecordBase( &e ),
	fDescriptionSize(0),
	fNoteSize(0)
{
	FUNCTIONSETUP;
	::memcpy(&fTodoInfo, &e.fTodoInfo, sizeof(fTodoInfo));
	// See PilotDateEntry::operator = for details
	fTodoInfo.description = 0L;
	fTodoInfo.note = 0L;

	setDescriptionP(e.getDescriptionP());
	setNoteP(e.getNoteP());
}


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
		fDescriptionSize = 0;
		fNoteSize = 0;

		setDescriptionP(e.getDescriptionP());
		setNoteP(e.getNoteP());

	}

	return *this;
}

QString PilotTodoEntry::getTextRepresentation(Qt::TextFormat richText)
{
	QString text, tmp;
	QString par = (richText==Qt::RichText) ?CSL1("<p>"): QString();
	QString ps = (richText==Qt::RichText) ?CSL1("</p>"):CSL1("\n");
	QString br = (richText==Qt::RichText) ?CSL1("<br/>"):CSL1("\n");

	// title + name
	text += par;
	tmp= (richText==Qt::RichText) ?CSL1("<b><big>%1</big></b>"):CSL1("%1");
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
		text += (richText==Qt::RichText) ?CSL1("<hr/>"):CSL1("-------------------------\n");
		text+=par;
		text+= (richText==Qt::RichText) ?i18n("<b><em>Note:</em></b><br>"):i18n("Note:\n");
		text+=rtExpand(getNote(), richText);
		text+=ps;
	}

	return text;
}

PilotRecord *PilotTodoEntry::pack() const
{
	int i;

	pi_buffer_t *b = pi_buffer_new( sizeof(fTodoInfo) );
	i = pack_ToDo(const_cast<ToDo_t *>(&fTodoInfo), b, todo_v1);
	if (i<0)
	{
		return 0;
	}
	// pack_ToDo sets b->used
	return new PilotRecord( b, this );
}

void PilotTodoEntry::setDescription(const QString &desc)
{
	if (desc.length() < fDescriptionSize)
	{
		Pilot::toPilot(desc, fTodoInfo.description, fDescriptionSize);
	}
	else
	{
		setDescriptionP(Pilot::toPilot(desc),desc.length());
	}
}

void PilotTodoEntry::setDescriptionP(const char *desc, int len)
{
	KPILOT_FREE(fTodoInfo.description);
	if (desc && *desc)
	{
		if (-1 == len)
		{
			len=::strlen(desc);
		}

		fDescriptionSize = len+1;
		fTodoInfo.description = (char *)::malloc(len + 1);
		if (fTodoInfo.description)
		{
			strncpy(fTodoInfo.description, desc, len);
			fTodoInfo.description[len] = 0;
		}
		else
		{
			WARNINGKPILOT << "malloc() failed, description not set"
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
	return Pilot::fromPilot(getDescriptionP());
}

void PilotTodoEntry::setNote(const QString &note)
{
	if (note.length() < fNoteSize)
	{
		Pilot::toPilot(note, fTodoInfo.note, fNoteSize);
	}
	else
	{
		setNoteP(Pilot::toPilot(note),note.length());
	}
}

void PilotTodoEntry::setNoteP(const char *note, int len)
{
	KPILOT_FREE(fTodoInfo.note);
	if (note && *note)
	{
		if (-1 == len)
		{
			len=::strlen(note);
		}

		fNoteSize = len+1;
		fTodoInfo.note = (char *)::malloc(len + 1);
		if (fTodoInfo.note)
		{
			strncpy(fTodoInfo.note, note, len);
			fTodoInfo.note[len] = 0;
		}
		else
		{
			WARNINGKPILOT << "malloc() failed, note not set" << endl;
		}
	}
	else
	{
		fTodoInfo.note = 0L;
	}
}

QString PilotTodoEntry::getNote() const
{
	return Pilot::fromPilot(getNoteP());
}

