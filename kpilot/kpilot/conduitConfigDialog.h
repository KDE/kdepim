#ifndef _KPILOT_CONDUITCONFIGDIALOG_H
#define _KPILOT_CONDUITCONFIGDIALOG_H
/* conduitConfigDialog.h                 KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

// #include "conduitConfigDialog_base.h"
#include "kcmodule.h"
//Added by qt3to4:
#include <QLabel>

class Q3ListView;
class Q3ListViewItem;
class QPushButton;
class QLabel;
class Q3WidgetStack;
class KProcess;
class ConduitConfigBase;
class ConduitConfig;

class ConduitConfigWidgetBase : public KCModule
{
Q_OBJECT
public:
	ConduitConfigWidgetBase(KInstance *inst,QWidget *p=0L);

	Q3ListView *fConduitList;
	Q3WidgetStack *fStack;
	QPushButton *fConfigureButton;
	QPushButton *fConfigureWizard,*fConfigureKontact;
	QLabel *fActionDescription;
	QLabel *fTitleText;  // Dialog title above fStack
} ;

class ConduitConfigWidget : public ConduitConfigWidgetBase
{
Q_OBJECT
public:
	ConduitConfigWidget(KInstance *inst, QWidget *,
		 bool ownButtons=false);
	virtual ~ConduitConfigWidget();

protected:
	void fillLists();

	void warnNoExec(const Q3ListViewItem *);
	void warnNoLibrary(const Q3ListViewItem *);

	void loadAndConfigure(Q3ListViewItem *); // ,bool);

public:
	/**
	* Get rid of the current conduit configuration widget,
	* saving changes if necessary. Returns false if the user
	* selects cancel for the action that is supposed to
	* release the conduit (ie. selecting a different one,
	* or closing the dialog.)
	*/
	bool release();
	bool validate() {return release(); }

public slots:
	virtual void save();
	virtual void load();
//	void slotOk();
//	void slotApply();

signals:
	void selectionChanged(Q3ListViewItem *);
	void sizeChanged();

protected slots:
	void configure();
	void configureWizard();

	void unselect(); // Helper slot when cancelling a change in selection
	void selected(Q3ListViewItem *);
	void conduitsChanged(Q3ListViewItem*);
	void reopenItem(Q3ListViewItem *);

private:
	QPushButton *fConfigure;
	Q3ListViewItem *fCurrentConduit;
	Q3ListViewItem *fGeneralPage;
	ConduitConfigBase *fCurrentConfig;
};

#endif
