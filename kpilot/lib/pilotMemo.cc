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

void PilotMemo::unpack(const void *text, int /* firstTime */)
{
	FUNCTIONSETUP;

	fText = codec()->toUnicode((const char *)text);
}

// The indirection just to make the base class happy
void *PilotMemo::internalPack(unsigned char *buf)
{
	FUNCTIONSETUP;
	QCString s = codec()->fromUnicode(fText);
	return strcpy((char *) buf, (const char *)s);
}

void *PilotMemo::pack(void *buf, int *len)
{
	FUNCTIONSETUP;
	if (((unsigned)*len) < fText.length())
		return NULL;

	*len = fText.length();

	return internalPack((unsigned char *) buf);
}


QString PilotMemo::getTitle() const
{
	if (fText.isEmpty()) return QString::null;
	
	int memoTitleLen = fText.find('\n');
	if (-1 == memoTitleLen) memoTitleLen=fText.length();
	return fText.left(memoTitleLen);
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
		return QString(i18n("[unknown]"));
	}
}
