/* KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2006 Adriaan de Groot <groot@kde.org>
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


#include "options.h"

#include <time.h>
#include <unistd.h>
#include <stdio.h>

#include <pi-file.h>
#include <pi-util.h>

#include <qtimer.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qvaluelist.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qthread.h>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kmessagebox.h>

#include "pilotUser.h"
#include "pilotRecord.h"
#include "actionQueue.h"
#include "pilotSerialDatabase.h"
#include "pilotLocalDatabase.h"
#include "pilotDatabase.h"
#include "kpilotSettings.h"

#include "hotSync.moc"

class BackupAction::Thread : public QThread
{
public:
	Thread( BackupAction *parent,
		KPilotLink *link,
		const QString &filename,
		const DBInfo *info );

	enum {
		TerminateOK = QEvent::User,
		TerminateFailure
	} ;

protected:
	virtual void run();
private:
	BackupAction *fParent;
	KPilotLink *fLink;
	QString fFilename;
	struct DBInfo fDBInfo;
} ;

class BackupAction::Private
{
public:
	bool fFullBackup; ///< Is this a full backup (all DBs, not just changed ones)?
	QStringList fNoBackupDBs;
	QValueList<unsigned long> fNoBackupCreators;
	QStringList fDeviceDBs;

	QString fPreferBackupDir; ///< Directory to write backup in, overrides default

	// Remainder is used to hand around info during sync

	int fDBIndex;       ///< Database number we're now doing
	QString fBackupDir; ///< Directory to write backup in.

	/**
	* Add the database described by the info block to the list of
	* databases definitely found on the handheld.
	*/
	void addDBInfo( const DBInfo *info )
	{
		FUNCTIONSETUP;
		fDBIndex = info->index + 1;

		// Each character of buff[] is written to
		char buff[7];
		buff[0] = '[';
		set_long( &buff[1], info->creator );
		buff[5] = ']';
		buff[6] = '\0';
		QString creator = QString::fromLatin1( buff );

		QString dbname = Pilot::fromPilot( info->name, 32 );

		if ( !fDeviceDBs.contains( creator ) )
		{
			fDeviceDBs << creator;
		}
		if ( !fDeviceDBs.contains( dbname ) )
		{
			fDeviceDBs << dbname;
		}

		DEBUGKPILOT << fname << ": Added <" << dbname
			<< "> " << creator << endl;
	}


	/**
	* Check if this database, described by @p info , should
	* be backed up (i.e. is allowed to be backed up by the
	* user settings for no-backup DBs).
	*
	* @return @c true if the database may be backed up.
	*/
	bool allowBackup( const DBInfo *info ) const
	{
		// Special case - skip database Unsaved Preferences
		if (   (info->creator == pi_mktag('p','s','y','s'))  &&
			(info->type == pi_mktag('p','r','e','f')) )
		{
			return false;
		}

		if (fNoBackupCreators.findIndex(info->creator) != -1)
		{
			return false;
		}

		// Now take wildcards into account
		QString db = Pilot::fromPilot(info->name);
		for (QStringList::const_iterator i = fNoBackupDBs.begin();
			i != fNoBackupDBs.end(); ++i)
		{
			QRegExp re(*i,true,true); // Wildcard match
			if (re.exactMatch(db))
			{
				return false;
			}
		}
		return true;
	}

} ;

BackupAction::BackupAction(KPilotLink * p, bool full) :
	SyncAction(p, "backupAction"),
	fP( new Private ),
	fBackupThread( 0L )
{
	FUNCTIONSETUP;

	fP->fFullBackup = full;
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

void BackupAction::setDirectory( const QString &p )
{
	fP->fPreferBackupDir = p;
	if (!p.endsWith(CSL1("/")))
	{
		fP->fPreferBackupDir.append(CSL1("/"));
	}
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
				WARNINGKPILOT << "Creator ID " << s << " is malformed." << endl;
			}
			else
			{
				QCString data =  s.mid(1,4).latin1();
				unsigned long creator = pi_mktag(data[0],data[1],data[2],data[3]);
				dbcreators.append(creator);
			}
		}
		else
		{
			dbnames.append(s);
		}
	}

	DEBUGKPILOT << fname << ": Will skip databases "
		<< dbnames.join(CSL1(",")) << endl;
	QString creatorids;
	char buf[5];
	for (QValueList<unsigned long>::const_iterator i = dbcreators.begin();
		i != dbcreators.end(); ++i)
	{
		unsigned long tag = *i;
		pi_untag(buf,tag);
		buf[4]=0;
		creatorids.append(CSL1("[%1]").arg(buf));
	}
	DEBUGKPILOT << fname << ": Will skip creators " << creatorids << endl;
}

