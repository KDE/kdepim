/* sysinfo-conduit.cc                           KPilot
**
** Copyright (C) 2003 by Reinhold Kainhofer
**
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
** Bug reports and questions can be sent to kde-pim@kde.org.
*/

#include "options.h"

#include <pi-version.h>

#include <qtimer.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <kconfig.h>
#include <kdebug.h>

#include <pilotSysInfo.h>
#include <pilotUser.h>
#include <pilotCard.h>
#include <kpilotlink.h>
#include <kstandarddirs.h>
#include <pilotSerialDatabase.h>

#include <sys/utsname.h>

#include "sysinfo-factory.h"
#include "sysinfo-conduit.moc"
#include "sysinfoSettings.h"

const QString SysInfoConduit::defaultpage = QString("KPilot System Information Page\n"
"==============================\n"
"(Kpilot was unable to find the correct template file, \n"
"so this simple template was used.)\n\n"
"<!--#ifhardware#\n"
"-) Hardware Information\n"
"     DeviceID:      #deviceid#\n"
"     Device name:   #devicename#\n"
"     Device model:  #devicemodel#\n"
"     Manufacturer:  #manufacturer#\n"
"     Connected via: #devicetype#\n"
"#endifhardware#-->\n"
"\n"
"<!--#ifuser#\n"
"-) User Information\n"
"     Handheld User Name: #username#\n"
"     Handheld Password:  #pw#\n"
"     Handheld User ID:   #uid#\n"
"     Viewer ID:          #viewerid#\n"
"#endifuser#-->\n"
"\n"
"<!--#ifmemory#\n"
"-) Memory Information\n"
"     ROM:       #rom# kB total\n"
"     Total RAM: #totalmem# kB total\n"
"     Free RAM:  #freemem# kB free\n"
"#endifmemory#-->\n"
"\n"
"<!--#ifstorage#\n"
"-) Storage Information\n"
"     Number of cards: #cards#\n"
"     Memory on cards: #storagemem#\n"
"#endifstorage#-->\n"
"\n"
"<!--#ifdblist#\n"
"-) List of Databases on Handheld\n"
"     Available Databases: #dblist(%1,)#\n"
"#endifdblist#-->\n"
"\n"
"<!--#ifrecords#\n"
"-) Number of addresses, to-dos, events, and memos\n"
"     Addresses: #addresses# entries in Addressbook\n"
"     Events:    #events# entries in Calendar\n"
"     To-dos:    #todos# entries in To-do list\n"
"     Memos:     #memos# memos\n"
"#endifrecords#-->\n"
"\n"
"<!--#ifsync#\n"
"-) Synchronization Information\n"
"     Last sync attempt:      #lastsync#\n"
"     Last successful sync:  #lastsuccsync#\n"
"     Last sync with PC (ID): #lastsyncpc#\n"
"#endifsync#-->\n"
"\n"
"<!--#ifpcversion#\n"
"-) Version Information (Desktop)\n"
"     Operating System:   #os#\n"
"     Hostname:           #hostname#\n"
"     Qt Version:         #qt#\n"
"     KDE Version:        #kde#\n"
"     KPilot Version:     #kpilot#\n"
"     Pilot-Link Version: #pilotlink#\n"
"#endifpcversion#-->\n"
"\n"
"<!--#ifpalmversion#\n"
"-) Version Information (Handheld)\n"
"     PalmOS: #palmos#\n"
"#endifpalmversion#-->\n"
"\n"
"<!--#ifdebug#\n"
"-) Debug Information\n"
"     #debug#\n"
"#endifdebug#-->\n"
"\n"
"------------------------------------------------------------\n"
"Page created <!--#date#--> by the KPilot System Information conduit.\n"
"");


	/** possible fields in the templates are:
	 *  - hardware
	 *  - user
	 *  - memory
	 *  - storage
	 *  - dblist
	 *  - recnumber
	 *  - syncinfo
	 *  - pcversion
	 *  - palmversion
	 *  - debug
	 */


// Something to allow us to check what revision
// the modules are that make up a binary distribution.

