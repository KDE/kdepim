/*expense.cc			KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot
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
#include <stream.h>
#include <time.h>

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

#include <pi-expense.h>
#include <stdlib.h>
#include <qcstring.h>
#include <qdatetime.h>
#include <qtextstream.h>
#include <stdio.h>
#include <string.h>

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
	
	QString mDBnm=config.readEntry("DBname");
	QString mDBsrv=config.readEntry("DBServer");
	QString mCSVname=config.readEntry("CSVFileName");

	kdDebug() << "expense" << ": Read config entry \n" << "Db name: " << mDBnm << endl;
	
	PilotRecord* rec;
        int recordcount=0;
	int index=0;

	while ((rec=readRecordByIndex(index++)))
        {
                if (rec->isDeleted())
                {
                        FUNCTIONSETUP;
			kdDebug() << ": In expense doSync - Deleted Record" << endl;
                }
                else
                {
                        FUNCTIONSETUP;
			kdDebug() << ": In doSync - Non-deleted Record" << endl;
			DEBUGCONDUIT << fname
               		 << ": Got record "
           		 << index-1
               		 << " @"
                	<< (int) rec
			<< endl;
			(void) unpack_Expense(&e,(unsigned char *)rec->getData(),rec->getLen());
			DEBUGCONDUIT << fname
			<< "Type: " 
			<< e.type 
			<< endl 
			<< "Amount: " 
			<< e.amount << endl;
			rec=0L;
			
			if (!mCSVname.isEmpty())
			{
				DEBUGCONDUIT << fname
				<< mCSVname 
				<< endl;
				QFile fp(mCSVname);
				fp.open(IO_ReadWrite|IO_Append);	
				int tmpyr=e.date.tm_year;
				char* dtstng;
				int tmpday=e.date.tm_mday;
				int tmpmon=e.date.tm_mon;
				sprintf(dtstng,"%i-%i-%i",tmpyr,tmpmon,tmpday);
				const char* mesg=dtstng;
				fp.writeBlock(mesg,qstrlen(mesg));	
				mesg=e.amount;
				fp.writeBlock(mesg,qstrlen(mesg));	
				const char* delim=",";
				fp.writeBlock(delim,qstrlen(delim));	
				mesg=e.vendor;
				fp.writeBlock(mesg,qstrlen(mesg));	
				fp.writeBlock(delim,qstrlen(delim));	
				mesg="ExpenseType";
				fp.writeBlock(mesg,qstrlen(mesg));	
				fp.writeBlock(delim,qstrlen(delim));	
				mesg=e.city;
				fp.writeBlock(mesg,qstrlen(mesg));	
				fp.writeBlock(delim,qstrlen(delim));	
				mesg=e.attendees;
				fp.writeBlock(mesg,qstrlen(mesg));	
				fp.writeBlock(delim,qstrlen(delim));	

				const char* endline="\n";	
				fp.writeBlock(endline,qstrlen(endline));	


				fp.close();	

				
				
			}	

		}
	DEBUGCONDUIT << "Records: " << recordcount << endl;
	}
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
