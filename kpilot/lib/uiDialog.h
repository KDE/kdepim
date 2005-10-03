#ifndef _KPILOT_UIDIALOG_H
#define _KPILOT_UIDIALOG_H
/*
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2004 by Adriaan de Groot
**
** This class defines a way to add an "about widget" to a tab widget.
**
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

class QTabWidget;
class QPushButton;
class QWidget;
class KAboutData;

/**
* This class should really be a namespace, since it contains only
* a pair of static methods, and they are not all that interesting
* anyway (I can imagine them living in factories, or even in 
* libs somewhere as a AboutDataWidget).
*/
class KDE_EXPORT UIDialog
{
public:
	/**
	* This is the function that does the work of adding an about
	* page to a tabwidget. It is made public and static so that
	* it can be used elsewhere wherever tabwidgets appear.
	*
	* The about tab is created using aboutPage(). The new about
	* widget is added to the tab widget @p w with the heading
	* "About".
	*
	* @param w The tab widget to which the about page is added.
	* @param data The KAboutData that is used.
	* @param aboutbutton Unused.
	*        
	*/
	static  void addAboutPage(QTabWidget *w,
		KAboutData *data=0L,
		bool aboutbutton=false);

	/**
	* This creates the actual about widget. Again, public & static so
	* you can slap in an about widget wherever.
	*
	* An about widget is created that shows the contributors to
	* the application, along with copyright information and the
	* application's icon. This widget can be used pretty much
	* anywhere. Copied from KAboutDialog, mostly.
	*
	* @param parent The widget that holds the about widget.
	* @param data The KAboutData that is used to populate the widget.
	*/
	static QWidget *aboutPage(QWidget *parent, KAboutData *data=0L);
} ;

#endif
