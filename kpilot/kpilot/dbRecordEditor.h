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
#include <kdialogbase.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;

namespace KHE {
class BytesEditInterface;
}
using namespace KHE;

class QButtonGroup;
class QCheckBox;
class QLabel;
class QLineEdit;

class DBRecordEditorBase;
class PilotRecord;

/**
@author Reinhold Kainhofer
*/
class DBRecordEditor : public KDialogBase
{
Q_OBJECT
public:
	DBRecordEditor(PilotRecord*r=0L, int n=-1, TQWidget *parent = 0);
	~DBRecordEditor();
	
protected:
	TQLabel* fRecordIndexLabel;
	TQLabel* fRecordIDLabel;
	TQLineEdit* fRecordIndex;
	TQLineEdit* fRecordID;
	TQButtonGroup* fFlagsGroup;
	TQCheckBox* fDirty;
	TQCheckBox* fDeleted;
	TQCheckBox* fBusy;
	TQCheckBox* fSecret;
	TQCheckBox* fArchived;
	TQWidget* fRecordData;
	KHE::BytesEditInterface*fRecordDataIf;
	
protected:
	TQGridLayout* DBRecordEditorBaseLayout;
	TQGridLayout* fFlagsGroupLayout;

protected:
//	DBRecordEditorBase*fWidget;
	TQWidget*fWidget;
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
