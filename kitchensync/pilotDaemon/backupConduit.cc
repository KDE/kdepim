/* backupConduit.cc             PilotDaemon
**
** Copyright (C) 2001 by Adriaan de Groot
**
** This program is part of KitchenSync, the KDE handheld-device
** synchronisation framework.
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

static const char *backupConduit_id = "$Id$";


#include <config.h>
#include "../lib/debug.h"

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <qstring.h>
#include <kdebug.h>

#include "pilot-link/include/pi-dlp.h"
#include "../syncManager/syncManagerIface_stub.h"

#include "pilotDatabase.h"
#include "pilotLocalDatabase.h"
#include "backupConduit.h"

bool BackupConduit::exec(PilotDatabase *db)
{
	int records;

	QString s(i18n("Backing up %1...").arg(db->name()));
	fStub->setProgress(0,s);

	if (!db->isDBOpen())
	{
		kdError() << "Can't open database " << db->name() << endl;
		return false;
	}

	records=db->recordCount();
	if (records<0)
	{
		kdWarning() << "Can't get number of records." << endl;
		records=0;
		fStub->setProgress(50,i18n("Size of %1 unknown.").arg(db->name()));
	}

	PilotLocalDatabase dl("/tmp",db->name());

	int count=0;
	int mods=-1;
	int modstep=1;
	if (records)
	{
		mods=records/100;
		if (records<100) modstep=100/records;
	}

	for (int i=0; i<records; i++)
	{
		if (!mods) 
		{
			count+=modstep;
			fStub->setProgress(count,QString::null);
			mods=records/100;
		}
		else
		{
			mods--;
		}

		PilotRecord *r=db->readRecordByIndex(i);
		if (!r)
		{
			kdWarning() << "Failed to read record " << i << endl;
		}
		else
		{
			dl.writeRecord(r);
			delete r;
		}
	}

	return true;
}
