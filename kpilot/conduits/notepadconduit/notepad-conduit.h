#ifndef _KPILOT_NOTEPAD_CONDUIT_H
#define _KPILOT_NOTEPAD_CONDUIT_H
/* notepad-conduit.h			KPilot
**
** Copyright (C) 2004 by Adriaan de Groot, Joern Ahrens
**
** This file is part of the Notepad conduit, a conduit for KPilot that
** store the notepad drawings to files.
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

#include "plugin.h"

class NotepadConduit : public ConduitAction
{
public:
	NotepadConduit(KPilotDeviceLink *,
		const char *name=0L,
		const QStringList &args = QStringList());
	virtual ~NotepadConduit();

protected:
	virtual bool exec();           // From ConduitAction
};

#endif