extern "C"
{

long version_conduit_sysinfo = KPILOT_PLUGIN_API;
const char *id_conduit_sysinfo =
	"$Id$";

}



SysInfoConduit::SysInfoConduit(KPilotDeviceLink * o,
	const char *n,
	const QStringList & a) :
	ConduitAction(o, n, a)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT<<id_conduit_sysinfo<<endl;
#endif
	fConduitName=i18n("System Information");
}



SysInfoConduit::~SysInfoConduit()
{
	FUNCTIONSETUP;
}



void SysInfoConduit::readConfig()
{
	FUNCTIONSETUP;
	
#ifdef DEBUG
	DEBUGCONDUIT<<"Output file="<<SysinfoSettings::outputFile()<<" with type "<<
		SysinfoSettings::outputFormat()<<" (Template:"<<SysinfoSettings::templateFile()<<")"<<endl;
	DEBUGCONDUIT<<"HW:"<<SysinfoSettings::hardwareInfo()<<",User:"<<SysinfoSettings::userInfo()<<
		",Mem:"<<SysinfoSettings::memoryInfo()<<",Sto:"<<SysinfoSettings::storageInfo()<<endl;
	DEBUGCONDUIT<<"DBL:"<<SysinfoSettings::databaseList()<<",Rec:"<<SysinfoSettings::recordNumbers()<<
		",KDE:"<<SysinfoSettings::kDEVersion()<<",PalmOS:"<<SysinfoSettings::palmOSVersion()<<endl;
#endif
}


/* virtual */ bool SysInfoConduit::exec()
{
	FUNCTIONSETUP;
	DEBUGCONDUIT<<id_conduit_sysinfo<<endl;

	readConfig();

	QTimer::singleShot(0, this, SLOT(hardwareInfo()));
	return true;
}

void SysInfoConduit::hardwareInfo()
{
	FUNCTIONSETUP;
	if (fHardwareInfo) {
		QString unknown = i18n("unknown");
		
		/* Retrieve values for
		* - #deviceid#
		* - #devicename#
		* - #devicemodel#
		* - #manufactorer#
		* - #devicetype#
		*/
		KPilotSysInfo *sysinfo = fHandle->getSysInfo();
		if (sysinfo)
		{
			fValues["deviceid"] = QString(sysinfo->getProductID());
		}
		else
		{
			fValues["deviceid"] = unknown;
		}
		
		KPilotCard *device = fHandle->getCardInfo();
		if (device)
		{
			fValues["devicename"] = QString(device->getCardName());
			fValues["devicemodel"] = unknown;  // TODO
			fValues["manufacturer"] = QString(device->getCardManufacturer());
		}
		else
		{
			fValues["devicename"] = unknown;
			fValues["devicemodel"] = unknown;
			fValues["manufacturer"] = unknown;
		}
		
		fValues["devicetype"] = unknown;
		
		KPILOT_DELETE(device);
		keepParts.append("hardware");
	} else removeParts.append("hardware");
	QTimer::singleShot(0, this, SLOT(userInfo()));
}

void SysInfoConduit::userInfo()
{
	FUNCTIONSETUP;
	if (fUserInfo) {
		/* Retrieve values for
		 * - #username#
		 * - #uid#
		 */
		KPilotUser*user=fHandle->getPilotUser();
		fValues["username"] = user->getUserName();
		if (user->getPasswordLength()>0)
			fValues["pw"] = i18n("Password set");
		else
			fValues["pw"] = i18n("No password set");
		fValues["uid"] = QString::number(user->getUserID());
		fValues["viewerid"] = QString::number(user->getViewerID());
		keepParts.append("user");
	} else removeParts.append("user");
	QTimer::singleShot(0, this, SLOT(memoryInfo()));
}

void SysInfoConduit::memoryInfo()
{
	FUNCTIONSETUP;
	if (fMemoryInfo) {
		/* Retrieve values for
		 * - #rom#
		 * - #totalmem#
		 * - #freemem#
		 */
		KPilotCard*device = fHandle->getCardInfo();
		fValues["rom"] =  QString::number(device->getRomSize()/1024);
		fValues["totalmem"] =  QString::number(device->getRamSize()/1024);
		fValues["freemem"] =  QString::number(device->getRamFree()/1024);
		keepParts.append("memory");
	} else removeParts.append("memory");
	QTimer::singleShot(0, this, SLOT(storageInfo()));
}

