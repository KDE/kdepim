/***************************************************************************
                          MultiDBWidgetPrivate.cpp  -  description
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

#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include "MultiDBWidgetPrivate.h"

MultiDBWidgetPrivate::MultiDBWidgetPrivate(QWidget *parent, const char *name ) : MultiDBWidget(parent,name) {
	widgPos=0;
}
MultiDBWidgetPrivate::~MultiDBWidgetPrivate(){
}
QRadioButton*MultiDBWidgetPrivate::InsertRadioButton(QString ln, const char* sn) {
	QRadioButton*btn = new QRadioButton( ln, DefaultSyncTypeGroup, sn );
	DefaultSyncTypeGroupLayout->addWidget( btn );
	return btn;
}
