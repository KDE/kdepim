/* pilotDOCEntry.h	-*- C++ -*-		KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
** See the .cc file for an explanation of what this file is for.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to the mailinlist kde-pim@kde.org
*/
#ifndef _KPILOT_PILOTDOCENTRY_H
#define _KPILOT_PILOTDOCENTRY_H

#include <pilotRecord.h>
#include "makedoc9.h"


class PilotRecord;


class PilotDOCEntry:public PilotRecordBase {
private:
	bool compress;
	tBuf fText;
public:
	static const int TEXT_SIZE;
	PilotDOCEntry();
	PilotDOCEntry(PilotRecord * rec, bool compressed = false);
	PilotDOCEntry(const PilotDOCEntry & e);
	~PilotDOCEntry() {};
	PilotDOCEntry & operator=(const PilotDOCEntry & e);


	QString getText() {
		fText.Decompress();
		return QString::fromLatin1((const char *) fText.text());
	};
	void setText(QString newtext, bool compressed = false) {
		fText.setText((const unsigned char *) newtext.latin1(),
			newtext.length(), compressed);
	};

	bool getCompress() const {
		return compress;
	}
	void setCompress(bool compressed) {
		compress = compressed;
	};

	PilotRecord *pack(); // Not const because it can change the compression
};



#endif

