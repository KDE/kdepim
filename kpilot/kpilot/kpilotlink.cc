// kpilotLink.cc
//
// Copyright (C) 1998,1999,2000 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING.
//
// This is the version of kpilotLink.cc for KDE2 / KPilot 4.
//
//



// REVISION HISTORY
//
// 3.1b9	By Dan Pilone
// 3.1b10	By Adriaan de Groot: comments added all over the place,
//
//		Remaining questions are marked with QADE.
//
// 3.1.14	Added an additional protocol message to the pilot link
//		protocol. If a conduit sends CStatusMessage::LOG_MESSAGE,
//		it must next write 4 bytes (in int format) with the length
//		of the log message to be added, followed by the actual
//		log message.

static const char *id="$Id$";

#include "options.h"

#include "statusMessages.h"
#include "kpilotlink.h"
#include "messageDialog.h"
#include "pi-source.h"
#include "pi-socket.h"
#include "pi-dlp.h"
#include "pi-file.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stream.h>
#include <qdir.h>

#include <kconfig.h>
#include <klocale.h>
#include <ksock.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kstatusbar.h>
#include <kapp.h>
#include <kprogress.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kservice.h>
#include <kdebug.h>

#include "kpilotlink.moc"

// This is to force source compatibility between the KDE1 and KDE2
// versions. For some things we define macros:
//
//
#ifdef KDE2
#define LOCALCONFIG	QString("/home/adridg/.kde/share/config")
#define LOCALKDE	QString("/home/adridg/.kde")
#else
#define LOCALCONFIG	kapp->localconfigdir()
#define LOCALKDE	kapp->localkdedir()
#endif

KPilotLink* KPilotLink::fKPilotLink = 0L;

const QString KPilotLink::BACKUP_DIR = "/share/apps/kpilot/DBBackup/";

// I know this is a bad place for them, but the only source file guaranteed
// to be in every program that needs statusMessages
const int CStatusMessages::CONDUIT_READY = 0;
const int CStatusMessages::SYNC_STARTING = 1;
const int CStatusMessages::SYNC_COMPLETED = 2;
const int CStatusMessages::SYNCING_DATABASE = 3;
const int CStatusMessages::RECORD_MODIFIED = 4;
const int CStatusMessages::RECORD_DELETED = 5;
const int CStatusMessages::FILE_INSTALL_REQUEST = 6;

const int CStatusMessages::NEW_RECORD_ID = 7;
const int CStatusMessages::WRITE_RECORD = 8;
const int CStatusMessages::NEXT_MODIFIED_REC = 9;
const int CStatusMessages::READ_REC_BY_INDEX = 10;
const int CStatusMessages::NO_SUCH_RECORD = 11;
const int CStatusMessages::REC_DATA = 12;
const int CStatusMessages::READ_REC_BY_ID = 13;
const int CStatusMessages::NEXT_REC_IN_CAT = 14;

const int CStatusMessages::LOG_MESSAGE = 17;

// This is a number indicating what configuration version
// we're dealing with. Whenever new configuration options are
// added that make it imperative for the user to take a
// look at the configuration of KPilot (for example the
// skipDB setting really needs user attention) we can change
// (increase) this number.
//
//
/* static */ const int KPilotLink::ConfigurationVersion = 400;

int KPilotLink::getConfigVersion(KConfig *config)
{

	if (!config)	return 0;
	else		return getConfigVersion(*config);
}

int KPilotLink::getConfigVersion(KConfig& config)
{
	FUNCTIONSETUP;

	config.setGroup(QString::null);
	int version=config.readNumEntry("Configured",0);
	if (version<ConfigurationVersion)
	{
		kdDebug() << fname << ": Config file has old version "
			<< version
			<< endl;
	}
	else
	{
		if (debug_level & UI_MINOR)
		{
			kdDebug() << fname
				<< ": Config file has version "
				<< version
				<< endl;
		}
	}

	return version;
}

void KPilotLink::readConfig()
{
	FUNCTIONSETUP;

	// When KPilot starts up we want to be able to find 
	// the last synced users data.
	//
	//
	KConfig& config = getConfig();
	// int version = getConfigVersion(config);
	getPilotUser().setUserName(config.readEntry("UserName").latin1());
}

