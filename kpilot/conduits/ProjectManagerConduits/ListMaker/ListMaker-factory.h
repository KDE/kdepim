#ifndef _KPILOT_ListMaker_FACTORY_H
#define _KPILOT_ListMaker_FACTORY_H
/* ListMaker-factory.h                       KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the factory for the ListMaker-conduit plugin.
** It also defines the class for the behavior of the setup dialog.
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

#include <klibloader.h>
#include <kinstance.h>
#include <kaboutdata.h>

#include "Organizer-factory.h"
#include "ListMaker-setup.h"
#include "options.h"
#include "plugin.h"
#include "kpilotlink.h"
#include "ListMaker-conduit.h"


class ListMakerConduitFactory : public OrganizerConduitFactory {
Q_OBJECT

public:
	ListMakerConduitFactory(QObject * w= 0L,const char * n= 0L);

	virtual ConduitConfig*createSetupWidget(QWidget*w, const char*n, const QStringList &l) { return new ListMakerWidgetSetup(w,n,l, synctypes, fAbout); }
	virtual ConduitAction*createConduit(KPilotDeviceLink *lnk, const char *n=0L, const QStringList &l=QStringList()) { return new ListMakerConduit(lnk,n,l, synctypes); }
};

extern "C"
{

void *init_libListMakerConduit();

};


#endif
