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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
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
#include <qtextcodec.h>

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
			DEBUGDAEMON << fname << ": No database index " << i << endl;
			continue;
		}
#endif

		count++;
		dbindex = db.index + 1;

#ifdef DEBUG
		DEBUGDAEMON << fname << ": Read database " << db.name << endl;
#endif

		// Let the Pilot User know what's happening
		openConduit();
		// Let the KDE User know what's happening
		// Pretty sure all database names are in latin1.
		emit logMessage(i18n("Syncing database %1...")
			.arg(QString::fromLatin1(db.name)));

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
	const QValueList<unsigned long> &dbcreators)
{
	// Special case - skip database Unsaved Preferences
	if (   (info->creator == pi_mktag('p','s','y','s'))  &&
		(info->type == pi_mktag('p','r','e','f')) ) return true;

	if (dbcreators.findIndex(info->creator) != -1) return true;

	// Now take wildcards into account
	QString db = QString::fromLatin1(info->name);
	for (QStringList::const_iterator i = dbnames.begin(); i != dbnames.end(); ++i)
	{
		QRegExp re(*i,true,true); // Wildcard match
		if (re.exactMatch(db)) return true;
	}
	return false;
}

static inline void initNoBackup(QStringList &dbnames,
	QValueList<unsigned long> &dbcreators)
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
				kdWarning() << k_funcinfo << ": Creator ID " << s << " is malformed." << endl;
			}
			else
			{
				QCString data =  s.mid(2,4).latin1();
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
	DEBUGDAEMON << fname << ": Will skip databases "
		<< dbnames.join(",") << endl;
	QString creatorids;
	for (QValueList<unsigned long>::const_iterator i = dbcreators.begin();
		i != dbcreators.end(); ++i)
	{
		creatorids.append(CSL1("[%1]").arg(*i,0,16));
	}
	DEBUGDAEMON << fname << ": Will skip creators " << creatorids << endl;
#endif
}

/* virtual */ bool BackupAction::exec()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": This Pilot user's name is \""
		<< fHandle->getPilotUser()->getUserName() << "\"" << endl;
#endif

	fBackupDir =
		fDatabaseDir +
		PilotAppCategory::codec()->toUnicode(fHandle->getPilotUser()->getUserName()) +
		CSL1("/");

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

	initNoBackup(fNoBackupDBs,fNoBackupCreators);

	fTimer = new QTimer(this);
	QObject::connect(fTimer, SIGNAL(timeout()),
		this, SLOT(backupOneDB()));

	fDBIndex = 0;

	fTimer->start(0, false);
	return true;
}

bool BackupAction::checkBackupDirectory(QString backupDir)
{
	FUNCTIONSETUP;
	QFileInfo fi(backupDir);

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
			kdError() << k_funcinfo
				<< ": Database backup directory "
				<< "doesn't exist."
				<< endl;
			return false;
		}

		QDir databaseDir(backupDir);

		if (!databaseDir.mkdir(backupDir, true))
		{
			kdError() << k_funcinfo
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
		DEBUGDAEMON << fname
			<< ": openConduit failed. User cancel?" << endl;
#endif

		addSyncLogEntry(i18n("Exiting on cancel."));
		endBackup();
		fActionStatus = BackupIncomplete;
		return;
	}

	// TODO: Is there a way to skip unchanged databases?
	int res=fHandle->getNextDatabase(fDBIndex, &info);
	if (res < 0)
	{
#ifdef DEBUG
		DEBUGDAEMON << fname << ": Backup complete." << endl;
#endif

		if (fFullBackup)
			addSyncLogEntry(i18n("Full backup complete."));
		else
			addSyncLogEntry(i18n("Fast backup complete."));
		endBackup();
		fActionStatus = BackupComplete;
		return;
	}

	fDBIndex = info.index + 1;


#ifdef DEBUG
	DEBUGDAEMON << fname << ": Checking database " << info.name
		<< " [" << QString::number(info.creator,16) << "]" << endl;
#endif
	if (dontBackup(&info,fNoBackupDBs,fNoBackupCreators))
	{
#ifdef DEBUG
		DEBUGDAEMON << fname << ": Skipping database " << info.name
			<< endl;
#endif
		QString s = i18n("Skipping %1")
			.arg(QString::fromLatin1(info.name));
		addSyncLogEntry(s);
		return;
	}


	// Pretty sure all database names are latin1.
	QString s = i18n("Backing up: %1")
		.arg(QString::fromLatin1(info.name));
	addSyncLogEntry(s);

	if (!createLocalDatabase(&info))
	{
		kdError() << k_funcinfo
			<< ": Couldn't create local database for "
			<< info.name << endl;
		addSyncLogEntry(i18n("Backup of %1 failed.\n")
			.arg(QString::fromLatin1(info.name)));
	}
	else
	{
		addSyncLogEntry(i18n(" .. OK\n"),false); // Not in kpilot log.
	}
}

