/* pilotMemo.cc			KPilot
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/
static const char *pilotMemo_id =
	"$Id$";

#include "options.h"

#include <qtextcodec.h>

#include "pilotMemo.h"



PilotMemo::PilotMemo(PilotRecord * rec) : PilotAppCategory(rec)
{
	FUNCTIONSETUP;
	unpack(rec->getData(), 1);
	(void) pilotMemo_id;
}

void PilotMemo::unpack(const void *text, int firstTime)
{
	FUNCTIONSETUP;
	if (!firstTime && fText)
	{
		delete fText;
	}

	fSize = strlen((const char *) text) + 1;
	fText = new char[fSize];

	(void) strcpy(fText, (const char *) text);

	int memoTitleLen = 0;

	while (fText[memoTitleLen] && (fText[memoTitleLen] != '\n'))
		memoTitleLen++;
	// Null-terminate for a moment so that toUnicode() works.
	char c = fText[memoTitleLen];
	fText[memoTitleLen]=0;
	fTitle = codec()->toUnicode(fText);
	fText[memoTitleLen]=c;
}

// The indirection just to make the base class happy
void *PilotMemo::internalPack(unsigned char *buf)
{
	FUNCTIONSETUP;
	return strcpy((char *) buf, fText);
}

void *PilotMemo::pack(void *buf, int *len)
{
	FUNCTIONSETUP;
	if (*len < fSize)
		return NULL;

	*len = fSize;

	return internalPack((unsigned char *) buf);
}


QString PilotMemo::shortTitle() const
{
	FUNCTIONSETUP;
	QString t = QString(getTitle()).simplifyWhiteSpace();

	if (t.length() < 32)
		return t;
	t.truncate(40);

	int spaceIndex = t.findRev(' ');

	if (spaceIndex > 32)
	{
		t.truncate(spaceIndex);
	}

	t += "...";

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
		return QString(i18n("[unknown]"));
	}
}
