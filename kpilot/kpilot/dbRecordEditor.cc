/* dbRecordEditor.cc                KPilot
**
** Copyright (C) 2003 by Dan Pilone
** Written 2003 by Reinhold Kainhofer
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
#include <qcheckbox.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>

#include <kmessagebox.h>

#include "pilotRecord.h"
#include "dbRecordEditor.h"
//#include "dbRecordEditor_base.h"

//#include <khexedit/khexedit.h>
//#include <khexedit/kwrappingrobuffer.h>
#include "ownkhexedit.h"

using namespace KHE;


DBRecordEditor::DBRecordEditor(PilotRecord*r, int n, QWidget *parent)
 : KDialogBase(parent, "RecordEditor",false,i18n("Edit Record"),
              Ok|Cancel), rec(r), nr(n)
{
//	fWidget=new DBRecordEditorBase(this);
	fWidget=new QWidget(this);
	setMainWidget(fWidget);

	initWidgets();
	fillWidgets();
}


DBRecordEditor::~DBRecordEditor()
{
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
		// TODO: Copy the data from the hex edit to the record
//	KHE::KWrappingROBuffer*buff=fWidget->fAppInfoEdit->hexBuffer();
//		if (buff) rec->setData(buff->data(), buff->size());
		KDialogBase::slotOk();
	}
}

void DBRecordEditor::slotCancel()
{
	KDialogBase::slotCancel();
}

void DBRecordEditor::languageChange()
{
    setCaption( tr2i18n( "Form1" ) );
    fRecordIndexLabel->setText( tr2i18n( "Record index:" ) );
    fRecordIDLabel->setText( tr2i18n( "Record ID:" ) );
    fRecordIndex->setText( tr2i18n( "1" ) );
    fRecordID->setText( tr2i18n( "1" ) );
    fFlagsGroup->setTitle( tr2i18n( "Flags" ) );
    fDirty->setText( tr2i18n( "&Dirty" ) );
    fDeleted->setText( tr2i18n( "&Deleted" ) );
    fBusy->setText( tr2i18n( "Busy" ) );
    fSecret->setText( tr2i18n( "&Secret" ) );
    fArchived->setText( tr2i18n( "&Archived" ) );
}

void DBRecordEditor::initWidgets()
{
	// FUNCTIONSETUP

    DBRecordEditorBaseLayout = new QGridLayout( fWidget, 1, 1, 11, 6, "DBRecordEditorBaseLayout"); 

    fRecordIndexLabel = new QLabel( fWidget, "fRecordIndexLabel" );

    DBRecordEditorBaseLayout->addWidget( fRecordIndexLabel, 0, 0 );

    fRecordIDLabel = new QLabel( fWidget, "fRecordIDLabel" );

    DBRecordEditorBaseLayout->addWidget( fRecordIDLabel, 0, 2 );

    fRecordIndex = new QLineEdit( fWidget, "fRecordIndex" );
    fRecordIndex->setReadOnly( TRUE );

    DBRecordEditorBaseLayout->addWidget( fRecordIndex, 0, 1 );

    fRecordID = new QLineEdit( fWidget, "fRecordID" );
    fRecordID->setReadOnly( TRUE );

    DBRecordEditorBaseLayout->addWidget( fRecordID, 0, 3 );

    fFlagsGroup = new QButtonGroup( fWidget, "fFlagsGroup" );
    fFlagsGroup->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)4, 0, 0, fFlagsGroup->sizePolicy().hasHeightForWidth() ) );
    fFlagsGroup->setColumnLayout(0, Qt::Vertical );
    fFlagsGroup->layout()->setSpacing( 6 );
    fFlagsGroup->layout()->setMargin( 11 );
    fFlagsGroupLayout = new QGridLayout( fFlagsGroup->layout() );
    fFlagsGroupLayout->setAlignment( Qt::AlignTop );

    fDirty = new QCheckBox( fFlagsGroup, "fDirty" );

    fFlagsGroupLayout->addWidget( fDirty, 0, 0 );

    fDeleted = new QCheckBox( fFlagsGroup, "fDeleted" );

    fFlagsGroupLayout->addWidget( fDeleted, 1, 0 );

    fBusy = new QCheckBox( fFlagsGroup, "fBusy" );

    fFlagsGroupLayout->addWidget( fBusy, 0, 1 );

    fSecret = new QCheckBox( fFlagsGroup, "fSecret" );

    fFlagsGroupLayout->addMultiCellWidget( fSecret, 1, 1, 1, 2 );

    fArchived = new QCheckBox( fFlagsGroup, "fArchived" );

    fFlagsGroupLayout->addWidget( fArchived, 0, 2 );

    DBRecordEditorBaseLayout->addMultiCellWidget( fFlagsGroup, 1, 1, 0, 3 );

    fRecordData = new KHE::KHexEdit( 0L, fWidget, "fRecordData" );

    DBRecordEditorBaseLayout->addMultiCellWidget( fRecordData, 2, 2, 0, 3 );
    languageChange();
    resize( QSize(600, 561).expandedTo(minimumSizeHint()) );
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

	KHE::KWrappingROBuffer*buf=new KWrappingROBuffer(rec->getData(), rec->getLen());
	fRecordData->setDataBuffer(buf);
}


#include "dbRecordEditor.moc"
