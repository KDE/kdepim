/* null-conduit.cc			KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include "options.h"

// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
static const char *null_conduit_id=
	"$Id$";

// Only include what we really need:
// First UNIX system stuff, then std C++,
// then Qt, then KDE, then local includes.
//
//
#include <time.h>

#include <kconfig.h>
#include <kdebug.h>

#include "pilotSerialDatabase.h"
#include "null-factory.h"
#include "null-conduit.h"


// A conduit that does nothing has a very
// simple constructor and destructor.
//
//
NullConduit::NullConduit(KPilotDeviceLink *d,
	const char *n,
	const QStringList &l) :
	ConduitAction(d,n,l),
	fDatabase(0L)
{
	FUNCTIONSETUP;

	(void) null_conduit_id;
}

NullConduit::~NullConduit()
{
	FUNCTIONSETUP;
	KPILOT_DELETE(fDatabase);
}

void NullConduit::exec()
{
	FUNCTIONSETUP;

	if (!fConfig)
	{
		kdWarning() << k_funcinfo
			<< ": No configuration set for NULL conduit."
			<< endl;
		emit syncDone(this);
		return;
	}

	fConfig->setGroup(NullConduitFactory::group);

	QString m=fConfig->readEntry(NullConduitFactory::message);
	addSyncLogEntry(m);
	emit logMessage(m);

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Message from null-conduit: "
		<< m
		<< endl;
#endif

	emit syncDone(this);
}


// $Log$
// Revision 1.25  2002/01/25 21:43:11  adridg
// ToolTips->WhatsThis where appropriate; vcal conduit discombobulated - it doesn't eat the .ics file anymore, but sync is limited; abstracted away more pilot-link
//
// Revision 1.24  2001/12/29 15:43:46  adridg
// Various config buglets
//
// Revision 1.23  2001/12/18 13:11:55  cschumac
// Make it compile.
//
// Revision 1.22  2001/12/18 07:43:25  adridg
// Actually do a (null) sync
//
// Revision 1.21  2001/04/26 19:19:26  adridg
// [GUI] i18n updates and QToolTips
//
// Revision 1.20  2001/04/16 13:36:03  adridg
// Removed --enable-final borkage
//
// Revision 1.19  2001/04/01 17:31:11  adridg
// --enable-final and #include fixes
//
// Revision 1.18  2001/03/27 11:10:38  leitner
// ported to Tru64 unix: changed all stream.h to iostream.h, needed some
// #ifdef DEBUG because qstringExpand etc. were not defined.
//
// Revision 1.17  2001/03/09 09:46:14  adridg
// Large-scale #include cleanup
//
// Revision 1.16  2001/03/05 23:57:53  adridg
// Added KPILOT_VERSION
//
// Revision 1.15  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.14  2001/02/08 08:13:44  habenich
// exchanged the common identifier "id" with source unique <sourcename>_id for --enable-final build
//
// Revision 1.13  2001/02/07 15:46:31  adridg
// Updated copyright headers for source release. Added CVS log. No code change.
//
// Revision 1.12  2000/12/21 00:42:50  adridg
// Mostly debugging changes -- added EFUNCTIONSETUP and more #ifdefs. KPilot should now compile -DNDEBUG or with DEBUG undefined
//
// Revision 1.5  2000/07/27 23:07:16  pilone
// 	Ported the conduits.  They build.  Don't know if they work, but they
// build.
