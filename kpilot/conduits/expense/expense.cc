/*expense.cc			KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot, Christopher Molnar
**
** This file is part of the Expense conduit, a conduit for KPilot that
** synchronises the Pilot's expense application with .. something?
** Actually it just writes a CSV file.
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
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/


#include <options.h>

// Only include what we really need:
// First UNIX system stuff, then std C++,
// then Qt, then KDE, then local includes.
//
//
#include <iostream.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include <qdir.h>
#include <qmap.h>
#include <qcstring.h>
#include <qobject.h>
#include <qdatetime.h>
#include <qtextstream.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kprocess.h>

#include <pi-expense.h>





#include "pilotSerialDatabase.h"
#include "pilotRecord.h"

#include "setupDialog.h"
#include "expense.moc"
#include <qtimer.h>
#define DATESIZE 10
/*  This was copied out of the pilot-link package.
*  I just like it here for quick reference.
struct Expense {
struct tm date;
enum ExpenseType type;
enum ExpensePayment payment;
int currency;
char * amount;
char * vendor;
char * city;
char * attendees;
char * note;
};
*/

const char *
get_entry_type(enum ExpenseType type)
 {
   switch(type) {
	case etAirfare:
		return "Airfare";
	case etBreakfast:
		return "Breakfast";
	case etBus:
		return "Bus";
	case etBusinessMeals:
		return "BusinessMeals";
	case etCarRental:
		return "CarRental";
	case etDinner:
		return "Dinner";
	case etEntertainment:
		return "Entertainment";
	case etFax:
		return "Fax";
	case etGas:
		return "Gas";
    case etGifts:
	      return "Gifts";
    case etHotel:
      return "Hotel";
    case etIncidentals:
      return "Incidentals";
    case etLaundry:
      return "Laundry";
    case etLimo:
      return "Limo";
    case etLodging:
      return "Lodging";
    case etLunch:
      return "Lunch";
    case etMileage:
      return "Mileage";
    case etOther:
      return "Other";
    case etParking:
      return "Parking";
    case etPostage:
      return "Postage";
    case etSnack:
      return "Snack";
    case etSubway:
      return "Subway";
    case etSupplies:
      return "Supplies";
    case etTaxi:
      return "Taxi";
    case etTelephone:
      return "Telephone";
    case etTips:
      return "Tips";
    case etTolls:
      return "Tolls";
    case etTrain:
      return "Train";
    default:
      return NULL;
   }
}

const char *
get_pay_type(enum ExpensePayment type)
{
   switch (type) {
    case epAmEx:
      return "AmEx";
    case epCash:
      return "Cash";
    case epCheck:
      return "Check";
    case epCreditCard:
      return "CreditCard";
    case epMasterCard:
      return "MasterCard";
    case epPrepaid:
      return "Prepaid";
    case epVISA:
      return "VISA";
    case epUnfiled:
      return "Unfiled";
    default:
      return NULL;
   }
}


// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
static const char *expense_id =
	"$Id$";




ExpenseConduit::ExpenseConduit(KPilotDeviceLink *d,
	const char *name,
	const QStringList &l) :
	ConduitAction(d,name,l),
	fDatabase(0L),
	fCSVFile(0L),
	fCSVStream(0L)
{
	FUNCTIONSETUP;

}

ExpenseConduit::~ExpenseConduit()
{
	FUNCTIONSETUP;
	cleanup();
}

/* virtual */ void ExpenseConduit::exec()
{
	FUNCTIONSETUP;

	if (!fConfig)
	{
		kdWarning() << k_funcinfo
			<< ": No configuration set for expense conduit."
			<< endl;
		cleanup();
		emit syncDone(this);
		return;
	}

	fDatabase=new PilotSerialDatabase(pilotSocket(),"ExpenseDB",
		this,"ExpenseDB");

	fConfig->setGroup("Expense-conduit");

	fDBType = fConfig->readNumEntry("DBTypePolicy",
			ExpenseDBPage::PolicyNone);

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Syncing with policy "
		<< fDBType
		<< endl;
#endif

	fDBnm=fConfig->readEntry("DBname");
	fDBsrv=fConfig->readEntry("DBServer");
	fDBtable=fConfig->readEntry("DBtable");
	fDBlogin=fConfig->readEntry("DBlogin");
	fDBpasswd=fConfig->readEntry("DBpasswd");

	fRecordCount = 0;

	if (isTest())
	{
		doTest();
		cleanup();
		emit syncDone(this);
		return;
	}
	else
	{
		QString CSVName=fConfig->readEntry("CSVFileName");
		if (!CSVName.isEmpty())
		{
			fCSVFile = new QFile(CSVName);

			// Change the flags value in the switch() below.
			//
			//
			int flags = 0;
			int logPolicy = fConfig->readNumEntry("CSVRotatePolicy",
				ExpenseCSVPage::PolicyAppend);

			switch(logPolicy)
			{
			case ExpenseCSVPage::PolicyAppend :
				flags = IO_ReadWrite | IO_Append;
				break;
			case ExpenseCSVPage::PolicyOverwrite :
				flags = IO_WriteOnly | IO_Truncate;
				break;
			default :
				flags = IO_ReadWrite | IO_Append;
			}

			if (fCSVFile && fCSVFile->open(flags))
			{
				fCSVStream = new QTextStream(fCSVFile);
			}
		}

		// Start the mechanism for reading one record
		// at a time while retaining responsiveness.
		//
		//
		QTimer::singleShot(0,this,SLOT(slotNextRecord()));
	}
}

