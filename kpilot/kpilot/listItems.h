/* listItems.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This file defines a subclasse of QListBoxText that carries
** additional information useful for Pilot records. In particular it
** carries an int (for the pilot's uid?) and a void * (for pilotrecord?)
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/

#ifndef _KPILOT_LISTITEMS_H
#define _KPILOT_LISTITEMS_H

#ifndef QLISTBOX_H
#include <qlistbox.h>
#endif

class PilotListItem : public QListBoxText
{
public:
	PilotListItem(const QString &text, int pilotid=0, void *r=0); 
	virtual ~PilotListItem();
	int id() const {return fid;};
	const void *rec() const {return fr;};

	static void counts();

protected:
	int fid;
	void *fr;

private:
	static int crt,del,bal,count;
};


#else
#warning "File doubly included"
#endif

// $Log$
// Revision 1.1  2001/03/04 11:22:12  adridg
// In response to bug 21392, replaced fixed-length lookup table by a subclass
// of QListBoxItem inserted into list box. This subclass carries data to
// lookup the relevant pilot record.
//
	