/** Make sure that the backup directory @p backupDir
*   exists and is a directory; returns @c false
*   if this is not the case. This method will try
*   to create the directory if it doesn't exist yet.
*/
static inline bool checkBackupDirectory( const QString &backupDir )
{
	FUNCTIONSETUP;
	QFileInfo fi(backupDir);

	if (fi.exists() && fi.isDir())
	{
		return true;
	}

	if (fi.exists() && !fi.isDir())
	{
		WARNINGKPILOT << "Requested backup directory "
			<< backupDir
			<< " exists but is not a directory."
			<< endl;
		return false;
	}

	if ( !backupDir.endsWith("/") )
	{
		WARNINGKPILOT << "Backup dir does not end with a / "
			<< endl;
		return false;
	}

	Q_ASSERT(!fi.exists());

	DEBUGKPILOT << fname
		<< ": Creating directory " << backupDir << endl;

	KStandardDirs::makeDir( backupDir );

	fi = QFileInfo(backupDir);

	return fi.exists() && fi.isDir();
}


/* virtual */ bool BackupAction::exec()
{
	FUNCTIONSETUP;

	fP->fDeviceDBs = KPilotSettings::deviceDBs();

	if (fP->fPreferBackupDir.isEmpty())
	{
		fP->fBackupDir =
			KGlobal::dirs()->saveLocation("data",CSL1("kpilot/DBBackup/")) +
				deviceLink()->getPilotUser().name() + '/';
	}
	else
	{
		fP->fBackupDir = fP->fPreferBackupDir;
	}

	logMessage(i18n("Backup directory: %1.").arg(fP->fBackupDir));

	DEBUGKPILOT << fname
		<< ": This Pilot user's name is \""
		<< deviceLink()->getPilotUser().name() << "\"" << endl;
	DEBUGKPILOT << fname
		<< ": Using backup dir: " << fP->fBackupDir << endl;
	DEBUGKPILOT << fname
		<< ": Full Backup? " << fP->fFullBackup << endl;


	if (fP->fFullBackup)
	{
		fActionStatus = FullBackup;
		addSyncLogEntry(i18n("Full backup started."));
	}
	else
	{
		fActionStatus = FastBackup;
		addSyncLogEntry(i18n("Fast backup started"));
	}

	if (!checkBackupDirectory(fP->fBackupDir))
	{
		fActionStatus=BackupIncomplete;
		// Don't issue an error message, checkBackupDirectory
		// did this already...
		return false;
	}

	initNoBackup( fP->fNoBackupDBs, fP->fNoBackupCreators );

	fP->fDBIndex = 0;
	QTimer::singleShot(0,this,SLOT(backupOneDB()));
	return true;
}

