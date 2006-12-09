#ifndef RESOLUTIONDIALOG_H
#define RESOLUTIONDIALOG_H
/* resolutionDialog.h			KPilot
**
** Copyright (C) 2002-2003 by Reinhold Kainhofer
**
** See the .cc file for an explanation of what this file is for.
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

#include <kdialogbase.h>
class KPilotLink;
class QTimer;
class QListView;
class ResolutionDialogBase;


class ResolutionTable;

class ResolutionDlg : public KDialogBase
{
    Q_OBJECT

public:
	ResolutionDlg( QWidget* parent=0, 
		KPilotLink*fH=0L, 
		const QString &caption=QString(), 
		const QString &helpText=QString(), 
		ResolutionTable *tab=0L );
	~ResolutionDlg();

public slots:
	void slotKeepBoth();
	void slotUseBackup();
	void slotUsePalm();
	void slotUsePC();
	void slotApply();
	void _tickle();
protected:
	void fillListView();
	void adjustButtons(ResolutionTable*tab);

	QTimer* tickleTimer;
	KPilotLink* fHandle;
	ResolutionTable*fTable;

	ResolutionDialogBase*fWidget;
};

#endif // RESOLUTIONDIALOG_H
