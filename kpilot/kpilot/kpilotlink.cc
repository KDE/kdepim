/* kpilotlink.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is a messed-up class that does
** 	- config file management
**	- local database management
**	- IPC
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
static const char *kpilotlink_id="$Id$";

#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif

#ifndef _PILOT_SOURCE_H_
#include <pi-source.h>
#endif
#ifndef _PILOT_SOCKET_H_
#include <pi-socket.h>
#endif
#ifndef _PILOT_DLP_H_
#include <pi-dlp.h>
#endif
#ifndef _PILOT_FILE_H_
#include <pi-file.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream.h>

#ifndef QDIR_H
#include <qdir.h>
#endif

#ifndef _KCONFIG_H
#include <kconfig.h>
#endif
#ifndef _KLOCALE_H
#include <klocale.h>
#endif
#ifndef _KSOCK_H
#include <ksock.h>
#endif
#ifndef _KMESSAGEBOX_H
#include <kmessagebox.h>
#endif
#ifndef _KPROCESS_H
#include <kprocess.h>
#endif
#ifndef _KSTATUSBAR_H
#include <kstatusbar.h>
#endif
#ifndef _KAPP_H
#include <kapp.h>
#endif
#ifndef _KPROGRESS_H
#include <kprogress.h>
#endif
#ifndef _KGLOBAL_H
#include <kglobal.h>
#endif
#ifndef _KSTDDIRS_H
#include <kstddirs.h>
#endif
#ifndef _KSERVICE_H
#include <kservice.h>
#endif
#ifndef _KDEBUG_H
#include <kdebug.h>
#endif

#ifndef _KPILOT_KPILOTCONFIG_H
#include "kpilotConfig.h"
#endif
#ifndef _KPILOT_STATUSMESSAGES_H
#include "statusMessages.h"
#endif
#ifndef _KPILOT_MESSAGEDIALOG_H
#include "messageDialog.h"
#endif

#ifndef _KPILOT_PILOTDATABASE_H
#include "pilotDatabase.h"
#endif
#ifndef _KPILOT_PILOTSERIALDATABASE_H
#include "pilotSerialDatabase.h"
#endif
#ifndef _KPILOT_PILOTLOCALDATABASE_H
#include "pilotLocalDatabase.h"
#endif

#include "kpilotlink.moc"

// This is for the singleton pattern that KPilotLink 
// is supposed to follow.
//
//
KPilotLink* KPilotLink::fKPilotLink = 0L;


void KPilotLink::readConfig()
{
	FUNCTIONSETUP;

	// When KPilot starts up we want to be able to find 
	// the last synced users data.
	//
	//
	KConfig& config = KPilotConfig::getConfig();
	getPilotUser().setUserName(config.readEntry("UserName").latin1());

	fDatabaseDir = KGlobal::dirs()->saveLocation("data", 
		QString("kpilot/DBBackup/")); 

	DEBUGDAEMON << fname
		<< ": Databases saved locally under " 
		<< fDatabaseDir
		<< endl;
}

KPilotLink::KPilotLink()
  : 
	fConduitRunStatus(None),
	fConnected(false), fCurrentPilotSocket(-1), 
	fSlowSyncRequired(false),
	fFastSyncRequired(false),
    fOwningWidget(0L), fStatusBar(0L), fProgressDialog(0L), fConduitSocket(0L),
    fCurrentDB(0L), fNextDBIndex(0), fConduitProcess(0L), fMessageDialog(0L),
	fStatus(Normal)
{
	initTickle();

	fKPilotLink = this;
	readConfig();

	(void) kpilotlink_id;
}


KPilotLink::KPilotLink(QWidget* owner, KStatusBar* statusBar,
		       const QString &devicePath) :
	fConduitRunStatus(None),
	fConnected(false), fCurrentPilotSocket(-1), 
	fSlowSyncRequired(false),
	fFastSyncRequired(false),
    fOwningWidget(owner), fStatusBar(statusBar), fProgressDialog(0L),
    fConduitSocket(0L), fCurrentDB(0L), fNextDBIndex(0), fConduitProcess(0L),
    fMessageDialog(0L),
	fStatus(Normal)
{
	initTickle();

  fKPilotLink = this;
	readConfig();
  initPilotSocket(devicePath);
	if (fStatus!=Normal) return;
  initConduitSocket();
  fMessageDialog = new MessageDialog(i18n("Sync Status"));
}

KPilotLink::~KPilotLink()
{
	if (fTimer)
	{
		delete fTimer;
		fTimer=0;
	}

	if(fMessageDialog)
	{
		delete fMessageDialog;
		fMessageDialog=0L;
	}
	if(fConduitSocket)
	{
		delete fConduitSocket;
		fConduitSocket=0L;
	}
	if(getConnected())
	{
		endHotSync();
	}
}


void
KPilotLink::initPilotSocket(const QString& devicePath, bool)
{
	FUNCTIONSETUP;

	struct pi_sockaddr addr;
	int ret;
	int e=0;
	QString msg;

	pi_close(getCurrentPilotSocket());
	fPilotPath = devicePath;

	if (fPilotPath.isEmpty())
	{
		kdWarning() << __FUNCTION__ 
			<< ": No point in trying empty device."
			<< endl;

		msg=i18n("The Pilot device is not configured yet.");
		e=0;
		goto errInit;
	}
  
	if (!(fPilotMasterSocket = 
		pi_socket(PI_AF_SLP, PI_SOCK_STREAM, PI_PF_PADP))) 
	{
		e=errno;
		msg=i18n("Cannot create socket for communicating "
			"with the Pilot");
		goto errInit;
	}

	addr.pi_family = PI_AF_SLP;
	strcpy(addr.pi_device, QFile::encodeName(fPilotPath));

	ret = pi_bind(fPilotMasterSocket, 
		(struct sockaddr*)&addr, sizeof(addr));
	if (ret>=0)
	{
		return;
	}
	else
	{
		e=errno;
		msg=i18n("Cannot open Pilot port \"%1\". ");
		// goto errInit;
	}


// We arrive here when some action for initializing the sockets
// has gone wrong, and we need to log that and alert the user
// that it has gone wrong.
//
//
errInit:
	fPilotMasterSocket = -1;

	if (msg.contains('%'))
	{
		if (fPilotPath.isEmpty())
		{
			msg=msg.arg(i18n("(empty)"));
		}
		else
		{
			msg=msg.arg(fPilotPath);
		}
	}
	switch(e)
	{
	case ENOENT :
		msg += i18n(" The port does not exist.");
		break;
	case ENODEV :
		msg += i18n(" These is no such device.");
		break;
	case EPERM :
		msg += i18n(" You don't have permission to open the "
			"Pilot device.");
		break;
	default :
		msg+= i18n(" Check Pilot path and permissions.");
	}

	// OK, so we may have to deal with a translated 
	// error message here. Big deal -- we have the line
	// number as well, right?
	//
	//
	kdError() << __FUNCTION__ << ": " << msg << endl;
	if (e)
	{
		kdError() << __FUNCTION__ 
			<< ": (" << strerror(e) << ")" << endl;
	}

	KMessageBox::error(fOwningWidget, msg,
		i18n("Error Initializing Daemon"));

	fStatus = PilotLinkError;
}

void
KPilotLink::initConduitSocket()
{
  fConduitSocket = new KServerSocket(KPILOTLINK_PORT);
  connect(fConduitSocket, SIGNAL(accepted(KSocket*)),
	  this, SLOT(slotConduitConnected(KSocket*)));
  fConduitProcess = new KProcess();
}

void KPilotLink::addSyncLogEntry(const char *entry)
{
	dlp_AddSyncLogEntry(getCurrentPilotSocket(), (char *)entry);
	emit logEntry(entry);
}



PilotRecord*
KPilotLink::readRecord(KSocket* theSocket)
{
  int len, attrib, cat;
  recordid_t uid;
  char* data;
  PilotRecord* newRecord;

  read(theSocket->socket(), &len, sizeof(int)); // REC_DATA tag
  read(theSocket->socket(), &len, sizeof(int));
  read(theSocket->socket(), &attrib, sizeof(int));
  read(theSocket->socket(), &cat, sizeof(int));
  read(theSocket->socket(), &uid, sizeof(recordid_t));
  data = new char[len];
  read(theSocket->socket(), data, len);
  newRecord = new PilotRecord((void*)data, len, attrib, cat, uid);
  delete [] data;
  return newRecord;
}

void
KPilotLink::writeRecord(KSocket* theSocket, PilotRecord* rec)
{
  int len = rec->getLen();
  int attrib = rec->getAttrib();
  int cat = rec->getCat();
  recordid_t uid = rec->getID();
  char* data = rec->getData();
  
  CStatusMessages::write(theSocket->socket(), CStatusMessages::REC_DATA);
  write(theSocket->socket(), &len, sizeof(int));
  write(theSocket->socket(), &attrib, sizeof(int));
  write(theSocket->socket(), &cat, sizeof(int));
  write(theSocket->socket(), &uid, sizeof(recordid_t));
  write(theSocket->socket(), data, len);
}

int KPilotLink::writeResponse(KSocket *k,CStatusMessages::LinkMessages m)
{
	// I have a bad feeling about using pointers
	// to parameters passed to me, so I'm going 
	// to copy the value parameter to a local (stack)
	// variable and use a pointer to that variable.
	//
	//
	int i=(int) m;

	return write(k->socket(), &i, sizeof(int));
}

void KPilotLink::slotConduitDone(KProcess *p)
{
	FUNCTIONSETUP;

	if (!p || !fConduitProcess)
	{
		kdWarning() << __FUNCTION__
			<< ": Called without a running conduit and process!"
			<< endl;
		return;
	}

	if (p != fConduitProcess)
	{
		kdWarning() << __FUNCTION__ 
			<< ": Process with id "
			<< p->pid()
			<< " exited while waiting on "
			<< fConduitProcess->pid()
			<< endl;
	}
	else
	{
#ifdef DEBUG
		kdDebug() << fname
			<< ": Conduit process with pid "
			<< p->pid()
			<< " exited"
			<< endl;
#endif
	}

	if (fConduitRunStatus != Done)
	{
		kdWarning() << __FUNCTION__
			<< ": It seems that a conduit has crashed."
			<< endl;

		// Force a resume even though the conduit never ran.
		//
		//
		resumeDB();
	}
}

void
KPilotLink::slotConduitRead(KSocket* cSocket)
{
	FUNCTIONSETUP;
  int message;
  PilotRecord* tmpRec = 0L;

  read(cSocket->socket(), &message, sizeof(int));
  //kdDebug() << fname << " read message " << message << endl;

	// This one message doesn't require a database to be open.
	//
	//
	if (message == CStatusMessages::LOG_MESSAGE)
	{
		int i;
		char *s;
		read(cSocket->socket(),&i,sizeof(int));
		s=new char[i+1];
		memset(s,0,i);
		read(cSocket->socket(),s,i);
		s[i]=0;
#ifdef DEBUG
		if (debug_level & SYNC_TEDIOUS)
		{
			kdDebug() << fname << ": Message length "
				<< i << " => "
				<< s
				<< endl;
		}
#endif
		addSyncLogEntry(s);
		delete s;

		return;
	}

	// The remaining messages all require a databse.
	//
	//
	if (!fCurrentDB)
	{
		kdWarning() << __FUNCTION__
			<< ": There is no open database."
			<< endl;

		writeResponse(cSocket,CStatusMessages::NO_SUCH_RECORD);
		return;
	}

	switch(message)
	    {
	    case CStatusMessages::READ_APP_INFO :
	        {
		unsigned char *buf = new unsigned char[BUFSIZ];
		int appLen = fCurrentDB->readAppBlock(buf,BUFSIZ);
		write(cSocket->socket(),&appLen,sizeof(int));
		write(cSocket->socket(),buf,appLen);
		delete buf;
		break;
		}
	case CStatusMessages::WRITE_RECORD :
		{
		tmpRec = readRecord(cSocket);
		fCurrentDB->writeRecord(tmpRec);
		CStatusMessages::write(cSocket->socket(), 
			CStatusMessages::NEW_RECORD_ID);
		recordid_t tmpID = tmpRec->getID();
		write(cSocket->socket(), &tmpID, sizeof(recordid_t));
		delete tmpRec;
		}
		break;
	case CStatusMessages::NEXT_MODIFIED_REC :
		{
		tmpRec = fCurrentDB->readNextModifiedRec();
		if(tmpRec)
		    {
		    writeRecord(cSocket, tmpRec);
		    delete tmpRec;
		    }
		else
		    {
		    CStatusMessages::write(cSocket->socket(), 
					   CStatusMessages::NO_SUCH_RECORD);
		    }
		}
		break;
	case CStatusMessages::NEXT_REC_IN_CAT :
		{
		int cat;
		read(cSocket->socket(), &cat, sizeof(int));
		tmpRec = fCurrentDB->readNextRecInCategory(cat);
		if(tmpRec)
		    {
		    writeRecord(cSocket, tmpRec);
		    delete tmpRec;
		    }
		else
		    CStatusMessages::write(cSocket->socket(), 
					   CStatusMessages::NO_SUCH_RECORD);
		}
		break;
	case CStatusMessages::READ_REC_BY_INDEX :
		{
		int index;
		read(cSocket->socket(), &index, sizeof(int));
		//kdDebug() << fname << " about to read record by index "
		//<< index << endl;
		tmpRec = fCurrentDB->readRecordByIndex(index);
		if(tmpRec)
		    {
		    //kdDebug() << fname << " record found!!! id = " <<
		    //tmpRec->getID() << endl;
		    writeRecord(cSocket, tmpRec);
		    delete tmpRec;
		    }
		else
		    {
		    //kdDebug() << fname << " no record found!!!" << endl; 

		    CStatusMessages::write(cSocket->socket(), 
					   CStatusMessages::NO_SUCH_RECORD);
		    }
		}
		break;
	case CStatusMessages::READ_REC_BY_ID :
		{
		recordid_t id;
		read(cSocket->socket(), &id, sizeof(recordid_t));
		tmpRec = fCurrentDB->readRecordById(id);
		if(tmpRec)
		    {
		    writeRecord(cSocket, tmpRec);
		    delete tmpRec;
		    }
		else
		    CStatusMessages::write(cSocket->socket(), 
					   CStatusMessages::NO_SUCH_RECORD);
		}
		break;
	default :
	    kdWarning() << __FUNCTION__ << ": Unknown status message " 
			<< message
			<< endl;
	    }
}
	
void
KPilotLink::slotConduitClosed(KSocket* theSocket)
{
	FUNCTIONSETUP;

	if (fConduitRunStatus != Connected)
	{
		kdWarning() << __FUNCTION__
			<< ": Strange -- unconnected conduit closed"
			<< endl;
	}

  disconnect(theSocket, SIGNAL(readEvent(KSocket*)),
	     this, SLOT(slotConduitRead(KSocket*)));
  disconnect(theSocket, SIGNAL(closeEvent(KSocket*)),
	     this, SLOT(slotConduitClosed(KSocket*)));
  delete theSocket;

	fConduitRunStatus = Done ;
	resumeDB();
}

void KPilotLink::resumeDB()
{
	if (!fCurrentDB)
	{
		kdWarning() << __FUNCTION__
			<< ": resumeDB called after the end of a sync."
			<< endl;
		return;
	}

	if (fCurrentDB) delete fCurrentDB;
	fCurrentDB=0L;

  // Get our backup copy.
  if(slowSyncRequired()) // We are in the middle of backing up, continue
    {
      doConduitBackup();
    }
  else // We are just doing a normal sync, so go for it.
    {
      syncDatabase(&fCurrentDBInfo);
      // Start up the next one
      syncNextDB();
    }
}

QString
KPilotLink::registeredConduit(const QString &dbName) const
{
	FUNCTIONSETUP;

	KConfig& config = KPilotConfig::getConfig("Database Names");

#ifdef DEBUG
	if (debug_level & SYNC_MINOR)
	{
		kdDebug() << fname << ": Looking for "
			<< dbName << endl;
	}
#endif

	QString result = config.readEntry(dbName);
	if (result.isNull())
	{
#ifdef DEBUG
		if (debug_level & SYNC_MINOR)
		{
			kdDebug() << fname << ": Not found." << endl;
		}
#endif

		return result;
	}

	config.setGroup("Conduit Names");
	QStringList installed = config.readListEntry("InstalledConduits");

#ifdef DEBUG
	if (debug_level & SYNC_TEDIOUS)
	{
#ifndef NDEBUG
		kdbgstream s = kdDebug();
#endif
		kdDebug() << fname << ": Found conduit "
			<< result << endl
			<< fname << ": Installed Conduits are"
			<< endl;

#ifndef NDEBUG
		listStrList(s,installed);
#endif
	}
#endif

	if (!installed.contains(result))
	{
#ifdef DEBUG
		if (debug_level & SYNC_MINOR)
		{
			kdDebug() << fname << ": Conduit not installed."
				<< endl;
		}
#endif
		return QString::null;
	}

	KService::Ptr conduit = KService::serviceByDesktopName(result);
	if (!conduit)
	{
#ifdef DEBUG
		if (debug_level & SYNC_MINOR)
		{
			kdDebug() << fname << ": No service for this conduit"
				<< endl;
		}
#endif
		return QString::null;
	}
	else
	{
#ifdef DEBUG
		if (debug_level & SYNC_MINOR)
		{
			kdDebug() << fname << ": Found service with exec="
				<< conduit->exec()
				<< endl;
		}
#endif
		return conduit->exec();
	}
}


void
KPilotLink::slotConduitConnected(KSocket* theSocket)
{
	FUNCTIONSETUP;

  connect(theSocket, SIGNAL(readEvent(KSocket*)),
	  this, SLOT(slotConduitRead(KSocket*)));
  connect(theSocket, SIGNAL(closeEvent(KSocket*)),
	  this, SLOT(slotConduitClosed(KSocket*)));
  theSocket->enableRead(true);

	fConduitRunStatus = Connected ;
}

// Requires the text is displayed in item 0
void KPilotLink::showMessage(const QString &message) const
{
  if(fStatusBar)
    {
      fStatusBar->changeItem(message,0);
    }
}

int 
KPilotLink::compare(struct db * d1, struct db * d2)
{
  /* types of 'appl' sort later than other types */
  if(d1->creator == d2->creator)
    if(d1->type != d2->type) 
      {
	if(d1->type == pi_mktag('a','p','p','l'))
	  return 1;
	if(d2->type == pi_mktag('a','p','p','l'))
	  return -1;
      }
  return d1->maxblock < d2->maxblock;
}

