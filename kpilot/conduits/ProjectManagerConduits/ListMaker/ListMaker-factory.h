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

// $Log$
// Revision 1.9  2002/04/07 11:56:19  reinhold
// Last version before moving to KDE CVS
//
// Revision 1.8  2002/04/05 21:17:00  reinhold
// *** empty log message ***
//
// Revision 1.7  2002/03/28 13:47:54  reinhold
// Added the list of synctypes, aboutbox is now directly passed on to the setup dlg (instead of being a static var)
//
// Revision 1.5  2002/03/23 21:46:42  reinhold
// config  dlg works, but the last changes crash the plugin itself
//
// Revision 1.4  2002/03/23 18:21:14  reinhold
// Cleaned up the structure. Works with QTimer instead of loops.
//
// Revision 1.3  2002/03/16 00:24:13  reinhold
// Some class definition cleanup
//
// Revision 1.2  2002/03/15 20:43:17  reinhold
// Fixed the crash on loading (member function not defined)...
//
// Revision 1.1  2002/03/10 23:59:17  reinhold
// Made the conduit compile...
//
// Revision 1.1.1.1  2002/03/09 15:38:45  reinhold
// Initial checin of the  project manager / List manager conduit.
//
//

#endif
