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

// #include <iostream>
// #include <pi-memo.h>
// #include <klocale.h>

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
		delete fTitle;
	}

	fSize = strlen((const char *) text) + 1;
	fText = new char[fSize];

	(void) strcpy(fText, (const char *) text);

	int memoTitleLen = 0;

	while (fText[memoTitleLen] && (fText[memoTitleLen] != '\n'))
		memoTitleLen++;
	fTitle = new char[memoTitleLen + 1];

	strncpy(fTitle, fText, memoTitleLen);
	fTitle[memoTitleLen] = 0;
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
	const char *s = getTitle();

	if (s && *s)
	{
		return QString(s);
	}
	else
	{
		return QString(i18n("[unknown]"));
	}
}


// $Log$
// Revision 1.2  2002/08/20 21:18:31  adridg
// License change in lib/ to allow plugins -- which use the interfaces and
// definitions in lib/ -- to use non-GPL'ed libraries, in particular to
// allow the use of libmal which is MPL.
//
// Revision 1.1  2001/10/10 21:47:14  adridg
// Shared files moved from ../kpilot/ and polished
//
// Revision 1.10  2001/09/29 16:26:18  adridg
// The big layout change
//
// Revision 1.9  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.8  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.7  2001/02/07 14:21:54  brianj
// Changed all include definitions for libpisock headers
// to use include path, which is defined in Makefile.
//
// Revision 1.6  2001/02/05 20:58:48  adridg
// Fixed copyright headers for source releases. No code changed
//
