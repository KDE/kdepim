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


#include "options.h"

// Only include what we really need:
// First UNIX system stuff, then std C++, 
// then Qt, then KDE, then local includes.
//
//
#ifndef STREAM_H
#include <stream.h>
#endif

#ifndef TIME_H
#include <time.h>
#endif


#ifndef QDIR_H
#include <qdir.h>
#endif

#ifndef QMAP_H
#include <qmap.h>
#endif

#ifndef _KGLOBAL_H
#include <kglobal.h>
#endif

#ifndef _KSTDDIRS_H
#include <kstddirs.h>
#endif

#ifndef _KMESSAGEBOX_H
#include <kmessagebox.h>
#endif

#ifndef _KSIMPLECONFIG_H
#include <ksimpleconfig.h>
#endif

#ifndef _KCONFIG_H
#include <kconfig.h>
#endif

#ifndef _DCOPCLIENT_H
#include <dcopclient.h>
#endif

#ifndef _KDEBUG_H
#include <kdebug.h>
#endif

#ifndef _KPROCESS_H
#include <kprocess.h>
#endif


#ifndef _PILOT_EXPENSE_H_
#include <pi-expense.h>
#endif

#ifndef _KPILOT_CONDUITAPP_H
#include "conduitApp.h"
#endif

#ifndef _KPILOT_KPILOTCONFIG_H
#include "kpilotConfig.h"
#endif

#ifndef _KPILOT_EXPENSE_H
#include "expense.h"
#endif

#ifndef _KPILOT_SETUPDIALOG_H
#include "setupDialog.h"
#endif

#ifndef _KPILOT_PILOTDATEENTRY_H
#include "pilotDateEntry.h"
#endif

#ifndef _KPILOT_PILOTDATABASE_H
#include "pilotDatabase.h"
#endif
 
#ifndef _KPILOT_PILOTRECORD_H
#include "pilotRecord.h"
#endif


#ifndef _PILOT_EXPENSE_H
#include <pi-expense.h>
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#ifndef _QCSTRING_H
#include <qcstring.h>
#endif

#ifndef _QOBJECT_H
#include <qobject.h>
#endif

#ifndef _QDATETIME_H
#include <qdatetime.h>
#endif

#ifndef _QTEXTSTREAM_H
#include <qtextstream.h>
#endif

#ifndef _STDIO_H
#include <stdio.h>
#endif

#ifndef _STRING_H
#include <string.h>
#endif


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

char *
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

char *
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


// This is a generic main() function, all
// conduits look basically the same,
// except for the name of the conduit.
//
//
int main(int argc, char* argv[])
{
	ConduitApp a(argc,argv,"conduitExpense",
		I18N_NOOP("Expense Conduit"),
		KPILOT_VERSION);


	a.addAuthor("Adriaan de Groot",
		"Expense Conduit author",
		"adridg@cs.kun.nl");

	a.addAuthor("Christopher Molnar",
		"Expense Conduit author",
		"molnarc@nebsllc.com");

	ExpenseConduit conduit(a.getMode());
	a.setConduit(&conduit);
	return a.exec();
}




ExpenseConduit::ExpenseConduit(eConduitMode mode) : 
	BaseConduit(mode)
{
	FUNCTIONSETUP;

}

ExpenseConduit::~ExpenseConduit()
{
	FUNCTIONSETUP;

}