/* slot */ void BackupAction::backupOneDB()
{
	FUNCTIONSETUP;

	struct DBInfo info;

	// TODO: make the progress reporting more accurate
	emit logProgress(QString::null, fP->fDBIndex);

	if (openConduit() < 0)
	{
		addSyncLogEntry(i18n("Exiting on cancel."));
		endBackup();
		fActionStatus = BackupIncomplete;
		return;
	}

	// TODO: Is there a way to skip unchanged databases?
	int res = deviceLink()->getNextDatabase( fP->fDBIndex, &info );
	if (res < 0)
	{
		if ( fP->fFullBackup )
		{
			addSyncLogEntry( i18n("Full backup complete.") );
		}
		else
		{
			addSyncLogEntry( i18n("Fast backup complete.") );
		}
		endBackup();
		fActionStatus = BackupComplete;
		return;
	}

	fP->addDBInfo( &info );

	// see if user told us not to back this creator or database up...
	if (fP->allowBackup(&info))
	{
		// back up DB if this is a full backup *or* in non-full backups,
		// only backup data, not applications.
		if ( (fP->fFullBackup) || !PilotDatabase::isResource(&info) )
		{
			addSyncLogEntry(i18n("Backing up: %1").arg(Pilot::fromPilot(info.name)));

			if (!startBackupThread(&info))
			{
				WARNINGKPILOT << "Could not create local database for <"
					<< info.name << ">" << endl;
			}
			else
			{
				// The thread has started, so we will be woken
				// up by it eventually when it is done. Do *NOT*
				// fall through to the single-shot timer below,
				// because that would return us to the backup
				// function too soon.
				return;
			}
		}
		else
		{
			// Just skip resource DBs during an update hotsync.
			DEBUGKPILOT << fname << ": Skipping database <" << info.name
				<< "> (resource database)" << endl;
		}
	}
	else
	{
		DEBUGKPILOT << fname << ": Skipping database <" << info.name
			<< "> (no-backup list)" << endl;
		QString s = i18n("Skipping %1")
			.arg(Pilot::fromPilot(info.name));
		addSyncLogEntry(s);
	}
	QTimer::singleShot(0,this,SLOT(backupOneDB()));
}

/**
 * This method will back up a single database from the Pilot to a directory on
 * our filesystem.  If our user asks us to do a full backup, then we will unconditionally
 * copy the database file from the Pilot into the backup directory.  Otherwise, we will
 * check to see if the database has any modified records in it on the pilot.  If the
 * database has not changed on the Pilot, then there's nothing to backup and we return.
 *
 * @return @c true if the backup has started (in another thread).
 *           You must wait on the thread to end with a User or User+1
 *            type event and not start another backup thread.
 * @return @c false if there is no backup to do. Diagnostic messages
 *           will already have been printed.
 */
bool BackupAction::startBackupThread(DBInfo *info)
{
	FUNCTIONSETUP;

	// now we look to see if the database on the pilot has at least one changed record
	// in it.  we do this so that we don't waste time backing up a database that has
	// not changed.  note: don't bother with this check if we're doing a full backup.
	if (!fP->fFullBackup)
	{
		// Check if this DB has modified records.
		PilotDatabase *serial=deviceLink()->database(info);
		if (!serial->isOpen())
		{
			WARNINGKPILOT << "Unable to open database <" << info->name << ">" << endl;
			KPILOT_DELETE(serial);
			addSyncLogEntry(i18n("Backup of %1 failed.\n")
				.arg(Pilot::fromPilot(info->name)));
			return false;
		}

		int index=0;
		PilotRecord*rec=serial->readNextModifiedRec(&index);
		if (!rec)
		{
			DEBUGKPILOT << fname << ": No modified records." << endl;
			KPILOT_DELETE(serial);
			return false;
		}
		// Exists, with modified records.
		KPILOT_DELETE(rec);
		KPILOT_DELETE(serial);
	}


	// if we're here then we are going to back this database up.  do some basic sanity
	// checks and proceed....
	QString databaseName(Pilot::fromPilot(info->name));
	databaseName.replace('/', '_');

	QString fullBackupName = fP->fBackupDir + databaseName;

	if (PilotDatabase::isResource(info))
	{
		fullBackupName.append(CSL1(".prc"));
	}
	else
	{
		fullBackupName.append(CSL1(".pdb"));
	}

	DEBUGKPILOT << fname
		<< ": Backing up database to: [" << fullBackupName << "]" << endl;

	/* Ensure that DB-open flag is not kept */
	info->flags &= ~dlpDBFlagOpen;

	if (fBackupThread)
	{
		WARNINGKPILOT << "Starting new backup thread before the old one is done." << endl;
		return false;
	}

	fBackupThread  = new Thread(this,deviceLink(),fullBackupName,info);
	fBackupThread->start();
	return true;
}

/* virtual */ bool BackupAction::event( QEvent *e )
{
	if (e->type() == (QEvent::Type)Thread::TerminateOK)
	{
		KPILOT_DELETE(fBackupThread);
		// This was a successful termination.
		addSyncLogEntry( i18n("... OK.\n"), false );
		QTimer::singleShot(0,this,SLOT(backupOneDB()));
		return true;
	}
	if (e->type() == (QEvent::Type)Thread::TerminateFailure)
	{
		KPILOT_DELETE(fBackupThread);
		// Unsuccessful termination.
		addSyncLogEntry( i18n("Backup failed.") );
		QTimer::singleShot(0,this,SLOT(backupOneDB()));
		return true;
	}
	return SyncAction::event(e);
}

