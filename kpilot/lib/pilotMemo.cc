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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
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
	fText = codec()->toUnicode((const char *)(rec->getData()),rec->getLen());
	(void) pilotMemo_id;
}

void PilotMemo::unpack(const void *text, int /* firstTime */)
{
	FUNCTIONSETUP;
	kdWarning() << k_funcinfo << ": deprecated and broken function." << endl;
	fText = codec()->toUnicode((const char *)text);
}

// The indirection just to make the base class happy
void *PilotMemo::internalPack(unsigned char *buf)
{
	FUNCTIONSETUP;
	kdWarning() << k_funcinfo << ": Deprecated." << endl;
	QCString s = codec()->fromUnicode(fText);
	return strcpy((char *) buf, (const char *)s);
}

void *PilotMemo::pack(void *buf, int *len)
{
	FUNCTIONSETUP;
	if (*len < 0) return NULL; // buffer size being silly
	if (fText.length() > (unsigned) *len) return NULL; // won't fit either

	QCString s = codec()->fromUnicode(fText);

	unsigned int use_length = QMIN(*len,MAX_MEMO_LEN);

	// It won't fit if the buffer is too small. This second test
	// is because the encoded length in bytes may be longer (?)
	// than the unencoded length in characters.
	if (s.length() > use_length) return NULL;

	// Zero out the buffer, up to the max memo size.
	memset(buf,0,use_length);

	// Copy the encoded string and make extra sure it's NUL terminated.
	// Yay, _every_ parameter needs a cast.
	strlcpy(( char *)buf,(const char *)s,use_length);

	*len = use_length;
	return buf;
}


QString PilotMemo::getTextRepresentation(bool richText)
{
	if (richText)
		return i18n("<i>Title:</i> %1<br>\n<i>MemoText:</i><br>%2").
			arg(rtExpand(getTitle(), richText)).arg(rtExpand(text(), richText));
	else
		return i18n("Title: %1\nMemoText:\n%2").arg(getTitle()).arg(text());
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
		return i18n("[unknown]");
	}
}
