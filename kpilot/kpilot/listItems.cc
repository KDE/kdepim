
/* listItem.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** Program description
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
#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif



#ifndef _QSTRING_H
#include <qstring.h>
#endif
#ifndef _QLISTBOX_H
#include <qlistbox.h>
#endif



#ifndef _KPILOT_LISTITEMS_H
#include "listItems.h"
#endif

#ifdef DEBUG
/* static */ int PilotListItem::crt = 0;
/* static */ int PilotListItem::del = 0;
/* static */ int PilotListItem::count = 0;

/* static */ void PilotListItem::counts()
{
	kdDebug() << __FUNCTION__
		<< ": created="
		<< crt
		<< " deletions="
		<< del
		<< endl;
}
#endif

PilotListItem::PilotListItem(const QString &text, 
	int pilotid, 
	void *r) : QListBoxText(text), 
	fid(pilotid), 
	fr(r)
{
#ifdef DEBUG
	crt++;
	count++;
	if (!(count & 0xff)) counts();
#endif
}

PilotListItem::~PilotListItem()
{
#ifdef DEBUG
	del++;
	count++;
	if (!(count & 0xff)) counts();
#endif
}



// $Log$
// Revision 1.3  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.2  2001/03/04 20:51:21  adridg
// Removed spurious .moc file
//
// Revision 1.1  2001/03/04 11:22:12  adridg
// In response to bug 21392, replaced fixed-length lookup table by a subclass
// of QListBoxItem inserted into list box. This subclass carries data to
// lookup the relevant pilot record.
//
