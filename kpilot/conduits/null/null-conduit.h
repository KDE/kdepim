#ifndef _NULL_NULL_CONDUIT_H
#define _NULL_NULL_CONDUIT_H
/* null-conduit.h			KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot
**
** This file is part of the NULL conduit, a conduit for KPilot that
** does nothing except add a log message to the Pilot's HotSync log.
** It is also intended as a programming example.
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

#include "plugin.h"

class PilotRecord;
class PilotDatabase;

class NullConduit : public ConduitAction
{
public:
	NullConduit(KPilotDeviceLink *,
		const char *name=0L,
		const QStringList &args = QStringList());
	virtual ~NullConduit();

	virtual void exec();

protected:
	PilotDatabase *fDatabase;
};

// $Log$
// Revision 1.9  2001/12/18 07:43:25  adridg
// Actually do a (null) sync
//
// Revision 1.8  2001/04/01 17:31:11  adridg
// --enable-final and #include fixes
//
// Revision 1.7  2001/03/09 09:46:14  adridg
// Large-scale #include cleanup
//
// Revision 1.6  2001/02/07 15:46:31  adridg
// Updated copyright headers for source release. Added CVS log. No code change.
//
// Revision 1.5  2000/11/02 23:10:32  adridg
// Added attach-to-database feature
//
#endif
