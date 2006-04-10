//Added by qt3to4:
#include <Q3CString>
/* KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org.
*/

static const char *hotsync_id =
	"$Id$";

#include "options.h"

#include <time.h>
#include <unistd.h>
#include <stdio.h>

#include <pi-file.h>

#include <qtimer.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <q3valuelist.h>
#include <qregexp.h>
#include <qtextcodec.h>
#include <qstringlist.h>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kapplication.h>

#include "pilotUser.h"
#include "pilotAppCategory.h"
#include "syncStack.h"
#include "pilotSerialDatabase.h"
#include "pilotLocalDatabase.h"
#include "pilotDatabase.h"
#include "kpilotSettings.h"

#include "hotSync.moc"

TestLink::TestLink(KPilotDeviceLink * p) :
	SyncAction(p, "testLink")
{
	FUNCTIONSETUP;

	(void) hotsync_id;
}

/* virtual */ bool TestLink::exec()
{
	FUNCTIONSETUP;

	int i;
	int dbindex = 0;
	int count = 0;
	struct DBInfo db;

	addSyncLogEntry(i18n("Testing.\n"));

#ifdef BRUTE_FORCE
	for (i=0; i<32; i++)
#else
	while ((i = fHandle->getNextDatabase(dbindex,&db)) > 0)
#endif
	{
#ifdef BRUTE_FORCE
		if (fHandle->getNextDatabase(i,&db) < 1)
		{
			DEBUGCONDUIT << fname << ": No database index " << i << endl;
			continue;
		}
#endif

		count++;
		dbindex = db.index + 1;

#ifdef DEBUG
		DEBUGCONDUIT << fname << ": Read database " << db.name << endl;
#endif

		// Let the Pilot User know what's happening
		openConduit();
		// Let the KDE User know what's happening
		// Pretty sure all database names are in latin1.
		emit logMessage(i18n("Syncing database %1...",
			 PilotAppCategory::codec()->toUnicode(db.name)));

		kapp->processEvents();
	}

	emit logMessage(i18n("HotSync finished."));
	emit syncDone(this);
	return true;
}

BackupAction::BackupAction(KPilotDeviceLink * p, bool full) :
	SyncAction(p, "backupAction"),
	fFullBackup(full)
{
	FUNCTIONSETUP;

	fDatabaseDir = KGlobal::dirs()->saveLocation("data",
		CSL1("kpilot/DBBackup/"));

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Will write to " << fDatabaseDir << endl;
	DEBUGCONDUIT << fname << ": Full sync? " << full << endl;
#endif
}

/* virtual */ QString BackupAction::statusString() const
{
	FUNCTIONSETUP;
	QString s(CSL1("BackupAction="));

	switch (status())
	{
	case Init:
		s.append(CSL1("Init"));
		break;
	case Error:
		s.append(CSL1("Error"));
		break;
	case FullBackup:
		s.append(CSL1("FullBackup"));
		break;
	case FastBackup:
		s.append(CSL1("FastBackup"));
		break;
	case BackupEnded:
		s.append(CSL1("BackupEnded"));
		break;
	case BackupIncomplete:
		s.append(CSL1("BackupIncomplete"));
		break;
	case BackupComplete:
		s.append(CSL1("BackupComplete"));
		break;
	default:
		s.append(CSL1("(unknown "));
		s.append(QString::number(status()));
		s.append(CSL1(")"));
	}

	return s;
}

static inline bool dontBackup(struct DBInfo *info,
	const QStringList &dbnames,
	const Q3ValueList<unsigned long> &dbcreators)
{
	// Special case - skip database Unsaved Preferences
	if (   (info->creator == pi_mktag('p','s','y','s'))  &&
		(info->type == pi_mktag('p','r','e','f')) ) return true;

	if (dbcreators.findIndex(info->creator) != -1) return true;

	// Now take wildcards into account
	QString db = PilotAppCategory::codec()->toUnicode(info->name);
	for (QStringList::const_iterator i = dbnames.begin(); i != dbnames.end(); ++i)
	{
		QRegExp re(*i,true,true); // Wildcard match
		if (re.exactMatch(db)) return true;
	}
	return false;
}

