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
#include <qhbox.h>

// #include "conduitConfigDialog_base.h"
#include "uiDialog.h"

class QListView;
class QListViewItem;
class QPushButton;
class QLabel;
class QWidgetStack;
class KProcess;
class ConduitConfigBase;
class ConduitConfig;

class ConduitConfigWidgetBase : public QObject
{
Q_OBJECT
public:
	ConduitConfigWidgetBase(QHBox *p=0L,const char *n=0L);

	QListView *fConduitList;
	QWidgetStack *fStack;
	QLabel *fOldStyleLabel;
	QPushButton *fConfigureButton;
} ;

class ConduitConfigWidget : public ConduitConfigWidgetBase
{
Q_OBJECT
public:
	ConduitConfigWidget(QHBox *,
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
public slots:
	void commitChanges();

signals:
	void selectionChanged(QListViewItem *);
	void sizeChanged();

protected slots:
	void selected(QListViewItem *);
	void configure();

private:
	QPushButton *fConfigure;
	QListViewItem *fCurrentConduit;
	ConduitConfigBase *fCurrentConfig;
	ConduitConfig *fCurrentOldStyle;
	QWidget *fParentWidget;
} ;

class ConduitConfigDialog : public UIDialog
{
Q_OBJECT;
public:
	ConduitConfigDialog(QWidget *,const char *,bool);
	virtual ~ConduitConfigDialog();

protected:
	// These are slots in the base class
	virtual void commitChanges();
	virtual bool validate();
protected slots:
	virtual void slotApply();

private:
	ConduitConfigWidget *fConfigWidget;
} ;

#endif