bool
KPilotLink::doFullRestore()
{
	FUNCTIONSETUP;

  struct DBInfo info;
  struct db * db[256];
  int dbcount = 0;
  int i,j,max,size;
  struct pi_file * f;
  char message[256];

	i=KMessageBox::questionYesNo(
		fOwningWidget,
		i18n("Replace all data on Pilot with local data?"),
		i18n("Full Restore"));
	if (i != KMessageBox::Yes) return false;

	// User said yes, so do it.
	//
	//
	QString dirname = KGlobal::dirs()->saveLocation("data", 
		QString("kpilot/DBBackup/") + 
		getPilotUser().getUserName() + "/");
	QStringList dbList;

	// Block for local vars.
	//
	{
		QDir dir(dirname);
		if (! dir.exists())
		{
			kdWarning() << __FUNCTION__
				<< ": Save directory "
				<< dirname
				<< " doesn't exist."
				<< endl;
			return false;
		}
		dbList = dir.entryList();
	}


	QStringList::ConstIterator it;

	for (it=dbList.begin(); it!=dbList.end(); ++it)
	{
		if ((*it == "..") || (*it == ".")) continue;

		DEBUGKPILOT << fname
			<< ": Trying database "
			<< *it
			<< endl;

		db[dbcount] = (struct db*)malloc(sizeof(struct db));
		sprintf(db[dbcount]->name,
			QFile::encodeName(dirname + *it));
	
		f = pi_file_open(db[dbcount]->name);
		if (f==0) 
		{
			kdWarning() << __FUNCTION__
				<< ": Unable to open "
				<< *it
				<< endl;
			continue;
		}
  	
		pi_file_get_info(f, &info);
  	
		db[dbcount]->creator = info.creator;
		db[dbcount]->type = info.type;
		db[dbcount]->flags = info.flags;
		db[dbcount]->maxblock = 0;

		pi_file_get_entries(f, &max);

		for (int dbi=0; dbi<max; dbi++) 
		{
			if (info.flags & dlpDBFlagResource)
			{
				pi_file_read_resource(f, dbi, 0, &size, 0, 0);
			}
			else
			{
				pi_file_read_record(f, dbi, 0, &size, 0, 0,0 );
			}

			if (size > db[dbcount]->maxblock)
			{
				db[dbcount]->maxblock = size;
			}
		}

		pi_file_close(f);
		dbcount++;
	}
    
  // Sort databases
  //
  //
  for (i=0;i<dbcount;i++)
    for (j=i+1;j<dbcount;j++)
      if (compare(db[i],db[j])>0) {
	struct db * temp = db[i];
	db[i] = db[j];
	db[j] = temp;
      }
    
  fMessageDialog->setMessage("Starting Sync.");
  fMessageDialog->show();
  for (i=0;i<dbcount;i++) 
    {
	
      if (dlp_OpenConduit(getCurrentPilotSocket()) < 0) 
	{
	  kdWarning() << __FUNCTION__ << ": Exiting on cancel. "
	    "All data not restored."
	       << endl;
	  return false;
	}
      showMessage(i18n("Restoring databases to Palm Pilot. "
		       "Slow sync required."));
      addSyncLogEntry("Restoring all data...");

      f = pi_file_open(db[i]->name);
      if (f==0) 
	{
	  printf("Unable to open '%s'!\n", db[i]->name);
	  break;
	}
      strcpy(message, "Syncing: ");
      strcat(message, strrchr(db[i]->name, '/') + 1);
      fMessageDialog->setMessage(message);

      //  	printf("Restoring %s... ", db[i]->name);
      //  	fflush(stdout);
      if(pi_file_install(f, getCurrentPilotSocket(), 0)<0)
	printf("Hmm..prefs file..\n");
      //   	else
      // 	    printf("OK\n");
      pi_file_close(f);
    }
  //     printf("Restore done\n");
  addSyncLogEntry("OK.\n");
  fMessageDialog->hide();
  //     delete messageDialog;
	finishDatabaseSync();
	return true;
}

