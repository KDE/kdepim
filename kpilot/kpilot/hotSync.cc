/* hotSync.cc                           KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines SyncActions, which are used to perform some specific
** task during a HotSync. Conduits are not included here, nor are 
** sync actions requiring user interaction. Those can be found in the
** conduits subdirectory or interactiveSync.h.
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
** Bug reports and questions can be sent to kde-pim@kde.org.
*/

static const char *hotsync_id =
	"$Id$";

#include "options.h"

#include <time.h>
#include <unistd.h>

#include <pi-file.h>

#include <qtimer.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qvaluelist.h>
#include <qregexp.h>

#include <kglobal.h>
#include <kstddirs.h>
#include <kapp.h>

#include "pilotUser.h"

#include "hotSync.moc"

TestLink::TestLink(KPilotDeviceLink * p) : SyncAction(p, 0, "testLink")
{
	FUNCTIONSETUP;

	(void) hotsync_id;
}

/* virtual slot */ void TestLink::exec()
{
	FUNCTIONSETUP;

	int i;
	int dbindex = 0;
	int count = 0;
	struct DBInfo db;

	addSyncLogEntry(i18n("Performing a TestSync."));

	while ((i = dlp_ReadDBList(pilotSocket(), 0, dlpDBListRAM,
				dbindex, &db)) > 0)
	{
		count++;
		dbindex = db.index + 1;

#ifdef DEBUG
		DEBUGKPILOT << fname << ": Read database " << db.name << endl;
#endif

		emit logMessage(i18n("Syncing database %1...").arg(db.name));

		kapp->processEvents();
	}

	emit logMessage(i18n("HotSync finished."));
	emit syncDone(this);
}

BackupAction::BackupAction(KPilotDeviceLink * p):
SyncAction(p, 0, "backupAction")
{
	FUNCTIONSETUP;

	fDatabaseDir = KGlobal::dirs()->saveLocation("data",
		QString("kpilot/DBBackup/"));
}

/* virtual */ QString BackupAction::statusString() const
{
	FUNCTIONSETUP;
	QString s("BackupAction=");

	switch (status())
	{
	case Init:
		s.append("Init");
		break;
	case Error:
		s.append("Error");
		break;
	case FullBackup:
		s.append("FullBackup");
		break;
	case BackupEnded:
		s.append("BackupEnded");
		break;
	default:
		s.append("(unknown ");
		s.append(QString::number(status()));
		s.append(")");
	}

	return s;
}


/* virtual */ void BackupAction::exec()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": This Pilot user's name is \""
		<< fHandle->getPilotUser()->getUserName() << "\"" << endl;
#endif

	addSyncLogEntry(i18n("Full backup started."));

	ASSERT(!fTimer);

	fTimer = new QTimer(this);
	QObject::connect(fTimer, SIGNAL(timeout()),
		this, SLOT(backupOneDB()));

	fDBIndex = 0;
	fStatus = FullBackup;

	fTimer->start(0, false);
}

/* slot */ void BackupAction::backupOneDB()
{
	FUNCTIONSETUP;

	struct DBInfo info;

	emit logProgress(QString::null, fDBIndex);

	if (dlp_OpenConduit(pilotSocket()) < 0)
	{
#ifdef DEBUG
		DEBUGDAEMON << fname
			<< ": dlp_OpenConduit failed. User cancel?" << endl;
#endif

		addSyncLogEntry(i18n("Exiting on cancel."));
		endBackup();
		fStatus = BackupIncomplete;
		return;
	}

	if (dlp_ReadDBList(pilotSocket(), 0, 0x80, fDBIndex, &info) < 0)
	{
#ifdef DEBUG
		DEBUGDAEMON << fname << ": Backup complete." << endl;
#endif

		addSyncLogEntry(i18n("Full backup complete."));
		endBackup();
		fStatus = BackupComplete;
		return;
	}

	fDBIndex = info.index + 1;

	addSyncLogEntry(i18n("Backing up: %1").arg(info.name));

	if (!createLocalDatabase(&info))
	{
		kdError() << __FUNCTION__
			<< ": Couldn't create local database for "
			<< info.name << endl;
		addSyncLogEntry(i18n("Backup of %1 failed.").arg(info.name));
	}
	else
	{
		addSyncLogEntry(i18n("OK"));
	}
}

