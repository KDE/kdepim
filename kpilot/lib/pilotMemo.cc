/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone <dan@kpilot.org>
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
#include "pilotMemo.h"

#include "options.h"
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
	
	// Total length including terminating NUL
	int len = qMin(fText.length() + 1, MAX_MEMO_LEN);
	pi_buffer_t *b = pi_buffer_new(len);
	if (!b)
	{
		return 0;
	}

	// put our text into buf; toPilot() doesn't NUL terminate
	i = Pilot::toPilot(fText, buf.text, len-1);
	b->data[len-1] = 0; // NUL terminate

	if (i<0)
	{
		pi_buffer_free(b);
		return 0;
	}

	b->used = len;
	PilotRecord *r = new PilotRecord(b, this); // Ownership of b given to r
	return r;
}


QString PilotMemo::getTextRepresentation(Qt::TextFormat richText)
{
	if (richText==Qt::RichText)
	{
		return i18n("<i>Title:</i> %1<br/>\n<i>MemoText:</i><br/>%2",rtExpand(getTitle(), richText),rtExpand(text(), richText));
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
		return QString();
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
		return i18nc("No sensible title known for this memo.","[unknown]");
	}
}