void BackupAction::endBackup()
{
	FUNCTIONSETUP;

	fP->fDBIndex = (-1);
	fActionStatus = BackupEnded;
	fP->fDeviceDBs.sort();
	QString old( QString::null );
	QStringList::Iterator itr = fP->fDeviceDBs.begin();
	while ( itr != fP->fDeviceDBs.end() ) {
		if ( old == *itr ) {
			itr = fP->fDeviceDBs.remove( itr );
		} else {
			old = *itr;
			++itr;
		}
	}
	KPilotSettings::setDeviceDBs( fP->fDeviceDBs );

	emit syncDone(this);
}

FileInstallAction::FileInstallAction(KPilotLink * p,
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
	DEBUGKPILOT << fname
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
	DEBUGKPILOT << fname
		<< ": Installing file index "
		<< fDBIndex << " (of " << fList.count() << ")" << endl;
#endif

	if ((!fList.count()) || ((unsigned) fDBIndex >= fList.count()))
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
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
	DEBUGKPILOT << fname << ": Installing file " << filePath << endl;
#endif

	QString m = i18n("Installing %1").arg(fileName);
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
		WARNINGKPILOT << "Failed to install." << endl;


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
		emit logError(i18n("Unable to open file &quot;%1&quot;.").
			arg(fileName));
		return false;
	}

	struct pi_file *f = pi_file_open(const_cast <char *>
		((const char *) QFile::encodeName(filePath)));

	if (!f)
	{
		emit logError(i18n("Unable to open file &quot;%1&quot;.").
			arg(fileName));
		return false;
	}

	struct DBInfo info;
#if PILOT_LINK_NUMBER < PILOT_LINK_0_12_0
	if (pi_file_get_info(f,&info) < 0)
	{
		emit logError(i18n("Unable to read file &quot;%1&quot;.").
			arg(fileName));
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
			"KPilot cannot install this database.").arg(fileName));
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

CheckUser::CheckUser(KPilotLink * p, QWidget * vp):
	SyncAction(p, vp, "userCheck")
{
	FUNCTIONSETUP;

}

CheckUser::~CheckUser()
{
	FUNCTIONSETUP;
}

