/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is a C++ wrapper for the Pilot's Memo Pad structures.
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

#include <qnamespace.h>

#include "pilotMemo.h"
#include "pilotDatabase.h"



PilotMemo::PilotMemo(const PilotRecord * rec) : PilotRecordBase(rec)
{
	FUNCTIONSETUP;
	fText = Pilot::fromPilot((const char *)(rec->data()),rec->size());
}

PilotRecord *PilotMemo::pack()
{
	FUNCTIONSETUPL(4);
	int i;
	
	int len = fText.length() + 8;
	struct Memo buf;
	buf.text = new char[len];

	// put our text into buf
	i = Pilot::toPilot(fText, buf.text, len);

	pi_buffer_t *b = pi_buffer_new(len);
	i = pack_Memo(&buf, b, memo_v1);

	DEBUGKPILOT << "Original text: [" << fText 
		<< "], buf.text: [" << buf.text 
		<< "], b->data: [" << b->data << ']';

	if (i<0)
	{
		// Generic error from the pack_*() functions.
		delete[] buf.text;
		return 0;
	}

	// pack_Appointment sets b->used
	PilotRecord *r = new PilotRecord(b, this);
	delete[] buf.text;
	return r;
}


QString PilotMemo::getTextRepresentation(Qt::TextFormat richText)
{
	if (richText==Qt::RichText)
	{
		return i18n("<i>Title:</i> %1<br>\n<i>MemoText:</i><br>%2",rtExpand(getTitle(), richText),rtExpand(text(), richText));
	}
	else
	{
		return i18n("Title: %1\nMemoText:\n%2",getTitle(),text());
	}
}


QString PilotMemo::getTitle() const
{
	if (fText.isEmpty())
	{
		return QString::null;
	}

	int memoTitleLen = fText.indexOf('\n');
	// If not found, then use the whole memo text
	if (memoTitleLen < 0)
	{
		return fText;
	}
	return fText.left(memoTitleLen);
}

QString PilotMemo::shortTitle() const
{
	FUNCTIONSETUP;
	QString t = QString(getTitle()).simplified();

	if (t.length() < 32)
	{
		return t;
	}

	// Chop off at 40 characters, then search backwards to the last
	// space and, if that makes sense, chop off there. This gives
	// us a string between 32 and 40 characters long.
	t.truncate(40);
	int spaceIndex = t.lastIndexOf(' ');
	if (spaceIndex > 32)
	{
		t.truncate(spaceIndex);
	}

	t += CSL1("...");

	return t;
}

QString PilotMemo::sensibleTitle() const
{
	FUNCTIONSETUP;
	QString s = getTitle();

	if (!s.isEmpty())
	{
		return s;
	}
	else
	{
		return i18n("[unknown]");
	}
}

