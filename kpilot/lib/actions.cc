/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2006 Adriaan de Groot <groot@kde.org>
**
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
** Bug reports and questions can be sent to kde-pim@kde.org
*/
#include "options.h"

#include <qapplication.h>
#include <qdir.h>
#include <qfile.h>

#include <ksavefile.h>

#include "pilot.h"
#include "pilotUser.h"

#include "actions.h"



WelcomeAction::WelcomeAction(KPilotLink *p) :
	SyncAction(p,"welcomeAction")
{
	FUNCTIONSETUP;
}

/* virtual */ bool WelcomeAction::exec()
{
	FUNCTIONSETUP;

	addSyncLogEntry(i18n("KPilot %1 HotSync starting...\n")
		.arg(QString::fromLatin1(KPILOT_VERSION)));
	emit logMessage( i18n("Using encoding %1 on the handheld.").arg(Pilot::codecName()) );
	emit syncDone(this);
	return true;
}

SorryAction::SorryAction(KPilotLink *p, const QString &s) :
	SyncAction(p,"sorryAction"),
	fMessage(s)
{
	if (fMessage.isEmpty())
	{
		fMessage = i18n("KPilot is busy and cannot process the "
			"HotSync right now.");
	}
}

bool SorryAction::exec()
{
	FUNCTIONSETUP;

	addSyncLogEntry(fMessage);
	return delayDone();
}

CleanupAction::CleanupAction(KPilotLink *p)  : SyncAction(p,"cleanupAction")
{
	FUNCTIONSETUP;
}

/* virtual */ bool CleanupAction::exec()
{
	FUNCTIONSETUP;

	if (deviceLink())
	{
		deviceLink()->endSync( KPilotLink::UpdateUserInfo );
	}
	emit syncDone(this);
	return true;
}


TestLink::TestLink(KPilotLink * p) :
	SyncAction(p, "testLink")
{
	FUNCTIONSETUP;

}

/* virtual */ bool TestLink::exec()
{
	FUNCTIONSETUP;

	int i;
	int dbindex = 0;
	int count = 0;
	struct DBInfo db;

	addSyncLogEntry(i18n("Testing.\n"));

	while ((i = deviceLink()->getNextDatabase(dbindex,&db)) > 0)
	{
		count++;
		dbindex = db.index + 1;

		DEBUGKPILOT << fname
			<< ": Read database " << db.name
			<< " with index " << db.index
			<< endl;

		// Let the Pilot User know what's happening
		openConduit();
		// Let the KDE User know what's happening
		// Pretty sure all database names are in latin1.
		emit logMessage(i18n("Syncing database %1...")
			.arg(Pilot::fromPilot(db.name)));
	}

	emit logMessage(i18n("HotSync finished."));
	emit syncDone(this);
	return true;
}
