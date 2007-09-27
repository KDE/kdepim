#ifndef _KPILOT_DBVIEWERWIDGET_H
#define _KPILOT_DBVIEWERWIDGET_H
/* dbViewerWidget.h		KPilot
**
** Copyright (C) 2003 by Dan Pilone <dan@kpilot.org>
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**	Written 2003 by Reinhold Kainhofer and Adriaan de Groot
**
** This is the generic DB viewer widget.
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

#include <QtCore/QList>

#include "viewer_page_base.h"

class PilotLocalDatabase;
class PilotRecord;

class GenericDBWidget : public ViewerPageBase
{
Q_OBJECT

public:
	GenericDBWidget(QWidget* parent, const QString& dbpath);
	virtual ~GenericDBWidget();

	// ViewerPageBase methods
	virtual void showPage();
	virtual void hidePage();
	virtual PilotAppInfoBase* loadAppInfo() { return 0L; }
	
protected:
	//void markDBDirty(const QString &db);

protected slots:
	void slotSelectionChanged();
	void enableWidgets( bool enable );
	void slotDBType( int mode );
	void reset();
	void slotShowDBInfo();
	void slotShowAppInfo();
	void slotShowRecord();

private:
	enum eDBType {
		eDatabase,
		eApplication
	} currentDBtype;

	class Private;
	Private *fP;
};


#endif
