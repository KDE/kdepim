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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to the mailinlist kde-pim@kde.org
*/
#ifndef _KPILOT_PILOTDOCENTRY_H
#define _KPILOT_PILOTDOCENTRY_H

#include <pilotAppCategory.h>
#include "makedoc9.h"


class PilotRecord;


class PilotDOCEntry:public PilotAppCategory {
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

protected:
	void *pack_(void *, int *);
	void unpack(const void *, int = 0) {
	}
};



#endif

