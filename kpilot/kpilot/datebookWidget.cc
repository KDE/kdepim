/* KPilot
**
** Copyright (C) 2003 by Dan Pilone.
**	Authored by Adriaan de Groot
**
** This is the viewer for datebook data.
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


#include <qlayout.h>
#include <qdir.h>
#include <qpushbutton.h>

#include <klistview.h>
#include <kdatepicker.h>
#include <kmessagebox.h>

#include "datebookWidget.moc"

DatebookWidget::DatebookWidget(QWidget *parent, const QString &dbpath) :
	PilotComponent(parent,"component_generic",dbpath)
{
	FUNCTIONSETUP;

	QGridLayout *g = new QGridLayout(this,1,1,SPACING);

	fDatePicker = new KDatePicker( this, "fDatePicker" );
	fDatePicker->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)5, 0, 0, fDatePicker->sizePolicy().hasHeightForWidth() ) );
	g->addMultiCellWidget(fDatePicker,0,0,0,2);

	QSpacerItem* spacer = new QSpacerItem( 20, 180, QSizePolicy::Minimum, QSizePolicy::Expanding );
	g->addItem( spacer, 1, 1 );

	fAddButton = new QPushButton( i18n( "&Add..." ), this, "pushButton1" );
	g->addWidget( fAddButton, 2, 0 );

	fEditButton = new QPushButton( i18n( "&Edit..." ), this, "pushButton2" );
	g->addWidget( fEditButton, 2, 1 );

	fDeleteButton = new QPushButton( i18n( "&Delete..." ), this, "pushButton3" );
	g->addWidget( fDeleteButton, 2, 2 );

	fEventList = new KListView( this, "kListView1" );
	fEventList->addColumn( i18n( "Time" ) );
	fEventList->addColumn( i18n( "Al" ) );
	fEventList->addColumn( i18n( "Rec" ) );
	fEventList->addColumn( i18n( "Description" ) );
//	fEventList->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, fEventList->sizePolicy().hasHeightForWidth() ) );
	fEventList->setAllColumnsShowFocus( TRUE );
	fEventList->setShowSortIndicator( TRUE );
	fEventList->setResizeMode( KListView::/*LastColumn*/AllColumns );
	fEventList->setFullWidth( TRUE );
//	fEventList->setAlternateBackground( QColor( 221, 146, 240 ) );
	g->addMultiCellWidget(fEventList, 0, 2, 3, 3);

	connect(fDatePicker, SIGNAL(dateChanged()), SLOT(slotDayChanged()));
	connect(fAddButton, SIGNAL(clicked()), SLOT(slotAddEvent()));
	connect(fEditButton, SIGNAL(clicked()), SLOT(slotEditEvent()));
	connect(fDeleteButton, SIGNAL(clicked()), SLOT(slotDeleteEvent()));
}

DatebookWidget::~DatebookWidget()
{
	FUNCTIONSETUP;
}


void DatebookWidget::showComponent()
{
	FUNCTIONSETUP;

	// TODO: Open the calendar database
	// TODO: Initialize the current month
	// TODO: Fill the calendar and the event list
}

void DatebookWidget::hideComponent()
{
	FUNCTIONSETUP;

	// TODO: Close the calendar database if open
	// TODO: clear the calendar and the event list
}

void DatebookWidget::slotDayChanged()
{
	FUNCTIONSETUP;
	KMessageBox::information(this, "slotDayChanged");
}

void DatebookWidget::slotAddEvent()
{
	FUNCTIONSETUP;
	KMessageBox::information(this, "slotAddEvent");
}

void DatebookWidget::slotEditEvent()
{
	FUNCTIONSETUP;
	KMessageBox::information(this, "slotEditEvent");
}

void DatebookWidget::slotDeleteEvent()
{
	FUNCTIONSETUP;
	KMessageBox::information(this, "slotDeleteEvent");
}


