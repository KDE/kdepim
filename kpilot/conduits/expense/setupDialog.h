#ifndef _EXPENSE_SETUPDIALOG_H
#define _EXPENSE_SETUPDIALOG_H
/* setupDialog.h			KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot
**
** This file is part of the Expense conduit, a conduit for KPilot that
** synchronises the Pilot's expense application with external files.
** It supports CSV export and (possibly) SQL database export.
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include "plugin.h"

class ExpenseWidget;

class ExpenseWidgetSetup : public ConduitConfig
{
Q_OBJECT
public:
	ExpenseWidgetSetup(QWidget *,const char *,const QStringList &);
	virtual ~ExpenseWidgetSetup();

	virtual void readSettings();

	typedef enum { PolicyOverwrite,
		PolicyAppend,
		PolicyRotate } RotatePolicy;
	int getRotatePolicy() const;
	void setRotatePolicy(RotatePolicy);

	typedef enum { PolicyNone,
		PolicyPostgresql,
		PolicyMysql } DBPolicy;

	int getDBPolicy() const;
	void setDBPolicy(DBPolicy);

public slots:
	void slotRotatePolicyChanged();
	void slotCSVBrowse();

	void slotDBPolicyChanged();

protected:
	virtual void commitChanges();

private:
	ExpenseWidget *fConfigWidget;
} ;

#endif
