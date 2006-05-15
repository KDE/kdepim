#ifndef _KPILOT_DBVIEWERWIDGET_H
#define _KPILOT_DBVIEWERWIDGET_H
/* dbViewerWidget.h		KPilot
**
** Copyright (C) 2003 by Dan Pilone.
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "pilotComponent.h"

class KListBox;
class KTextEdit;
class KPushButton;
class KComboBox;
class PilotLocalDatabase;
class PilotRecord;
class KListView;

class GenericDBWidget : public PilotComponent
{
Q_OBJECT

public:
	GenericDBWidget(QWidget* parent, const QString& dbpath);
	virtual ~GenericDBWidget();

	// Pilot component methods
	/* virtual */ void showComponent();
	/* virtual */ void hideComponent();

	QString getCurrentDB() const {  return currentDB; }
protected:
	void setupWidget();

protected slots:
	void slotSelected(const QString &dbname);
	void slotDBType(int mode);
	void reset();
	void slotAddRecord();
	void slotEditRecord();
	bool slotEditRecord(QListViewItem*);
	void slotDeleteRecord();
	void slotShowAppInfo();
	void slotShowDBInfo();
	void enableWidgets(bool enable);
	void writeRecord(PilotRecord*r);

private:
	KListBox*fDBList;
	KComboBox*fDBType;
	KTextEdit*fDBInfo;
	KPushButton*fDBInfoButton, *fAppInfoButton;
	KListView*fRecordList;
	KPushButton*fAddRecord, *fEditRecord, *fDeleteRecord;

	enum eDBType {
		eDatabase,
		eApplication
	} currentDBtype;

	PilotLocalDatabase*fDB;
	QString currentDB;
	QPtrList<PilotRecord> fRecList;
};


#endif
