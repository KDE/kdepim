/* JPilotProxy-conduit.cc  
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
** JPilotProxy-Conduit for syncing KPilot and KOrganizer
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/
 
/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#if QT_VERSION < 300
#include <qmsgbox.h>
#else
#include <qmessagebox.h>
#endif

#include "pilotRecord.h"
#include "pilotSerialDatabase.h"
#include "JPilotProxy-conduit.h"
#include "JPilotProxy-factory.h"
#include "jplugin.h"


static const char *JPilotProxy_conduit_id = "$Id$";



JPilotProxyConduit::JPilotProxyConduit(KPilotDeviceLink *d, const char *n, const QStringList &l) : ConduitAction(d,n,l) {
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT<<JPilotProxy_conduit_id<<endl;
#endif
}




/* virtual */ bool JPilotProxyConduit::exec() {
	FUNCTIONSETUP;
	DEBUGCONDUIT<<JPilotProxy_conduit_id<<endl;

	if (!JPilotProxyConduitFactory::pluginsloaded) 
		JPilotProxyConduitFactory::loadPlugins();
		
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