void
ExpenseConduit::doSync()
{
	FUNCTIONSETUP;
	struct Expense e;
	kdDebug() << "expense" << ": In expense doSync" << endl;

	KConfig& config=KPilotConfig::getConfig();
	config.setGroup(ExpenseOptions::ExpenseGroup);
	
	QString mDBType=config.readEntry("DBTypePolicy");
	QString mDBnm=config.readEntry("DBname");
	QString mDBsrv=config.readEntry("DBServer");
	QString mDBtable=config.readEntry("DBtable");
	QString mDBlogin=config.readEntry("DBlogin");
	QString mDBpasswd=config.readEntry("DBpasswd");
	QString mCSVname=config.readEntry("CSVFileName");
	
	PilotRecord* rec;
    
	KShellProcess proc;
        int recordcount=0;
	int index=0;
	int syscall=0;

// Now let's open databases

	if (mDBType=="1")
	{
		
		DEBUGCONDUIT << fname << " Postgres database requested" << endl;
// next three lines just for debug purposes - Remove for final creates a dump of table.		
		char sqlcmd[300];
		sprintf(sqlcmd,"echo \"%s\"|psql -h %s -U %s -c \"select * from %s;\" %s >testpg.txt",mDBpasswd.latin1(),mDBsrv.latin1(),mDBlogin.latin1(),mDBtable.latin1(),mDBnm.latin1());
		proc.clearArguments();
		proc << sqlcmd;
		proc.start(KShellProcess::Block, KShellProcess::NoCommunication);
		while (proc.isRunning())
			{
			DEBUGCONDUIT << fname << " " << proc.pid() << " still running" << endl;
			}
		// DEBUGCONDUIT << fname << proc.args() << endl;
		// DEBUGCONDUIT << fname << sqlcmd << endl;
	}

	if (mDBType=="2")
	{
		DEBUGCONDUIT << fname << "MySQL database requested" << endl;
	}

// Now let's read records
	while ((rec=readNextModifiedRecord()))
// 	while ((rec=readRecordByIndex(index++)))
        {
                if (rec->isDeleted())
                {
                        FUNCTIONSETUP;
                }
                else
                {
                        FUNCTIONSETUP;
			DEBUGCONDUIT << fname
               		 << ": Got record "
           		 << index-1
               		 << " @"
                	<< (int) rec
			<< endl;
			(void) unpack_Expense(&e,(unsigned char *)rec->getData(),rec->getLen());
			rec=0L;
			
			if (!mCSVname.isEmpty())
			{
				DEBUGCONDUIT << fname
				<< mCSVname 
				<< endl;
//open file
				QFile fp(mCSVname);
				fp.open(IO_ReadWrite|IO_Append);	
				
//format date for csv file
				int tmpyr=e.date.tm_year+1900;
				char dtstng[DATESIZE];
				int tmpday=e.date.tm_mday;
				int tmpmon=e.date.tm_mon+1;
				sprintf(dtstng,"%d-%d-%d",tmpyr,tmpmon,tmpday);
				const char* mesg=dtstng;
				fp.writeBlock(mesg,qstrlen(mesg));	
				const char* delim=",";
				fp.writeBlock(delim,qstrlen(delim));	
//write rest of record
				mesg=e.amount;
				fp.writeBlock(mesg,qstrlen(mesg));	
				fp.writeBlock(delim,qstrlen(delim));	
				mesg=get_pay_type(e.payment);
				fp.writeBlock(mesg,qstrlen(mesg));	
				fp.writeBlock(delim,qstrlen(delim));	
				mesg=e.vendor;
				fp.writeBlock(mesg,qstrlen(mesg));	
				fp.writeBlock(delim,qstrlen(delim));	
				mesg=get_entry_type(e.type);
				fp.writeBlock(mesg,qstrlen(mesg));	
				fp.writeBlock(delim,qstrlen(delim));	
				mesg=e.city;
				fp.writeBlock(mesg,qstrlen(mesg));	
				fp.writeBlock(delim,qstrlen(delim));	

// remove line breaks from list of attendees - can't have in csv files

				QString tmpatt=e.attendees;
				QString tmpatt2=tmpatt.simplifyWhiteSpace();
				mesg=tmpatt2.latin1();				
				fp.writeBlock(mesg,qstrlen(mesg));	
				fp.writeBlock(delim,qstrlen(delim));	

// remove extra formatting from notes - can't have in csv files

				QString tmpnotes=e.note;
				QString tmpnotes2=tmpnotes.simplifyWhiteSpace();
				DEBUGCONDUIT << tmpnotes2 << endl;
				mesg=tmpnotes2.latin1();				
				DEBUGCONDUIT << mesg << endl;
				fp.writeBlock(mesg,qstrlen(mesg));	
				fp.writeBlock(delim,qstrlen(delim));	

//Finish line
				const char* endline="\n";	
				fp.writeBlock(endline,qstrlen(endline));	

//be nice and close file
				fp.close();	
				
			}

// Now let's write record to correct database
	
			if (mDBType=="0")
			{
				DEBUGCONDUIT << fname << " No database requested" << endl;

			}

			if (mDBType=="1")
			{
				DEBUGCONDUIT << fname << " Postgres database requested" << endl;
			int tmpyr=e.date.tm_year+1900;
                                char dtstng[DATESIZE];
                                int tmpday=e.date.tm_mday;
                                int tmpmon=e.date.tm_mon+1;
                                sprintf(dtstng,"%d-%d-%d",tmpyr,tmpmon,tmpday);
			QString tmpnotes=e.note;
                        QString tmpnotes2=tmpnotes.simplifyWhiteSpace();
			const char* nmsg=tmpnotes2.latin1();

			QString tmpatt=e.attendees;
                        QString tmpatt2=tmpatt.simplifyWhiteSpace();
                        const char* amesg=tmpatt2.latin1();
			const char* etmsg=get_entry_type(e.type);
			const char* epmsg=get_pay_type(e.payment);
			char sqlcmd[400];
			sprintf(sqlcmd,"echo \"%s\"|psql -h %s -U %s -c \"INSERT INTO \"%s\" (\"fldTdate\", \"fldAmount\", \"fldPType\", \"fldVName\", \"fldEType\", \"fldLocation\", \"fldAttendees\", \"fldNotes\") VALUES ('%s', '%s', '%s',
 '%s', '%s', '%s', '%s', '%s');\" %s",mDBpasswd.latin1(),mDBsrv.latin1(),mDBlogin.latin1(),mDBtable.latin1(),dtstng,e.amount,epmsg,e.vendor,etmsg,e.city,amesg,nmsg,mDBnm.latin1());
			// DEBUGCONDUIT << fname << " " << sqlcmd << endl;
		        //	DEBUGCONDUIT << fname << " " << proc.args() << endl;
			proc.clearArguments();
			proc << sqlcmd;
			proc.start(KShellProcess::Block, KShellProcess::NoCommunication);
			DEBUGCONDUIT << fname << " " << proc.pid() << " finished OK " << endl;
			DEBUGCONDUIT << fname << " " << syscall << endl;
			}

			if (mDBType=="2")
			{
				DEBUGCONDUIT << fname << " MySQL database requested" << endl;

			}

// REMEMBER to CLOSE database			

		}
	DEBUGCONDUIT << fname << " Records: " << recordcount << endl;
	}
	proc.kill();
}



// aboutAndSetup is pretty much the same
// on all conduits as well.
//
//
QWidget*
ExpenseConduit::aboutAndSetup()
{
	FUNCTIONSETUP;

	return new ExpenseOptions(0L);
}

const char *
ExpenseConduit::dbInfo()
{
	return "ExpenseDB";
}



/* virtual */ void
ExpenseConduit::doTest()
{
	FUNCTIONSETUP;

	(void) expense_id;
}

// $Log$
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
