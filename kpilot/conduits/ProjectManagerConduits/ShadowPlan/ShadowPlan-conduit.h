#ifndef _ShadowPlanCONDUIT_H
#define _ShadowPlanCONDUIT_H

/* ShadowPlan-conduit.h			KPilot
**
** Copyright (C) 1998-2001 Dan Pilone
** Copyright (C) 1998-2000 Preston Brown
** Copyright (C) 1998 Herwin-Jan Steehouwer
** Copyright (C) 1998 Reinhold Kainhofer
**
** This file is part of the ShadowPlan conduit, a conduit for KPilot that
** synchronises the Pilot's ShadowPlan application with the outside world,
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

/*
** Bug reports and questions can be sent to groot@kde.org
*/

#include "options.h"
using namespace KCal;

class ShadowPlanConduit : public GenericOrganizerConduit {
	Q_OBJECT
public:
	ShadowPlanConduit(KPilotDeviceLink *, const char *n=0L, const QStringList &l=QStringList());
	virtual ~ShadowPlanConduit() {};

protected:
	virtual long dbtype() { return 0x44415441; }
	virtual long dbcreator() { return 0x6c625047; }
};


#endif
