// kpilotLink.cc
//
// Copyright (C) 1998,1999 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 



// REVISION HISTORY 
//
// 3.1b9	By Dan Pilone
// 3.1b10	By Adriaan de Groot: comments added all over the place,
//
//		Remaining questions are marked with QADE.



#include <sys/stat.h>
#include <iostream.h>
#include <string.h>
#include <qobject.h>
#include <qdir.h>
#include <qlist.h>
#include <kapp.h>
#include <kurl.h>
#include <kmsgbox.h>
#include <kstatusbar.h>
#include <kprogress.h>
#include <ksock.h>
#include <ksimpleconfig.h>
#include "statusMessages.h"
#include "kpilotlink.h"
#include "pi-source.h"
#include "pi-socket.h"
#include "pi-dlp.h"
#include "pi-file.h"
#include "messageDialog.h"
#include "kpilot.h"

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


KPilotLink::KPilotLink()
  : fConnected(false), fCurrentPilotSocket(-1), fSlowSyncRequired(false), 
    fOwningWidget(0L), fStatusBar(0L), fProgressDialog(0L), fConduitSocket(0L),
    fCurrentDB(0L), fNextDBIndex(0), fConduitProcess(0L), fMessageDialog(0L)
{
    fKPilotLink = this;
    
    // When KPilot starts up we want to be able to find the last synced users data.
    KSimpleConfig* config = new KSimpleConfig(kapp->localconfigdir() + "/kpilotrc");
    config->setGroup(0L);
    getPilotUser().setUserName(config->readEntry("UserName"));
    delete config;
}


KPilotLink::KPilotLink(QWidget* owner, KStatusBar* statusBar, char* devicePath)
  : fConnected(false), fCurrentPilotSocket(-1), fSlowSyncRequired(false), 
    fOwningWidget(owner), fStatusBar(statusBar), fProgressDialog(0L), 
    fConduitSocket(0L), fCurrentDB(0L), fNextDBIndex(0), fConduitProcess(0L),
    fMessageDialog(0L)
    {
    fKPilotLink = this;
    initPilotSocket(devicePath);
    initConduitSocket();
    fMessageDialog = new MessageDialog(klocale->translate("Sync Status"));

    // When KPilot starts up we want to find the last synced users data.
    KSimpleConfig* config = new KSimpleConfig(kapp->localconfigdir() + "/kpilotrc");
    config->setGroup(0L);
    getPilotUser().setUserName(config->readEntry("UserName"));
    delete config;
    }

KPilotLink::~KPilotLink()
{
  if(fMessageDialog)
    delete fMessageDialog;
  if(fConduitSocket)
    delete fConduitSocket;
  if(getConnected())
    endHotSync();
}

