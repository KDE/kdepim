#ifndef TODOVIEWER_PAGE_H
#define TODOVIEWER_PAGE_H
/* todoviewer_page.h			KPilot
**
** Copyright (C) 2007 Bertjan Broeksema <b.broeksema@kdemail.net>
**
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "viewer_page_base.h"

class TodoViewerPage : public ViewerPageBase
{

public:
	TodoViewerPage( QWidget *parent, const QString &dbPath );
	
protected:
	/**
	 * Loads the appInfo block.
	 */
	virtual PilotAppInfoBase* loadAppInfo();
	
	/**
	 * This can be used to change the display of the items in the list. If this
	 * returns 0 then the default delegate will be used.
	 */
	virtual QListWidgetItem* getListWidgetItem( PilotRecord *rec );
};

#endif
