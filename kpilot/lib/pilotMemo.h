#ifndef _KPILOT_PILOTMEMO_H
#define _KPILOT_PILOTMEMO_H
/* pilotMemo.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** See the .cc file for an explanation of what this file is for.
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

#include <qstring.h>

#include "pilotAppCategory.h"


class KDE_EXPORT PilotMemo : public PilotAppCategory
{
public:
	PilotMemo(void) : PilotAppCategory() { } ;
	PilotMemo(const QString &s) : PilotAppCategory() { setText(s); } ;
	PilotMemo(const PilotRecord* rec);
	PilotMemo(void *buf) : PilotAppCategory() { unpack(buf, 1); } ;
	PilotMemo(void *buf, int attr, recordid_t id, int category)
		: PilotAppCategory(attr, id, category) { unpack(buf, 1); } ;
	~PilotMemo() { } ;

	virtual QString getTextRepresentation(bool richText=false);
	QString text(void) const { return fText; } ;
	void setText(const QString &text) { fText = text; } ;
	QString getTitle(void) const ;
	PilotRecord* pack() { return PilotAppCategory::pack(); } ;

	static const int MAX_MEMO_LEN=8192;

	/**
	* Return a "short but sensible" title. getTitle() returns the
	* first line of the memo, which may be very long
	* and inconvenient. sshortTitle() returns about 30
	* characters.
	*/
	QString shortTitle() const;

	/**
	* Returns a (complete) title if there is one and [unknown]
	* otherwise.
	*/
	QString sensibleTitle() const;

protected:
	void *pack(void *, int *);
	void unpack(const void *, int = 0);

private:
	QString fText;

	void *internalPack(unsigned char *);
};

#endif