void KPilotLink::finishDatabaseSync()
{
	// Since we're done with the DB anyway
	// we can forget it. This also flags 
	// for resumeDB not to do anything
	// stupid :)
	//
	//
	fCurrentDB=0;
	emit(databaseSyncComplete());
}

bool
KPilotLink::createLocalDatabase(DBInfo* info)
{
  FUNCTIONSETUP;

  char temp[256];
  char name[256];
  int j;
  struct pi_file* f;

  QString fullBackupDir = KGlobal::dirs()->saveLocation("data", QString("kpilot/DBBackup/")
							+ getPilotUser().getUserName() + "/");
  strcpy(temp, info->name);
  j = -1;
  // Fix the filename, incase there is a forward slash in it.
  while(temp[++j])
    {
      if(temp[j] == '/') temp[j] = '_';
    }

  sprintf(name, "%s/%s", fullBackupDir.latin1(), info->name);
  if (info->flags & dlpDBFlagResource)
    {
      strcat(name,".prc");
    }
  else
    {
      strcat(name,".pdb");
    }

#ifdef DEBUG
  if (debug_level & DB_TEDIOUS)
    {
      kdDebug() << fname << ": Creating local database "
	   << name << endl;
    }
#endif
  /* Ensure that DB-open flag is not kept */
  info->flags &= 0xff;

  f = pi_file_create(name, info);
  if (f==0) 
    {
      kdWarning() << __FUNCTION__ << ": Failed, unable to create file"
	   << endl;
      return false;
    }

  if(pi_file_retrieve(f, getCurrentPilotSocket(), 0)<0)
    {
      kdWarning() << __FUNCTION__ << ": Failed, unable to back up database"
	   << endl;
      pi_file_close(f);
      return false;
    }
  pi_file_close(f);
  return true;
}


