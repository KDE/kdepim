#ifndef _KPILOT_DBRECORDEDITOR_H
#define _KPILOT_DBRECORDEDITOR_H
/* dbRecordEditor.h                 KPilot
**
** Copyright (C) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>
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

#include "options.h"
#include <kdialog.h>
#include <QLabel>

class QGridLayout;

namespace KHE {
class BytesEditInterface;
}
using namespace KHE;

class Q3ButtonGroup;
class QCheckBox;
class QLabel;

class KLineEdit;

class DBRecordEditorBase;
class PilotRecord;

/**
@author Reinhold Kainhofer
*/
class DBRecordEditor : public KDialog
{
Q_OBJECT
public:
	explicit DBRecordEditor(PilotRecord*r=0L, int n=-1, QWidget *parent = 0);
	~DBRecordEditor();
	
protected:
	QLabel* fRecordIndexLabel;
	QLabel* fRecordIDLabel;
	KLineEdit* fRecordIndex;
	KLineEdit* fRecordID;
	Q3ButtonGroup* fFlagsGroup;
	QCheckBox* fDirty;
	QCheckBox* fDeleted;
	QCheckBox* fBusy;
	QCheckBox* fSecret;
	QCheckBox* fArchived;
	QWidget* fRecordData;
	KHE::BytesEditInterface*fRecordDataIf;
	
protected:
	QGridLayout* DBRecordEditorBaseLayout;
	QGridLayout* fFlagsGroupLayout;

protected:
//	DBRecordEditorBase*fWidget;
	QWidget*fWidget;
	char*fBuffer;
protected slots:
    virtual void languageChange();
protected:
	void initWidgets();
	void fillWidgets();
	PilotRecord*rec;
	int nr;
protected slots:
	virtual void slotOk();
	virtual void slotCancel();
};

#endif