KPilotLink::KPilotLink()
  : fConnected(false), fCurrentPilotSocket(-1), fSlowSyncRequired(false),
    fOwningWidget(0L), fStatusBar(0L), fProgressDialog(0L), fConduitSocket(0L),
    fCurrentDB(0L), fNextDBIndex(0), fConduitProcess(0L), fMessageDialog(0L)
{
	fKPilotLink = this;
	readConfig();
}


KPilotLink::KPilotLink(QWidget* owner, KStatusBar* statusBar,
		       const QString &devicePath)
  : fConnected(false), fCurrentPilotSocket(-1), fSlowSyncRequired(false),
    fOwningWidget(owner), fStatusBar(statusBar), fProgressDialog(0L),
    fConduitSocket(0L), fCurrentDB(0L), fNextDBIndex(0), fConduitProcess(0L),
    fMessageDialog(0L)
{
  fKPilotLink = this;
	readConfig();
  initPilotSocket(devicePath.latin1());
  initConduitSocket();
  fMessageDialog = new MessageDialog(i18n("Sync Status"));

}

KPilotLink::~KPilotLink()
{
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
KPilotLink::initPilotSocket(const char* devicePath)
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
		kdDebug() << fname << ": No point in trying empty device."
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
	strcpy(addr.pi_device, fPilotPath.ascii());

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
	kdDebug() << fname << ": " << msg << endl;
	if (e)
	{
		kdDebug() << fname 
			<< ": (" << strerror(e) << ")" << endl;
	}

	KMessageBox::error(fOwningWidget, msg,
		i18n("Error Initializing Daemon"));

	exit(1);
}

void
KPilotLink::initConduitSocket()
{
  fConduitSocket = new KServerSocket(KPILOTLINK_PORT);
  connect(fConduitSocket, SIGNAL(accepted(KSocket*)),
	  this, SLOT(slotConduitConnected(KSocket*)));
  fConduitProcess = new KProcess();
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
  
  write(theSocket->socket(), &CStatusMessages::REC_DATA, sizeof(int));
  write(theSocket->socket(), &len, sizeof(int));
  write(theSocket->socket(), &attrib, sizeof(int));
  write(theSocket->socket(), &cat, sizeof(int));
  write(theSocket->socket(), &uid, sizeof(recordid_t));
  write(theSocket->socket(), data, len);
}

int KPilotLink::writeResponse(KSocket *k,int m)
{
	// I have a bad feeling about using pointers
	// to parameters passed to me, so I'm going 
	// to copy the value parameter to a local (stack)
	// variable and use a pointer to that variable.
	//
	//
	int i=m;

	return write(k->socket(), &i, sizeof(int));
}

