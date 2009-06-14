#ifndef KPILOT_PILOTMEMO_H
#define KPILOT_PILOTMEMO_H
/* pilotMemo.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone <dan@kpilot.org>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

// KPilot headers
#include "options.h"
#include "pilotRecord.h"
#include "pilotAppInfo.h"

// pilot-link headers
#include <pi-memo.h>

class KPILOT_EXPORT PilotMemo : public PilotRecordBase
{
public:
	/**
	* Constructor. Create an empty memo.
	*/
	PilotMemo(void) : PilotRecordBase() { } ;

	/**
	* Constructor. Create a memo in the Unfiled category with
	* text @p s .
	*/
	PilotMemo(const QString &s) : PilotRecordBase()
	{
		setText(s);
	} ;

	/**
	* Constructor. Create a memo with the category and
	* attributes of the given record @p rec, and extract
	* the text from that record as if it comes from the MemoDB.
	*/
	PilotMemo(const PilotRecord* rec);

	/**
	* Constructor. Create a memo with category and
	* attributes from the argument @p r, and set the
	* text of the memo from string @p s.
	*/
	PilotMemo(const PilotRecordBase *r, const QString &s) :
		PilotRecordBase(r)
	{
		setText(s);
	}

	~PilotMemo() { } ;

	virtual QString getTextRepresentation(Qt::TextFormat richText);
	QString text(void) const { return fText; } ;
	void setText(const QString &text) { fText = text.left(MAX_MEMO_LEN); } ;
	QString getTitle(void) const ;
	PilotRecord* pack();

	enum { MAX_MEMO_LEN=8192 } ;

	/**
	* Return a "short but sensible" title. getTitle() returns the
	* first line of the memo, which may be very long
	* and inconvenient. shortTitle() returns about 30
	* characters.
	*/
	QString shortTitle() const;

	/**
	* Returns a (complete) title if there is one and [unknown]
	* otherwise.
	*/
	QString sensibleTitle() const;

private:
	QString fText;

};

inline int _upMAI(struct MemoAppInfo *m, const unsigned char *b, size_t s)
{
	return unpack_MemoAppInfo(m,b,s);
}

inline int _pMAI(const struct MemoAppInfo *m, unsigned char *b, size_t s)
{
	return pack_MemoAppInfo(m,b,s);
}

typedef PilotAppInfo<struct MemoAppInfo, _upMAI, _pMAI> PilotMemoInfo;


#endif
