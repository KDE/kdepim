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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <kdialogbase.h>

class QTabWidget;
class QPushButton;
class KAboutData;

class KDE_EXPORT UIDialog : public KDialogBase
{
Q_OBJECT
public:
	/**
	* Create a UIDialog with the given parent and object name.
	* The dialog can be made modal if you wish. The buttons
	* can be specified using the second constructor, but you should
	* always include an OK button for the purpose of committing
	* whatever changes need to be made.
	*/
	UIDialog(QWidget *parent=0L, const char *name=0L, bool modal=false);
	UIDialog(QWidget *parent=0L, const char *name=0L,
		int buttonmask=Ok|Cancel, bool modal=false);
	virtual ~UIDialog();

protected:
	/**
	* Adds an "About" page to the tabwidget. If @p aboutbutton is
	* true, include an additional "About" button that pops
	* up the KAboutDialog. If @p data is non-null, use that
	* data instead of the global KInstance about data.
	*/
	void addAboutPage(bool aboutbutton=false,KAboutData *data=0L);

public:
	/**
	* This is the function that does the work of adding an about
	* page to a tabwidget. It is made public and static so that
	* it can be used elsewhere wherever tabwidgets appear.
	*/
	static  void addAboutPage(QTabWidget *,
		KAboutData *data=0L,
		bool aboutbutton=false);

	/**
	* This creates the actual about widget. Again, public & static so
	* you can slap in an about widget wherever.
	*/
	static QWidget *aboutPage(QWidget *parent, KAboutData *data=0L);

protected slots:
	void showAbout();
	virtual void slotOk();

protected:
	/**
	* validate() is called when the user OK's the dialog.
	* If any checking needs to be done, then this is the place
	* to do it, to provide the user with a chance to cancel the OK.
	*/
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
