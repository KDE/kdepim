/* Bonsai-conduit.cc  Bonsai-Conduit for syncing KPilot and KOrganizer
**
** Copyright (C) 1998-2001 Dan Pilone
** Copyright (C) 1998-2000 Preston Brown
** Copyright (C) 1998 Herwin-Jan Steehouwer
** Copyright (C) 2001 Cornelius Schumacher
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file is part of the Bonsai conduit, a conduit for KPilot that
** synchronises the Pilot's Bonsai application with the outside world,
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

#if KDE_VERSION < 300
#include <libkcal/todo.h>
#else
#include <todo.h>
#endif

#include "pilotRecord.h"
#include "pilotSerialDatabase.h"
//#include "pilotBonsaiEntry.h"

//#include "Bonsai-factory.h"
#include "Bonsai-conduit.h"

using namespace KCal;

static const char *Bonsai_conduit_id = "$Id$";



BonsaiConduit::BonsaiConduit(KPilotDeviceLink *d, 	const char *n, 	const QStringList &l) : GenericOrganizerConduit(d,n,l) {
	FUNCTIONSETUP;
}


#include "Bonsai-conduit.moc"