void
KPilotLink::initPilotSocket(const char* devicePath)
{
  struct pi_sockaddr addr;
  int ret;
  
  pi_close(getCurrentPilotSocket());
  fPilotPath = devicePath;
  
  if (!(fPilotMasterSocket = pi_socket(PI_AF_SLP, PI_SOCK_STREAM, PI_PF_PADP))) 
    {
      KMsgBox::message(fOwningWidget, klocale->translate("Error Initializing?"), 
		       klocale->translate("Cannot create socket."), KMsgBox::STOP);
      perror("pi_socket");
      fPilotMasterSocket = -1;
      return;
    }
  addr.pi_family = PI_AF_SLP;
  strcpy(addr.pi_device,fPilotPath);
  
  ret = pi_bind(fPilotMasterSocket, (struct sockaddr*)&addr, sizeof(addr));
  if(ret == -1) 
    {
      KMsgBox::message(fOwningWidget, klocale->translate("Error Initializing?"), 
		       klocale->translate("Cannot connect to pilot.\n(check pilot path)"), KMsgBox::STOP);
      exit(1);
    }
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

void
KPilotLink::slotConduitRead(KSocket* cSocket)
{
  int message;
  PilotRecord* tmpRec = 0L;

  read(cSocket->socket(), &message, sizeof(int));
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
     cout <<"ERROR: Unknown status message." << endl;
}

void
KPilotLink::slotConduitClosed(KSocket* theSocket)
{
//   cout << "Got conduit disconnection. " << endl;
  // Get rid of this conduit connection
  disconnect(theSocket, SIGNAL(readEvent(KSocket*)),
	     this, SLOT(slotConduitRead(KSocket*)));
  disconnect(theSocket, SIGNAL(closeEvent(KSocket*)),
	     this, SLOT(slotConduitClosed(KSocket*)));
  delete theSocket;
  closeDatabase(fCurrentDB);
  // Get our backup copy.
  if(slowSyncRequired()) // We are in the middle of backing up, continue
    doConduitBackup();
  else // We are just doing a normal sync, so go for it.
    {
      syncDatabase(&fCurrentDBInfo);
      // Start up the next one
      syncNextDB();
    }
}

QString
KPilotLink::registeredConduit(QString dbName)
{
  QString conduitPath = kapp->kde_datadir() + "/kpilot/conduits/";
  KSimpleConfig* config = new KSimpleConfig(kapp->localconfigdir() + "/kpilotconduits");

  config->setGroup("Database Names");
  QString result = config->readEntry(dbName);
  delete config;
  if(result.isNull())
    return result;
  return conduitPath + "/" + result;
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
void
KPilotLink::showMessage(QString message) const
    {
    if(fStatusBar)
	{
	fStatusBar->changeItem(message.data(), 0);
	kapp->processEvents();
	}
    }

int 
KPilotLink::compare(struct db * d1, struct db * d2)
    {
    /* types of 'appl' sort later then other types */
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
    DIR * dir;
    struct dirent * dirent;
    struct DBInfo info;
    struct db * db[256];
    int dbcount = 0;
    int i,j,max,size;
    struct pi_file * f;
    char dirname[256];
    char message[256];

    if(KMsgBox::yesNo(0L, klocale->translate("Full Restore"), 
		      klocale->translate("Replace all data on pilot with local data?")) == 2)
      return false;

    strcpy(dirname, kapp->localkdedir().data());
    strcat(dirname, BACKUP_DIR.data());
    strcat(dirname, getPilotUser().getUserName());
    strcat(dirname, "/");

    dir = opendir(dirname);
    
    while( (dirent = readdir(dir)) ) 
	{
	char name[256];
	
	if (dirent->d_name[0] == '.')
	    continue;
	
	
	db[dbcount] = (struct db*)malloc(sizeof(struct db));
	
	sprintf(db[dbcount]->name, "%s/%s", dirname, dirent->d_name);
	
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
	    puts("Exiting on cancel. All data not restored.");
	    exit(1);
	    }
	showMessage(klocale->translate("Restoring databases to Palm Pilot. Slow sync required."));
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
    char temp[256];
    char name[256];
    int j;
    struct pi_file* f;
    char fullBackupDir[256];

    strcpy(fullBackupDir, kapp->localkdedir().data());
    strcat(fullBackupDir, BACKUP_DIR.data());
    strcat(fullBackupDir, getPilotUser().getUserName());
    strcat(fullBackupDir, "/");
    mkdir(fullBackupDir, 0700);

    strcpy(temp, info->name);
    j = -1;
    // Fix the filename, incase there is a forward slash in it.
    while(temp[++j])
	if(temp[j] == '/')
	    temp[j] = '_';
    sprintf(name, "%s/%s", fullBackupDir, info->name);
    if (info->flags & dlpDBFlagResource)
	strcat(name,".prc");
    else
	strcat(name,".pdb");
    
    /* Ensure that DB-open flag is not kept */
    info->flags &= 0xff;
    
    f = pi_file_create(name, info);
    if (f==0) 
	{
	printf("Failed, unable to create file\n");
	return false;
	}
    
    if(pi_file_retrieve(f, getCurrentPilotSocket(), 0)<0)
	printf("Failed, unable to back up database\n");
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
    showMessage(klocale->translate("Backing up Palm Pilot... Slow sync required."));
    addSyncLogEntry("Backing up all data...");
    for(;;) 
	{
	struct DBInfo info;

	if (dlp_OpenConduit(getCurrentPilotSocket())<0) 
	    {
	    KMsgBox::message(fOwningWidget, klocale->translate("Backup"), 
			     klocale->translate("Exiting on canel.\n All Data NOT backed up"), 
			     KMsgBox::STOP);
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
	    KMsgBox::message(fOwningWidget, klocale->translate("Backup"), 
			     klocale->translate("Could not backup data!"), KMsgBox::STOP);
	}
    addSyncLogEntry("OK.\n");
    // Set up so the conduits can run through the DB's and backup.  doConduitBackup()
    // will emit the databaseSyncComplete when done.
    fNextDBIndex = 0;
    doConduitBackup();
    return;
    }

void 
KPilotLink::installFiles(QString path)
    {
    struct pi_file * f;
    QString errorMessage;
    int fileNum = 0;

    path += "/";
    QDir installDir(path);
    const QStrList* fileNameList = installDir.entryList();
    QListIterator<char> fileList(*fileNameList);
    ++fileList; ++fileList; // Skip . & ..
    if(fileList.current() == 0L)
      return;
    createNewProgressBar(klocale->translate("Installing Files"), 
			 klocale->translate("Percentage of files installed:"), 0, fileList.count(), 0);
    showMessage("Installing files...");
    if(getConnected() == false)
	{
	cerr << "KPilotLink::installFiles() No HotSync started!" << endl;
	return;
	}
    updateProgressBar(0);
    while(fileList.current())
	{
	updateProgressBar(fileNum++);
 	f = pi_file_open((path + fileList.current()).data());

	if (f==0) 
	    {
	    errorMessage.sprintf("Unable to open '%s'!\n", fileList.current());
	    KMsgBox::message(fOwningWidget, klocale->translate("Missing file?"), errorMessage, KMsgBox::STOP);
	    }
	else
	    {
//  	    cout << "Installing " << fileList.current() << "..." << flush;
	    if(pi_file_install(f, getCurrentPilotSocket(), 0) <0)
// 		cout << "\tfailed." << endl;
		KMsgBox::message(fOwningWidget, klocale->translate("Missing file?"), "Cannot install file.", KMsgBox::STOP);
 	    else
	      unlink((path + fileList.current()).data());
	    pi_file_close(f);
	    }
	++fileList;
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
	cerr << "KPilotLink::syncFlags() No HotSync started!" << endl;
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
    label->setAlignment(AlignBottom | AlignCenter | WordBreak);
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
    
    createNewProgressBar(klocale->translate("Waiting to Sync"), klocale->translate("Reading user information..."), 0, 10, 0);
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
    addSyncLogEntry("Sync started with KPilot-v3.1\n");
    fNextDBIndex = 0;
    fCurrentDB = 0L;
    }

void
KPilotLink::quickHotSync()
{ 
  fMessageDialog->setMessage("Beginning Sync"); 
  fMessageDialog->show(); 
  syncNextDB();
}

void
KPilotLink::doConduitBackup()
{
  char message[256];
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
  sprintf(message, "%s: Running conduit", info.name);
  fMessageDialog->setMessage(message);
  fCurrentDBInfo = info;
  fCurrentDB = openDatabase(info.name);
  fCurrentDB->resetDBIndex();
  if(fConduitProcess->isRunning())
    cout << "Waiting for conduit to die.. " << endl;
  while(fConduitProcess->isRunning());

  fConduitProcess->clearArguments();
  *fConduitProcess << conduitName;
  *fConduitProcess << "-backup";
  fConduitProcess->start(KProcess::DontCare);
}

void
KPilotLink::syncNextDB()
{
  char message[256];
  struct DBInfo info;
  do
    {
//       cout << "KPilotLink::syncNextDB() - looking for first DB.." << endl;
      if(dlp_ReadDBList(getCurrentPilotSocket(), 0, 0x80, fNextDBIndex, &info) < 0)
	{
	  fMessageDialog->hide();
	  emit(databaseSyncComplete());
	  return;
	}
      fNextDBIndex = info.index + 1;
    }
  while(info.flags & dlpDBFlagResource);
  
//   cout << info.name << ": ";
  QString conduitName = registeredConduit(info.name);
  while(conduitName.isNull())
    {
//       cout << "No registered conduit.  Syncing " << info.name 
// 	   << " the old fashioned way.. " << endl;
      sprintf(message, "Syncing: %s...", info.name);
      fMessageDialog->setMessage(message);
      addSyncLogEntry(message);
      if(syncDatabase(&info))
	addSyncLogEntry("OK.\n");
      else
	addSyncLogEntry("FAILED!\n");
      do
	{
	  if(dlp_ReadDBList(getCurrentPilotSocket(), 0, 0x80, fNextDBIndex, &info) < 0)
	    {
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
  sprintf(message, "%s: Running conduit", info.name);
  fMessageDialog->setMessage(message);
  fCurrentDBInfo = info;
  fCurrentDB = openDatabase(info.name);
  fCurrentDB->resetDBIndex();
  if(fConduitProcess->isRunning())
    cout << "Waiting for conduit to die.. " << endl;
  while(fConduitProcess->isRunning());
  fConduitProcess->clearArguments();
  *fConduitProcess << conduitName;
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
	showMessage("No previous copy.  Copying data from pilot...");
	closeDatabase(firstDB); // So we can reopen it to copy it
	closeDatabase(secondDB);
	if(createLocalDatabase(database) == false)
	    KMsgBox::message(fOwningWidget, klocale->translate("Backup"), klocale->translate("Could not backup data!"), KMsgBox::EXCLAMATION);
	firstDB = openDatabase(database->name);
	secondDB = openLocalDatabase(database->name);
	showMessage("Hot-Syncing Pilot. Looking for modified data...");
	}
    if((secondDB->isDBOpen() == false) || (firstDB->isDBOpen() == false))
       {
       closeDatabase(firstDB);
       closeDatabase(secondDB);
       char message[255];
       strcpy(message, "Cannot find ");
       strcat(message, database->name);
       KMsgBox::message(fOwningWidget, klocale->translate("Error syncing."), message, KMsgBox::EXCLAMATION);
       return false;
       }
    KSimpleConfig* config = new KSimpleConfig(kapp->localconfigdir() + "/kpilotrc");
    config->setGroup(0L);
    // If local changes should modify pilot changes, switch the order.
    int localOverride = config->readNumEntry("OverwriteRemote");
    delete config;

    if(localOverride)
	{
	PilotDatabase* tmp = firstDB;
	firstDB = secondDB;
	secondDB = tmp;
	}
    int len = firstDB->readAppBlock(buffer, 0xffff);
    if(len > 0)
	secondDB->writeAppBlock(buffer, len);
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
  KSimpleConfig* config = new KSimpleConfig(kapp->localconfigdir() + "/kpilotrc");
  config->setGroup(0L);
  if (config->readBoolEntry("AlwaysTrustPilotUser"))
    {
      delete config;
      return;
    }
  
  QString guiUserName;
  guiUserName = config->readEntry("UserName");
  
  if (guiUserName != getPilotUser().getUserName())
    {
      QString message;
      message.sprintf("The Palm Pilot thinks the user name is %s, however KPilot says you are %s.\n  Should I assume the Pilot is right?",
		      getPilotUser().getUserName(), guiUserName.data());
      if (KMsgBox::yesNo(0, "Different User?", message) == 1)
	  config->writeEntry("UserName", getPilotUser().getUserName());
      else
	{
	  // The gui was right.
	  getPilotUser().setUserName(guiUserName);
	  cout << "Pilot User set to " << getPilotUser().getUserName() << endl;
	}
    }
  delete config;
}

#include "kpilotlink.moc"