void ExpenseConduit::doTest()
{
	DEBUGCONDUIT << k_funcinfo
		<< ": Got settings "
		<< fDBType << " "
		<< fDBnm << " "
		<< fDBsrv << " "
		<< fDBtable << " "
		<< fDBlogin
		<< endl;
}

void ExpenseConduit::csvOutput(QTextStream *out,Expense *e)
{
	FUNCTIONSETUP;

	//format date for csv file
	int tmpyr=e->date.tm_year+1900;
	char dtstng[DATESIZE];
	int tmpday=e->date.tm_mday;
	int tmpmon=e->date.tm_mon+1;

	(*out) << tmpyr << "-" << tmpmon << "-" << tmpday << "," ;

	//write rest of record
	(*out) << e->amount << ","
		<< get_pay_type(e->payment) << ","
		<< e->vendor << ","
		<< get_entry_type(e->type) << ","
		<< e->city << ","
		;

	// remove line breaks from list of attendees -
	// can't have in csv files
	//
	//
	QString tmpatt=e->attendees;
	QString tmpatt2=tmpatt.simplifyWhiteSpace();

	(*out) << tmpatt2 << "," ;

	// remove extra formatting from notes -
	// can't have in csv files
	QString tmpnotes=e->note;
	QString tmpnotes2=tmpnotes.simplifyWhiteSpace();

	(*out) << tmpnotes2 << endl;
}

void ExpenseConduit::slotNextRecord()
{
	FUNCTIONSETUP;

	Expense e;

	PilotRecord *rec=fDatabase->readNextModifiedRec();
	if (!rec)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< ": No more records left."
			<< endl;
#endif

		QString msg(i18n("Synced one record.",
			"Synced %n records.",fRecordCount));
		addSyncLogEntry(msg);

		fDatabase->resetSyncFlags();

		cleanup();
		emit syncDone(this);
		return;
	}
	else
	{
		fRecordCount++;
#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< ": Got record "
			<< fRecordCount
			<< " @"
			<< (int) rec
			<< endl;
#endif
	}

	(void) unpack_Expense(&e,
		(unsigned char *)rec->getData(),rec->getLen());

	delete rec;
	rec = 0L;

	if (fCSVStream)
	{
		csvOutput(fCSVStream,&e);
	}

	switch(fDBType)
	{
	case ExpenseDBPage::PolicyPostgresql :
		postgresOutput(&e);
		break;
	}

	QTimer::singleShot(0,this,SLOT(slotNextRecord()));
}


void ExpenseConduit::dumpPostgresTable()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	// next three lines just for debug purposes -
	// Remove for final creates a dump of table.
	//
	//
	char sqlcmd[300];
	KShellProcess *shproc = new KShellProcess;
	sprintf(sqlcmd,"echo \"%s\"|psql -h %s -U %s -c \"select * from %s;\" %s >testpg.txt",
		fDBpasswd.latin1(),
		fDBsrv.latin1(),
		fDBlogin.latin1(),
		fDBtable.latin1(),fDBnm.latin1());
	shproc->clearArguments();
	(*shproc) << sqlcmd;
	shproc->start(KShellProcess::Block, KShellProcess::NoCommunication);
	while (shproc->isRunning())
	{
		DEBUGCONDUIT << fname << " " << shproc->pid() << " still running" << endl;
		sleep(1);
	}
	delete shproc;
#endif
}