void SysInfoConduit::storageInfo()
{
	FUNCTIONSETUP;
	if (fStorageInfo) {
		/* Retrieve values for
		 * - $cards$
		 */
		KPilotCard*device = fHandle->getCardInfo(1);
		if (device) {
			fValues["cards"] = QString("%1 (%2, %3 kB of %3 kB free)")
				.arg(device->getCardName())
				.arg(device->getCardManufacturer())
				.arg(device->getRamFree()/1024)
				.arg(device->getRamSize()/1024);
			KPILOT_DELETE(device);
		} else {
			fValues["cards"] = i18n("No Cards available via pilot-link");
		}
		keepParts.append("storage");
	} else removeParts.append("storage");
	QTimer::singleShot(0, this, SLOT(dbListInfo()));
}

void SysInfoConduit::dbListInfo()
{
	FUNCTIONSETUP;
	if (fDBList) {
		/* Retrieve values for
		 * - #dblist(structure)#
		 */
		dblist=fHandle->getDBList();
		keepParts.append("dblist");
	} else removeParts.append("dblist");
	QTimer::singleShot(0, this, SLOT(recNumberInfo()));
}

void SysInfoConduit::recNumberInfo()
{
	FUNCTIONSETUP;
	if (fRecordNumber) {
		/* Retrieve values for
		 * - #addresses#
		 * - #events#
		 * - #todos#
		 * - #memos#
		 */
		PilotDatabase*fDatabase;
		fValues["addresses"] = "ERROR";
		fValues["events"] = "ERROR";
		fValues["todos"] = "ERROR";
		fValues["memos"] = "ERROR";
		fDatabase = new PilotSerialDatabase(pilotSocket(), "AddressDB");
		if (fDatabase) {
			fValues["addresses"] = QString::number(fDatabase->recordCount());
			KPILOT_DELETE(fDatabase);
		}
		fDatabase = new PilotSerialDatabase(pilotSocket(), "DatebookDB");
		if (fDatabase) {
			fValues["events"] = QString::number(fDatabase->recordCount());
			KPILOT_DELETE(fDatabase);
		}
		fDatabase = new PilotSerialDatabase(pilotSocket(), "ToDoDB");
		if (fDatabase) {
			fValues["todos"] = QString::number(fDatabase->recordCount());
			KPILOT_DELETE(fDatabase);
		}
		fDatabase = new PilotSerialDatabase(pilotSocket(), "MemoDB");
		if (fDatabase) {
			fValues["memos"] = QString::number(fDatabase->recordCount());
			KPILOT_DELETE(fDatabase);
		}
		keepParts.append("records");
	} else removeParts.append("records");
	QTimer::singleShot(0, this, SLOT(syncInfo()));
}

void SysInfoConduit::syncInfo()
{
	FUNCTIONSETUP;
	if (fSyncInfo) {
		/* Retrieve values for
		 * - #lastsync#
		 * - #lastsuccsync#
		 * - #lastsyncpc#
		 */
		KPilotUser*user=fHandle->getPilotUser();
		time_t lastsync = user->getLastSyncDate();
		QDateTime qlastsync;
		qlastsync.setTime_t(lastsync);
		fValues["lastsync"] = qlastsync.toString(Qt::LocalDate);
		lastsync = user->getLastSuccessfulSyncDate();
		qlastsync.setTime_t(lastsync);
		fValues["lastsuccsync"] = qlastsync.toString(Qt::LocalDate);
		fValues["lastsyncpc"] = QString::number(user->getLastSyncPC());
		keepParts.append("sync");
	} else removeParts.append("sync");
	QTimer::singleShot(0, this, SLOT(pcVersionInfo()));
}

