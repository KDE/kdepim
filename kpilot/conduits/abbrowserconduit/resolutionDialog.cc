/* resolutionDialog.h			KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
** See the .cc file for an explanation of what this file is for.
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
#include "resolutionDialog.h"

#include <qlayout.h>
#include <qradiobutton.h>
#include <qvbuttongroup.h>
#include <qlabel.h>
#include <qcheckbox.h>

#include <kdialogbase.h>

ResolutionDlg::ResolutionDlg( QWidget* parent, QString caption, QString Text, QStringList lst, QString remember) : 
	KDialogBase( parent, "resolutiondlg", true, caption, Ok|Cancel, Ok, true ), ResolutionButtonGroup(0L), rememberCheck(0L)
{
	QWidget *page = new QWidget( this );
	setMainWidget(page);

	setSizeGripEnabled( TRUE );
	QGridLayout* topLayout = new QGridLayout( page, 1, 1, 11, 6, "MyDialogLayout"); 
	
	QLabel* label = new QLabel(Text, page, "TextLabel1" );
//	TextLabel1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)2, (QSizePolicy::SizeType)5, 0, 0, TextLabel1->sizePolicy().hasHeightForWidth() ) );
	label->setAlignment(WordBreak);
	topLayout->addMultiCellWidget( label, 0, 0, 0, 2 );

	QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
	topLayout->addItem( spacer, 1, 2 );
	spacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
	topLayout->addItem( spacer, 1, 0 );

	ResolutionButtonGroup = new QVButtonGroup(this, "ResolutionButtonGroup" );
	
	for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
		new QRadioButton(*it, ResolutionButtonGroup);
	}
//	ResolutionButtonGroup->setButton(0);
	topLayout->addWidget( ResolutionButtonGroup, 1, 1 );

	spacer = new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
	topLayout->addItem( spacer, 2, 1 );

	if (!remember.isEmpty() )
	{
		rememberCheck = new QCheckBox( remember, this, "rememberCheck" );
		rememberCheck->setChecked( TRUE );
		topLayout->addMultiCellWidget( rememberCheck, 3, 3, 0, 2 );
	}

//	topLayout->addStretch(10);
		// tab order
//	setTabOrder( RadioButton1, RadioButton1_2 );
//	setTabOrder( RadioButton1_2, RadioButton1_2_2 );
//	setTabOrder( RadioButton1_2_2, buttonOk );
//	setTabOrder( buttonOk, buttonCancel );
//	setTabOrder( buttonCancel, buttonHelp );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
ResolutionDlg::~ResolutionDlg()
{
    // no need to delete child widgets, Qt does it all for us
}

