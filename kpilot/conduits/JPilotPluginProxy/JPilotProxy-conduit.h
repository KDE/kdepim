#ifndef _JPilotProxyCONDUIT_H
#define _JPilotProxyCONDUIT_H

/* JPilotProxy-conduit.h            KPilot
**
** Copyright (C) 1998-2001 Dan Pilone
** Copyright (C) 1998-2000 Preston Brown
** Copyright (C) 1998 Herwin-Jan Steehouwer
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to groot@kde.org
*/

#include "klistview.h"
#include "plugin.h"


class JPilotProxyConduit : public ConduitAction {
public:
	JPilotProxyConduit(KPilotDeviceLink *, const char *n=0L, const QStringList &l=QStringList());
	virtual ~JPilotProxyConduit() {};
	virtual bool exec();
	/* pure virtual functions, need to be overloaded in child classes */
	virtual const QString conduitSettingsGroup() { return QString("JPilotPluginProxy");};
};


// $Log$
// Revision 1.1  2002/04/07 11:17:54  kainhofe
// First Version of the JPilotPlugin Proxy conduit. it can be activated, but loading a plugin or syncing a plugin crashes the palm (if no plugin is explicitely enabled, this conduit can be enabled and it won't crash KPIlot). A lot of work needs to be done, see the TODO
//
// Revision 1.2  2002/04/01 14:37:33  reinhold
// Use DirListIterator to find the plugins in a directorz
// User KLibLoader to load the JPilot plugins
//
// Revision 1.1  2002/03/18 23:15:55  reinhold
// Plugin compiles now
//
// Revision 1.5  2002/03/15 20:43:17  reinhold
// Fixed the crash on loading (member function not defined)...
//
// Revision 1.4  2002/03/13 22:14:40  reinhold
// GUI should work now...
//
// Revision 1.3  2002/03/10 23:58:32  reinhold
// Made the conduit compile...
//
// Revision 1.2  2002/03/10 16:06:43  reinhold
// Cleaned up the class hierarchy, implemented some more features (should be quite finished now...)
//
// Revision 1.1.1.1  2002/03/09 15:38:45  reinhold
// Initial checin of the generic project manager / List manager conduit.
//
//
//
#endif
