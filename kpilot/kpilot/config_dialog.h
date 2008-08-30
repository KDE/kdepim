#ifndef KPILOT_CONDUITCONFIGDIALOG_H
#define KPILOT_CONDUITCONFIGDIALOG_H
/* conduitConfigDialog.h                 KPilot
**
** Copyright (C) 2001 by Dan Pilone <dan@kpilot.org>
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This file defines a dialog for configuring all of KPilot, using
** the pages from config_pages and the conduit (plugin) pages.
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

#include "kcmodule.h"

class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;
class QLabel;
class QStackedWidget;
class ConduitConfigBase;
class ConduitConfig;

class ConduitConfigWidgetBase : public KCModule
{
Q_OBJECT
public:
	ConduitConfigWidgetBase(QWidget *parent, const QVariantList &);

	QTreeWidget *fConduitList;
	QStackedWidget *fStack;
	QPushButton *fConfigureWizard,*fConfigureKontact;
	QLabel *fActionDescription;
	QLabel *fTitleText;  // Dialog title above fStack
} ;

class ConduitConfigWidget : public ConduitConfigWidgetBase
{
Q_OBJECT
public:
	ConduitConfigWidget(QWidget *, const QVariantList &);
	virtual ~ConduitConfigWidget();

protected:
	void fillLists();

	void warnNoExec(const QTreeWidgetItem *);
	void warnNoLibrary(const QTreeWidgetItem *);

	void loadAndConfigure(QTreeWidgetItem *);

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

signals:
	void selectionChanged(QTreeWidgetItem *);
	void sizeChanged();

protected slots:
	void configure();
	void autoDetectDevice();

	void unselect(); // Helper slot when cancelling a change in selection
	void selected(QTreeWidgetItem *, QTreeWidgetItem *);
	void reopenItem(QTreeWidgetItem *);

private:
	QTreeWidgetItem *fCurrentConduit;
	QTreeWidgetItem *fGeneralPage;
	QTreeWidgetItem *fConduitsItem; // Parent of all the conduits items
	ConduitConfigBase *fCurrentConfig;
};

#endif
