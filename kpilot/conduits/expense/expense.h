#ifndef _KPILOT_EXPENSE_H
#define _KPILOT_EXPENSE_H
/* expense.h			KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot
**
** This file is part of the Expense conduit.
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


#include "plugin.h"

class QTextStream;
class QFile;

class Expense;

class PilotRecord;
class PilotDatabase;



class ExpenseConduit : public ConduitAction
{
Q_OBJECT

public:
	// The public methods of a conduit's action are
	// just boilerplate. The factory is expected to
	// do the right dynamic casts to give the conduit
	// a KPilotDeviceLink. The conduit can interpret args
	// however it wishes. The expense conduit has no
	// additional arguments.
	//
	//
	ExpenseConduit(KPilotDeviceLink *,
		const char *name=0L,
		const QStringList &args = QStringList());
	virtual ~ExpenseConduit();


protected:
	virtual bool exec();

protected:
	/**
	* This is the conduit's pointer to the *serial* database
	* on the Pilot. It's opened when exec() is called.
	*/
	PilotDatabase *fDatabase;

	/**
	* For CSV output, we use these two data members. We write
	* to the *stream* to get output. If fCSVStream==0 then we
	* don't want any CSV output.
	*/
	QFile *fCSVFile;
	QTextStream *fCSVStream;

	/**
	* Settings from the config file. Read once, when exec() starts,
	* and used for the rest of the session.
	*/
	int fDBType;
	QString fDBnm,fDBsrv,fDBtable,fDBlogin,fDBpasswd;
	int fRecordCount;

	/**
	* Print out some testing information.
	*/
	void doTest();

	/**
	* Actually output an expense record in some format.
	* CSV obviously writes to the CSV file, while postgres
	* invokes an external postgres process to do the work.
	*/
	void csvOutput(QTextStream *,Expense *);
	void postgresOutput(Expense *);

protected slots:
	/**
	* To avoid blocking the UI, we use QTimer(0) to spread
	* the workload. This function handles the next record in the
	* database.
	*/
	void slotNextRecord();

private:
	/**
	* For debugging purposes, you can get a dump of the postgres
	* table in a hardcoded place on your system.
	*/
	void dumpPostgresTable();

	/**
	* Delete any allocated data. This MUST be called -- to close
	* the serial database -- before emitting syncDone().
	*/
	void cleanup();
};

#endif