void
KPilotLink::doFullBackup()
{
	FUNCTIONSETUP;
	int i = 0;

	setSlowSyncRequired(true);

	fMessageDialog->setMessage(i18n("Starting HotSync."));
	fMessageDialog->show();
	showMessage(i18n("Backing up Pilot..."));
	addSyncLogEntry(i18n("Backing up all data...").latin1());

	for(;;)
	{
		struct DBInfo info;

		if (dlp_OpenConduit(getCurrentPilotSocket())<0) 
		{
			// The text suggests that the user
			// has chosen to cancel the backup,
			// which is why I've made this a warning,
			// but why is this warning based on the return
			// value of OpenConduit??
			//
			//
			kdWarning() << __FUNCTION__ 
				<< ": dlp_OpenConduit failed -- means cancel?"
				<< endl;

			KMessageBox::sorry(fOwningWidget,
				i18n("Exiting on cancel.\n"
				     "<B>Not</B> all the data was backed up."),
				i18n("Backup"));
			addSyncLogEntry("FAILED.\n");
			return;
		}

		// Is parameter i important here???
		//
		//
		if( dlp_ReadDBList(getCurrentPilotSocket(), 
			0, 0x80, i, &info) < 0)
		{
			DEBUGKPILOT << fname
				<< ": Last database encountered."
				<< endl;
			break;
		}
		i = info.index + 1;

		{
		QString logmsg(i18n("Backing Up: "));
		logmsg.append(info.name);
		fMessageDialog->setMessage(logmsg);
		}

		if(createLocalDatabase(&info) == false)
		{
			kdError() << __FUNCTION__
				<< ": Couldn't create local database for "
				<< info.name
				<< endl;

			KMessageBox::error(fOwningWidget,
				i18n("Could not backup data!"),
				i18n("Backup failed"));
		}
	}
	addSyncLogEntry(i18n("OK.\n").local8Bit());
	// Set up so the conduits can run through the DB's and backup.  
	// doConduitBackup() will emit the databaseSyncComplete when done.
	//
	//
	fNextDBIndex = 0;
	doConduitBackup();
	return;
}