bool BackupAction::createLocalDatabase(DBInfo * info)
{
	FUNCTIONSETUP;

	int j;
	struct pi_file *f;

	QString fullBackupDir =
		fDatabaseDir + fHandle->getPilotUser()->getUserName() + "/";

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Looking in directory " << fullBackupDir << endl;
#endif

	QFileInfo fi(fullBackupDir);

	if (!(fi.exists() && fi.isDir()))
	{
#ifdef DEBUG
		DEBUGDAEMON << fname
			<< ": Need to create backup directory for user "
			<< fHandle->getPilotUser()->getUserName() << endl;
#endif

		fi = QFileInfo(fDatabaseDir);
		if (!(fi.exists() && fi.isDir()))
		{
			kdError() << __FUNCTION__
				<< ": Database backup directory "
				<< "doesn't exist."
				<< endl;
			return false;
		}

		QDir databaseDir(fDatabaseDir);

		if (!databaseDir.mkdir(fullBackupDir, true))
		{
			kdError() << __FUNCTION__
				<< ": Can't create backup directory." << endl;
			return false;
		}
	}

	QString databaseName(info->name);

	databaseName.replace(QRegExp("/"), "_");

	QString fullBackupName = fullBackupDir + databaseName;

	if (info->flags & dlpDBFlagResource)
	{
		fullBackupName.append(".prc");
	}
	else
	{
		fullBackupName.append(".pdb");
	}

#ifdef DEBUG
	DEBUGDB << fname
		<< ": Creating local database " << fullBackupName << endl;
#endif

	/* Ensure that DB-open flag is not kept */
	info->flags &= ~dlpDBFlagOpen;

	// The casts here look funny because:
	//
	// fullBackupName is a QString
	// QFile::encodeName() gives us a QCString
	// which needs an explicit cast to become a const char *
	// which needs a const cast to become a char *
	//
	//
	f = pi_file_create(const_cast < char *>
		((const char *) (QFile::encodeName(fullBackupName))),
		info);

	if (f == 0)
	{
		kdWarning() << __FUNCTION__
			<< ": Failed, unable to create file" << endl;
		return false;
	}

	if (pi_file_retrieve(f, pilotSocket(), 0) < 0)
	{
		kdWarning() << __FUNCTION__
			<< ": Failed, unable to back up database" << endl;

		pi_file_close(f);
		return false;
	}

	pi_file_close(f);
	return true;
}

void BackupAction::endBackup()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fTimer);
	fDBIndex = (-1);
	fStatus = BackupEnded;

	emit syncDone(this);
}

FileInstallAction::FileInstallAction(KPilotDeviceLink * p,
	const QString & d,
	const QStringList & l) :
	SyncAction(p, 0, "fileInstall"),
	fDir(d), 
	fList(l), 
	fDBIndex(-1), 
	fTimer(0L)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGDAEMON << fname << ": File list has " 
		<< fList.  count() << " entries" << endl;

	QStringList::ConstIterator i;

	for (i = fList.begin(); i != fList.end(); ++i)
	{
		DEBUGDAEMON << fname << ": " << *i << endl;
	}
#endif
}

FileInstallAction::~FileInstallAction()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fTimer);
}

/* slot */ void FileInstallAction::exec()
{
	FUNCTIONSETUP;

	fDBIndex = 0;

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Installing " << fList.count() << " files" << endl;
#endif

	// Possibly no files to install?
	if (!fList.count())
	{
		emit logMessage(i18n("No Files to install"));
		emit syncDone(this);

		return;
	}

	fTimer = new QTimer(this);
	QObject::connect(fTimer, SIGNAL(timeout()),
		this, SLOT(installNextFile()));

	fTimer->start(0, false);

	emit logProgress(i18n("Installing Files"), 0);
}

/* slot */ void FileInstallAction::installNextFile()
{
	FUNCTIONSETUP;

	ASSERT(fDBIndex >= 0);
	ASSERT(fDBIndex < fList.count());

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Installing file index "
		<< fDBIndex << " (of " << fList.count() << ")" << endl;
#endif

	if ((!fList.count()) || (fDBIndex >= fList.count()))
	{
#ifdef DEBUG
		DEBUGDAEMON << fname
			<< ": Peculiar file index, bailing out." << endl;
#endif
		if (fTimer)
			fTimer->stop();
		emit syncDone(this);
	}

	const QString s = fDir + fList[fDBIndex];

	fDBIndex++;

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Installing file " << s << endl;
#endif

	if (fDBIndex >= fList.count())
	{
		fDBIndex = (-1);
		fTimer->stop();
		emit logProgress(i18n("Done Installing Files"), 100);
	}
	else
	{
		emit logProgress(QString::null,
			(100 * fDBIndex) / fList.count());
	}


	struct pi_file *f = 0L;

	f = pi_file_open(const_cast <char *>
		((const char *) QFile::encodeName(s)));

	if (!f)
	{
		kdWarning() << __FUNCTION__
			<< ": Unable to open file." << endl;

		emit logError(i18n("Unable to open file &quot;%1&quot;!").
			arg(s));
		goto nextFile;
	}

	if (pi_file_install(f, pilotSocket(), 0) < 0)
	{
		kdWarning() << __FUNCTION__ << ": failed to install." << endl;


		emit logError(i18n("Cannot install file &quot;%1&quot;!").
			arg(s));
	}
	else
	{
		QFile::remove(s);
	}


nextFile:
	if (f) pi_file_close(f);
	if (fDBIndex == -1)
	{
		emit syncDone(this);
	}
}