/* virtual */ bool CheckUser::exec()
{
	FUNCTIONSETUP;

	QString guiUserName = KPilotSettings::userName();
	QString pilotUserName = fHandle->getPilotUser().name();
	bool pilotUserEmpty = pilotUserName.isEmpty();
	// 4 cases to handle:
	//    guiUserName empty / not empty
	//    pilotUserName empty / not empty
	//
	//
	if (guiUserName.isEmpty())
	{
		if (pilotUserEmpty)
		{
			QString defaultUserName =
				i18n("A common name", "John Doe");

			QString q = i18n("<qt>Neither KPilot nor the "
				"handheld have a username set. "
				"They <i>should</i> be set. "
				"Should KPilot set them to a default value "
				"(<i>%1</i>)?</qt>").arg(defaultUserName);

			if (questionYesNo(q, i18n("User Unknown") /* ,"askUserNone" */) ==
				KMessageBox::Yes)
			{
				KPilotSettings::setUserName(defaultUserName);
				fHandle->getPilotUser().setName(defaultUserName);
				guiUserName=defaultUserName;
				pilotUserName=defaultUserName;
			}

		}
		else
		{
			QString q = i18n("<qt>The handheld has a username set "
				"(<i>%1</i>) but KPilot does not. Should "
				"KPilot use this username in future?</qt>").
				arg(pilotUserName);

			if (questionYesNo(q, i18n("User Unknown") /* ,"askUserSome" */ ) ==
				KMessageBox::Yes)
			{
				KPilotSettings::setUserName(pilotUserName);
				guiUserName=pilotUserName;
			}
		}
	}
	else
	{
		if (pilotUserEmpty)
		{
			QString q = CSL1("<qt>");
			q += i18n("KPilot has a username set "
				"(<i>%1</i>) but the handheld does not. "
				"Should KPilot's username be set in the "
				"handheld as well?").arg(guiUserName);
			q += i18n("<br/>(<i>Note:</i> If your handheld "
				"has been reset to factory defaults, you "
				"should use <i>Restore</i> instead of a "
				"regular HotSync. Click on Cancel to "
				"stop this sync.)");
			q += CSL1("</qt>");

			int r = questionYesNoCancel(q, i18n("User Unknown"));
			switch (r)
			{
			case KMessageBox::Yes:
				DEBUGKPILOT << fname
					<< ": Setting user name in pilot to "
					<< guiUserName << endl;
				fHandle->getPilotUser().setName(guiUserName);
				pilotUserName=guiUserName;
				break;
			case KMessageBox::No:
				// Nothing to do .. continue with sync
				break;
			case KMessageBox::Cancel:
			default:
				return false;
			}
		}
		else
		{
			if (guiUserName != pilotUserName)
			{
				QString q = i18n("<qt>The handheld thinks that "
					"the username is %1; "
					"however, KPilot says you are %2."
					"Which of these is the correct name?\n"
					"If you click on Cancel, the sync will proceed, "
					"but the usernames will not be changed.</qt>").
					arg(pilotUserName).
					arg(guiUserName);

				int r = questionYesNoCancel(q,
					i18n("User Mismatch"),
					QString::null,
					20,
					i18n("Use KPilot Name"),
					i18n("Use Handheld Name"));
				switch (r)
				{
				case KMessageBox::Yes:
					fHandle->getPilotUser().setName(guiUserName);
					pilotUserName=guiUserName;
					break;
				case KMessageBox::No:
					KPilotSettings::setUserName(pilotUserName);
					guiUserName=pilotUserName;
					break;
				case KMessageBox::Cancel:
				default:
					// TODO: cancel the sync... Or just don't change any user name?
					break;
				}
			}
		}
	}


#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": User name set to pc <"
		<< guiUserName
		<< "> hh <"
		<< fHandle->getPilotUser().name() << ">" << endl;
#endif

	KPilotSettings::writeConfig();

	// Now we've established which user will be used,
	// fix the database location for local databases.
	//
	//
	QString pathName = KGlobal::dirs()->saveLocation("data",
		CSL1("kpilot/DBBackup/"));
	if (!guiUserName.isEmpty())
	{
		pathName.append(guiUserName);
		pathName.append(CSL1("/"));
	}
	PilotLocalDatabase::setDBPath(pathName);

	delayDone();
	return true;
}

class RestoreInfo
{
public:
	struct DBInfo DBInfo;
	QString path;
} ;

class RestoreAction::Private
{
public:
	QString fPreferRestoreDir; /**< Preference setting where to get data from. */

	QValueList<RestoreInfo> fDBList;
	QTimer fTimer;
	QValueList<RestoreInfo>::ConstIterator fDBIterator;
	int fDBIndex;
};


RestoreAction::RestoreAction(KPilotLink * p, QWidget * visible ) :
	SyncAction(p, visible, "restoreAction")
{
	FUNCTIONSETUP;

	fP = new Private;
}

void RestoreAction::setDirectory( const QString &path )
{
	fP->fPreferRestoreDir = path;
}

