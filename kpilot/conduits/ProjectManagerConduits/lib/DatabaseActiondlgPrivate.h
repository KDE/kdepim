/***************************************************************************
                          DatabaseActiondlgPrivate.h  -  description
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

#ifndef DATABASEACTIONDLGPRIVATE_H
#define DATABASEACTIONDLGPRIVATE_H

#include <qwidget.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qbuttongroup.h>
#include <DatabaseActionDialog.h>

/**This child class of DBSettingsWidget is needed to gain access to the layout and add radio buttons to the group...
  *@author reinhold
  */

class DatabaseActionDlgPrivate : public DBSettingsWidget  {
   Q_OBJECT
public:
	DatabaseActionDlgPrivate(QWidget *parent=0, const char *name=0);
	~DatabaseActionDlgPrivate();
	QRadioButton*InsertRadioButton(QString ln, const char* sn);
protected:
	int widgPos;
};

#endif