static inline void initNoBackup(QStringList &dbnames,
	Q3ValueList<unsigned long> &dbcreators)
{
	FUNCTIONSETUP;
	dbnames.clear();
	dbcreators.clear();

	QStringList configuredSkip = KPilotSettings::skipBackupDB();
	QStringList::const_iterator e = configuredSkip.end();
	for (QStringList::const_iterator i = configuredSkip.begin();
		i!= e; ++i)
	{
		QString s = *i;
		if (s.startsWith(CSL1("[")) && s.endsWith(CSL1("]")))
		{
			if (s.length() != 6)
			{
				kWarning() << k_funcinfo << ": Creator ID " << s << " is malformed." << endl;
			}
			else
			{
				Q3CString data =  s.mid(1,4).toLatin1();
				unsigned long creator = pi_mktag(data[0],data[1],data[2],data[3]);
				dbcreators.append(creator);
			}
		}
		else
		{
			dbnames.append(s);
		}
	}

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Will skip databases "
		<< dbnames.join(CSL1(",")) << endl;
	QString creatorids;
	for (Q3ValueList<unsigned long>::const_iterator i = dbcreators.begin();
		i != dbcreators.end(); ++i)
	{
		creatorids.append(CSL1("[%1]").arg(*i,0,16));
	}
	DEBUGCONDUIT << fname << ": Will skip creators " << creatorids << endl;
#endif
}

/* virtual */ bool BackupAction::exec()
{
	FUNCTIONSETUP;

	mDeviceDBs = KPilotSettings::deviceDBs();

	fBackupDir =
		fDatabaseDir +
		PilotAppCategory::codec()->toUnicode(fHandle->getPilotUser()->getUserName()) +
		CSL1("/");

	logMessage(i18n("Backup directory: %1.", fBackupDir));

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": This Pilot user's name is \""
		<< fHandle->getPilotUser()->getUserName() << "\"" << endl;
	DEBUGCONDUIT << fname
		<< ": Using backup dir: " << fBackupDir << endl;
	DEBUGCONDUIT << fname
		<< ": Full Backup? " << fFullBackup << endl;
#endif


	if (fFullBackup)
	{
		fActionStatus = FullBackup;
		addSyncLogEntry(i18n("Full backup started."));
	}
	else
	{
		fActionStatus = FastBackup;
		addSyncLogEntry(i18n("Fast backup started"));
	}

	if (!checkBackupDirectory(fBackupDir))
	{
		fActionStatus=BackupIncomplete;
		// Don't issue an error message, checkBackupDirectory
		// did this already...
		return false;
	}

	initNoBackup( fNoBackupDBs, fNoBackupCreators );

	fTimer = new QTimer( this );
	QObject::connect( fTimer, SIGNAL( timeout() ),
		this, SLOT( backupOneDB() ) );

	fDBIndex = 0;

	fTimer->start( 0, false );
	return true;
}

bool BackupAction::checkBackupDirectory(QString backupDir)
{
	FUNCTIONSETUP;
	QFileInfo fi(backupDir);

	if (!(fi.exists() && fi.isDir()))
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< ": Need to create backup directory for user "
			<< fHandle->getPilotUser()->getUserName() << endl;
#endif

		fi = QFileInfo(fDatabaseDir);
		if (!(fi.exists() && fi.isDir()))
		{
			kError() << k_funcinfo
				<< ": Database backup directory "
				<< "doesn't exist."
				<< endl;
			return false;
		}

		QDir databaseDir(backupDir);

		if (!databaseDir.mkdir(backupDir, true))
		{
			kError() << k_funcinfo
				<< ": Can't create backup directory." << endl;
			return false;
		}
	}
	return true;
}