void 
KPilotLink::installFiles(const QString &path)
{
	FUNCTIONSETUP;

  struct pi_file * f;
  int fileNum = 0;
  QDir installDir(path);
  QString actualPath=installDir.path() + "/";

  QStringList fileNameList = installDir.entryList(QDir::Files);
  QValueListIterator<QString> fileIter(fileNameList.begin());
  if (fileIter == fileNameList.end()) 
    return;

  createNewProgressBar(i18n("Installing Files"), 
		       i18n("Percentage of files installed:"), 
		       0, fileNameList.count(), 0);
  showMessage(i18n("Installing files..."));

  if(getConnected() == false)
    {
      kdWarning() << __FUNCTION__ << ": No HotSync started!" << endl;
      return;
    }

  updateProgressBar(0);
#ifdef DEBUG
  if (debug_level & SYNC_MINOR)
    {
      kdDebug() << fname << ": Installing from directory "
	   << actualPath << endl;
    }
#endif

  while(fileIter != fileNameList.end())
    {
#ifdef DEBUG
      if (debug_level & SYNC_MAJOR)
	{
	  kdDebug() << fname << ": Installing file "
	       << *fileIter << endl;
	}
#endif

      updateProgressBar(fileNum++);

      // Block to isolate extra QString
      //
      //
      {
	QString fullPath(actualPath);
	fullPath+= *fileIter;

	// Yuckyness to avoid warning when
	// passing QString to pi library.
	//
	//
	f = pi_file_open((char *)fullPath.latin1());
      }

      if (f==0) 
	{
	  kdWarning() << __FUNCTION__ << ": Unable to open file." << endl;

	  QString message;

	  message=i18n("Unable to open file &quot;%1&quot;!")
	    .arg(*fileIter);
	  KMessageBox::error(fOwningWidget,
			     message,
			     i18n("Missing File"));
	}
      else
	{
	  if(pi_file_install(f, getCurrentPilotSocket(), 0) <0)
	    {
	      kdWarning() << __FUNCTION__ << ": failed to install." << endl;
	      KMessageBox::error(fOwningWidget,
				 i18n("Cannot install file on Pilot"),
				 i18n("Install File Error"));
	    }
	  else
	    {
	      unlink((actualPath + (*(fileIter))).latin1());
	    }
	  pi_file_close(f);
	}
      ++fileIter;
    }
  showMessage("File install complete.");
  addSyncLogEntry("File install complete.\n");
  destroyProgressBar();
}
  

void KPilotLink::syncFlags()
{
  if(getConnected() == false) 
    {
      kdDebug() << "KPilotLink::syncFlags() No HotSync started!" << endl;
      return;
    }

  getPilotUser().setLastSyncPC((unsigned long) gethostid());
  getPilotUser().setLastSyncDate(time(0));
}
  
void 
KPilotLink::createNewProgressBar(const QString &title, const QString &text, int min, int max, int value)
{
  if(fProgressDialog)
    delete fProgressDialog;
	
  fProgressDialog = new QDialog(0L);
  fProgressDialog->setCaption(title);
  QLabel* label = new QLabel(fProgressDialog);
  label->setAutoResize(true);
  label->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  label->setAlignment(AlignBottom | AlignHCenter | WordBreak);
  label->setText(text);
  label->setFixedWidth(200);
  label->move(10, 0);
  fProgressMeter = new KProgress(min, max, value, KProgress::Horizontal, fProgressDialog);
  fProgressMeter->setGeometry(10,label->height() + 10,200,20);
  fProgressDialog->setFixedWidth(220);
  fProgressDialog->setFixedHeight(label->height() + 40);
  fProgressDialog->show();
  kapp->processEvents();
}

void
KPilotLink::updateProgressBar(int newValue) const
{
  if(fProgressMeter)
    {
      fProgressMeter->setValue(newValue);
      kapp->processEvents();
    }
}

void
KPilotLink::destroyProgressBar()
{
  if(fProgressDialog)
    {
      delete fProgressDialog;
      fProgressMeter = 0L;
      fProgressDialog = 0L;
    }
  kapp->processEvents();
}

