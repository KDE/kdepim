#ifndef _ListMakerCONDUIT_H
#define _ListMakerCONDUIT_H

/* ListMaker-conduit.h			KPilot
**
** Copyright (C) 1998-2001 Dan Pilone
** Copyright (C) 1998-2000 Preston Brown
** Copyright (C) 1998 Herwin-Jan Steehouwer
** Copyright (C) 1998 Reinhold Kainhofer
**
** This file is part of the ListMaker conduit, a conduit for KPilot that
** synchronises the Pilot's ListMaker application with the outside world,
** which currently means KOrganizer.
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
** Bug reports and questions can be sent to groot@kde.org
*/

#include "options.h"
#include "pilotListMakerEntry.h"
#include "Organizer-conduit.h"

using namespace KCal;

class ListMakerConduit : public OrganizerConduit {
Q_OBJECT
public:
	ListMakerConduit(KPilotDeviceLink *, const char *n=0L, const QStringList &l=QStringList(), SyncTypeList_t *tps=NULL);
	virtual ~ListMakerConduit() {};

protected:
	virtual const long dbtype() { return 0x44617461; }
	virtual const long dbcreator() { return 0x53536c6d; }
	virtual const QString conduitName() {return "ListMaker conduit";};
	virtual const QString conduitSettingsGroup() {return "ListMaker";};
	virtual PilotOrganizerEntry*createOrganizerEntry(PilotRecord *rec=NULL) {return new PilotListMakerEntry(rec);};
	virtual PilotOrganizerEntry*createOrganizerEntry(KCal::Todo *todo=NULL) {return new PilotListMakerEntry(todo);};
};


// $Log$
// Revision 1.7  2002/04/07 11:56:18  reinhold
// Last version before moving to KDE CVS
//
// Revision 1.6  2002/04/06 00:51:08  reinhold
// Finally the conduit compiles again... Still have a lot of TODOS
//
// Revision 1.5  2002/04/05 21:17:01  reinhold
// *** empty log message ***
//
// Revision 1.4  2002/03/23 21:46:43  reinhold
// config  dlg works, but the last changes crash the plugin itself
//
// Revision 1.3  2002/03/10 23:58:32  reinhold
// Made the conduit compile...
//
// Revision 1.2  2002/03/10 16:06:43  reinhold
// Cleaned up the class hierarchy, implemented some more features (should be quite finished now...)
//
// Revision 1.1  2002/03/09 15:45:48  reinhold
// Moved the files around
//
// Revision 1.1.1.1  2002/03/09 15:38:45  reinhold
// Initial checin of the  project manager / List manager conduit.
//
//
//
#endif