/* slot */ void BackupAction::backupOneDB()
{
	FUNCTIONSETUP;

	struct DBInfo info;

	emit logProgress(QString::null, fDBIndex);

	if (openConduit() < 0)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< ": openConduit failed. User cancel?" << endl;
#endif

		addSyncLogEntry(i18n("Exiting on cancel."));
		endBackup();
		fActionStatus = BackupIncomplete;
		return;
	}

	// TODO: Is there a way to skip unchanged databases?
	int res = fHandle->getNextDatabase( fDBIndex, &info );
	if (res < 0)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": Backup complete." << endl;
#endif

		if ( fFullBackup )
			addSyncLogEntry( i18n("Full backup complete.") );
		else
			addSyncLogEntry( i18n("Fast backup complete.") );
		endBackup();
		fActionStatus = BackupComplete;
		return;
	}

	fDBIndex = info.index + 1;

	char buff[8];
	memset(buff, 0, 8);
	buff[0] = '[';
	set_long( &buff[1], info.creator );
	buff[5] = ']';
	buff[6] = '\0';
	QString creator = QString::fromLatin1( buff );
	info.name[33]='\0';
	QString dbname = QString::fromLatin1( info.name );
	if ( !mDeviceDBs.contains( creator ) )
		mDeviceDBs << creator;
	if ( !mDeviceDBs.contains( dbname ) )
		mDeviceDBs << dbname;


#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Checking to see if we should backup database " << info.name
		<< " [" << QString::number(info.creator,16) << "]" << endl;
#endif

	// see if user told us not to back this creator or database up...
	if (dontBackup(&info,fNoBackupDBs,fNoBackupCreators))
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": Skipping database " << info.name
			<< " (database in no-backup list)" << endl;
#endif
		QString s = i18n("Skipping %1",
			 PilotAppCategory::codec()->toUnicode(info.name));
		addSyncLogEntry(s);
		return;
	}

	// don't backup resource databases...
	if ( (!fFullBackup) && PilotDatabase::isResource(&info))
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": Skipping database " << info.name
			<< " (resource database)" << endl;
#endif
		// Just skip resource DBs during an update hotsync.
		return;
	}

	QString s = i18n("Backing up: %1",
		 PilotAppCategory::codec()->toUnicode(info.name));
	addSyncLogEntry(s);

	if (!createLocalDatabase(&info))
	{
		kError() << k_funcinfo
			<< ": Couldn't create local database for "
			<< info.name << endl;
		addSyncLogEntry(i18n("Backup of %1 failed.\n",
			 PilotAppCategory::codec()->toUnicode(info.name)));
	}
	else
	{
		addSyncLogEntry(i18n(" .. OK\n"),false); // Not in kpilot log.
	}
}

/**
 * This method will back up a single database from the Pilot to a directory on
 * our filesystem.  If our user asks us to do a full backup, then we will unconditionally
 * copy the database file from the Pilot into the backup directory.  Otherwise, we will
 * check to see if the database has any modified records in it on the pilot.  If the
 * database has not changed on the Pilot, then there's nothing to backup and we return.
 * If we do backup the database from the Pilot to the filesystem, we will reset the
 * dirty flags so that we won't unnecessarily backup this database again.
 */
