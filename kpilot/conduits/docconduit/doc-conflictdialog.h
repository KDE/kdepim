#ifndef CONFLICTDIALOG_H
#define CONFLICTDIALOG_H
/* doc-conflictdialog.h                           KPilot
**
** Copyright (C) 2002-2003 by Reinhold Kainhofer <reinhold@kainhofer.com>
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


#include <kdialog.h>
#include "doc-conduit.h"
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3ValueList>
#include <QLabel>


class Q3GridLayout;
class Q3GroupBox;
class QLabel;
class QPushButton;
class QTimer;

class KComboBox;

class KPilotLink;


typedef struct conflictEntry {
	QLabel*dbname;
	KComboBox* resolution;
	QPushButton*info;
	int index;
	bool conflict;
};


class ResolutionDialog : public KDialog
{
	Q_OBJECT

public:
	explicit ResolutionDialog( QWidget* parent=0, const QString& caption=i18n("Resolution Dialog"), syncInfoList*sinfo=0L, KPilotLink*lnk=0L);
	~ResolutionDialog();

	bool hasConflicts;
public slots:
	void _tickle();
protected:
	QTimer* tickleTimer;
	KPilotLink* fHandle;

protected:
	Q3GroupBox* resolutionGroupBox;
	Q3GridLayout*resolutionGroupBoxLayout;

	syncInfoList*syncInfo;
	Q3ValueList<conflictEntry> conflictEntries;
	QLabel *textLabel1,*textLabel2;

protected slots:
	virtual void slotOk();
	void slotInfo(int index);

};

#endif // CONFLICTDIALOG_H