void 
KPilotLink::startHotSync()
{
  int ret;

  if (getConnected() || (getPilotMasterSocket() == -1))
    {
	kdError() << __FUNCTION__
		<<": Already connected or unable to connect!" 
		<< endl;
      return;
    }

  createNewProgressBar(i18n("Waiting to Sync"), 
		       i18n("Reading user information..."), 
		       0, 10, 0);
  updateProgressBar(0);

    
  ret = pi_listen(getPilotMasterSocket(),1);
  if(ret == -1) 
    {
	kdError() << __FUNCTION__ 
		<< ": pi_listen failed: "
		<< perror
		<< endl;
	return;
    }

  fCurrentPilotSocket = pi_accept(getPilotMasterSocket(),0,0);
  if(fCurrentPilotSocket == -1) 
    {
	kdError() << __FUNCTION__
		<< ": pi_accept failed: "
		<< perror
		<< endl;
	return;
    }
  setConnected(true);
  updateProgressBar(3);
  /* Ask the pilot who it is.  And see if it's who we think it is. */
  dlp_ReadUserInfo(getCurrentPilotSocket(), &fPilotUser);
  getPilotUser().boundsCheck();
  checkPilotUser();

  updateProgressBar(7);
	if(fPilotUser.getLastSyncPC() != (unsigned long)gethostid())
	{
		KConfig& c = KPilotConfig::getConfig();
		if (c.readNumEntry("SyncLastPC",1))
		{
			setSlowSyncRequired(true);
		}
	}

  /* Tell user (via Pilot) that we are starting things up */
  if (dlp_OpenConduit(getCurrentPilotSocket()) < 0) 
    {
      showMessage("Exiting on cancel. All data not restored.");
      return;
    }
  updateProgressBar(10);
  destroyProgressBar();
  showMessage("Hot-Sync started.");
  addSyncLogEntry("Sync started with KPilot-v" KPILOT_VERSION "\n");
  fNextDBIndex = 0;
  fCurrentDB = 0L;
}

void
KPilotLink::quickHotSync()
{
  FUNCTIONSETUP;

  fMessageDialog->setMessage("Beginning Sync"); 
  fMessageDialog->show(); 
  syncNextDB();
}

void
KPilotLink::doConduitBackup()
{
	FUNCTIONSETUP;
	QString displaymessage;

  struct DBInfo info;
  do
    {
      if(dlp_ReadDBList(getCurrentPilotSocket(), 0, 0x80, fNextDBIndex, &info) < 0)
	{
	  setSlowSyncRequired(false);
		finishDatabaseSync();
	  fMessageDialog->hide();
	  return;
	}
      fNextDBIndex = info.index + 1;
    }
  while(info.flags & dlpDBFlagResource);
  
  QString conduitName = registeredConduit(info.name);
  while(conduitName.isNull())
    {
      do
	{
	  if(dlp_ReadDBList(getCurrentPilotSocket(), 0, 0x80, fNextDBIndex, &info) < 0)
	    {
	      setSlowSyncRequired(false);
		finishDatabaseSync();
	      fMessageDialog->hide();
	      return;
	    }
	  fNextDBIndex = info.index + 1;
	} while(info.flags & dlpDBFlagResource);
      conduitName = registeredConduit(info.name);
    }

  // Fire up the conduit responsible for this db and when it's finished
  // we'll get called again.
  displaymessage=i18n("%1: Running conduit").arg(info.name);
  fMessageDialog->setMessage(displaymessage);
  fCurrentDBInfo = info;
	fCurrentDB = new PilotSerialDatabase(this,info.name);
  fCurrentDB->resetDBIndex();
  if(fConduitProcess->isRunning())
    {
      kdWarning() << __FUNCTION__ << ": Waiting for conduit to die.. " << endl;
    }
  // Eek! Busy waiting w/no event loop?
  // Well, some kind of event loop now,
  // but it's still kinda dodgy.
  //
  //
  while(fConduitProcess->isRunning())
    {
      sleep(1);
      kapp->processEvents();
    }

  fConduitProcess->clearArguments();
  *fConduitProcess << conduitName;
  *fConduitProcess << "--backup";
#ifdef DEBUG
  if (debug_level)
    {
      *fConduitProcess << "--debug";
      *fConduitProcess << QString::number(debug_level);
    }
#endif
	connect(fConduitProcess,SIGNAL(processExited(KProcess *)),
		this,SLOT(slotConduitDone(KProcess *)));
	fConduitRunStatus = Running ;
	fConduitProcess->start(KProcess::NotifyOnExit);
}

int KPilotLink::findNextDB(DBInfo *info)
{
  FUNCTIONSETUP;

  do
    {
      if(dlp_ReadDBList(getCurrentPilotSocket(), 0, 0x80, 
			fNextDBIndex, info) < 0)
	{
		setSlowSyncRequired(false);
		KConfig& config = KPilotConfig::getConfig();
		setFastSyncRequired(
			config.readBoolEntry("PreferFastSync",false));
	  fMessageDialog->hide();
		finishDatabaseSync();
	  return 0;
	}
      fNextDBIndex = info->index + 1;
    }
  while(info->flags & dlpDBFlagResource);

#ifdef DEBUG
  if (debug_level & SYNC_TEDIOUS)
    {
      kdDebug() << fname << ": Found database with:\n"
	   << fname << ": Index=" << fNextDBIndex
	   << endl;
    }
#endif

  return 1;
}

int KPilotLink::findDisposition(const QString &dbList,
				const struct DBInfo *currentDB)
{
  FUNCTIONSETUP;
  char *m=printlong(currentDB->creator);
  int r=dbList.find(m);

  if (r==0 || (r>0 && dbList[r-1]==',')) return 1;
  return 0;
}