/* virtual */ QString FileInstallAction::statusString() const
{
	FUNCTIONSETUP;
	if (fDBIndex < 0)
	{
		return QString("Idle");
	}
	else
	{
		if (fDBIndex >= fList.count())
		{
			return QString("Index out of range");
		}
		else
		{
			return QString("Installing %1").arg(fList[fDBIndex]);
		}
	}
}

#if 0
// Something for readConfig of subclasses:


void KPilotDeviceLink::initConduitSocket()
{
	FUNCTIONSETUP;

	fConduitSocket = new KServerSocket(KPILOTLINK_PORT);
	connect(fConduitSocket, SIGNAL(accepted(KSocket *)),
		this, SLOT(slotConduitConnected(KSocket *)));
}

PilotRecord *KPilotDeviceLink::readRecord(KSocket * theSocket)
{
	FUNCTIONSETUP;
	int len, attrib, cat;
	recordid_t uid;
	char *data;
	PilotRecord *newRecord;

	read(theSocket->socket(), &len, sizeof(int));	// REC_DATA tag
	read(theSocket->socket(), &len, sizeof(int));
	read(theSocket->socket(), &attrib, sizeof(int));
	read(theSocket->socket(), &cat, sizeof(int));

	read(theSocket->socket(), &uid, sizeof(recordid_t));
	data = new char[len];

	read(theSocket->socket(), data, len);
	newRecord = new PilotRecord((void *) data, len, attrib, cat, uid);
	delete[]data;
	return newRecord;
}

void KPilotDeviceLink::writeRecord(KSocket * theSocket, PilotRecord * rec)
{
	FUNCTIONSETUP;
	int len = rec->getLen();
	int attrib = rec->getAttrib();
	int cat = rec->getCat();
	recordid_t uid = rec->getID();
	char *data = rec->getData();

	CStatusMessages::write(theSocket->socket(),
		CStatusMessages::REC_DATA);
	write(theSocket->socket(), &len, sizeof(int));
	write(theSocket->socket(), &attrib, sizeof(int));
	write(theSocket->socket(), &cat, sizeof(int));

	write(theSocket->socket(), &uid, sizeof(recordid_t));
	write(theSocket->socket(), data, len);
}

int KPilotDeviceLink::writeResponse(KSocket * k,
	CStatusMessages::LinkMessages m)
{
	FUNCTIONSETUP;
	// I have a bad feeling about using pointers
	// to parameters passed to me, so I'm going 
	// to copy the value parameter to a local (stack)
	// variable and use a pointer to that variable.
	//
	//
	int i = (int) m;

	return write(k->socket(), &i, sizeof(int));
}

void KPilotDeviceLink::slotConduitDone(KProcess * p)
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
			<< fConduitProcess->pid() << endl;
	}
	else
	{
#ifdef DEBUG
		DEBUGDAEMON << fname
			<< ": Conduit process with pid "
			<< p->pid() << " exited" << endl;
#endif
	}

	if (fConduitRunStatus != Done)
	{
		kdWarning() << __FUNCTION__
			<< ": It seems that a conduit has crashed." << endl;

		// Force a resume even though the conduit never ran.
		//
		//
		resumeDB();
	}
}

