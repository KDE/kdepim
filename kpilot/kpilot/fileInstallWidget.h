#ifndef KPILOT_FILEINSTALLWIDGET_H
#define KPILOT_FILEINSTALLWIDGET_H
/* fileInstallWidget.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone <dan@kpilot.org>
**
** This file defines the file install widget, which is the thing
** that accepts file drags for later installation into the Pilot.
**
** This file also defines the log window widget, which logs
** sync-messages during a HotSync.
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

#include "component_page_base.h"

class KPilotInstaller;
class FileInstaller;

class FileInstallWidget : public ComponentPageBase
{
Q_OBJECT

public:
	FileInstallWidget( QWidget* parent );
	virtual ~FileInstallWidget();

	// ComponentPageBase Methods:
	void showPage();
	void hidePage() {}

private:
	class Private;
	Private *fP;
	
	bool fSaveFileList;
	KPilotInstaller *fKPilotInstaller;
	FileInstaller *fInstaller;

signals:
	void fileInstallWidgetDone();

protected slots:
	void slotAddFile();
	void slotClearButton();
	void slotDropEvent( QDropEvent *drop );
	void refreshFileInstallList();

protected:
	bool eventFilter( QObject *watched, QEvent *event );
	void contextMenu( QContextMenuEvent *event );
	void dragEnterEvent( QDragEnterEvent* event );
	void dropEvent( QDropEvent* drop );
	void setSaveFileList( bool saveIt ) { fSaveFileList = saveIt; }
	bool getSaveFileList() { return fSaveFileList; }
	KPilotInstaller* getPilotInstallerApp() { return fKPilotInstaller; }
};

#endif
