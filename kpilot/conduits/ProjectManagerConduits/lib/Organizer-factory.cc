/* Organizer-factory.cc                      KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the factory for the Organizer-conduit plugin.
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
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"
#include "Organizer-factory.h"

void OrganizerConduitFactory::customConduitInfo() {
	FUNCTIONSETUP;
	synctypes->append( new KPilotSyncType(i18n("vcal"), i18n("Save as iCalendar file"), st_vcal, SYNC_NEEDSFILE) );
/*	synctypes->append( new KPilotSyncType(i18n("ldap"), i18n("Store to LDAP directory"), st_ldap, 0) );
	synctypes->append( new KPilotSyncType(i18n("SQL"), i18n("Sync with SQL database"), st_sql, 0) );
	synctypes->append( new KPilotSyncType(i18n("csv"), i18n("Save as comma separated values"), st_csv, SYNC_NEEDSFILE) );*/
}


