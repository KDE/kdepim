#ifndef _KPILOT_FILEINSTALLWIDGET_H
#define _KPILOT_FILEINSTALLWIDGET_H
/* fileInstallWidget.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#ifndef _KPILOT_PILOTCOMPONENT_H
#include "pilotComponent.h"
#endif

class QMultiLineEdit;
class QPushButton;

class KPilotInstaller;
class FileInstaller;

class FileInstallWidget : public PilotComponent
{
Q_OBJECT

public:
	FileInstallWidget(QWidget* parent, const QString& dbPath);
	virtual ~FileInstallWidget();

	// Pilot Component Methods:
	void initialize();
	virtual bool preHotSync(QString &);
	void postHotSync();


signals:
	void fileInstallWidgetDone();

protected:
	void setSaveFileList(bool saveIt) { fSaveFileList = saveIt; }
	bool getSaveFileList() { return fSaveFileList; }

	/* virtual */ void dragEnterEvent(QDragEnterEvent* event);
	/* virtual */ void dropEvent(QDropEvent* drop);

	KPilotInstaller* getPilotInstallerApp() { return fKPilotInstaller; }

private:
	QListBox*   fListBox;
	bool        fSaveFileList;

	KPilotInstaller* fKPilotInstaller;
	FileInstaller *fInstaller;
	QPushButton *clearButton,*addButton;

protected slots:
	void slotClearButton();
	void slotAddFile();

public slots:
	void refreshFileInstallList();
};



// $Log$
// Revision 1.17  2001/11/18 16:59:55  adridg
// New icons, DCOP changes
//
// Revision 1.16  2001/09/30 16:59:22  adridg
// Cleaned up preHotSync
//
// Revision 1.15  2001/09/29 16:26:18  adridg
// The big layout change
//
// Revision 1.14  2001/09/06 22:33:43  adridg
// Cruft cleanup
//
// Revision 1.13  2001/04/16 13:48:35  adridg
// --enable-final cleanup and #warning reduction
//
// Revision 1.12  2001/04/14 15:21:35  adridg
// XML GUI and ToolTips
//
// Revision 1.11  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.10  2001/03/04 22:22:29  adridg
// DCOP cooperation between daemon & kpilot for d&d file install
//
// Revision 1.9  2001/03/04 13:11:58  adridg
// Actually use the fileInstaller object
//
// Revision 1.8  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.7  2001/02/06 08:05:19  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
#endif
