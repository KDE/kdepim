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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include "options.h"

// Only include what we really need:
// First UNIX system stuff, then std C++,
// then Qt, then KDE, then local includes.
//
//
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
#include <qtextcodec.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kprocess.h>

#include <pi-expense.h>





#include "pilotSerialDatabase.h"
#include "pilotRecord.h"
#include "pilotAppCategory.h"

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
#ifdef DEBUG
	DEBUGCONDUIT<<expense_id<<endl;
#endif
	fConduitName=i18n("Expense");
}

ExpenseConduit::~ExpenseConduit()
{
	FUNCTIONSETUP;
	cleanup();
}

/* virtual */ bool ExpenseConduit::exec()
{
	FUNCTIONSETUP;
	DEBUGCONDUIT<<expense_id<<endl;

	if (!fConfig)
	{
		kdWarning() << k_funcinfo
			<< ": No configuration set for expense conduit."
			<< endl;
		cleanup();
		return false;
	}

	fDatabase=new PilotSerialDatabase(pilotSocket(),CSL1("ExpenseDB"),
		this,"ExpenseDB");

	fConfig->setGroup("Expense-conduit");

	fDBType = fConfig->readNumEntry("DBTypePolicy",
			ExpenseWidgetSetup::PolicyNone);

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
		return true;
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
				ExpenseWidgetSetup::PolicyAppend);

			switch(logPolicy)
			{
			case ExpenseWidgetSetup::PolicyAppend :
				flags = IO_ReadWrite | IO_Append;
				break;
			case ExpenseWidgetSetup::PolicyOverwrite :
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
	return true;
}

void ExpenseConduit::doTest()
{
#ifdef DEBUG
	DEBUGCONDUIT << k_funcinfo
		<< ": Got settings "
		<< fDBType << " "
		<< fDBnm << " "
		<< fDBsrv << " "
		<< fDBtable << " "
		<< fDBlogin
		<< endl;
#endif
}

void ExpenseConduit::csvOutput(QTextStream *out,Expense *e)
{
	FUNCTIONSETUP;

	//format date for csv file
	int tmpyr=e->date.tm_year+1900;
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
	QString tmpatt=PilotAppCategory::codec()->toUnicode(e->attendees);
	QString tmpatt2=tmpatt.simplifyWhiteSpace();

	(*out) << tmpatt2 << "," ;

	// remove extra formatting from notes -
	// can't have in csv files
	QString tmpnotes=PilotAppCategory::codec()->toUnicode(e->note);
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
	case ExpenseWidgetSetup::PolicyPostgresql :
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
	QString query = CSL1("select * from \"%1\";").arg(fDBtable);
	
	QString cmd = CSL1("echo ");
	cmd += KShellProcess::quote(fDBpasswd);
	cmd += CSL1("|psql -h ");
	cmd += KShellProcess::quote(fDBsrv);
	cmd += CSL1(" -U ");
	cmd += KShellProcess::quote(fDBlogin);
	cmd += CSL1(" -c ");
	cmd += KShellProcess::quote(query);
	cmd += CSL1(" ");
	cmd += KShellProcess::quote(fDBnm);
	cmd += CSL1(" > ~/testpg.txt");

	KShellProcess shproc;
	shproc.clearArguments();
	shproc << cmd;
	shproc.start(KShellProcess::Block, KShellProcess::NoCommunication);
#endif
}

void ExpenseConduit::postgresOutput(Expense *e)
{
	FUNCTIONSETUP;

        // int recordcount=0;
	// int index=0;
	// int syscall=0;


	int tmpyr=e->date.tm_year+1900;
	char dtstng[DATESIZE];
	int tmpday=e->date.tm_mday;
	int tmpmon=e->date.tm_mon+1;
	sprintf(dtstng,"%d-%d-%d",tmpyr,tmpmon,tmpday);
	QString tmpnotes=PilotAppCategory::codec()->toUnicode(e->note);
	QString tmpnotes2=tmpnotes.simplifyWhiteSpace();
	const char* nmsg=tmpnotes2.local8Bit();

	QString tmpatt=PilotAppCategory::codec()->toUnicode(e->attendees);
	QString tmpatt2=tmpatt.simplifyWhiteSpace();
	const char* amesg=tmpatt2.local8Bit();
	const char* etmsg=get_entry_type(e->type);
	const char* epmsg=get_pay_type(e->payment);

	QString query;
	query.sprintf(
		"INSERT INTO \"%s\" (\"fldTdate\", \"fldAmount\", \"fldPType\", "
		"\"fldVName\", \"fldEType\", \"fldLocation\", \"fldAttendees\", "
		"\"fldNotes\") VALUES ('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s');",
		fDBtable.latin1(),
		dtstng,
		e->amount,epmsg,e->vendor,etmsg,e->city,amesg,nmsg);

	QString cmd = CSL1("echo ");
	cmd += KShellProcess::quote(fDBpasswd);
	cmd += CSL1("|psql -h ");
	cmd += KShellProcess::quote(fDBsrv);
	cmd += CSL1(" -U ");
	cmd += KShellProcess::quote(fDBlogin);
	cmd += CSL1(" -c ");
	cmd += KShellProcess::quote(query);
	cmd += CSL1(" ");
	cmd += KShellProcess::quote(fDBnm);

	KShellProcess shproc;
	shproc.clearArguments();
	shproc << cmd;
	shproc.start(KShellProcess::Block, KShellProcess::NoCommunication);
}

void ExpenseConduit::cleanup()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fCSVStream);
	KPILOT_DELETE(fCSVFile);
	KPILOT_DELETE(fDatabase);
}
