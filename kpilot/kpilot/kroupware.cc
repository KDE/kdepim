/* kroupware.cc			KPilot
** 
** Copyright still to be determined.
**
** This file defines the actions taken when KPilot
** is Kroupware-enabled. Basically it just does a
** little communication with the local Kroupware agent (KMail).
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

#include "options.h"

#include <qfile.h>

#include <dcopclient.h>
#include <ktempfile.h>

#include <kabc/addressbook.h>
#include <kabc/addressee.h>
#include <kabc/stdaddressbook.h>


#include <kapplication.h>
#include "kroupware.h"
#include "kpilotConfig.h"

static const char *kroupware_id =
	"$Id$";
	
KroupwareSync::KroupwareSync(bool pre,int parts,KPilotDeviceLink *p) :
	SyncAction(p,pre ? "KroupwarePreSync" : "KroupwarePostSync"),
	fPre(pre),
	fParts(parts)
{
	(void) kroupware_id;
}

/* virtual */ bool KroupwareSync::exec()
{
	FUNCTIONSETUP;
	if (fPre)
	{
		preSync();
	}
	else
	{
		postSync();
	}
	// delayDone();
	emit syncDone(this);
	return true;
}

void KroupwareSync::cleanupConfig()
{
  // tempfile check in case app has terminated during sync
  KPilotConfigSettings &c = KPilotConfig::getConfig();
  c.setGroup("todoOptions");
  if ( c.hasKey( "CalFileBackup") ) {
    QString fn = c.readEntry( "CalFileBackup" , QString::null );
    if ( fn != CSL1("empty") ) {
      c.writeEntry( "CalFile" ,fn );
      c.writeEntry( "CalFileBackup" , "empty" );
    } 
  }
  c.setGroup("vcalOptions");
  if ( c.hasKey( "CalFileBackup") ) {
    QString fn = c.readEntry( "CalFileBackup" , QString::null );
    if ( fn != CSL1("empty") ) {
      c.writeEntry( "CalFile" ,fn );
      c.writeEntry( "CalFileBackup" , "empty" );
    } 
  } 
  c.setGroup("Abbrowser-conduit");
  c.writeEntry( "KMailTempFile" , "empty" );
  c.sync();
}

// For the log messages, I've added TODO_I18N to the
// ones I consider relevant for the user. The rest is
// really debug info, and shouldn't go to the normal
// sync log for the user.
//
void KroupwareSync::start_syncCal_TodosWithKMail( bool cal, bool todos )
{
  if ( !cal && ! todos )
    return;
  KPilotConfigSettings &c = KPilotConfig::getConfig();
  DCOPClient *client = kapp->dcopClient();
  KTempFile  tempfile;
  QString filename = tempfile.name();
  QByteArray  data, reply_data;  
  QCString reply_type;
  QDataStream arg(data, IO_WriteOnly);
  arg << filename;
  if (!client->call( "kmail" ,
		     "KOrganizerSyncIface",
		     "pullSyncData(QString)",
		     data,     
		     reply_type,
		     reply_data)) {
    logMessage( CSL1("Calling KMail over DCOP failed!" ));
    logMessage(CSL1("Not syncing Calendars with KMail"));
    logMessage(CSL1("Not syncing Todos with KMail"));
  }
  else {
    logMessage(CSL1("Calling Cal/Todo over DCOP succeeded"));
    // now prepare for syncing
    _syncWithKMail = true;
    if ( todos ) {
      logMessage( TODO_I18N("Syncing todos with KMail" ));
      c.setGroup("todoOptions");
      QString fn = c.readEntry( "CalFile" , QString::null );
      c.writeEntry( "CalFileBackup" ,fn );
      c.writeEntry( "CalFile" ,filename );
    }
    else
      logMessage( CSL1("Not syncing todos with KMail" ));
    if ( cal ) {
      logMessage( TODO_I18N("Syncing calendar with KMail" ));
      c.setGroup("vcalOptions");
      QString fn = c.readEntry( "CalFile" , QString::null );
      c.writeEntry( "CalFileBackup" ,fn );
      c.writeEntry( "CalFile" ,filename );
    }
    else
      logMessage( CSL1("Not syncing calendar with KMail" ));
  }
  c.sync();
}

void KroupwareSync::start_syncAddWithKMail()
{
  logMessage( CSL1("Syncing Addresses with KMail" ));
  DCOPClient *client = kapp->dcopClient();
  KTempFile  tempfile;
  QString filename = tempfile.name();
  QByteArray  data, reply_data;  
  QCString reply_type;
  QString name  = KABC::StdAddressBook::fileName();
  logMessage( name );
  QDataStream arg(data, IO_WriteOnly);
  arg << filename;
  if (!client->call( "kmail" ,
		     "KMailIface",
		     "requestAddresses(QString)",
		     data,     
		     reply_type,
		     reply_data)) {
    logMessage(CSL1("Calling KMail over DCOP failed!" ));
    logMessage(CSL1("Not syncing Addresses with KMail"));
  }
  else {
    KPilotConfigSettings &c = KPilotConfig::getConfig();
    logMessage(CSL1("Calling addresses over DCOP succeeded"));
    c.setGroup("Abbrowser-conduit");
    c.writeEntry( "KMailTempFile" , filename );
    c.sync();
  }
}
void KroupwareSync::start_syncNotesWithKMail()
{
  logMessage( TODO_I18N("Syncing Notes with Mail" ));
  logMessage( CSL1("Syncing Notes-sorry not implemented" ));
}