void
KPilotLink::syncNextDB()
{
	FUNCTIONSETUP;

	QString message;
  QString skip;
  QString backupOnly;
  DBInfo info;

  // Confine config reads to a local block
  {
    KConfig& c = KPilotConfig::getConfig();
    skip=c.readEntry("SkipSync");
    backupOnly=c.readEntry("BackupForSync");
  }

#ifdef DEBUG
  if (debug_level & SYNC_TEDIOUS)
    {
      kdDebug() << fname << ": Special dispositions are: \n"
	   << fname << ": * BackupOnly=" << backupOnly << endl
	   << fname << ": * Skip=" << skip << endl ;
    }
#endif
  if (!findNextDB(&info)) return;

#ifdef DEBUG
  if (debug_level & SYNC_MAJOR)
    {
      kdDebug() << fname << ": Syncing " << info.name << endl;
    }
#endif



  QString conduitName = registeredConduit(info.name);
  while(conduitName.isNull())
    {
#ifdef DEBUG
      if (debug_level & SYNC_MINOR)
	{
	  kdDebug() << fname << ": No registered conduit for " 
	       << info.name << endl;
	}
#endif

	// If we want a FastSync, skip all DBs without conduits.
	//
	if (fFastSyncRequired)
	{
		DEBUGKPILOT << fname
			<< ": Skipping database "
			<< info.name
			<< " during FastSync."
			<< endl;
		goto nextDB;
	}

      message=i18n("Syncing: %1 ...").arg(info.name);
      fMessageDialog->setMessage(message);
      addSyncLogEntry(message.local8Bit());

      // Find out if this database has a special disposition
      //
      //
#ifdef DEBUG
      if (debug_level & SYNC_MINOR)
	{
	  char *m=printlong(info.creator);
	  kdDebug() << fname << ": Looking for disposition of "
	       << m
	       << endl;
	}
#endif
      if (findDisposition(skip,&info)) goto nextDB;
      if (findDisposition(backupOnly,&info) && !fFastSyncRequired) 
	{
	  if (!createLocalDatabase(&info))
	    {
	      QString message(i18n("Could not backup data "
				   "for database &quot;%1&quot;")
			      .arg(info.name));
	      KMessageBox::error(fOwningWidget,
				 message,
				 i18n("Backup for Sync"));
	    }
	  goto nextDB;
	}

      if(syncDatabase(&info))
	{
#ifdef DEBUG
	  if (debug_level & SYNC_TEDIOUS)
	    {
	      kdDebug() << fname << ": Sync OK" << endl;
	    }
#endif
	  addSyncLogEntry("OK.\n");
	}
      else
	{
	  kdWarning() << __FUNCTION__ << ": Sync " 
	       << info.name << " failed."
	       << endl;

	  addSyncLogEntry("FAILED!\n");
	}

    nextDB:
      if (!findNextDB(&info)) return;

      conduitName = registeredConduit(info.name);
#ifdef DEBUG
      if (debug_level & SYNC_MAJOR)
	{
	  kdDebug() << fname << ": Syncing " << info.name << endl;
	}
#endif
    }

  // Fire up the conduit responsible for this db and when it's finished
  // we'll get called again.
  message=i18n("%1: Running conduit").arg(info.name);
  fMessageDialog->setMessage(message);
  addSyncLogEntry(message.local8Bit());
  fCurrentDBInfo = info;

#ifdef DEBUG
  if (debug_level & SYNC_MAJOR)
    {
      kdDebug() << fname << ": " 
	   << message << endl;
    }
#endif


	fCurrentDB = new PilotSerialDatabase(this,info.name);
  fCurrentDB->resetDBIndex();
  if(fConduitProcess->isRunning())
    {
      kdWarning() << __FUNCTION__ << ": Waiting for conduit to die.. " << endl;
    }

  // This is busy waiting, but make sure that
  // (Qt) signals do get delivered and the
  // display is maintained.
  //
  while(fConduitProcess->isRunning())
    {
      sleep(1);
      kapp->processEvents();
    }

	fConduitProcess->clearArguments();
	*fConduitProcess << conduitName;
	*fConduitProcess << "--hotsync" ;
#ifdef DEBUG
	if (debug_level)
	{
		*fConduitProcess << "--debug" ;
		*fConduitProcess << QString::number(debug_level);
	}
#endif
	connect(fConduitProcess,SIGNAL(processExited(KProcess *)),
		this,SLOT(slotConduitDone(KProcess *)));
	fConduitRunStatus = Running ;
	fConduitProcess->start(KProcess::NotifyOnExit);

}

bool 
KPilotLink::syncDatabase(DBInfo* database)
{
	unsigned char buffer[0xffff];

	PilotDatabase* firstDB;
	PilotDatabase* secondDB;
	PilotRecord* pilotRec;

	QString currentDBPath = fDatabaseDir + 
		getPilotUser().getUserName() + "/";

	firstDB = new PilotSerialDatabase(this,database->name);
	secondDB = new PilotLocalDatabase(currentDBPath,database->name);

	if(firstDB->isDBOpen() && (secondDB->isDBOpen() == false))
	{
		// Must be a new Database...
		showMessage(i18n("No previous copy.  Copying data from pilot..."));

		delete firstDB;
		delete secondDB;
		firstDB = secondDB = 0L;

		if(createLocalDatabase(database) == false)
		{
			KMessageBox::error(fOwningWidget,
				i18n("Could not create local copy of database "
				"&quot;%1&quot;").arg(database->name),
				i18n("Backup"));

			// Why continue here? The database isn't open, so
			// we'll just get another error message shortly.
			//
			//
			return false;
		}

		firstDB = new PilotSerialDatabase(this,database->name);
		secondDB = new PilotLocalDatabase(currentDBPath,database->name);

		showMessage(i18n("Hot-Syncing Pilot. Looking for modified data..."));
	}

	if((secondDB->isDBOpen() == false) || (firstDB->isDBOpen() == false))
	{
		delete firstDB;
		delete secondDB;
		firstDB = secondDB = 0L;

		QString message(i18n("Cannot find database &quot;%1&quot;")
			.arg(database->name));
		KMessageBox::error(fOwningWidget,
			message,
			i18n("Error Syncing Database"));
		return false;
	}


	// Move this functionality into mode ...
	//
	//
	KConfig& config = KPilotConfig::getConfig();
	config.setGroup(QString());
	// If local changes should modify pilot changes, switch the order.
	int localOverride = config.readNumEntry("OverwriteRemote");

	if(localOverride)
	{
		PilotDatabase* tmp = firstDB;
		firstDB = secondDB;
		secondDB = tmp;
	}

	int len = firstDB->readAppBlock(buffer, 0xffff);
	if(len > 0)
	{
		secondDB->writeAppBlock(buffer, len);
	}

	firstDB->resetDBIndex();
	while((pilotRec = firstDB->readNextModifiedRec()) != 0L)
	{
		secondDB->writeRecord(pilotRec);
		firstDB->writeID(pilotRec);
		delete pilotRec;
	}

	secondDB->resetDBIndex();
	while((pilotRec = secondDB->readNextModifiedRec()) != 0L)
	{
		firstDB->writeRecord(pilotRec);
		secondDB->writeID(pilotRec);
		delete pilotRec;
	}

	firstDB->resetSyncFlags();
	firstDB->cleanUpDatabase(); // Purge deleted entries
	secondDB->resetSyncFlags();
	secondDB->cleanUpDatabase();
	delete firstDB;
	delete secondDB;
	firstDB = secondDB = 0L;
	return true;
}