/* virtual */ bool RestoreAction::exec()
{
	FUNCTIONSETUP;

	QString dirname;
	if (fP->fPreferRestoreDir.isEmpty())
	{
		dirname = PilotLocalDatabase::getDBPath();
	}
	else
	{
		dirname = fP->fPreferRestoreDir;
	}

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Restoring user " << dirname << endl;
#endif

	QDir dir(dirname, QString::null, QDir::Name,
		QDir::Files | QDir::Readable | QDir::NoSymLinks);

	if (!dir.exists())
	{
		WARNINGKPILOT << "Restore directory "
			<< dirname << " does not exist." << endl;
		fActionStatus = Error;
		addSyncLogEntry(i18n("Restore directory does not exist.") +
			CSL1(" ") + i18n("Restore not performed."));
		return false;
	}

	dirname = dir.absPath();
	if (questionYesNo(i18n("<qt>Are you sure you want to completely "
				"restore your Pilot from the backup directory "
				"(<i>%1</i>)? This will erase any information "
				"you currently have on your Pilot.</qt>").
			arg(dirname),
			i18n("Restore Pilot")) != KMessageBox::Yes)
	{
		emit logError(i18n("Restore <i>not</i> performed."));

		addSyncLogEntry(i18n("Canceled by user.") + CSL1(" ") +
			i18n("Restore not performed."));

		// You might call this an error, but that causes
		// a frightening message in the log .. and the
		// user already _knows_ the restore didn't happen.
		// So instead, act as if everything was ok.
		delayDone();
		return true;
	}


	emit logProgress(i18n("Restoring %1...").arg(QString::null),1);

	for (unsigned int i = 0; i < dir.count(); i++)
	{
		QString s;
		RestoreInfo info;

		s = dirname + QDir::separator() + dir[i];

		DEBUGKPILOT << fname
			<< ": Adding " << s << " to restore list." << endl;

		if ( PilotLocalDatabase::infoFromFile( s, &info.DBInfo ) )
		{
			info.path = s;
			fP->fDBList.append(info);
		}
		else
		{
			WARNINGKPILOT << "Can't open " << s << endl;
			logMessage(i18n("File '%1' cannot be read.").arg(s));
		}
	}

	fP->fDBIndex = 0;
	fP->fDBIterator = fP->fDBList.begin();
	fActionStatus = InstallingFiles;

	QObject::connect(&(fP->fTimer), SIGNAL(timeout()),
		this, SLOT(installNextFile()));

	fP->fTimer.start(0, false);
	return true;
}

/* slot */ void RestoreAction::installNextFile()
{
	FUNCTIONSETUP;

	Q_ASSERT(fActionStatus == InstallingFiles);


	if (fP->fDBIterator == fP->fDBList.end())
	{
		fP->fTimer.stop();

		fActionStatus = Done;
		addSyncLogEntry(i18n("OK."));
		delayDone();
		return;
	}

	const RestoreInfo dbi = *(fP->fDBIterator);
	++(fP->fDBIterator);
	++(fP->fDBIndex);

	DEBUGKPILOT << fname << ": Trying to install " << dbi.path << endl;

	if (openConduit() < 0)
	{
		WARNINGKPILOT << "Restore apparently canceled." << endl;
		logMessage(i18n("Restore incomplete."));
		fActionStatus = Done;
		emit syncDone(this);

		return;
	}

	QFileInfo databaseInfo(dbi.path);
	addSyncLogEntry(databaseInfo.fileName());
	emit logProgress(i18n("Restoring %1...").arg(databaseInfo.fileName()),
		(100*fP->fDBIndex) / (fP->fDBList.count()+1)) ;

	if ( !deviceLink()->installFiles( dbi.path, false /* don't delete */ ) )
	{
		WARNINGKPILOT << "Couldn't  restore " << dbi.path << endl;
		logError(i18n("Cannot restore file `%1'.")
			.arg(databaseInfo.fileName()));
	}
}

/* virtual */ QString RestoreAction::statusString() const
{
	FUNCTIONSETUP;
	QString s;

	switch (status())
	{
	case InstallingFiles:
		s.append(CSL1("Installing Files ("));
		s.append(QString::number(fP->fDBIndex));
		s.append(CSL1(")"));
		break;
	case GettingFileInfo:
		s.append(CSL1("Getting File Info ("));
		s.append(QString::number(fP->fDBIndex));
		s.append(CSL1(")"));
		break;
	default:
		return SyncAction::statusString();
	}

	return s;
}



BackupAction::Thread::Thread( BackupAction *parent,
	KPilotLink *link,
	const QString &filename,
	const DBInfo *info )
{
	fParent = parent;
	fLink = link;
	fFilename = filename;
	memcpy(&fDBInfo,info,sizeof(DBInfo));
}

void BackupAction::Thread::run()
{
	if (fLink->retrieveDatabase(fFilename,&fDBInfo))
	{
		// Successful.
		QApplication::postEvent( fParent, new QEvent( (QEvent::Type)TerminateOK ) );
	}
	else
	{
		// Failed
		QApplication::postEvent( fParent, new QEvent( (QEvent::Type)TerminateFailure ) );
	}
}