bool BackupAction::createLocalDatabase(DBInfo * info)
{
	FUNCTIONSETUP;

	QString databaseName(PilotAppCategory::codec()->toUnicode(info->name));
	// default this to true.  we will set it to false if there are no modified
	// records and we are not doing a full sync.
	bool doBackup = true;

	// make sure that our directory is available...
	if (!checkBackupDirectory(fBackupDir)) return false;

	// we always need to open the database on the pilot because if we're going to sync
	// it, we need to reset the "dirty" flags, etc.
	PilotSerialDatabase*serial=new PilotSerialDatabase(pilotSocket(), databaseName);
	if (!serial->isDBOpen())
	{
#ifdef DEBUG
		DEBUGCONDUIT<<"Unable to open database "<<info->name<<" to check for modified records and reset sync flags."<<endl;
#endif
	}

	// now we look to see if the database on the pilot has at least one changed record
	// in it.  we do this so that we don't waste time backing up a database that has
	// not changed.  note: don't bother with this check if we're doing a full backup.
	if (!fFullBackup && serial->isDBOpen())
	{
		int index=0;
		PilotRecord*rec=serial->readNextModifiedRec(&index);
		if (!rec)
		{
			doBackup = false;
		}
		KPILOT_DELETE(rec);
	}

	// close this database with the Pilot so we can back it up to the
	// filesystem if necessary
	KPILOT_DELETE(serial);

	// if we don't need to do a backup for this database, clean up and
	// return true (our work here is done)
	if (!doBackup)
	{
#ifdef DEBUG
		DEBUGDB << fname
			<< ": don't need to backup this database (no changes)." << endl;
#endif
		return true;
	}

	// if we're here then we are going to back this database up.  do some basic sanity
	// checks and proceed....
	databaseName.replace('/', CSL1("_"));

	QString fullBackupName = fBackupDir + databaseName;

	if (PilotDatabase::isResource(info))
	{
		fullBackupName.append(CSL1(".prc"));
	}
	else
	{
		fullBackupName.append(CSL1(".pdb"));
	}

#ifdef DEBUG
	DEBUGDB << fname
		<< ": Backing up database to: [" << fullBackupName << "]" << endl;
#endif

	/* Ensure that DB-open flag is not kept */
	info->flags &= ~dlpDBFlagOpen;

	bool backedUp = fHandle->retrieveDatabase(fullBackupName,info);

	// if we've backed this one up, clean it up so we won't do it again next
	// sync unless it's truly changed
	serial=new PilotSerialDatabase(pilotSocket(), databaseName);
	if (backedUp && serial->isDBOpen())
	{
		serial->cleanup();
		serial->resetSyncFlags();
	}
	KPILOT_DELETE(serial);

	return backedUp;
}

void BackupAction::endBackup()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fTimer);
	fDBIndex = (-1);
	fActionStatus = BackupEnded;
	mDeviceDBs.sort();
	QString old( QString::null );
	QStringList::Iterator itr = mDeviceDBs.begin();
	while ( itr != mDeviceDBs.end() ) {
		if ( old == *itr ) {
			itr = mDeviceDBs.remove( itr );
		} else {
			old = *itr;
			++itr;
		}
	}
	KPilotSettings::setDeviceDBs( mDeviceDBs );

	emit syncDone(this);
}

FileInstallAction::FileInstallAction(KPilotDeviceLink * p,
	const QString & d) :
	SyncAction(p, "fileInstall"),
	fDBIndex(-1),
	fTimer(0L),
	fDir(d)
{
	FUNCTIONSETUP;
}

FileInstallAction::~FileInstallAction()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fTimer);
}

/* virtual */ bool FileInstallAction::exec()
{
	FUNCTIONSETUP;

	QDir installDir(fDir);
	fList = installDir.entryList(QDir::Files |
		QDir::NoSymLinks | QDir::Readable);
#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Installing " << fList.count() << " files" << endl;
#endif

	fDBIndex = 0;
	emit logMessage(i18n("[File Installer]"));

	// Possibly no files to install?
	if (!fList.count())
	{
		emit logMessage(i18n("No Files to install"));
		delayDone();
		return true;
	}

	fTimer = new QTimer(this);
	QObject::connect(fTimer, SIGNAL(timeout()),
		this, SLOT(installNextFile()));

	fTimer->start(0, false);

	emit logProgress(i18np("Installing one file",
		"Installing %n Files",fList.count()), 0);
	return true;
}

