/* notepad-conduit.cc			KPilot
**
** Copyright (C) 2004 by Adriaan de Groot, Joern Ahrens
**
** This file is part of the Notepad conduit, a conduit for KPilot that
** stores the notepad drawings to files.
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#ifdef DEBUG
#undef DEBUG
#define DEBUG (1)
#else
#define DEBUG (1)
#endif

#include "options.h"


#include "notepad-conduit.h"  // The Conduit action
#include "notepadconduit.h"   // The settings class

#include <qthread.h>
#include <qapplication.h>

extern "C"
{
long version_conduit_notepad = KPILOT_PLUGIN_API;
const char *id_conduit_notepad =
	"$Id$";
}

NotepadConduit::NotepadConduit(KPilotDeviceLink *d,
	const char *n,
	const QStringList &l) :
	ConduitAction(d,n,l)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT << id_conduit_notepad << endl;
#endif
	fConduitName=i18n("Notepad");

	(void) id_conduit_notepad;
}

NotepadConduit::~NotepadConduit()
{
	FUNCTIONSETUP;
}

/* virtual */ bool NotepadConduit::exec()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": In exec() @" << (unsigned long) this << endl;
#endif
	return delayDone();
}


