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



PilotMemo::PilotMemo(const PilotRecord * rec) : PilotAppCategory(rec)
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
	// Nasty assumption the buffer is big enough
	strlcpy((char *) buf, (const char *)s, s.length()+1);
	return buf;
}

void *PilotMemo::pack(void *buf, int *len)
{
	FUNCTIONSETUP;
	if (!*len) return NULL;
	if (*len < 0) return NULL; // buffer size being silly
	if (fText.length() > (unsigned) *len) return NULL; // won't fit either

	QCString s = codec()->fromUnicode(fText);

	int use_length = *len;
	if (MAX_MEMO_LEN < use_length) use_length = MAX_MEMO_LEN;

	// Zero out the buffer, up to the max memo size.
	memset(buf,0,use_length);

	// Copy the encoded string and make extra sure it's NUL terminated.
	// Yay, _every_ parameter needs a cast.
	// *NOTE* This will truncate the memo text if it was passed in as being
	//        too long, but this is better than allowing garbage in
	strlcpy(( char *)buf,(const char *)s,use_length);

	// Finally, we set the length of the memo to the used length
	// of the data buffer, which might be the length of the string.
	if ((int)s.length() < use_length) use_length = s.length()+1;
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
