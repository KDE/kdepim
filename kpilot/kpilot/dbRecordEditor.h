#ifndef _KPILOT_DBRECORDEDITOR_H
#define _KPILOT_DBRECORDEDITOR_H
/* dbRecordEditor.h                 KPilot
**
** Copyright (C) 2003 by Dan Pilone
** Written 2003 by Reinhold Kainhofer
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"
#include <kdialogbase.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
namespace KHE {
class KHexEdit;
}
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
	DBRecordEditor(PilotRecord*r=0L, int n=-1, QWidget *parent = 0);
	~DBRecordEditor();
	
	QLabel* fRecordIndexLabel;
	QLabel* fRecordIDLabel;
	QLineEdit* fRecordIndex;
	QLineEdit* fRecordID;
	QButtonGroup* fFlagsGroup;
	QCheckBox* fDirty;
	QCheckBox* fDeleted;
	QCheckBox* fBusy;
	QCheckBox* fSecret;
	QCheckBox* fArchived;
	KHE::KHexEdit* fRecordData;
	
protected:
	QGridLayout* DBRecordEditorBaseLayout;
	QGridLayout* fFlagsGroupLayout;

protected:
//	DBRecordEditorBase*fWidget;
	QWidget*fWidget;
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
