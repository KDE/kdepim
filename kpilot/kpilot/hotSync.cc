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
			DEBUGKPILOT << fname << ": No database index " << i << endl;
			continue;
		}
#endif

		count++;
		dbindex = db.index + 1;

#ifdef DEBUG
		DEBUGKPILOT << fname << ": Read database " << db.name << endl;
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

BackupAction::BackupAction(KPilotDeviceLink * p) :
	SyncAction(p, "backupAction")
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
	case BackupEnded:
		s.append(CSL1("BackupEnded"));
		break;
	default:
		s.append(CSL1("(unknown "));
		s.append(QString::number(status()));
		s.append(CSL1(")"));
	}

	return s;
}


/* virtual */ bool BackupAction::exec()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": This Pilot user's name is \""
		<< fHandle->getPilotUser()->getUserName() << "\"" << endl;
#endif

	addSyncLogEntry(i18n("Full backup started."));

	// ASSERT(!fTimer);

	fTimer = new QTimer(this);
	QObject::connect(fTimer, SIGNAL(timeout()),
		this, SLOT(backupOneDB()));

	fDBIndex = 0;
	fStatus = FullBackup;

	fTimer->start(0, false);
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
		fStatus = BackupIncomplete;
		return;
	}

	if (fHandle->getNextDatabase(fDBIndex, &info) < 0)
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

	QString fullBackupDir =
		fDatabaseDir + 
		PilotAppCategory::codec()->toUnicode(fHandle->getPilotUser()->getUserName()) +
		CSL1("/");

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
			kdError() << k_funcinfo
				<< ": Database backup directory "
				<< "doesn't exist."
				<< endl;
			return false;
		}

		QDir databaseDir(fDatabaseDir);

		if (!databaseDir.mkdir(fullBackupDir, true))
		{
			kdError() << k_funcinfo
				<< ": Can't create backup directory." << endl;
			return false;
		}
	}

	QString databaseName(QString::fromLatin1(info->name));

#if QT_VERSION < 0x30100
	databaseName.replace(QRegExp(CSL1("/")), CSL1("_"));
#else
	databaseName.replace('/', CSL1("_"));
#endif

	QString fullBackupName = fullBackupDir + databaseName;

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
	fStatus = BackupEnded;

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

	// Possibly no files to install?
	if (!fList.count())
	{
		emit logMessage(i18n("No Files to install"));
		emit syncDone(this);

		return true;
	}

	fTimer = new QTimer(this);
	QObject::connect(fTimer, SIGNAL(timeout()),
		this, SLOT(installNextFile()));

	fTimer->start(0, false);

	emit logProgress(i18n("Installing Files"), 0);
	return true;
}

/* slot */ void FileInstallAction::installNextFile()
{
	FUNCTIONSETUP;

	ASSERT(fDBIndex >= 0);
	ASSERT((unsigned) fDBIndex <= fList.count());

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
		emit syncDone(this);
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
	emit addSyncLogEntry(m,true /* Don't print in KPilot's log. */ );


	struct pi_file *f = 0L;

	f = pi_file_open(const_cast <char *>
		((const char *) QFile::encodeName(filePath)));

	if (!f)
	{
		kdWarning() << k_funcinfo
			<< ": Unable to open file." << endl;

		emit logError(i18n("Unable to open file &quot;%1&quot;!").
			arg(fileName));
		goto nextFile;
	}

	if (pi_file_install(f, pilotSocket(), 0) < 0)
	{
		kdWarning() << k_funcinfo << ": failed to install." << endl;


		emit logError(i18n("Cannot install file &quot;%1&quot;!").
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
		<< ": Deleting @" << (int)this << endl;
#endif
}

/* virtual */ bool CleanupAction::exec()
{
	FUNCTIONSETUP;

	fHandle->finishSync();
	emit syncDone(this);
	return true;
}