void
KPilotLink::slotConduitRead(KSocket* cSocket)
{
  FUNCTIONSETUP;

  int message;
  PilotRecord* tmpRec = 0L;

  read(cSocket->socket(), &message, sizeof(int));

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
		if (debug_level & SYNC_TEDIOUS)
		{
			kdDebug() << fname << ": Message length "
				<< i << " => "
				<< s
				<< endl;
		}
		addSyncLogEntry(s);
		delete s;

		return;
	}

	// The remaining messages all require a databse.
	//
	//
	if (!fCurrentDB)
	{
		writeResponse(cSocket,CStatusMessages::NO_SUCH_RECORD);
		return;
	}

	// This whole nested-if thing should become
	// a switch() statement, that tends to be clearer.
	//
	//
  if(message ==  CStatusMessages::WRITE_RECORD)
    {
      //       cout <<"Writing Record..." << endl;
      tmpRec = readRecord(cSocket);
      //       cout << "\tRecord ID: " << tmpRec->getID();
      //       cout << "\tLen: " << tmpRec->getLen();
      fCurrentDB->writeRecord(tmpRec);
      //       cout << "\tRecord ID: " << tmpRec->getID();
      //       cout << "\tLen: " << tmpRec->getLen();
      write(cSocket->socket(), &CStatusMessages::NEW_RECORD_ID, sizeof(int));
      recordid_t tmpID = tmpRec->getID();
      write(cSocket->socket(), &tmpID, sizeof(recordid_t));
      delete tmpRec;
    }
  else if(message == CStatusMessages::NEXT_MODIFIED_REC)
    {
      //       cout << "Looking for MOD_REC..." << endl;
      tmpRec = fCurrentDB->readNextModifiedRec();
      if(tmpRec)
	{
	  // 	  cout << "KPilotLink::NEXT_MODIFIED_RECORD - Found a record." << endl;
	  writeRecord(cSocket, tmpRec);
	  delete tmpRec;
	}
      else
	{
		writeResponse(cSocket,CStatusMessages::NO_SUCH_RECORD);
	  write(cSocket->socket(), &CStatusMessages::NO_SUCH_RECORD, sizeof(int));
	  // 	cout << "KPilotLink::NEXT_MODIFIED_RECORD - No more modified records." << endl;
	}
    }
  else if(message == CStatusMessages::NEXT_REC_IN_CAT)
    {
      //       cout << "Looking for next record in category..." << endl;
      int cat;
      read(cSocket->socket(), &cat, sizeof(int));
      tmpRec = fCurrentDB->readNextRecInCategory(cat);
      if(tmpRec)
	{
	  writeRecord(cSocket, tmpRec);
	  delete tmpRec;
	}
      else
	write(cSocket->socket(), &CStatusMessages::NO_SUCH_RECORD, sizeof(int));
    }
  else if(message ==  CStatusMessages::READ_REC_BY_INDEX)
    {
      //       cout <<"Reading record by index..." << endl;
      int index;
      read(cSocket->socket(), &index, sizeof(int));
      tmpRec = fCurrentDB->readRecordByIndex(index);
      if(tmpRec)
	{
	  writeRecord(cSocket, tmpRec);
	  delete tmpRec;
	}
      else
	write(cSocket->socket(), &CStatusMessages::NO_SUCH_RECORD, sizeof(int));
    }
  else if (message == CStatusMessages::READ_REC_BY_ID)
    {
      //       cout <<"Reading record by id..." << endl;
      recordid_t id;
      read(cSocket->socket(), &id, sizeof(recordid_t));
      tmpRec = fCurrentDB->readRecordById(id);
      if(tmpRec)
	{
	  writeRecord(cSocket, tmpRec);
	  delete tmpRec;
	}
      else
	write(cSocket->socket(), &CStatusMessages::NO_SUCH_RECORD, sizeof(int));
    }
  else
    {
      kdDebug() << fname << ": Unknown status message " 
	   << message
	   << endl;
    }
}

