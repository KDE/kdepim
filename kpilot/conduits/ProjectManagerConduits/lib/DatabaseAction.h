#ifndef _KPILOT_DatabaseAction_SETUP_H
#define _KPILOT_DatabaseAction_SETUP_H
/* MultiDB-setup.h                         KPilot
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
#include "MultiDB-conduit.h"
#include "MultiDB-setup.h"
#include "MultiDB-factory.h"
#include "DatabaseActiondlgPrivate.h"

class DBSettings : public KDialogBase {
Q_OBJECT
public:
	DBSettings(QWidget *, const char *, DBSyncInfo*itm, SyncTypeList_t *tps, bool changeDBName=true, bool allowask=true);
	virtual ~DBSettings();

protected:
	virtual void commitChanges();
	virtual int IdToSyncType(int tp);
	virtual int SyncTypeToId(int tt);

protected slots:
	void slotOk();
	void slotApply();
	void slotBrowseFile();
private:
	DatabaseActionDlgPrivate *fConfigWidget;
	DBSyncInfo*item;
	QWidget *fMainWidget;
	SyncTypeList_t *synctypes;
protected:
	QWidget* widget() {return fMainWidget;}
} ;

// $Log$
// Revision 1.2  2002/04/07 11:56:19  reinhold
// Last version before moving to KDE CVS
//
// Revision 1.1  2002/04/07 01:03:52  reinhold
// the list of possible actions is now created dynamically
//
// Revision 1.8  2002/04/05 21:17:00  reinhold
// *** empty log message ***
//
// Revision 1.7  2002/03/28 13:47:53  reinhold
// Added the list of synctypes, aboutbox is now directly passed on to the setup dlg (instead of being a static var)
//
// Revision 1.6  2002/03/15 20:43:17  reinhold
// Fixed the crash on loading (member function not defined)...
//
// Revision 1.5  2002/03/13 22:14:40  reinhold
// GUI should work now...
//
// Revision 1.4  2002/03/10 23:58:32  reinhold
// Made the conduit compile...
//
// Revision 1.3  2002/03/10 16:06:43  reinhold
// Cleaned up the class hierarchy, implemented some more features (should be quite finished now...)
//
// Revision 1.2  2002/03/09 18:03:47  reinhold
// Reworked the makefiles
//
// Revision 1.1.1.1  2002/03/09 15:38:45  reinhold
// Initial checin of the  project manager / List manager conduit.
//
//
//

#endif