/* slot */ void FileInstallAction::installNextFile()
{
	FUNCTIONSETUP;

	Q_ASSERT(fDBIndex >= 0);
	Q_ASSERT( fDBIndex <= fList.count());

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Installing file index "
		<< fDBIndex << " (of " << fList.count() << ")" << endl;
#endif

	if ((!fList.count()) || ( fDBIndex >= fList.count()))
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< ": Peculiar file index, bailing out." << endl;
#endif
		KPILOT_DELETE(fTimer);
		fDBIndex = (-1);
		emit logProgress(i18n("Done Installing Files"), 100);
		delayDone();
		return;
	}

	const QString filePath = fDir + fList[fDBIndex];
	const QString fileName = fList[fDBIndex];

	fDBIndex++;

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Installing file " << filePath << endl;
#endif

	QString m = i18n("Installing %1", fileName);
	emit logProgress(m,(100 * fDBIndex) / (fList.count()+1));
	m+=CSL1("\n");
	emit addSyncLogEntry(m,false /* Don't print in KPilot's log. */ );

	struct pi_file *f = 0L;

	// Check DB is ok, return false after warning user
	if (!resourceOK(fileName,filePath)) goto nextFile;

	f = pi_file_open(const_cast <char *>
		((const char *) QFile::encodeName(filePath)));


#if PILOT_LINK_NUMBER < PILOT_LINK_0_12_0
	if (pi_file_install(f, pilotSocket(), 0) < 0)
#else
	if (pi_file_install(f, pilotSocket(), 0, NULL) < 0)
#endif
	{
		kWarning() << k_funcinfo << ": failed to install." << endl;


		emit logError(i18n("Cannot install file &quot;%1&quot;.", 
			fileName));
	}
	else
	{
		QFile::remove(filePath);
	}


nextFile:
	if (f) pi_file_close(f);
	if (fDBIndex == -1)
	{
		fTimer->stop();
		delayDone();
		// emit syncDone(this);
	}
}

// Check that the given file path is a good resource
// file - in particular that the resource name is ok.
bool FileInstallAction::resourceOK(const QString &fileName, const QString &filePath)
{
	FUNCTIONSETUP;

	if (!QFile::exists(filePath))
	{
		emit logError(i18n("Unable to open file &quot;%1&quot;.", 
			fileName));
		return false;
	}

	struct pi_file *f = pi_file_open(const_cast <char *>
		((const char *) QFile::encodeName(filePath)));

	if (!f)
	{
		emit logError(i18n("Unable to open file &quot;%1&quot;.", 
			fileName));
		return false;
	}

	struct DBInfo info;
#if PILOT_LINK_NUMBER < PILOT_LINK_0_12_0
	if (pi_file_get_info(f,&info) < 0)
	{
		emit logError(i18n("Unable to read file &quot;%1&quot;.", 
			fileName));
		return false;
	}
#else
	pi_file_get_info(f,&info);
#endif

	// Looks like strlen, but we can't be sure of a NUL
	// termination.
	info.name[sizeof(info.name)-1]=0;
	bool r = (strlen(info.name) < 32);
	pi_file_close(f);

	if (!r)
	{
		emit logError(i18n("The database in &quot;%1&quot; has a "
			"resource name that is longer than 31 characters. "
			"This suggests a bug in the tool used to create the database. "
			"KPilot cannot install this database.", fileName));
	}

	return r;
}

/* virtual */ QString FileInstallAction::statusString() const
{
	FUNCTIONSETUP;
	if (fDBIndex < 0)
	{
		return QString(CSL1("Idle"));
	}
	else
	{
		if ( fDBIndex >= fList.count())
		{
			return QString(CSL1("Index out of range"));
		}
		else
		{
			return QString(CSL1("Installing %1")).arg(fList[fDBIndex]);
		}
	}
}

CleanupAction::CleanupAction(KPilotDeviceLink *p)  : SyncAction(p,"cleanupAction")
{
	FUNCTIONSETUP;
}

CleanupAction::~CleanupAction()
{
#ifdef DEBUG
	FUNCTIONSETUP;
	DEBUGCONDUIT << fname
		<< ": Deleting @" << (long)this << endl;
#endif
}

/* virtual */ bool CleanupAction::exec()
{
	FUNCTIONSETUP;

	if (deviceLink())
	{
		deviceLink()->finishSync();
	}
	emit syncDone(this);
	return true;
}


