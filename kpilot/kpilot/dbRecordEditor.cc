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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qlineedit.h>
#include <qcheckbox.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include <q3buttongroup.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
//Added by qt3to4:
#include <QGridLayout>
#include <Q3Frame>

#include <kmessagebox.h>

#include "pilotRecord.h"
#include "dbRecordEditor.h"
//#include "dbRecordEditor_base.h"

#include <khexedit/byteseditinterface.h>
#include <khexedit/valuecolumninterface.h>
#include <khexedit/charcolumninterface.h>
using namespace KHE;



DBRecordEditor::DBRecordEditor(PilotRecord*r, int n, QWidget *parent)
	: KDialog(parent),
	rec(r), nr(n)
{
	setCaption(i18n("Edit Record"));
	setButtons(Ok|Cancel);
	setDefaultButton(Ok);
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
	if (KMessageBox::questionYesNo(this, i18n("Changing the record data and flags might corrupt the whole record, or even make the database unusable. Do not change the values unless you are absolutely sure you know what you are doing.\n\nReally assign these new flags?"), i18n("Changing Record"),KGuiItem(i18n("Assign")),KStandardGuiItem::cancel())==KMessageBox::Yes)
	{
		int att=rec->attributes();
#define setFlag(ctrl, flag) if (ctrl->isChecked()) att|=flag; else att &= ~flag;
		setFlag(fDirty, dlpRecAttrDirty);
		setFlag(fDeleted, dlpRecAttrDeleted);
		setFlag(fBusy, dlpRecAttrBusy);
		setFlag(fSecret, dlpRecAttrSecret);
		setFlag(fArchived, dlpRecAttrArchived);
		rec->setAttributes(att);
#undef setFlag

		if ( fRecordDataIf->isModified() )
		{
			DEBUGKPILOT << "record data changed, new Length of record: " <<
				fRecordDataIf->dataSize() << endl;
			// take over data
			rec->setData( fRecordDataIf->data(), fRecordDataIf->dataSize() );
		}

		accept();
	}
}

void DBRecordEditor::slotCancel()
{
	reject();
}

void DBRecordEditor::languageChange()
{
	fRecordIndexLabel->setText( i18n( "Record index:" ) );
	fRecordIDLabel->setText( i18n( "Record ID:" ) );
	fRecordIndex->setText( i18n( "1" ) );
	fRecordID->setText( i18n( "1" ) );
	fFlagsGroup->setTitle( i18n( "Flags" ) );
	fDirty->setText( i18n( "&Dirty" ) );
	fDeleted->setText( i18n( "De&leted" ) );
	fBusy->setText( i18n( "&Busy" ) );
	fSecret->setText( i18n( "&Secret" ) );
	fArchived->setText( i18n( "&Archived" ) );
}

void DBRecordEditor::initWidgets()
{
	// FUNCTIONSETUP

	DBRecordEditorBaseLayout = new QGridLayout( fWidget, 1, 1 );
	DBRecordEditorBaseLayout->setMargin( 11 );
	DBRecordEditorBaseLayout->setSpacing( 6 );
	DBRecordEditorBaseLayout->setObjectName( QLatin1String( "DBRecordEditorBaseLayout" ) );

	fRecordIndexLabel = new QLabel( fWidget );
	fRecordIndexLabel->setObjectName( QLatin1String( "fRecordIndexLabel" ) );
	DBRecordEditorBaseLayout->addWidget( fRecordIndexLabel, 0, 0 );

	fRecordIDLabel = new QLabel( fWidget );
	fRecordIDLabel->setObjectName( QLatin1String( "fRecordIDLabel" ) );
	DBRecordEditorBaseLayout->addWidget( fRecordIDLabel, 0, 2 );

	fRecordIndex = new QLineEdit( fWidget );
	fRecordIndex->setObjectName( QLatin1String( "fRecordIndex" ) );
	fRecordIndex->setReadOnly( TRUE );

	DBRecordEditorBaseLayout->addWidget( fRecordIndex, 0, 1 );

	fRecordID = new QLineEdit( fWidget );
	fRecordID->setObjectName( QLatin1String( "fRecordID" ) );
	fRecordID->setReadOnly( TRUE );

	DBRecordEditorBaseLayout->addWidget( fRecordID, 0, 3 );

	fFlagsGroup = new Q3ButtonGroup( fWidget, "fFlagsGroup" );
	fFlagsGroup->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5,
		(QSizePolicy::SizeType)4, 0, 0, fFlagsGroup->sizePolicy().hasHeightForWidth() ) );
	fFlagsGroup->setColumnLayout(0, Qt::Vertical );
	fFlagsGroup->layout()->setSpacing( 6 );
	fFlagsGroup->layout()->setMargin( 11 );
	fFlagsGroupLayout = new QGridLayout( fFlagsGroup->layout() );
	fFlagsGroupLayout->setAlignment( Qt::AlignTop );

	fDirty = new QCheckBox( fFlagsGroup );
	fDirty->setObjectName( QLatin1String( "fDirty" ) );
	fFlagsGroupLayout->addWidget( fDirty, 0, 0 );

	fDeleted = new QCheckBox( fFlagsGroup );
	fDeleted->setObjectName( QLatin1String( "fDeleted" ) );
	fFlagsGroupLayout->addWidget( fDeleted, 1, 0 );

	fBusy = new QCheckBox( fFlagsGroup );
	fBusy->setObjectName( QLatin1String( "fBusy" ) );
	fFlagsGroupLayout->addWidget( fBusy, 0, 1 );

	fSecret = new QCheckBox( fFlagsGroup );
	fSecret->setObjectName( QLatin1String( "fSecret" ) );
	fFlagsGroupLayout->addMultiCellWidget( fSecret, 1, 1, 1, 2 );

	fArchived = new QCheckBox( fFlagsGroup );
	fArchived->setObjectName( QLatin1String( "fArchived" ) );
	fFlagsGroupLayout->addWidget( fArchived, 0, 2 );

	DBRecordEditorBaseLayout->addMultiCellWidget( fFlagsGroup, 1, 1, 0, 3 );
	fRecordData = KHE::createBytesEditWidget( fWidget);
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
		tmpW->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
		tmpW->setWordWrap( true );
		tmpW->setFrameShape( Q3Frame::Panel );
		tmpW->setFrameShadow( Q3Frame::Sunken );
		fRecordData = tmpW;
		fRecordDataIf = 0;
	}

	DBRecordEditorBaseLayout->addMultiCellWidget( fRecordData, 2, 2, 0, 3 );

	languageChange();
	resize( QSize(600, 561).expandedTo(minimumSizeHint()) );
}

void DBRecordEditor::fillWidgets()
{
	// FUNCTIONSETUP

	fRecordIndex->setText(QString::number(nr));
	fRecordID->setText(QString::number(rec->id()));

	int att=rec->attributes();
	fDirty->setChecked(att & dlpRecAttrDirty);
	fDeleted->setChecked(att & dlpRecAttrDeleted);
	fBusy->setChecked(att & dlpRecAttrBusy);
	fSecret->setChecked(att & dlpRecAttrSecret);
	fArchived->setChecked(att & dlpRecAttrArchived);

	if( fRecordDataIf )
	{
		int len = rec->size();
		memcpy( fBuffer, rec->data(), len );
		fRecordDataIf->setData( fBuffer, len, 4096 );
		fRecordDataIf->setMaxDataSize( 4096 );
		fRecordDataIf->setReadOnly( false );
		// We are managing the buffer ourselves:
		fRecordDataIf->setAutoDelete( false );
	}
}


#include "dbRecordEditor.moc"
