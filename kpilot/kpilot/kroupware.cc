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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qfile.h>

#include <dcopclient.h>
#include <ktempfile.h>


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
  // TODO!!! Use sensitive groups/keys for the kroupware branch...
  KConfig* c = KPilotSettings::self()->config();
  c->setGroup("todoOptions");
  if ( c->hasKey( "CalFileBackup") ) {
    QString fn = c->readPathEntry( "CalFileBackup" );
    if ( fn != CSL1("empty") ) {
      c->writePathEntry( "CalFile" ,fn );
      c->writeEntry( "CalFileBackup" , "empty" );
    }
  }
  c->setGroup("vcalOptions");
  if ( c->hasKey( "CalFileBackup") ) {
    QString fn = c->readPathEntry( "CalFileBackup" );
    if ( fn != CSL1("empty") ) {
      c->writePathEntry( "CalFile" ,fn );
      c->writeEntry( "CalFileBackup" , "empty" );
    }
  }
  c->setGroup("Abbrowser-conduit");
  c->writeEntry( "KMailTempFile" , "empty" );
  KPilotSettings::writeConfig();
}

// For the log messages, I've added i18n to the
// ones I consider relevant for the user. The rest is
// really debug info, and shouldn't go to the normal
// sync log for the user.
//
// TODO!!! better way to read the config options!
void KroupwareSync::start_syncCal_TodosWithKMail( bool cal, bool todos )
{
  if ( !cal && ! todos )
    return;
  KConfig*c = KPilotSettings::self()->config();
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
    logMessage(CSL1("Not syncing calendars with KMail"));
    logMessage(CSL1("Not syncing to-dos with KMail"));
  }
  else {
    logMessage(CSL1("Calling Cal/Todo over DCOP succeeded"));
    // now prepare for syncing
    _syncWithKMail = true;
    if ( todos ) {
      logMessage( i18n("Syncing to-dos with KMail" ));
      c->setGroup("todoOptions");
      QString fn = c->readPathEntry( "CalFile" );
      c->writePathEntry( "CalFileBackup" ,fn );
      c->writePathEntry( "CalFile" ,filename );
    }
    else
      logMessage( CSL1("Not syncing todos with KMail" ));
    if ( cal ) {
      logMessage( i18n("Syncing calendar with KMail" ));
      c->setGroup("vcalOptions");
      QString fn = c->readPathEntry( "CalFile" );
      c->writePathEntry( "CalFileBackup" ,fn );
      c->writePathEntry( "CalFile" ,filename );
    }
    else
      logMessage( CSL1("Not syncing calendar with KMail" ));
  }
  KPilotSettings::self()->writeConfig();
}

void KroupwareSync::start_syncAddWithKMail()
{
  logMessage( CSL1("Syncing Addresses with KMail" ));
  DCOPClient *client = kapp->dcopClient();
  KTempFile  tempfile;
  QString filename = tempfile.name();
  QByteArray  data, reply_data;
  QCString reply_type;
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
    // TODO!!! better config handling!
    KConfig*c = KPilotSettings::self()->config();
    logMessage(CSL1("Calling addresses over DCOP succeeded"));
    c->setGroup("Abbrowser-conduit");
    c->writePathEntry( "KMailTempFile" , filename );
    KPilotSettings::self()->writeConfig();
  }
}
void KroupwareSync::start_syncNotesWithKMail()
{
  logMessage( i18n("Syncing Notes with Mail" ));
  logMessage( CSL1("Syncing Notes-sorry not implemented" ));
}

void KroupwareSync::end_syncCal_TodosWithKMail( bool cal, bool todos)
{
 if ( !cal && ! todos )
    return;
 QString filename;
 KConfig*c=KPilotSettings::self()->config();
 if ( todos ) {
   logMessage( i18n("Rewriting to-dos to KMail..." ));
   c->setGroup("todoOptions");
   filename = c->readPathEntry( "CalFile" );
   c->writePathEntry( "CalFile", c->readPathEntry( "CalFileBackup" ) );
   c->writeEntry( "CalFileBackup", "empty");
 }
 if ( cal ) {
   logMessage( i18n("Rewriting Calendar to KMail" ));
   c->setGroup("vcalOptions");
   filename = c->readPathEntry( "CalFile" );
   QString tf = c->readPathEntry( "CalFileBackup" ) ;
   c->writePathEntry( "CalFile" , tf  );
   c->writeEntry( "CalFileBackup" ,"empty");
 }
 KPilotSettings::writeConfig();
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
  logMessage( i18n("Syncing KMail with Addresses " ));
  DCOPClient *client = kapp->dcopClient();
  // TODO!! better config handling (KConfig XT)
  KConfig*c = KPilotSettings::self()->config();
  c->setGroup("Abbrowser-conduit");
  QString filename = c->readPathEntry( "KMailTempFile" );
  c->writeEntry( "KMailTempFile" , "empty" );
  KPilotSettings::writeConfig();
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
  logMessage( i18n("Syncing KMail with Notes" ));
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