void SysInfoConduit::pcVersionInfo()
{
	FUNCTIONSETUP;
	if (fKDEVersion) {
		/* Retrieve values for
		 * - #os#
		 * - #qt#
		 * - #kde#
		 * - #kpilot#
		 * - #pilotlink#
		 */
		fValues["kpilot"] = QString::fromLatin1(KPILOT_VERSION);
		fValues["kde"] = i18n("unknown");
		fValues["qt"] = i18n("unknown");
		fValues["os"] = i18n("unknown");
		fValues["hostname"] = i18n("unknown");
		struct utsname name;
		if (uname (&name) >= 0) {
			fValues["os"] = QString("%1 %3, %5")
				.arg(name.sysname)
				.arg(name.release)
				.arg(name.machine);
			fValues["hostname"] = QString("%2").arg(name.nodename);
		}
#ifdef KDE_VERSION_STRING
		fValues["kde"] = QString::fromLatin1(KDE_VERSION_STRING);
#endif
#ifdef QT_VERSION_STR
		fValues["qt"] = QString::fromLatin1(QT_VERSION_STR);
#endif
		fValues["pilotlink"] = CSL1("%1.%2.%3%4")
			.arg(PILOT_LINK_VERSION)
			.arg(PILOT_LINK_MAJOR)
			.arg(PILOT_LINK_MINOR)
#ifdef PILOT_LINK_PATCH
			.arg(QString::fromLatin1(PILOT_LINK_PATCH));
#else
			.arg(QString());
#endif
		keepParts.append("pcversion");
	} else removeParts.append("pcversion");
	QTimer::singleShot(0, this, SLOT(palmVersionInfo()));
}

void SysInfoConduit::palmVersionInfo()
{
	FUNCTIONSETUP;
	if (fPalmOSVersion) {
		/* Retrieve values for
		 * - #palmos#
		 */
/*		fValues["palmos"] = QString("PalmOS® %1.%2 (compat %3.%4)")
			.arg(fHandle->getSysInfo()->getMajorVersion())
			.arg(fHandle->getSysInfo()->getMinorVersion())
			.arg(fHandle->getSysInfo()->getCompatMajorVersion())
			.arg(fHandle->getSysInfo()->getCompatMinorVersion());*/
		fValues["palmos"] = QString("PalmOS® %1.%2").arg(fHandle->majorVersion()).arg(fHandle->minorVersion());

		keepParts.append("palmversion");
	} else removeParts.append("palmversion");
	QTimer::singleShot(0, this, SLOT(debugInfo()));
}

void SysInfoConduit::debugInfo()
{
	FUNCTIONSETUP;
	if (fDebugInfo) {
		/* Retrieve values for
		 * - #debug#
		 */
		fValues["debug"] = i18n("No debug data");
		keepParts.append("debug");
	} else removeParts.append("debug");
	QTimer::singleShot(0, this, SLOT(writeFile()));
}

