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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

// #include "conduitConfigDialog_base.h"
#include "kcmodule.h"

class QListView;
class QListViewItem;
class QPushButton;
class QLabel;
class QWidgetStack;
class KProcess;
class ConduitConfigBase;
class ConduitConfig;

class ConduitConfigWidgetBase : public KCModule
{
Q_OBJECT
public:
	ConduitConfigWidgetBase(QWidget *p=0L,const char *n=0L);

	QListView *fConduitList;
	QWidgetStack *fStack;
	QPushButton *fConfigureButton;
	QPushButton *fConfigureWizard,*fConfigureKontact;
	QLabel *fActionDescription;
	QLabel *fTitleText;  // Dialog title above fStack
} ;

class ConduitConfigWidget : public ConduitConfigWidgetBase
{
Q_OBJECT
public:
	ConduitConfigWidget(QWidget *,
		const char *name=0L, bool ownButtons=false);
	virtual ~ConduitConfigWidget();

protected:
	void fillLists();

	void warnNoExec(const QListViewItem *);
	void warnNoLibrary(const QListViewItem *);

	void loadAndConfigure(QListViewItem *); // ,bool);

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
	void selectionChanged(QListViewItem *);
	void sizeChanged();

protected slots:
	void configure();
	void configureWizard();

	void unselect(); // Helper slot when cancelling a change in selection
	void selected(QListViewItem *);
	void conduitsChanged(QListViewItem*);
	void reopenItem(QListViewItem *);

private:
	QPushButton *fConfigure;
	QListViewItem *fCurrentConduit;
	QListViewItem *fGeneralPage;
	ConduitConfigBase *fCurrentConfig;
};

#endif
