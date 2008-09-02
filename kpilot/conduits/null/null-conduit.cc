/* KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot <groot@kde.org>
**
** This file is part of the NULL conduit, a conduit for KPilot that
** does nothing except add a log message to the Pilot's HotSync log.
** It is also intended as a programming example.
**
** This file does the actual conduit work.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "null-conduit.h"

// Only include what we really need:
// First UNIX system stuff, then std C++,
// then Qt, then KDE, then local includes.
//
//
#include <time.h>

#include <kconfig.h>
#include <kdebug.h>

#include "options.h"
#include "pilotSerialDatabase.h"
#include "nullSettings.h"

// A conduit that does nothing has a very
// simple constructor and destructor.
//
//
NullConduit::NullConduit(KPilotLink *d,
	const QVariantList &l) :
	ConduitAction(d, "Null", l),
	fDatabase(0L),
	fFailImmediately( l.contains( CSL1("--fail") ))
{
	FUNCTIONSETUP;
	fConduitName=i18n("Null");
}

NullConduit::~NullConduit()
{
	FUNCTIONSETUP;
	KPILOT_DELETE(fDatabase);
}

/* virtual */ bool NullConduit::exec()
{
	FUNCTIONSETUP;

	DEBUGKPILOT << "Mode: " << syncMode().name();

	if ( fFailImmediately )
	{
		DEBUGKPILOT << "Config says to fail now.";
		emit logError(i18n("NULL conduit is programmed to fail."));
		return false;
	}

	QString m(NullConduitSettings::logMessage());
	if (!m.isEmpty())
	{
		addSyncLogEntry(m);
	}

	return delayDone();
}
