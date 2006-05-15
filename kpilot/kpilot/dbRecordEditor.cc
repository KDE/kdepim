/* KPilot
**
** Copyright (C) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>
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
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>

#include <kmessagebox.h>

#include "pilotRecord.h"
#include "dbRecordEditor.h"
//#include "dbRecordEditor_base.h"

#ifdef USE_KHEXEDIT
#include <khexedit/byteseditinterface.h>
#include <khexedit/valuecolumninterface.h>
#include <khexedit/charcolumninterface.h>
using namespace KHE;
#endif



DBRecordEditor::DBRecordEditor(PilotRecord*r, int n, QWidget *parent)
	: KDialogBase(parent, "RecordEditor",false,i18n("Edit Record"),
				Ok|Cancel), rec(r), nr(n)
{
//	fWidget=new DBRecordEditorBase(this);
	fWidget=new QWidget(this);
	setMainWidget(fWidget);
	fBuffer = new char[4096];

	initWidgets();
	fillWidgets();
}


DBRecordEditor::~DBRecordEditor()
{
	KPILOT_DELETE( fBuffer );
}


void DBRecordEditor::slotOk()
{
	FUNCTIONSETUP;
	if (KMessageBox::questionYesNo(this, i18n("Changing the record data and flags might corrupt the whole record, or even make the database unusable. Do not change the values unless you are absolutely sure you know what you are doing.\n\nReally assign these new flags?"), i18n("Changing Record"))==KMessageBox::Yes)
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

#ifdef USE_KHEXEDIT
		if ( fRecordDataIf->isModified() )
		{
#ifdef DEBUG
			DEBUGKPILOT << "record data changed, new Length of record: " <<
				fRecordDataIf->dataSize() << endl;
#endif
			// take over data
			rec->setData( fRecordDataIf->data(), fRecordDataIf->dataSize() );
		}
#endif

		KDialogBase::slotOk();
	}
}

void DBRecordEditor::slotCancel()
{
	KDialogBase::slotCancel();
}

void DBRecordEditor::languageChange()
{
	fRecordIndexLabel->setText( tr2i18n( "Record index:" ) );
	fRecordIDLabel->setText( tr2i18n( "Record ID:" ) );
	fRecordIndex->setText( tr2i18n( "1" ) );
	fRecordID->setText( tr2i18n( "1" ) );
	fFlagsGroup->setTitle( tr2i18n( "Flags" ) );
	fDirty->setText( tr2i18n( "&Dirty" ) );
	fDeleted->setText( tr2i18n( "De&leted" ) );
	fBusy->setText( tr2i18n( "&Busy" ) );
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
	fFlagsGroup->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5,
		(QSizePolicy::SizeType)4, 0, 0, fFlagsGroup->sizePolicy().hasHeightForWidth() ) );
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

#ifdef USE_KHEXEDIT
	fRecordData = KHE::createBytesEditWidget( fWidget, "fRecordData" );
	if( fRecordData )
	{
		// fetch the editor interface
		fRecordDataIf = KHE::bytesEditInterface( fRecordData );
		Q_ASSERT( fRecordDataIf ); // This should not fail!

		KHE::ValueColumnInterface *ValueColumn = valueColumnInterface( fRecordData );
		if( ValueColumn )
		{
			ValueColumn->setNoOfBytesPerLine( 16 );
			ValueColumn->setResizeStyle( KHE::ValueColumnInterface::LockGrouping );
//			ValueColumn->setCoding( ValueColumnInterface::HexadecimalCoding );
//			ValueColumn->setByteSpacingWidth( 2 );
			ValueColumn->setNoOfGroupedBytes( 4 );
			ValueColumn->setGroupSpacingWidth( 8 );
		}

		KHE::CharColumnInterface *CharColumn = charColumnInterface( fRecordData );
		if( CharColumn )
		{
			CharColumn->setShowUnprintable( false );
//			CharColumn->setSubstituteChar( '*' );
		}
	}
	else
	{
		QLabel*tmpW = new QLabel( i18n("To view and edit the record data, please install a hex editor (e.g. kbytesedit from kdeutils)."), fWidget );
		tmpW->setBackgroundMode( Qt::PaletteMid );
		tmpW->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter | Qt::WordBreak);
		tmpW->setFrameShape( QFrame::Panel );
		tmpW->setFrameShadow( QFrame::Sunken );
		fRecordData = tmpW;
		fRecordDataIf = 0;
	}

	DBRecordEditorBaseLayout->addMultiCellWidget( fRecordData, 2, 2, 0, 3 );
#endif

	languageChange();
	resize( QSize(600, 561).expandedTo(minimumSizeHint()) );
}

void DBRecordEditor::fillWidgets()
{
	// FUNCTIONSETUP

	fRecordIndex->setText(QString::number(nr));
	fRecordID->setText(QString::number(rec->id()));

	int att=rec->getAttrib();
	fDirty->setChecked(att & dlpRecAttrDirty);
	fDeleted->setChecked(att & dlpRecAttrDeleted);
	fBusy->setChecked(att & dlpRecAttrBusy);
	fSecret->setChecked(att & dlpRecAttrSecret);
	fArchived->setChecked(att & dlpRecAttrArchived);

#ifdef USE_KHEXEDIT
	if( fRecordDataIf )
	{
		int len = rec->getLen();
		memcpy( fBuffer, rec->getData(), len );
		fRecordDataIf->setData( fBuffer, len, 4096 );
		fRecordDataIf->setMaxDataSize( 4096 );
		fRecordDataIf->setReadOnly( false );
		// We are managing the buffer ourselves:
		fRecordDataIf->setAutoDelete( false );
	}
#endif
}


#include "dbRecordEditor.moc"
