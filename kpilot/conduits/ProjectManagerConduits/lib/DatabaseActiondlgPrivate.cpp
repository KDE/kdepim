/***************************************************************************
                          DatabaseActiondlgPrivate.cpp  -  description
                             -------------------
    begin                : Sat Apr 6 2002
    copyright            : (C) 2002 by reinhold
    email                : reinhold@albert
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlayout.h>
#include "DatabaseActiondlgPrivate.h"

DatabaseActionDlgPrivate::DatabaseActionDlgPrivate(QWidget *parent, const char *name ) : DBSettingsWidget(parent,name) {
	widgPos=0;
}
DatabaseActionDlgPrivate::~DatabaseActionDlgPrivate(){
}
QRadioButton*DatabaseActionDlgPrivate::InsertRadioButton(QString ln, const char* sn) {
//	QRadioButton*btn = new QRadioButton( DefaultSyncTypeGroup, sn );
//	btn->setText( ln );
//	DefaultSyncTypeGroupLayout->addWidget( btn );
	
	QRadioButton*btn = new QRadioButton( ln, DefaultSyncTypeGroup, sn );
//	DefaultSyncTypeGroupLayout->addWidget( btn, widgPos++, 0 );
	DefaultSyncTypeGroupLayout->addWidget( btn );
	return btn;
}