void KPilotDeviceLink::slotConduitRead(KSocket * cSocket)
{
	FUNCTIONSETUP;
	int message;
	PilotRecord *tmpRec = 0L;

	read(cSocket->socket(), &message, sizeof(int));


	// This one message doesn't require a database to be open.
	//
	//
	if (message == CStatusMessages::LOG_MESSAGE)
	{
		int i;
		char *s;
		read(cSocket->socket(), &i, sizeof(int));
		s = new char[i + 1];

		memset(s, 0, i);
		read(cSocket->socket(), s, i);
		s[i] = 0;
#ifdef DEBUG
		DEBUGDB << fname << ": Message length "
			<< i << " => " << s << endl;
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
			<< ": There is no open database." << endl;

		writeResponse(cSocket, CStatusMessages::NO_SUCH_RECORD);
		return;
	}

	switch (message)
	{
	case CStatusMessages::READ_APP_INFO:
	{
		unsigned char *buf = new unsigned char[BUFSIZ];
		int appLen = fCurrentDB->readAppBlock(buf, BUFSIZ);
		write(cSocket->socket(), &appLen, sizeof(int));

		write(cSocket->socket(), buf, appLen);
		delete buf;

		break;
	}
	case CStatusMessages::WRITE_RECORD:
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
	case CStatusMessages::NEXT_MODIFIED_REC:
	{
		tmpRec = fCurrentDB->readNextModifiedRec();
		if (tmpRec)
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
	case CStatusMessages::NEXT_REC_IN_CAT:
	{
		int cat;
		read(cSocket->socket(), &cat, sizeof(int));

		tmpRec = fCurrentDB->readNextRecInCategory(cat);
		if (tmpRec)
		{
			writeRecord(cSocket, tmpRec);
			delete tmpRec;
		}
		else
			CStatusMessages::write(cSocket->socket(),
				CStatusMessages::NO_SUCH_RECORD);
	}
		break;
	case CStatusMessages::READ_REC_BY_INDEX:
	{
		int index;
		read(cSocket->socket(), &index, sizeof(int));

		tmpRec = fCurrentDB->readRecordByIndex(index);
		if (tmpRec)
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
	case CStatusMessages::READ_REC_BY_ID:
	{
		recordid_t id;

		read(cSocket->socket(), &id, sizeof(recordid_t));
		tmpRec = fCurrentDB->readRecordById(id);
		if (tmpRec)
		{
			writeRecord(cSocket, tmpRec);
			delete tmpRec;
		}
		else
			CStatusMessages::write(cSocket->socket(),
				CStatusMessages::NO_SUCH_RECORD);
	}
		break;
	default:
		kdWarning() << __FUNCTION__ << ": Unknown status message "
			<< message << endl;
	}
}

void KPilotDeviceLink::slotConduitClosed(KSocket * theSocket)
{
	FUNCTIONSETUP;

	if (fConduitRunStatus != Connected)
	{
		kdWarning() << __FUNCTION__
			<< ": Strange -- unconnected conduit closed" << endl;
	}

	if (theSocket != fCurrentConduitSocket)
	{
		kdWarning() << __FUNCTION__
			<< ": Strange -- different socket closed" << endl;
	}

	disconnect(theSocket, SIGNAL(readEvent(KSocket *)),
		this, SLOT(slotConduitRead(KSocket *)));
	disconnect(theSocket, SIGNAL(closeEvent(KSocket *)),
		this, SLOT(slotConduitClosed(KSocket *)));
	delete theSocket;

	fConduitRunStatus = Done;
	fCurrentConduitSocket = 0L;
	resumeDB();
}

void KPilotDeviceLink::resumeDB()
{
	FUNCTIONSETUP;
	if (!fCurrentDB)
	{
		kdWarning() << __FUNCTION__
			<< ": resumeDB called after the end of a sync."
			<< endl;
		return;
	}

	if (fCurrentDB)
		delete fCurrentDB;

	fCurrentDB = 0L;

	// Get our backup copy.
	if (slowSyncRequired())	// We are in the middle of backing up, continue
	{
		doConduitBackup();
	}
	else			// We are just doing a normal sync, so go for it.
	{
		syncDatabase(&fCurrentDBInfo);
		// Start up the next one
		syncNextDB();
	}
}

QString KPilotDeviceLink::registeredConduit(const QString & dbName) const
{
	FUNCTIONSETUP;

	KConfig & config = KPilotConfig::getConfig("Database Names");

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Looking for " << dbName << endl;
#endif

	QString result = config.readEntry(dbName);

	if (result.isNull())
	{
#ifdef DEBUG
		DEBUGDAEMON << fname << ": Not found." << endl;
#endif

		return result;
	}

	config.setGroup("Conduit Names");
	QStringList installed = config.readListEntry("InstalledConduits");

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Found conduit " << result << endl;
	DEBUGDAEMON << fname << ": Installed Conduits are" << endl;

	kdbgstream s = kdDebug(DAEMON_AREA);

	listStrList(s, installed);
#endif

	if (!installed.contains(result))
	{
#ifdef DEBUG
		DEBUGDAEMON << fname << ": Conduit not installed." << endl;
#endif
		return QString::null;
	}

	KService::Ptr conduit = KService::serviceByDesktopName(result);
	if (!conduit)
	{
#ifdef DEBUG
		DEBUGDAEMON << fname << ": No service for this conduit" <<
			endl;
#endif
		return QString::null;
	}
	else
	{
#ifdef DEBUG
		DEBUGDAEMON << fname << ": Found service with exec="
			<< conduit->exec() << endl;
#endif
		return conduit->exec();
	}

	// NOTREACHED
	return QString::null;
}


void KPilotDeviceLink::slotConduitConnected(KSocket * theSocket)
{
	FUNCTIONSETUP;

	connect(theSocket, SIGNAL(readEvent(KSocket *)),
		this, SLOT(slotConduitRead(KSocket *)));
	connect(theSocket, SIGNAL(closeEvent(KSocket *)),
		this, SLOT(slotConduitClosed(KSocket *)));
	theSocket->enableRead(true);

	fConduitRunStatus = Connected;
	fCurrentConduitSocket = theSocket;
}

// Requires the text is displayed in item 0
void KPilotDeviceLink::showMessage(const QString & message)
{
	FUNCTIONSETUP;
	emit logMessage(message);
}

int KPilotDeviceLink::compare(struct db *d1, struct db *d2)
{
	FUNCTIONSETUP;
	/* types of 'appl' sort later than other types */
	if (d1->creator == d2->creator)
		if (d1->type != d2->type)
		{
			if (d1->type == pi_mktag('a', 'p', 'p', 'l'))
				return 1;
			if (d2->type == pi_mktag('a', 'p', 'p', 'l'))
				return -1;
		}
	return d1->maxblock < d2->maxblock;
}

bool KPilotDeviceLink::doFullRestore()
{
	FUNCTIONSETUP;

	struct DBInfo info;
	struct db *db[256];
	int dbcount = 0;
	int i, j, max, size;
	struct pi_file *f;
	char message[256];

	i = KMessageBox::questionYesNo(0L,
		i18n("Replace all data on Pilot with local data?"),
		i18n("Full Restore"));
	if (i != KMessageBox::Yes)
		return false;

	// User said yes, so do it.
	//
	//
	QString dirname = KGlobal::dirs()->saveLocation("data",
		QString("kpilot/DBBackup/") +
		getPilotUser()->getUserName() + "/");
	QStringList dbList;

	// Block for local vars.
	//
	{
		QDir dir(dirname);

		if (!dir.exists())
		{
			kdWarning() << __FUNCTION__
				<< ": Save directory "
				<< dirname << " doesn't exist." << endl;
			return false;
		}
		dbList = dir.entryList();
	}


	QStringList::ConstIterator it;

	for (it = dbList.begin(); it != dbList.end(); ++it)
	{
		if ((*it == "..") || (*it == "."))
			continue;

#ifdef DEBUG
		DEBUGKPILOT << fname << ": Trying database " << *it << endl;
#endif

		db[dbcount] = (struct db *) malloc(sizeof(struct db));

		sprintf(db[dbcount]->name, QFile::encodeName(dirname + *it));

		f = pi_file_open(db[dbcount]->name);
		if (f == 0)
		{
			kdWarning() << __FUNCTION__
				<< ": Unable to open " << *it << endl;
			continue;
		}

		pi_file_get_info(f, &info);

		db[dbcount]->creator = info.creator;
		db[dbcount]->type = info.type;
		db[dbcount]->flags = info.flags;
		db[dbcount]->maxblock = 0;

		pi_file_get_entries(f, &max);

		for (int dbi = 0; dbi < max; dbi++)
		{
			if (info.flags & dlpDBFlagResource)
			{
				pi_file_read_resource(f, dbi, 0, &size, 0, 0);
			}
			else
			{
				pi_file_read_record(f, dbi, 0, &size, 0, 0,
					0);
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
	for (i = 0; i < dbcount; i++)
		for (j = i + 1; j < dbcount; j++)
			if (compare(db[i], db[j]) > 0)
			{
				struct db *temp = db[i];

				db[i] = db[j];
				db[j] = temp;
			}

	for (i = 0; i < dbcount; i++)
	{

		if (dlp_OpenConduit(getCurrentPilotSocket()) < 0)
		{
			kdWarning() << __FUNCTION__ << ": Exiting on cancel. "
				"All data not restored." << endl;
			return false;
		}
		showMessage(i18n("Restoring databases to Palm Pilot. "
				"Slow sync required."));
		addSyncLogEntry("Restoring all data...");

		f = pi_file_open(db[i]->name);
		if (f == 0)
		{
			printf("Unable to open '%s'!\n", db[i]->name);
			break;
		}
		strcpy(message, "Syncing: ");
		strcat(message, strrchr(db[i]->name, '/') + 1);

		//        printf("Restoring %s... ", db[i]->name);
		//        fflush(stdout);
		if (pi_file_install(f, getCurrentPilotSocket(), 0) < 0)
			printf("Hmm..prefs file..\n");
		//        else
		//            printf("OK\n");
		pi_file_close(f);
	}
	//     printf("Restore done\n");
	addSyncLogEntry("OK.\n");
	finishDatabaseSync();
	return true;
}

void KPilotDeviceLink::finishDatabaseSync()
{
	FUNCTIONSETUP;
	// Since we're done with the DB anyway
	// we can forget it. This also flags 
	// for resumeDB not to do anything
	// stupid :)
	//
	//
	fCurrentDB = 0;
	emit(databaseSyncComplete());
}



void KPilotDeviceLink::doFullBackup()
{
	FUNCTIONSETUP;
	int i = 0;

	setSlowSyncRequired(true);

	showMessage(i18n("Backing up Pilot..."));
	addSyncLogEntry(i18n("Backing up all data...").latin1());

	for (;;)
	{
		struct DBInfo info;

		if (dlp_OpenConduit(getCurrentPilotSocket()) < 0)
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

			KMessageBox::sorry(0L,
				i18n("Exiting on cancel.\n"
					"<B>Not</B> all the data was backed up."),
				i18n("Backup"));
			addSyncLogEntry("FAILED.\n");
			return;
		}

		// Is parameter i important here???
		//
		//
		if (dlp_ReadDBList(getCurrentPilotSocket(),
				0, 0x80, i, &info) < 0)
		{
#ifdef DEBUG
			DEBUGKPILOT << fname
				<< ": Last database encountered." << endl;
#endif
			break;
		}
		i = info.index + 1;

		{
			QString logmsg(i18n("Backing Up: "));

			logmsg.append(info.name);
		}

		if (createLocalDatabase(&info) == false)
		{
			kdError() << __FUNCTION__
				<< ": Couldn't create local database for "
				<< info.name << endl;

			KMessageBox::error(0L,
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

void KPilotDeviceLink::installFiles(const QString & path)
{
	FUNCTIONSETUP;

	struct pi_file *f;
	int fileNum = 0;
	QDir installDir(path);
	QString actualPath = installDir.path() + "/";

	QStringList fileNameList = installDir.entryList(QDir::Files);

	QValueListIterator < QString > fileIter(fileNameList.begin());
	if (fileIter == fileNameList.end())
		return;

	emit logProgress(i18n("Installing Files"), 0);

	if (getConnected() == false)
	{
		kdWarning() << __FUNCTION__ << ": No HotSync started!" <<
			endl;
		return;
	}

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Installing from directory "
		<< actualPath << endl;
#endif

	while (fileIter != fileNameList.end())
	{
#ifdef DEBUG
		DEBUGDAEMON << fname << ": Installing file "
			<< *fileIter << endl;
#endif

		emit logProgress(QString::null,
			(100 / fileNameList.count()) * fileNum);

		// Block to isolate extra QString
		//
		//
		{
			QString fullPath(actualPath);

			fullPath += *fileIter;

			// Yuckyness to avoid warning when
			// passing QString to pi library.
			//
			//
			f = pi_file_open((char *) fullPath.latin1());
		}

		if (f == 0)
		{
			kdWarning() << __FUNCTION__ <<
				": Unable to open file." << endl;

			QString message;

			message =
				i18n("Unable to open file &quot;%1&quot;!").
				arg(*fileIter);
			KMessageBox::error(0L, message, i18n("Missing File"));
		}
		else
		{
			if (pi_file_install(f, getCurrentPilotSocket(),
					0) < 0)
			{
				kdWarning() << __FUNCTION__ <<
					": failed to install." << endl;
				KMessageBox::error(0L,
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
}




void KPilotDeviceLink::quickHotSync()
{
	FUNCTIONSETUP;

	syncNextDB();
}

void KPilotDeviceLink::doConduitBackup()
{
	FUNCTIONSETUP;
	QString displaymessage;

	struct DBInfo info;

	do
	{
		if (dlp_ReadDBList(getCurrentPilotSocket(), 0, 0x80,
				fNextDBIndex, &info) < 0)
		{
			setSlowSyncRequired(false);
			finishDatabaseSync();
			return;
		}
		fNextDBIndex = info.index + 1;
	}
	while (info.flags & dlpDBFlagResource);

	QString conduitName = registeredConduit(info.name);

	while (conduitName.isNull())
	{
		do
		{
			if (dlp_ReadDBList(getCurrentPilotSocket(), 0, 0x80,
					fNextDBIndex, &info) < 0)
			{
				setSlowSyncRequired(false);
				finishDatabaseSync();
				return;
			}
			fNextDBIndex = info.index + 1;
		}
		while (info.flags & dlpDBFlagResource);
		conduitName = registeredConduit(info.name);
	}

	// Fire up the conduit responsible for this db and when it's finished
	// we'll get called again.
	displaymessage = i18n("%1: Running conduit").arg(info.name);
	fCurrentDBInfo = info;
	fCurrentDB =
		new PilotSerialDatabase(getCurrentPilotSocket(), info.name);
	fCurrentDB->resetDBIndex();
	if (fConduitProcess->isRunning())
	{
		kdWarning() << __FUNCTION__ <<
			": Waiting for conduit to die.. " << endl;
	}
	// Eek! Busy waiting w/no event loop?
	// Well, some kind of event loop now,
	// but it's still kinda dodgy.
	//
	//
	while (fConduitProcess->isRunning())
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
	connect(fConduitProcess, SIGNAL(processExited(KProcess *)),
		this, SLOT(slotConduitDone(KProcess *)));
	fConduitRunStatus = Running;
	fConduitProcess->start(KProcess::NotifyOnExit);
}

int KPilotDeviceLink::findNextDB(DBInfo * info)
{
	FUNCTIONSETUP;

	do
	{
		if (dlp_ReadDBList(getCurrentPilotSocket(), 0, 0x80,
				fNextDBIndex, info) < 0)
		{
			setSlowSyncRequired(false);
			KConfig & config = KPilotConfig::getConfig();
			setFastSyncRequired(config.
				readBoolEntry("PreferFastSync", false));
			finishDatabaseSync();
			return 0;
		}
		fNextDBIndex = info->index + 1;
	}
	while (info->flags & dlpDBFlagResource);

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Found database with:\n"
		<< fname << ": Index=" << fNextDBIndex << endl;
#endif

	return 1;
}

int KPilotDeviceLink::findDisposition(const QString & dbList,
	const struct DBInfo *currentDB)
{
	FUNCTIONSETUP;
	char *m = printlong(currentDB->creator);
	int r = dbList.find(m);

	if (r == 0 || (r > 0 && dbList[r - 1] == ','))
		return 1;
	return 0;
}

void KPilotDeviceLink::syncNextDB()
{
	FUNCTIONSETUP;

	QString message;
	QString skip;
	QString backupOnly;
	DBInfo info;

	// Confine config reads to a local block
	{
		KConfig & c = KPilotConfig::getConfig();
		skip = c.readEntry("SkipSync");
		backupOnly = c.readEntry("BackupForSync");
	}

#ifdef DEBUG
	DEBUGDB << fname << ": Special dispositions are: \n"
		<< fname << ": * BackupOnly=" << backupOnly << endl
		<< fname << ": * Skip=" << skip << endl;
#endif
	if (!findNextDB(&info))
		return;

#ifdef DEBUG
	DEBUGDB << fname << ": Syncing " << info.name << endl;
#endif



	QString conduitName = registeredConduit(info.name);

	while (conduitName.isNull())
	{
#ifdef DEBUG
		DEBUGDB << fname << ": No registered conduit for "
			<< info.name << endl;
#endif

		// If we want a FastSync, skip all DBs without conduits.
		//
		if (fFastSyncRequired)
		{
#ifdef DEBUG
			DEBUGKPILOT << fname
				<< ": Skipping database "
				<< info.name << " during FastSync." << endl;
#endif
			goto nextDB;
		}

		message = i18n("Syncing: %1 ...").arg(info.name);
		addSyncLogEntry(message.local8Bit());

		// Find out if this database has a special disposition
		//
		//
#ifdef DEBUG
		{
			char *m = printlong(info.creator);

#ifdef DEBUG
			DEBUGDB << fname << ": Looking for disposition of "
				<< m << endl;
#endif
		}
#endif
		if (findDisposition(skip, &info))
			goto nextDB;
		if (findDisposition(backupOnly, &info) && !fFastSyncRequired)
		{
			if (!createLocalDatabase(&info))
			{
				QString message(i18n("Could not backup data "
						"for database &quot;%1&quot;").
					arg(info.name));

				KMessageBox::error(0L, message,
					i18n("Backup for Sync"));
			}
			goto nextDB;
		}

		if (syncDatabase(&info))
		{
#ifdef DEBUG
			DEBUGDB << fname << ": Sync OK" << endl;
#endif
			addSyncLogEntry("OK.\n");
		}
		else
		{
			kdWarning() << __FUNCTION__ << ": Sync "
				<< info.name << " failed." << endl;

			addSyncLogEntry("FAILED!\n");
		}

	      nextDB:
		if (!findNextDB(&info))
			return;

		conduitName = registeredConduit(info.name);
#ifdef DEBUG
		DEBUGDB << fname << ": Syncing " << info.name << endl;
#endif
	}

	// Fire up the conduit responsible for this db and when it's finished
	// we'll get called again.
	message = i18n("%1: Running conduit").arg(info.name);
	addSyncLogEntry(message.local8Bit());
	fCurrentDBInfo = info;

#ifdef DEBUG
	DEBUGDB << fname << ": " << message << endl;
#endif


	fCurrentDB =
		new PilotSerialDatabase(getCurrentPilotSocket(), info.name);
	fCurrentDB->resetDBIndex();
	if (fConduitProcess->isRunning())
	{
		kdWarning() << __FUNCTION__ <<
			": Waiting for conduit to die.. " << endl;
	}

	// This is busy waiting, but make sure that
	// (Qt) signals do get delivered and the
	// display is maintained.
	//
	while (fConduitProcess->isRunning())
	{
		sleep(1);
		kapp->processEvents();
	}

	fConduitProcess->clearArguments();
	*fConduitProcess << conduitName;
	*fConduitProcess << "--hotsync";
#ifdef DEBUG
	if (debug_level)
	{
		*fConduitProcess << "--debug";
		*fConduitProcess << QString::number(debug_level);
	}
#endif
	connect(fConduitProcess, SIGNAL(processExited(KProcess *)),
		this, SLOT(slotConduitDone(KProcess *)));
	fConduitRunStatus = Running;
	fConduitProcess->start(KProcess::NotifyOnExit);

}

bool KPilotDeviceLink::syncDatabase(DBInfo * database)
{
	FUNCTIONSETUP;

	unsigned char buffer[0xffff];

	PilotDatabase *firstDB;
	PilotDatabase *secondDB;
	PilotRecord *pilotRec;

	QString currentDBPath = fDatabaseDir +
		getPilotUser()->getUserName() + "/";

	firstDB =
		new PilotSerialDatabase(getCurrentPilotSocket(),
		database->name);
	secondDB = new PilotLocalDatabase(currentDBPath, database->name);

	if (firstDB->isDBOpen() && (secondDB->isDBOpen() == false))
	{
		// Must be a new Database...
		showMessage(i18n
			("No previous copy.  Copying data from pilot..."));

		delete firstDB;
		delete secondDB;

		firstDB = secondDB = 0L;

		if (createLocalDatabase(database) == false)
		{
			KMessageBox::error(0L,
				i18n
				("Could not create local copy of database "
					"&quot;%1&quot;").arg(database->name),
				i18n("Backup"));

			// Why continue here? The database isn't open, so
			// we'll just get another error message shortly.
			//
			//
			return false;
		}

		firstDB =
			new PilotSerialDatabase(getCurrentPilotSocket(),
			database->name);
		secondDB =
			new PilotLocalDatabase(currentDBPath, database->name);

		showMessage(i18n
			("Hot-Syncing Pilot. Looking for modified data..."));
	}

	if ((secondDB->isDBOpen() == false) || (firstDB->isDBOpen() == false))
	{
		delete firstDB;
		delete secondDB;

		firstDB = secondDB = 0L;

		QString message(i18n("Cannot find database &quot;%1&quot;").
			arg(database->name));

		KMessageBox::error(0L, message,
			i18n("Error Syncing Database"));
		return false;
	}


	// Move this functionality into mode ...
	//
	//
	KConfig & config = KPilotConfig::getConfig();
	config.setGroup(QString());
	// If local changes should modify pilot changes, switch the order.
	int localOverride = config.readNumEntry("OverwriteRemote");

	if (localOverride)
	{
		PilotDatabase *tmp = firstDB;

		firstDB = secondDB;
		secondDB = tmp;
	}

	int len = firstDB->readAppBlock(buffer, 0xffff);

	if (len > 0)
	{
		secondDB->writeAppBlock(buffer, len);
	}

	firstDB->resetDBIndex();
	while ((pilotRec = firstDB->readNextModifiedRec()) != 0L)
	{
		secondDB->writeRecord(pilotRec);
		firstDB->writeID(pilotRec);
		delete pilotRec;
	}

	secondDB->resetDBIndex();
	while ((pilotRec = secondDB->readNextModifiedRec()) != 0L)
	{
		firstDB->writeRecord(pilotRec);
		secondDB->writeID(pilotRec);
		delete pilotRec;
	}

	firstDB->resetSyncFlags();
	firstDB->cleanUpDatabase();	// Purge deleted entries
	secondDB->resetSyncFlags();
	secondDB->cleanUpDatabase();
	delete firstDB;
	delete secondDB;

	firstDB = secondDB = 0L;
	return true;
}




#endif

void CleanupAction::exec()
{
	FUNCTIONSETUP;

	fHandle->getPilotUser()->setLastSyncPC((unsigned long) gethostid());
	fHandle->getPilotUser()->setLastSyncDate(time(0));

	dlp_WriteUserInfo(pilotSocket(),
		fHandle->getPilotUser()->pilotUser());
	addSyncLogEntry("End of Hot-Sync\n");
	dlp_EndOfSync(pilotSocket(), 0);

	emit syncDone(this);
}


// $Log$
// Revision 1.7  2001/09/29 16:26:18  adridg
// The big layout change
//
// Revision 1.6  2001/09/24 22:17:41  adridg
// () Removed lots of commented out code from previous incarnations.
// () Added a cleanup action.
// () Removed a heap-corruption bug caused by using QStringList & and
//    then deleting what it points to in FileInstallAction.
// () Removed deadlock when last file to install couldn't be read.
// () Moved RestoreAction to interactiveSync.{h,cc}, since I feel it
//    needs to ask "Are you sure?" at the very least.
//
