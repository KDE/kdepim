/***************************************************************************
                          MultiDBWidgetPrivate.h  -  description
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

#ifndef MULTIDBWIDGETPRIVATE_H
#define MULTIDBWIDGETPRIVATE_H

#include <qwidget.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlayout.h>
#include <MultiDB-conduitDialog.h>

/**
  *@author reinhold
  */

class MultiDBWidgetPrivate : public MultiDBWidget  {
   Q_OBJECT
public: 
	MultiDBWidgetPrivate(QWidget *parent=0, const char *name=0);
	~MultiDBWidgetPrivate();
	QRadioButton*InsertRadioButton(QString ln, const char* sn);
protected:
	int widgPos;
};

#endif