void
KPilotLink::slotConduitClosed(KSocket* theSocket)
{
  FUNCTIONSETUP;

  disconnect(theSocket, SIGNAL(readEvent(KSocket*)),
	     this, SLOT(slotConduitRead(KSocket*)));
  disconnect(theSocket, SIGNAL(closeEvent(KSocket*)),
	     this, SLOT(slotConduitClosed(KSocket*)));
  delete theSocket;

  closeDatabase(fCurrentDB);

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
KPilotLink::registeredConduit(const QString &dbName)
{
	FUNCTIONSETUP;

	KConfig& config = getConfig("Database Names");

	if (debug_level & SYNC_MINOR)
	{
		kdDebug() << fname << ": Looking for "
			<< dbName << endl;
	}

	QString result = config.readEntry(dbName);
	if (result.isNull())
	{
		if (debug_level & SYNC_MINOR)
		{
			kdDebug() << fname << ": Not found." << endl;
		}

		return result;
	}

	config.setGroup("Conduit Names");
	QStringList installed = config.readListEntry("InstalledConduits");

	if (debug_level & SYNC_TEDIOUS)
	{
		QStringList::Iterator i;
		kdDebug() << fname << ": Found conduit "
			<< result << endl
			<< fname << ": Installed Conduits are"
			<< endl;

		for (i=installed.begin(); i!=installed.end(); ++i)
		{
			kdDebug() << fname << ": * "
				<< (*i)
				<< endl;
		}
	}

	if (!installed.contains(result))
	{
		if (debug_level & SYNC_MINOR)
		{
			kdDebug() << fname << ": Conduit not installed."
				<< endl;
		}
		return QString::null;
	}

#ifdef KDE2
	KService::Ptr conduit = KService::serviceByDesktopName(result);
	if (!conduit)
	{
		if (debug_level & SYNC_MINOR)
		{
			kdDebug() << fname << ": No service for this conduit"
				<< endl;
		}
		return QString::null;
	}
	else
	{
		if (debug_level & SYNC_MINOR)
		{
			kdDebug() << fname << ": Found service with exec="
				<< conduit->exec()
				<< endl;
		}
		return conduit->exec();
	}
#else
  QString conduitPath = kapp->kde_datadir() + "/kpilot/conduits/";

  if(result.isNull()) return result;
  else return conduitPath + "/" + result;
#endif
}


void
KPilotLink::slotConduitConnected(KSocket* theSocket)
{
  //   cout << "Got conduit connection" << endl;
  connect(theSocket, SIGNAL(readEvent(KSocket*)),
	  this, SLOT(slotConduitRead(KSocket*)));
  connect(theSocket, SIGNAL(closeEvent(KSocket*)),
	  this, SLOT(slotConduitClosed(KSocket*)));
  theSocket->enableRead(true);
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

  DIR * dir = NULL;
  struct dirent * dirent;
  struct DBInfo info;
  struct db * db[256];
  int dbcount = 0;
  int i,j,max,size;
  struct pi_file * f;
  char message[256];

  i=KMessageBox::questionYesNo(
			       fOwningWidget,
			       i18n("Replace all data on pilot with local data?"),
			       i18n("Full Restore"));
  if (i != KMessageBox::Yes) return false;

  QString dirname = KGlobal::dirs()->saveLocation("data", QString("kpilot/DBBackup/")
						  + getPilotUser().getUserName() + "/");

  // FIXME: This should be done with a QDirectory-thingee
  dir = opendir(dirname.latin1());
    
  while( (dirent = readdir(dir)) )
    {
      char name[256];

      if (dirent->d_name[0] == '.')
	continue;
	
	
      db[dbcount] = (struct db*)malloc(sizeof(struct db));
	
      sprintf(db[dbcount]->name, "%s/%s", (const char *)dirname.latin1(), dirent->d_name);
	
      f = pi_file_open(db[dbcount]->name);
      if (f==0) 
	{
	  printf("Unable to open '%s'!\n", name);
	  break;
	}
  	
      pi_file_get_info(f, &info);
  	
      db[dbcount]->creator = info.creator;
      db[dbcount]->type = info.type;
      db[dbcount]->flags = info.flags;
      db[dbcount]->maxblock = 0;
  	
      pi_file_get_entries(f, &max);
  	
      for (i=0;i<max;i++) 
	{
	  if (info.flags & dlpDBFlagResource)
	    pi_file_read_resource(f, i, 0, &size, 0, 0);
	  else
	    pi_file_read_record(f, i, 0, &size, 0, 0,0 );
  	    
	  if (size > db[dbcount]->maxblock)
	    db[dbcount]->maxblock = size;
	}
  	
      pi_file_close(f);
      dbcount++;
    }
    
  closedir(dir);
    
  // #ifdef DEBUG      
  //     printf("Unsorted:\n");
  //     for (i=0;i<dbcount;i++) {
  //     printf("%d: %s\n", i, db[i]->name);
  //     printf("  maxblock: %d\n", db[i]->maxblock);
  //     printf("  creator: '%s'\n", printlong(db[i]->creator));
  //     printf("  type: '%s'\n", printlong(db[i]->type));
  //     }
  // #endif
    
  for (i=0;i<dbcount;i++)
    for (j=i+1;j<dbcount;j++)
      if (compare(db[i],db[j])>0) {
	struct db * temp = db[i];
	db[i] = db[j];
	db[j] = temp;
      }
    
  // #ifdef DEBUG      
  //     printf("Sorted:\n");
  //     for (i=0;i<dbcount;i++) {
  //     printf("%d: %s\n", i, db[i]->name);
  //     printf("  maxblock: %d\n", db[i]->maxblock);
  //     printf("  creator: '%s'\n", printlong(db[i]->creator));
  //     printf("  type: '%s'\n", printlong(db[i]->type));
  //     }
  // #endif
    
  //     MessageDialog* messageDialog = new MessageDialog(klocale->translate("Full Sync Status"));
  fMessageDialog->setMessage("Starting Sync.");
  fMessageDialog->show();
  for (i=0;i<dbcount;i++) 
    {
	
      if (dlp_OpenConduit(getCurrentPilotSocket()) < 0) 
	{
	  kdDebug() << fname << ": Exiting on cancel. "
	    "All data not restored."
	       << endl;
	  exit(1);
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
  emit(databaseSyncComplete());
  return true;
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

  if (debug_level & DB_TEDIOUS)
    {
      kdDebug() << fname << ": Creating local database "
	   << name << endl;
    }
  /* Ensure that DB-open flag is not kept */
  info->flags &= 0xff;

  f = pi_file_create(name, info);
  if (f==0) 
    {
      kdDebug() << fname << ": Failed, unable to create file"
	   << endl;
      return false;
    }

  if(pi_file_retrieve(f, getCurrentPilotSocket(), 0)<0)
    {
      kdDebug() << fname << ": Failed, unable to back up database"
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
  int i = 0;
  char message[256];

  setSlowSyncRequired(true);
  fMessageDialog->setMessage("Starting Sync.");
  fMessageDialog->show();
  showMessage(i18n("Backing up Palm Pilot... Slow sync required."));
  addSyncLogEntry("Backing up all data...");
  for(;;)
    {
      struct DBInfo info;

      if (dlp_OpenConduit(getCurrentPilotSocket())<0) 
	{
	  KMessageBox::error(fOwningWidget,
			     i18n("Exiting on cancel.\n"
				  "<B>Not</B> all the data was backed up."),
			     i18n("Backup"));
	  addSyncLogEntry("FAILED.\n");
	  return;
	}
	
      if( dlp_ReadDBList(getCurrentPilotSocket(), 0, 0x80, i, &info) < 0)
	break;
      i = info.index + 1;

      strcpy(message, "Backing Up: ");
      strcat(message, info.name);
      fMessageDialog->setMessage(message);
  	
      if(createLocalDatabase(&info) == false)
	{
	  KMessageBox::error(fOwningWidget,
			     i18n("Could not backup data!"),
			     i18n("Backup failed"));
	}
    }
  addSyncLogEntry("OK.\n");
  // Set up so the conduits can run through the DB's and backup.  doConduitBackup()
  // will emit the databaseSyncComplete when done.
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
      kdDebug() << fname << ": No HotSync started!" << endl;
      return;
    }

  updateProgressBar(0);
  if (debug_level & SYNC_MINOR)
    {
      kdDebug() << fname << ": Installing from directory "
	   << actualPath << endl;
    }

  while(fileIter != fileNameList.end())
    {
      if (debug_level & SYNC_MAJOR)
	{
	  kdDebug() << fname << ": Installing file "
	       << *fileIter << endl;
	}

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
	  kdDebug() << fname << ": Unable to open file." << endl;

	  QString message;

	  message=i18n("Unable to open file &quot;%1&quot;!")
	    .arg(*fileIter);
	  KMessageBox::error(fOwningWidget,
			     message,
			     i18n("Missing File"));
	}
      else
	{
	  //  	    cout << "Installing " << (*(fileIter)) << "..." << flush;
	  if(pi_file_install(f, getCurrentPilotSocket(), 0) <0)
	    {
	      kdDebug() << fname << ": failed to install." << endl;
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
  //     cout << "Install done\n";
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
KPilotLink::createNewProgressBar(QString title, QString text, int min, int max, int value)
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
      cout << "KPilotLink::connect() Error - Already connected or unable to connect!" << endl;
      return;
    }

  // I should really set these up...
  //   signal(SIGHUP, SigHandler);
  //   signal(SIGINT, SigHandler);
  //   signal(SIGSEGV, SigHandler);
    
  createNewProgressBar(i18n("Waiting to Sync"), 
		       i18n("Reading user information..."), 
		       0, 10, 0);
  updateProgressBar(0);

  // BAD HACK!
  for(int i = 0; i < 100000; i++);
    
  ret = pi_listen(getPilotMasterSocket(),1);
  if(ret == -1) 
    {
      perror("pi_listen");
      exit(1);
    }

  fCurrentPilotSocket = pi_accept(fPilotMasterSocket,0,0);
  if(fCurrentPilotSocket == -1) 
    {
      perror("pi_accept");
      exit(1);
    }
  setConnected(true);
  updateProgressBar(3);
  /* Ask the pilot who it is.  And see if it's who we think it is. */
  dlp_ReadUserInfo(getCurrentPilotSocket(), &fPilotUser);
  getPilotUser().boundsCheck();
  checkPilotUser();

  updateProgressBar(7);
  if(fPilotUser.getLastSyncPC() != (unsigned long)gethostid())
    setSlowSyncRequired(true);
  //     cout << "Got user info: " << endl;
  //     cout << "\tName: " << getPilotUser().getUserName() << endl;
  //     cout << "\tPassword: " << getPilotUser().getPassword() << endl;

  /* Tell user (via Pilot) that we are starting things up */
  if (dlp_OpenConduit(getCurrentPilotSocket()) < 0) 
    {
      showMessage("Exiting on cancel. All data not restored.");
      return;
    }
  updateProgressBar(10);
  destroyProgressBar();
  showMessage("Hot-Sync started.");
  addSyncLogEntry("Sync started with KPilot-v4.0b\n");
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
      //       cout << "KPilotLink::doConduitBackup() - looking for first DB.." << endl;
      if(dlp_ReadDBList(getCurrentPilotSocket(), 0, 0x80, fNextDBIndex, &info) < 0)
	{
	  setSlowSyncRequired(false);
	  emit(databaseSyncComplete());
	  fMessageDialog->hide();
	  return;
	}
      fNextDBIndex = info.index + 1;
    }
  while(info.flags & dlpDBFlagResource);
  
  //   cout << info.name << ": ";
  QString conduitName = registeredConduit(info.name);
  while(conduitName.isNull())
    {
      //       cout << "doConduitBackup() - No registered conduit.  Ignoring." << endl;
      do
	{
	  if(dlp_ReadDBList(getCurrentPilotSocket(), 0, 0x80, fNextDBIndex, &info) < 0)
	    {
	      setSlowSyncRequired(false);
	      emit(databaseSyncComplete());
	      fMessageDialog->hide();
	      return;
	    }
	  fNextDBIndex = info.index + 1;
	} while(info.flags & dlpDBFlagResource);
      //       cout << info.name << ": ";
      conduitName = registeredConduit(info.name);
    }

  // Fire up the conduit responsible for this db and when it's finished
  // we'll get called again.
  displaymessage=i18n("%1: Running conduit").arg(info.name);
  fMessageDialog->setMessage(displaymessage);
  fCurrentDBInfo = info;
  fCurrentDB = openDatabase(info.name);
  fCurrentDB->resetDBIndex();
  if(fConduitProcess->isRunning())
    {
      kdDebug() << fname << ": Waiting for conduit to die.. " << endl;
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
  if (debug_level)
    {
      *fConduitProcess << "--debug";
      *fConduitProcess << QString::number(debug_level);
    }
  fConduitProcess->start(KProcess::DontCare);
}

int KPilotLink::findNextDB(DBInfo *info)
{
  FUNCTIONSETUP;

  do
    {
      if(dlp_ReadDBList(getCurrentPilotSocket(), 0, 0x80, 
			fNextDBIndex, info) < 0)
	{
	  fMessageDialog->hide();
	  emit(databaseSyncComplete());
	  return 0;
	}
      fNextDBIndex = info->index + 1;
    }
  while(info->flags & dlpDBFlagResource);

  if (debug_level & SYNC_TEDIOUS)
    {
      kdDebug() << fname << ": Found database with:\n"
	   << fname << ": Index=" << fNextDBIndex
	   << endl;
    }

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
    KConfig& c = getConfig();
    skip=c.readEntry("SkipSync");
    backupOnly=c.readEntry("BackupForSync");
  }

  if (debug_level & SYNC_TEDIOUS)
    {
      kdDebug() << fname << ": Special dispositions are: \n"
	   << fname << ": * BackupOnly=" << backupOnly << endl
	   << fname << ": * Skip=" << skip << endl ;
    }
  if (!findNextDB(&info)) return;

  if (debug_level & SYNC_MAJOR)
    {
      kdDebug() << fname << ": Syncing " << info.name << endl;
    }



  QString conduitName = registeredConduit(info.name);
  while(conduitName.isNull())
    {
      if (debug_level & SYNC_MINOR)
	{
	  kdDebug() << fname << ": No registered conduit for " 
	       << info.name << endl;
	}

      message=i18n("Syncing: %1 ...").arg(info.name);
      fMessageDialog->setMessage(message);
      addSyncLogEntry(message.local8Bit());

      // Find out if this database has a special disposition
      //
      //
      if (debug_level & SYNC_MINOR)
	{
	  char *m=printlong(info.creator);
	  kdDebug() << fname << ": Looking for disposition of "
	       << m
	       << endl;
	}
      if (findDisposition(skip,&info)) goto nextDB;
      if (findDisposition(backupOnly,&info)) 
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
	  if (debug_level & SYNC_TEDIOUS)
	    {
	      kdDebug() << fname << ": Sync OK" << endl;
	    }
	  addSyncLogEntry("OK.\n");
	}
      else
	{
	  kdDebug() << fname << ": Sync " 
	       << info.name << " failed."
	       << endl;

	  addSyncLogEntry("FAILED!\n");
	}

    nextDB:
      if (!findNextDB(&info)) return;

      conduitName = registeredConduit(info.name);
      if (debug_level & SYNC_MAJOR)
	{
	  kdDebug() << fname << ": Syncing " << info.name << endl;
	}
    }

  // Fire up the conduit responsible for this db and when it's finished
  // we'll get called again.
  message=i18n("%1: Running conduit").arg(info.name);
  fMessageDialog->setMessage(message);
  fCurrentDBInfo = info;

  if (debug_level & SYNC_MAJOR)
    {
      kdDebug() << fname << ": " 
	   << message << endl;
    }


  fCurrentDB = openDatabase(info.name);
  fCurrentDB->resetDBIndex();
  if(fConduitProcess->isRunning())
    {
      kdDebug() << fname << ": Waiting for conduit to die.. " << endl;
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
	if (debug_level)
	{
		*fConduitProcess << "--debug" ;
		*fConduitProcess << QString::number(debug_level);
	}
  fConduitProcess->start(KProcess::DontCare);
}

bool 
KPilotLink::syncDatabase(DBInfo* database)
{
  unsigned char buffer[0xffff];
  
  PilotDatabase* firstDB;
  PilotDatabase* secondDB;

  firstDB = openDatabase(database->name);
  secondDB = openLocalDatabase(database->name);
  PilotRecord* pilotRec;

  if(firstDB->isDBOpen() && (secondDB->isDBOpen() == false))
    {
      // Must be a new Database...
      showMessage(i18n("No previous copy.  Copying data from pilot..."));
      closeDatabase(firstDB); // So we can reopen it to copy it
      closeDatabase(secondDB);
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
      firstDB = openDatabase(database->name);
      secondDB = openLocalDatabase(database->name);
      showMessage(i18n("Hot-Syncing Pilot. Looking for modified data..."));
    }
  if((secondDB->isDBOpen() == false) || (firstDB->isDBOpen() == false))
    {
      closeDatabase(firstDB);
      closeDatabase(secondDB);
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
  KConfig& config = getConfig();
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
  closeDatabase(firstDB);
  closeDatabase(secondDB);
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
}

void KPilotLink::checkPilotUser()
{
  KConfig& config = getConfig();
  if (config.readBoolEntry("AlwaysTrustPilotUser"))
    {
      return;
    }

  QString guiUserName;
  guiUserName = config.readEntry("UserName");
  
  if (guiUserName != getPilotUser().getUserName())
    {
      QString message(i18n(
			   "The Palm Pilot thinks the user name is %1, "
			   "however KPilot says you are %2.\n"
			   "Should I assume the Pilot is right and set the "
			   "user name for KPilot to %1? "
			   "(Otherwise I'll use %2 for now)"));
	message=message.arg(getPilotUser().getUserName());
	message=message.arg(guiUserName);

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
	  kdDebug() << "Pilot User set to " << getPilotUser().getUserName() << endl;
	}
    }
}


static KConfig* theconfig = 0L;
KConfig& KPilotLink::getConfig(const QString &s)
{
	FUNCTIONSETUP;

	if (theconfig)
	{
		theconfig->setGroup(s);
		return *theconfig;
	}

	/**
	* This causes a crash if no instance has been created
	* yet. A standard KDE error message reports this fact.
	* It is a grave programming error, so we will let that
	* stand.
	*/
	QString existingConfig=
		KGlobal::dirs()->findResource("config", "kpilotrc");

	
	if (existingConfig.isNull())
	{
		if (debug_level & UI_MAJOR)
		{
			kdDebug() << fname 
				<< ": Making a new config file"
				<< endl;
		}
		theconfig=new KConfig("kpilotrc",false,false);
	}
	else
	{
		theconfig=new KConfig(existingConfig,false,false);
	}

	if (theconfig == 0L)
	{
		kdDebug() << fname << ": No configuration was found."
			<< endl;
	}

	theconfig->setGroup(s);
	return *theconfig;
}

PilotLocalDatabase *KPilotLink::openLocalDatabase(const QString &database)
{
  QString pathName = KGlobal::dirs()->saveLocation("data", QString("kpilot/DBBackup/") + getPilotUser().getUserName() + "/");
  return new PilotLocalDatabase(pathName.latin1(), database.latin1());
}

// $Log:$
