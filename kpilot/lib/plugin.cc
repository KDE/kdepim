/* plugin.cc                            KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the base class of all KPilot conduit plugins configuration
** dialogs. This is necessary so that we have a fixed API to talk to from
** inside KPilot.
**
** The factories used by KPilot plugins are also documented here.
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

#include <stdlib.h>

#include <qstringlist.h>

#include <dcopclient.h>

#include "plugin.moc"



ConduitConfig::ConduitConfig(QWidget *parent=0L,
	const char *name,
	const QStringList &args) : 
	UIDialog(parent,name,PluginUtility::isModal(args)),
	fConfig(0L)
{
	FUNCTIONSETUP;
}


/* virtual */ ConduitConfig::~ConduitConfig()
{
	FUNCTIONSETUP;
}

ConduitAction::ConduitAction(KPilotDeviceLink *p,
	const char *name,
	const QStringList &args) :
	SyncAction(p,name),
	fConfig(0L),
	fTest(args.contains("test")),
	fBackup(args.contains("backup"))
{
	FUNCTIONSETUP;
}

/* virtual */ ConduitAction::~ConduitAction()
{
	FUNCTIONSETUP;
}

int PluginUtility::findHandle(const QStringList &a)
{
	FUNCTIONSETUP;

	int handle = -1;
	for (QStringList::ConstIterator i = a.begin();
		i != a.end(); ++i)
	{
		if ((*i).left(7) == "handle=")
		{
			QString s = (*i).mid(7);
			if (s.isEmpty()) continue;

			handle = atoi((const char *)s);
#ifdef DEBUG
			DEBUGCONDUIT << fname
				<< ": Got handle "
				<< handle
				<< endl;
#endif
			return handle;
		}
	}

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": No handle= parameter found."
		<< endl;
#endif

	return -1;
}

bool PluginUtility::isModal(const QStringList &a)
{
	return a.contains("modal");
}

/* static */ bool PluginUtility::isRunning(const QCString &n)
{
	FUNCTIONSETUP;

	DCOPClient *dcop = KApplication::kApplication()->dcopClient();
	QCStringList apps = dcop->registeredApplications();
	return apps.contains(n);
}

// $Log$
// Revision 1.4  2002/01/18 12:47:21  adridg
// CVS_SILENT: More compile fixes
//
// Revision 1.3  2001/12/28 12:55:24  adridg
// Fixed email addresses; added isBackup() to interface
//
// Revision 1.2  2001/10/17 08:46:08  adridg
// Minor cleanups
//
// Revision 1.1  2001/10/08 21:56:02  adridg
// Start of making a separate KPilot lib
//

