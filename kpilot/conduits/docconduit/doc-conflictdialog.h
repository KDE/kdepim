#ifndef CONFLICTDIALOG_H
#define CONFLICTDIALOG_H
/* doc-conflictdialog.h                           KPilot
**
** Copyright (C) 2002-2003 by Reinhold Kainhofer
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include <kdialogbase.h>
#include "doc-conduit.h"


#define SCROLL_TABLE

#ifdef SCROLL_TABLE
class QComboTableItem;
class QTable;
#else
class QComboBox;
class QGridLayout;
class QGroupBox;
#endif

class QLabel;
class QPushButton;
class QTimer;
class KPilotDeviceLink;


typedef struct conflictEntry {
#ifdef SCROLL_TABLE
	QString dbname;
	QComboTableItem*resolution;
#else
	QLabel*dbname;
	QComboBox* resolution;
#endif
	QPushButton*info;
	int index;
	bool conflict;
};


class ResolutionDialog : public KDialogBase
{
	Q_OBJECT

public:
	ResolutionDialog( QWidget* parent=0, const QString& caption=i18n("Resolution Dialog"), syncInfoList*sinfo=0L, KPilotDeviceLink*lnk=0L);
	~ResolutionDialog();
	
	bool hasConflicts;
public slots:
	void _tickle();
protected:
	QTimer* tickleTimer;
	KPilotDeviceLink* fHandle;

protected:
	syncInfoList*syncInfo;
	
#ifdef SCROLL_TABLE
	QTable* resolutionTable;
#else
	QGroupBox* resolutionGroupBox;
	QGridLayout*resolutionGroupBoxLayout;
#endif

	QValueList<conflictEntry> conflictEntries;
	QLabel *textLabel1,*textLabel2;

protected slots:
	virtual void slotOk();
	void slotInfo(int index);

};

#endif // CONFLICTDIALOG_H
