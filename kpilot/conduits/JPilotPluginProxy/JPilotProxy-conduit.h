#ifndef _JPilotProxyCONDUIT_H
#define _JPilotProxyCONDUIT_H

/* JPilotProxy-conduit.h            KPilot
**
** Copyright (C) 1998 Reinhold Kainhofer
**
** This file is part of the JPilotProxy conduit, a conduit for KPilot that
** synchronises the Pilot's JPilotProxy application with the outside world,
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "klistview.h"
#include "plugin.h"


class JPilotProxyConduit : public ConduitAction {
public:
	JPilotProxyConduit(KPilotDeviceLink *, const char *n=0L, const QStringList &l=QStringList());
	virtual ~JPilotProxyConduit() {};
	virtual bool exec();
};


#endif