void ExpenseConduit::postgresOutput(Expense *e)
{
	FUNCTIONSETUP;

	KShellProcess *shproc=0L;

        // int recordcount=0;
	// int index=0;
	// int syscall=0;


	int tmpyr=e->date.tm_year+1900;
	char dtstng[DATESIZE];
	int tmpday=e->date.tm_mday;
	int tmpmon=e->date.tm_mon+1;
	sprintf(dtstng,"%d-%d-%d",tmpyr,tmpmon,tmpday);
	QString tmpnotes=e->note;
	QString tmpnotes2=tmpnotes.simplifyWhiteSpace();
	const char* nmsg=tmpnotes2.latin1();

	QString tmpatt=e->attendees;
	QString tmpatt2=tmpatt.simplifyWhiteSpace();
	const char* amesg=tmpatt2.latin1();
	const char* etmsg=get_entry_type(e->type);
	const char* epmsg=get_pay_type(e->payment);
	char sqlcmd[400];

	sprintf(sqlcmd,"echo \"%s\"|psql -h %s -U %s -c "
		"\"INSERT INTO \"%s\" (\"fldTdate\", \"fldAmount\", \"fldPType\", "
		"\"fldVName\", \"fldEType\", \"fldLocation\", \"fldAttendees\", "
		"\"fldNotes\") VALUES ('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s');\" %s",
		fDBpasswd.latin1(),fDBsrv.latin1(),
		fDBlogin.latin1(),fDBtable.latin1(),
		dtstng,
		e->amount,epmsg,e->vendor,etmsg,e->city,amesg,nmsg,fDBnm.latin1());

	shproc = new KShellProcess;
	shproc->clearArguments();
	(*shproc) << sqlcmd;
	shproc->start(KShellProcess::Block, KShellProcess::NoCommunication);

	KPILOT_DELETE(shproc);
}

void ExpenseConduit::cleanup()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fCSVStream);
	KPILOT_DELETE(fCSVFile);
	KPILOT_DELETE(fDatabase);
}


// $Log$
// Revision 1.18  2001/12/02 22:03:07  adridg
// Expense conduit finally works
//
// Revision 1.17  2001/11/25 22:03:44  adridg
// Port expense conduit to new arch. Doesn't compile yet.
//
// Revision 1.16  2001/05/25 16:06:52  adridg
// DEBUG breakage
//
// Revision 1.15  2001/04/06 08:23:40  cschumac
// Adding some const definition to get rid of some compiler warnings.
//
// Revision 1.14  2001/03/27 11:10:38  leitner
// ported to Tru64 unix: changed all stream.h to iostream.h, needed some
// #ifdef DEBUG because qstringExpand etc. were not defined.
//
// Revision 1.13  2001/03/23 21:08:39  molnarc
//
// more cleanup and commit before I rebuild all.
//
// Revision 1.12  2001/03/23 15:49:05  molnarc
//
// more cleanup
//
// Revision 1.11  2001/03/23 15:29:39  molnarc
//
// clean up some debug
//
// Revision 1.10  2001/03/23 14:30:55  molnarc
//
// Now it actually writes to a postrgres db. (Not the right way yet but that is still in progress).
//
// Revision 1.9  2001/03/18 02:00:42  molnarc
//
// Added connect code for postgres and mysql. Just wondering if anyone
// ever tested kdb.
//
// Revision 1.8  2001/03/17 01:25:56  molnarc
//
// start of db work
//
// Revision 1.7  2001/03/17 00:15:11  molnarc
//
// expenses.cc --> some fixes
// Makefile.am --> added libkdbcore libkdb_mysql libkdb_postgres
//    if this is the wrong way someone please fix this and tell me
//    the right way to do it.
//
// Revision 1.6  2001/03/16 13:31:33  molnarc
//
// all data now written to csv file and formatted.
// data is in the following order:
//
// transaction date,
// amount,
// payment type (cash, visa, amex,....),
// vendor name,
// Expense Type (car, tolls, parking, food, ....),
// location,
// attendees (strips all line breaks - can't have in csv file),
// notes (again, strips all line breaks - can't have in csv file)
//
// Revision 1.5  2001/03/16 01:19:49  molnarc
//
// added date to csv output
//
// Revision 1.4  2001/03/15 21:10:07  molnarc
//
//
// CJM - now it saves a csv file to the path in kpilotrc if
//       the path exists. csv file needs some work, but its
//       a start.
//
// Revision 1.3  2001/03/09 09:46:14  adridg
// Large-scale #include cleanup
//
// Revision 1.2  2001/03/05 23:57:53  adridg
// Added KPILOT_VERSION
//
// Revision 1.1  2001/03/04 21:45:30  adridg
// New expense conduit, non-functional but it compiles
//