void KPilotLink::endHotSync()
{
  if(getConnected() == false)
    return;
  
  syncFlags();
  dlp_WriteUserInfo(getCurrentPilotSocket(), &getPilotUser());
  addSyncLogEntry("End of Hot-Sync\n");
  dlp_EndOfSync(getCurrentPilotSocket(), 0);
  pi_close(getCurrentPilotSocket());
  fCurrentPilotSocket = -1;
  setConnected(false);
  showMessage("Hot-Sync completed");
	fConduitRunStatus = None;
}

void KPilotLink::checkPilotUser()
{
	FUNCTIONSETUP;

  KConfig& config = KPilotConfig::getConfig();
  if (config.readBoolEntry("AlwaysTrustPilotUser"))
    {
      return;
    }

  QString guiUserName;
  guiUserName = config.readEntry("UserName");
  
  if (guiUserName != getPilotUser().getUserName())
    {
      QString imessage(i18n(
			   "The Palm Pilot thinks the user name is %1, "
			   "however KPilot says you are %2.\n"
			   "Should I assume the Pilot is right and set the "
			   "user name for KPilot to %1? "
			   "(Otherwise I'll use %2 for now)"));
	QString message = imessage.arg(getPilotUser().getUserName())
		.arg(getPilotUser().getUserName())
		.arg(guiUserName)
		.arg(guiUserName);

      if (KMessageBox::warningYesNo(0L,
				    message,
				    i18n("Pilot User Changed"))==KMessageBox::Yes)
	{
	  config.writeEntry("UserName", getPilotUser().getUserName());
	}
      else
	{
	  // The gui was right.
	  getPilotUser().setUserName(guiUserName.latin1());

	  kdWarning() << __FUNCTION__ 
	  	<< ": Pilot User set to " 
		<< getPilotUser().getUserName() << endl;
	}
    }
}


/* Tickle support */

void KPilotLink::initTickle()
{
	fTimer=0L;
	fTickleCount=0;
	setTickleTimeout();
}

void KPilotLink::startTickle()
{
	fTickleCount=0;
	if (!fTimer)
	{
		fTimer=new QTimer(this);
	}

	if (fTimer->isActive())
	{
		fTimer->stop();
	}

	QObject::connect(fTimer,SIGNAL(timeout()),this,SLOT(tickle()));
	fTimer->start(1000,false);
}

void KPilotLink::stopTickle()
{
	if (fTimer)
	{
		fTimer->stop();
	}
}

void KPilotLink::tickle()
{
	fTickleCount++;

	// Note that if fTickleTimeout == 0 then this
	// test will never be true until unsigned wraps
	// around, which is 2^32 seconds, which is a long time.
	//
	// This is intentional.
	//
	//
	if (fTickleCount==fTickleTimeout)
	{
		emit timeout();
	}
	else
	{
		if (pi_tickle(getCurrentPilotSocket()))
		{
			kdWarning() << __FUNCTION__
				<< "Couldn't tickle Pilot!"
				<< endl;
		}
	}
}

// $Log$
// Revision 1.48  2001/04/29 00:36:13  stern
// Fixed mismatched brackes in switch
//
// Revision 1.47  2001/04/29 00:34:24  stern
// Fixed mismatched brackes in switch
//
// Revision 1.46  2001/04/29 00:26:42  stern
// Fixed nasty missing break statement
//
// Revision 1.45  2001/04/26 21:59:00  adridg
// CVS_SILENT B0rkage with previous commit
//
// Revision 1.44  2001/04/26 19:25:24  adridg
// Real change in addSyncLogEntry; muchos reformatting
//
// Revision 1.43  2001/04/23 21:08:02  adridg
// Small changes for code integrity
//
// Revision 1.42  2001/04/16 13:54:17  adridg
// --enable-final file inclusion fixups
//
// Revision 1.41  2001/04/11 21:33:07  adridg
// Make version number consistent across KPilot applications
//
// Revision 1.40  2001/03/27 11:10:39  leitner
// ported to Tru64 unix: changed all stream.h to iostream.h, needed some
// #ifdef DEBUG because qstringExpand etc. were not defined.
//
// Revision 1.39  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.38  2001/03/02 16:59:35  adridg
// Added new protocol message READ_APP_INFO for conduit->daemon communication
//
// Revision 1.37  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.36  2001/02/08 13:17:19  adridg
// Fixed crash when conduits run during a backup and exit after the
// end of that backup (because the event loop is blocked by the backup
// itself). Added better debugging error exit message (no i18n needed).
//
// Revision 1.35  2001/02/08 08:13:44  habenich
// exchanged the common identifier "id" with source unique <sourcename>_id for --enable-final build
//
// Revision 1.34  2001/02/07 14:21:40  brianj
// Changed all include definitions for libpisock headers
// to use include path, which is defined in Makefile.
//
// Revision 1.33  2001/02/05 20:58:48  adridg
// Fixed copyright headers for source releases. No code changed
//
// Revision 1.32  2001/02/05 19:16:32  adridg
// Removing calls to exit() from internal functions
//
// Revision 1.31  2001/02/02 17:31:32  adridg
// Fixed conduit bug
//
// Revision 1.30  2001/02/01 15:29:44  adridg
// Fixed very confusing message -- QString::arg used properly now
//
// Revision 1.29  2001/01/04 22:19:37  adridg
// Stuff for Chris and Bug 18072
//
// Revision 1.24  2000/12/20 19:42:18  bero
// Fix build with -DNDEBUG
//
// Revision 1.16  2000/11/10 13:22:00  adridg
// Failed to catch all the changed getConfig() calls
//
