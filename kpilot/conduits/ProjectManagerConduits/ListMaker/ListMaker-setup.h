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


#endif