void SysInfoConduit::writeFile()
{
	FUNCTIONSETUP;

	fValues["date"] = QDateTime::currentDateTime().toString(Qt::LocalDate);

	QString output;
	// Open the template file
	QString templatefile;
	switch(fOutputType)
	{
		case eOutputText:
			templatefile=locate("data", "kpilot/sysinfoconduit/Template.txt");
			break;
		case eOutputTemplate:
			templatefile=fTemplateFile;
			break;
		case eOutputHTML:
		default:
			templatefile=locate("data", "kpilot/sysinfoconduit/Template.html");
			break;
	}

	// Read in the template, close the file
	bool loaded=false;
	if (!templatefile.isEmpty()){
#ifdef DEBUG
		DEBUGCONDUIT<<"Loading template file "<<templatefile<<endl;
#endif
		QFile infile(templatefile);
		if (infile.open(IO_ReadOnly)) {
			QTextStream instream(&infile);
			output = instream.read();
			infile.close();
			loaded=true;
		}
	}

	if (!loaded) {
		kdWarning()<<"Loading template file "<<templatefile<<" failed. Using default template instead."<< endl;
		output=defaultpage;
	}

	// Remove all parts not extracted
	for ( QStringList::Iterator it = removeParts.begin(); it != removeParts.end(); ++it ) {
		QRegExp re(QString("<!--#if%1#.*#endif%1#-->").arg(*it).arg(*it));
		re.setMinimal(true);
		output.remove(re);
	}
	for ( QStringList::Iterator it = keepParts.begin(); it != keepParts.end(); ++it ) {
		QRegExp re(QString("<!--#if%1#(.*)#endif%1#-->").arg(*it).arg(*it));
		re.setMinimal(true);
		output.replace(re, "\\1");
	}

	// Do a loop through all keys in fValues
	QMap<QString,QString>::Iterator it;
	for ( it = fValues.begin(); it != fValues.end(); ++it ) {
		output.replace(QString("#%1#").arg(it.key().latin1()), it.data().latin1());
	}

	// Insert the list of databases
	QRegExp re("#dblist\\[(.*)\\]#");
	re.setMinimal(true);
	while (re.search(output)>=0){
		QString dbstring;
		QString subpatt=re.cap(1);
		DBInfo*dbi;
		for (dbi=dblist.first(); dbi; dbi=dblist.next() ) {
			QString newpatt(subpatt);
			char tmpchr[5];
			::memset(&tmpchr[0], 0, 5);
			/* Patterns for the dblist argument:
			 * %0 .. Database name
			 * %1 .. type
			 * %2 .. creator
			 * %3 .. index
			 * %4 .. flags
			 * %5 .. miscFlags
			 * %6 .. version
			 * %7 .. createDate
			 * %8 .. modifyDate
			 * %9 .. backupDate
			 */
			newpatt.replace("%0", QString(dbi->name));
			set_long(&tmpchr[0],dbi->type);
			newpatt.replace("%1", QString(tmpchr));
			set_long(&tmpchr[0],dbi->creator);
			newpatt.replace("%2", tmpchr);
			newpatt.replace("%3", QString::number(dbi->index));
			newpatt.replace("%4", QString::number(dbi->flags));
			newpatt.replace("%5", QString::number(dbi->miscFlags));
			newpatt.replace("%6", QString::number(dbi->version));
			QDateTime tm;
			tm.setTime_t(dbi->createDate);
			newpatt.replace("%7", tm.toString(Qt::LocalDate));
			tm.setTime_t(dbi->modifyDate);
			newpatt.replace("%8", tm.toString(Qt::LocalDate));
			tm.setTime_t(dbi->backupDate);
			newpatt.replace("%9", tm.toString(Qt::LocalDate));

			dbstring.append(newpatt);
		}
		// Now, just replace the whole found pattern by the string we just constructed.
		output.replace(re.cap(0), dbstring);
	}

	// Write out the result
	QFile outfile(fOutputFile);
	if (fOutputFile.isEmpty() || (!outfile.open(IO_WriteOnly)) ) {
		QFileInfo fi(QDir::home(), QString("KPilotSysInfo.")+QFileInfo(templatefile).extension() );
		fOutputFile=fi.absFilePath();
		kdWarning()<<i18n("Unable to open output file, using %1 instead.").arg(fOutputFile).latin1()<<endl;
		emit logMessage(i18n("Unable to open output file, using %1 instead.").arg(fOutputFile));
		outfile.setName(fOutputFile);
		if (!outfile.open(IO_WriteOnly)) {
			kdWarning()<<i18n("Unable to open %1").arg(fOutputFile).latin1()<<endl;
			emit logError(i18n("Unable to open %1").arg(fOutputFile));
			QTimer::singleShot(0, this, SLOT(cleanup()));
			return;
		}
	}

	// Finally, write the actual text out to the file.
	QTextStream outstream(&outfile);
	outstream<<output;
	outfile.close();

	emit logMessage(i18n("Handheld system information written to the file %1").arg(fOutputFile));
	QTimer::singleShot(0, this, SLOT(cleanup()));
}

void SysInfoConduit::cleanup()
{
	FUNCTIONSETUP;
	// Nothing to clean up so far (Do I have memory leaks somewhere?)
	emit syncDone(this);
}
