/* JPilotProxy-conduit.cc  JPilotProxy-Conduit for syncing KPilot and KOrganizer
**
** Copyright (C) 1998-2001 Dan Pilone
** Copyright (C) 1998-2000 Preston Brown
** Copyright (C) 1998 Herwin-Jan Steehouwer
** Copyright (C) 2001 Cornelius Schumacher
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file is part of the JPilotProxy conduit, a conduit for KPilot that
** synchronises the Pilot's JPilotProxy application with the outside world,
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

#include "options.h"

#if QT_VERSION < 300
#include <qmsgbox.h>
#else
#include <qmessagebox.h>
#endif

#include <kconfig.h>

#include "pilotRecord.h"
#include "pilotSerialDatabase.h"
#include "JPilotProxy-conduit.h"
#include "JPilotProxy-factory.h"
#include "jplugin.h"


static const char *JPilotProxy_conduit_id = "$Id$";



JPilotProxyConduit::JPilotProxyConduit(KPilotDeviceLink *d, const char *n, const QStringList &l) : ConduitAction(d,n,l) {
	FUNCTIONSETUP;
	(void)JPilotProxy_conduit_id;
}




/* virtual */ bool JPilotProxyConduit::exec() {
	FUNCTIONSETUP;
	DEBUGCONDUIT<<JPilotProxy_conduit_id<<endl;

	if (!fConfig) return false;

	KConfigGroupSaver cfgs(fConfig, conduitSettingsGroup());
	
	if (!JPilotProxyConduitFactory::pluginsloaded) 
		JPilotProxyConduitFactory::loadPlugins(fConfig);
		
	if (JPilotProxyConduitFactory::pluginsloaded) {
		PluginIterator_t it(*JPilotProxyConduitFactory::plugins); // iterator for plugin list
		for ( ; it.current(); ++it ) {
			JPlugin*plug=it.current();
			if (plug->info.sync_on) {
				#ifdef DEBUG
				DEBUGCONDUIT<<"syncing "<<plug->info.name<<endl;
				#endif
				plug->pre_sync();
				//TODO: first sync the db as defined by plug->info.db_name
				
				// then call the plugin's sync routine
				plug->sync(pilotSocket());
				plug->post_sync();
			}
		}
	}

	
//	emit syncDone(this);
	return;
}

