#ifndef _KPILOT_ListMaker_SETUP_H
#define _KPILOT_ListMaker_SETUP_H
/* ListMaker-setup.h                         KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the class for the behavior of the setup dialog.
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
#include "plugin.h"
#include "MultiDB-setup.h"


//class ListMakerWidget;

class ListMakerWidgetSetup : public MultiDBWidgetSetup {
Q_OBJECT
public:
	ListMakerWidgetSetup(QWidget *w, const char *n, const QStringList & a, SyncTypeList_t *lst=0L, KAboutData *abt=NULL) : MultiDBWidgetSetup(w,n,a,lst,abt) {FUNCTIONSETUP;};
	virtual ~ListMakerWidgetSetup() {};
protected:
	virtual const QString getSettingsGroup() { return "ListMaker";};
};

// $Log$
// Revision 1.6  2002/04/07 11:56:19  reinhold
// Last version before moving to KDE CVS
//
// Revision 1.5  2002/04/06 00:51:08  reinhold
// Finally the conduit compiles again... Still have a lot of TODOS
//
// Revision 1.4  2002/04/05 21:17:00  reinhold
// *** empty log message ***
//
// Revision 1.3  2002/03/28 13:47:54  reinhold
// Added the list of synctypes, aboutbox is now directly passed on to the setup dlg (instead of being a static var)
//
// Revision 1.2  2002/03/13 22:14:40  reinhold
// GUI should work now...
//
// Revision 1.1  2002/03/10 23:59:17  reinhold
// Made the conduit compile...
//
// Revision 1.2  2002/03/10 16:06:43  reinhold
// Cleaned up the class hierarchy, implemented some more features (should be quite finished now...)
//
// Revision 1.1.1.1  2002/03/09 15:38:45  reinhold
// Initial checin of the  project manager / List manager conduit.
//
//
//

#endif
