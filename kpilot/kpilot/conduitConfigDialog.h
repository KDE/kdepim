#ifndef _KPILOT_CONDUITCONFIGDIALOG_H
#define _KPILOT_CONDUITCONFIGDIALOG_H
/* conduitConfigDialog.h                 KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines a dialog that uses the .ui-defined widget for
** configuring conduits.
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

#include <qstringlist.h>

#include "conduitConfigDialog_base.h"
#include "uiDialog.h"

class QListViewItem;
class KProcess;

class ConduitConfigWidget : public ConduitConfigWidgetBase
{
Q_OBJECT
public:
	ConduitConfigWidget(QWidget *, const char *name=0L);
	virtual ~ConduitConfigWidget();

protected:
	void fillLists();

	void warnNoExec(const QListViewItem *);
	void warnNoLibrary(const QListViewItem *);

public slots:
	void commitChanges();

protected slots:
	void selected(QListViewItem *);
	void enableConduit();
	void disableConduit();

	void configureConduit();

} ;

class ConduitConfigDialog : public UIDialog
{
Q_OBJECT
public:
	ConduitConfigDialog(QWidget *,const char *,bool);
	virtual ~ConduitConfigDialog();

protected:
	virtual void commitChanges();

private:
	ConduitConfigWidget *fConfigWidget;
} ;

#endif