void KroupwareSync::end_syncCal_TodosWithKMail( bool cal, bool todos)
{
 if ( !cal && ! todos )
    return;
 QString filename;
 KPilotConfigSettings &c = KPilotConfig::getConfig();
 if ( todos ) {
   logMessage( TODO_I18N("Rewriting Todos to KMail..." ));
   c.setGroup("todoOptions");
   filename = c.readEntry( "CalFile" , QString::null );
   c.writeEntry( "CalFile" ,c.readEntry( "CalFileBackup", QString::null ) );
   c.writeEntry( "CalFileBackup" ,"empty");
 }
 if ( cal ) {
   logMessage( TODO_I18N("Rewriting Calendar to KMail" ));
   c.setGroup("vcalOptions");
   filename = c.readEntry( "CalFile" , QString::null );
   QString tf = c.readEntry( "CalFileBackup", QString::null ) ;
   c.writeEntry( "CalFile" , tf  );
   c.writeEntry( "CalFileBackup" ,"empty");
 }
 c.sync(); 
 if ( !filename.isEmpty() ) {
   logMessage(CSL1("Try to call KMail via DCOP to finish sync..."));
   // try DCOP connection to KMail
   DCOPClient *client = kapp->dcopClient();
   QByteArray  data, reply_data;  
   QCString reply_type;
   QDataStream arg(data, IO_WriteOnly);
   arg << filename;
   if (!client->call( "kmail" /*"korganizer" kmdcop */,
		      "KOrganizerSyncIface",
		      "pushSyncData(QString)",
		      data,     
		      reply_type,
		      reply_data)) {
     logMessage( CSL1("Calling KMail over DCOP failed!" ));
     logMessage( CSL1("Sync is not complete"));
     logMessage( CSL1("Data from Palm stored in file:"));
     logMessage(filename);
   } else {
     logMessage(CSL1("Calling over DCOP succeeded"));
     logMessage(CSL1("Sync to KMail has finished successfully"));
   }
   QFile::remove( filename );
 }
}
void KroupwareSync::end_syncAddWithKMail()
{
  logMessage( TODO_I18N("Syncing KMail with Addresses " ));
  DCOPClient *client = kapp->dcopClient();
  KPilotConfigSettings &c = KPilotConfig::getConfig();
  c.setGroup("Abbrowser-conduit");
  QString filename = c.readEntry( "KMailTempFile" );
  c.writeEntry( "KMailTempFile" , "empty" );
  c.sync();
  QByteArray  data, reply_data;  
  QCString reply_type;
  QDataStream arg(data, IO_WriteOnly);
  arg << filename;
  arg << QStringList();
  if (!client->call( "kmail" ,
		     "KMailIface",
		     "storeAddresses(QString, QStringList)",
		     data,     
		     reply_type,
		     reply_data)) {
    logMessage(CSL1("Calling KMail over DCOP failed!" ));
    logMessage(CSL1("Not syncing Addresses with KMail"));
  }
  else {  
    logMessage(CSL1("Calling  store addresses over DCOP succeeded"));
  }
  //QFile::remove( filename );
}
void KroupwareSync::end_syncNotesWithKMail()
{
  logMessage( TODO_I18N("Syncing KMail with Notes" ));
  logMessage( CSL1("Syncing Notes-sorry not implemented" ));
}



/* static */ bool KroupwareSync::startKMail(QString *error)
{
	FUNCTIONSETUP;
	
	QCString kmdcop;
	QString mess;
	int pid;
	
	return KApplication::startServiceByDesktopName(CSL1("kmail"),
						      QString::null, 
						      error, 
						      &kmdcop, 
						      &pid
						      )==0;
}


void KroupwareSync::preSync()
{
	cleanupConfig();
	start_syncCal_TodosWithKMail( fParts & Cal, fParts & Todo );
	if (fParts & Notes)
	{
		start_syncNotesWithKMail();
	}
	if (fParts & Address)
	{
		start_syncAddWithKMail();
	}
}

void KroupwareSync::postSync()
{
	cleanupConfig();
	end_syncCal_TodosWithKMail( fParts & Cal, fParts & Todo );
	if (fParts & Notes)
	{
		end_syncNotesWithKMail();
	}
	if (fParts & Address)
	{
		end_syncAddWithKMail();
	}
}
	
	
	
