#ifndef _KPILOT_UIDIALOG_H
#define _KPILOT_UIDIALOG_H
/* uiDialog.h                           KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This defines a subclass of KDialogBase that handles the setup for
** KPilot -- and conduits -- configuration dialogs. It also provides
** some support for the default about-page in setup dialogs, for applications
** (like conduits) with no main window or menu.
*/
 
/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/
 
/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <kdialogbase.h>

class QTabWidget;
class KAboutData;

class UIDialog : public KDialogBase
{
Q_OBJECT
public:
	UIDialog(QWidget *parent=0L, const char *name=0L,bool modal=false);
	virtual ~UIDialog();

protected:
	/**
	* Adds an "About" page to the tabwidget. If @p aboutbutton is
	* true, include an additional "About" button that pops
	* up the KAboutDialog. If @p data is non-null, use that
	* data instead of the global KInstance about data.
	*/
	void addAboutPage(bool aboutbutton=false,KAboutData *data=0L);

protected slots:
	void showAbout();
	virtual void slotOk();

protected:
	virtual bool validate() { return true; } ;
	virtual void commitChanges() = 0;

	QWidget *widget() const { return fMainWidget; } ;
	QTabWidget *tabWidget() const { return fP; } ;
	void setTabWidget(QTabWidget *w);

private:
	QWidget *fMainWidget;
	QTabWidget *fP;
} ;

#endif
