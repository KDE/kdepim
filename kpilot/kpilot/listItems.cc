
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
#include "options.h"
#include <qstring.h>
#include <qlistbox.h>
#include "listItems.moc"
#include "listItems.h"

PilotListItem::PilotListItem(const QString &text, 
	int pilotid, 
	void *r) : QListBoxText(text), 
	fid(pilotid), 
	fr(r)
{
	FUNCTIONSETUP;
}


// $Log:$
