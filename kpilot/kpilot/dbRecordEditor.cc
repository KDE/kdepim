/* dbRecordEditor.cc                KPilot
**
** Copyright (C) 2003 by Dan Pilone
** Writeen 2003 by Reinhold Kainhofer
**
**/

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

#include <qlineedit.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qwidget.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>

#include <kmessagebox.h>

#include "hexviewwidget.h"
#include "pilotRecord.h"
#include "dbRecordEditor.h"

DBRecordEditor::DBRecordEditor(PilotRecord*r, int n, QWidget *parent)
 : KDialogBase(parent, "RecordEditor",false,i18n("Edit Record"),
              Ok|Cancel), rec(r), nr(n)
{
	createWidgets();
	fillWidgets();
}


DBRecordEditor::~DBRecordEditor()
{
}

void DBRecordEditor::createWidgets()
{
	QWidget*page=new QWidget(this);
	setMainWidget(page);

	QGridLayout *RecordEditorLayout = new QGridLayout( page, 1, 1, 11, 6, "RecordEditorLayout");

	QLabel*fRecordIndexLabel = new QLabel(i18n("Record index:"), page);
	RecordEditorLayout->addWidget( fRecordIndexLabel, 0, 0 );
	fRecordIndex = new QLineEdit( page, "fRecordIndex" );
	fRecordIndex->setReadOnly( TRUE );
	RecordEditorLayout->addWidget( fRecordIndex, 0, 1 );

	QLabel*fRecordIDLabel = new QLabel( i18n("Record ID:"), page);
	RecordEditorLayout->addWidget( fRecordIDLabel, 0, 2 );
	fRecordID = new QLineEdit( page, "fRecordID" );
	fRecordID->setReadOnly( TRUE );
	RecordEditorLayout->addWidget( fRecordID, 0, 3 );


	QButtonGroup*fFlagsGroup = new QButtonGroup(i18n("Flags"), page, "fFlagsGroup" );
	fFlagsGroup->setColumnLayout(0, Qt::Vertical );
	QGridLayout*fFlagsGroupLayout = new QGridLayout( fFlagsGroup->layout() );
	fFlagsGroupLayout->setAlignment( Qt::AlignTop );
	fFlagsGroup->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Maximum) );

	fDirty = new QCheckBox(i18n("&Dirty"), fFlagsGroup, "fDirty" );
	fFlagsGroupLayout->addWidget( fDirty, 0, 0 );

	fDeleted = new QCheckBox(i18n("&Deleted"), fFlagsGroup, "fDeleted" );
	fFlagsGroupLayout->addWidget( fDeleted, 1, 0 );

	fBusy = new QCheckBox(i18n("&Busy"), fFlagsGroup, "fBusy" );
	fFlagsGroupLayout->addWidget( fBusy, 0, 1 );

	fSecret = new QCheckBox(i18n("&Secret"), fFlagsGroup, "fSecret" );
	fFlagsGroupLayout->addMultiCellWidget( fSecret, 1, 1, 1, 2 );

	fArchived = new QCheckBox(i18n("&Archived"), fFlagsGroup, "fArchived" );
	fFlagsGroupLayout->addWidget( fArchived, 0, 2 );

	RecordEditorLayout->addMultiCellWidget( fFlagsGroup, 1, 1, 0, 3 );

	CHexBuffer*buff=new CHexBuffer();
	fRecordData = new CHexViewWidget(page, "fRecordData", buff);
	RecordEditorLayout->addMultiCellWidget( fRecordData, 2, 2, 0, 3 );
}

void DBRecordEditor::slotOk()
{
	if (KMessageBox::questionYesNo(this, i18n("Changing the record data and flags might corrupt the whole record, or even make the database unusable. Do not change the values unless you are absolutely sure you know what you are doing.\n\nReally assign these new flags?"), i18n("Changing record"))==KMessageBox::Yes)
	{
		int att=rec->getAttrib();
#define setFlag(ctrl, flag) if (ctrl->isChecked()) att|=flag; else att &= ~flag;
		setFlag(fDirty, dlpRecAttrDirty);
		setFlag(fDeleted, dlpRecAttrDeleted);
		setFlag(fBusy, dlpRecAttrBusy);
		setFlag(fSecret, dlpRecAttrSecret);
		setFlag(fArchived, dlpRecAttrArchived);
		rec->setAttrib(att);
#undef setFlag
		const CHexBuffer* buff=fRecordData->hexBuffer();
		if (buff) rec->setData(buff->data(), buff->size());
		KDialogBase::slotOk();
	}
}

void DBRecordEditor::slotCancel()
{
	KDialogBase::slotCancel();
}

void DBRecordEditor::fillWidgets()
{
	// FUNCTIONSETUP

	fRecordIndex->setText(QString::number(nr));
	fRecordID->setText(QString::number(rec->getID()));

	int att=rec->getAttrib();
	fDirty->setChecked(att & dlpRecAttrDirty);
	fDeleted->setChecked(att & dlpRecAttrDeleted);
	fBusy->setChecked(att & dlpRecAttrBusy);
	fSecret->setChecked(att & dlpRecAttrSecret);
	fArchived->setChecked(att & dlpRecAttrArchived);

	CHexBuffer*buff=fRecordData->hexBuffer();
	buff->duplicate(rec->getData(), rec->getLen());
//  mDocumentModified = false;
//  setDocumentSize( file.size() );
//  computeNumLines();
}


#include "dbRecordEditor.moc"
