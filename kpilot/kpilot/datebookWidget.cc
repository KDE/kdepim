/* KPilot
**
** Copyright (C) 2003 by Dan Pilone. <dan@kpilot.org>
** Copyright (C) 2003, 2007 by Adriaan de Groot <groot@kde.org>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include "options.h"


#include <qdir.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include <k3listview.h>
#include <kdatepicker.h>
#include <kmessagebox.h>

#include "datebookWidget.moc"

DatebookWidget::DatebookWidget(QWidget *parent, const QString &dbpath) :
	PilotComponent(parent,"component_generic",dbpath)
{
	FUNCTIONSETUP;

	QGridLayout *g = new QGridLayout(this);
	g->setSpacing(SPACING);

	fDatePicker = new KDatePicker( this);
	g->addWidget(fDatePicker,0,0,1,2);

	QSpacerItem *spacer = new QSpacerItem( 20, 180, QSizePolicy::Minimum, QSizePolicy::Expanding );
	g->addItem( spacer, 1, 1 );

	fAddButton = new QPushButton( i18n( "&Add..." ), this );
	fAddButton->setObjectName( QLatin1String( "pushButton1" ) );
	g->addWidget( fAddButton, 2, 0 );

	fEditButton = new QPushButton( i18n( "&Edit..." ), this );
	fEditButton->setObjectName( QLatin1String( "pushButton2" ) );
	g->addWidget( fEditButton, 2, 1 );

	fDeleteButton = new QPushButton( i18n( "&Delete..." ), this );
	fDeleteButton->setObjectName( QLatin1String( "pushButton3" ) );
	g->addWidget( fDeleteButton, 2, 2 );

	fEventList = new K3ListView( this);
	fEventList->addColumn( i18n( "Time" ) );
	fEventList->addColumn( i18n( "Al" ) );
	fEventList->addColumn( i18n( "Rec" ) );
	fEventList->addColumn( i18n( "Description" ) );
	fEventList->setAllColumnsShowFocus( true );
	fEventList->setShowSortIndicator( true );
	fEventList->setResizeMode( K3ListView::/*LastColumn*/AllColumns );
	fEventList->setFullWidth( true );
	g->addWidget(fEventList, 0, 3, 2, 1);

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
	KMessageBox::information(this, CSL1("slotDayChanged"));
}

void DatebookWidget::slotAddEvent()
{
	FUNCTIONSETUP;
	KMessageBox::information(this, CSL1("slotAddEvent"));
}

void DatebookWidget::slotEditEvent()
{
	FUNCTIONSETUP;
	KMessageBox::information(this, CSL1("slotEditEvent"));
}

void DatebookWidget::slotDeleteEvent()
{
	FUNCTIONSETUP;
	KMessageBox::information(this, CSL1("slotDeleteEvent"));
}