bool BackupAction::createLocalDatabase(DBInfo * info)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Looking in directory " << fBackupDir << endl;
#endif

	QString databaseName(QString::fromLatin1(info->name));
	if (!fFullBackup)
	{
		// open the serial db first so that the local db is not read into memory
		// in case of an error
		PilotSerialDatabase*serial=new PilotSerialDatabase(pilotSocket(), databaseName);
		if (serial->isDBOpen())
		{
			PilotLocalDatabase*local=new PilotLocalDatabase(fBackupDir, databaseName);
			if (local->isDBOpen())
			{
				// Now walk through all modified records
				int index=0;
				PilotRecord*rec=serial->readNextModifiedRec(&index);
				while (rec)
				{
					local->writeRecord(rec);
					KPILOT_DELETE(rec);
					rec=serial->readNextModifiedRec(&index);
				}
				KPILOT_DELETE(local);
				KPILOT_DELETE(serial);
				return true;
			}
			KPILOT_DELETE(local);
		}
		KPILOT_DELETE(serial);
#ifdef DEBUG
		DEBUGCONDUIT<<"Fast backup not possible with database "<<info->name<<". Will do full backup on it"<<endl;
#endif
	}

	// Either we want a full backup, or fast backup encoutered a problem
	// Just fetch the database to the backup dir
	if (!checkBackupDirectory(fBackupDir)) return false;

#if QT_VERSION < 0x30100
	databaseName.replace(QRegExp(CSL1("/")), CSL1("_"));
#else
	databaseName.replace('/', CSL1("_"));
#endif

	QString fullBackupName = fBackupDir + databaseName;

	if (info->flags & dlpDBFlagResource)
	{
		fullBackupName.append(CSL1(".prc"));
	}
	else
	{
		fullBackupName.append(CSL1(".pdb"));
	}

#ifdef DEBUG
	DEBUGDB << fname
		<< ": Creating local database " << fullBackupName << endl;
#endif

	/* Ensure that DB-open flag is not kept */
	info->flags &= ~dlpDBFlagOpen;

	return fHandle->retrieveDatabase(fullBackupName,info);
}

void BackupAction::endBackup()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fTimer);
	fDBIndex = (-1);
	fActionStatus = BackupEnded;

	emit syncDone(this);
}

FileInstallAction::FileInstallAction(KPilotDeviceLink * p,
	const QString & d,
	const QStringList & l) :
	SyncAction(p, "fileInstall"),
	fDBIndex(-1),
	fTimer(0L),
	fDir(d),
	fList(l)
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

/* virtual */ bool FileInstallAction::exec()
{
	FUNCTIONSETUP;

	fDBIndex = 0;

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Installing " << fList.count() << " files" << endl;
#endif

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

	emit logProgress(i18n("Installing one file",
		"Installing %n Files",fList.count()), 0);
	return true;
}

/* slot */ void FileInstallAction::installNextFile()
{
	FUNCTIONSETUP;

	Q_ASSERT(fDBIndex >= 0);
	Q_ASSERT((unsigned) fDBIndex <= fList.count());

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Installing file index "
		<< fDBIndex << " (of " << fList.count() << ")" << endl;
#endif

	if ((!fList.count()) || ((unsigned) fDBIndex >= fList.count()))
	{
#ifdef DEBUG
		DEBUGDAEMON << fname
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
	DEBUGDAEMON << fname << ": Installing file " << filePath << endl;
#endif

	QString m = i18n("Installing %1").arg(fileName);
	emit logProgress(m,(100 * fDBIndex) / (fList.count()+1));
	m+=QString::fromLatin1("\n");
	emit addSyncLogEntry(m,false /* Don't print in KPilot's log. */ );

	struct pi_file *f = 0L;

	f = pi_file_open(const_cast <char *>
		((const char *) QFile::encodeName(filePath)));

	if (!f)
	{
		kdWarning() << k_funcinfo
			<< ": Unable to open file." << endl;

		emit logError(i18n("Unable to open file &quot;%1&quot;.").
			arg(fileName));
		goto nextFile;
	}

	if (pi_file_install(f, pilotSocket(), 0) < 0)
	{
		kdWarning() << k_funcinfo << ": failed to install." << endl;


		emit logError(i18n("Cannot install file &quot;%1&quot;.").
			arg(fileName));
	}
	else
	{
		QFile::remove(filePath);
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
		return QString(CSL1("Idle"));
	}
	else
	{
		if ((unsigned) fDBIndex >= fList.count())
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
	DEBUGDAEMON << fname
		<< ": Deleting @" << (long)this << endl;
#endif
}

/* virtual */ bool CleanupAction::exec()
{
	FUNCTIONSETUP;

	fHandle->finishSync();
	emit syncDone(this);
	return true;
}


