/* KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This file specializes SyncAction to a kind that can have interaction
** with the user without the Sync timing out.
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

static const char *interactivesync_id =
	"$Id$";

#include "options.h"

#include <time.h>
#include <unistd.h>
#include <stdio.h>

#include <pi-socket.h>
#include <pi-file.h>

#include <qtimer.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtl.h>
#include <qstyle.h>
#include <qtextcodec.h>

#include <kdialogbase.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

#include <kapplication.h>

#include "pilotUser.h"
#include "pilotAppCategory.h"
#include "pilotLocalDatabase.h"
#include "kpilotConfig.h"
#include "kpilotlink.h"

#include "interactiveSync.moc"


CheckUser::CheckUser(KPilotDeviceLink * p, QWidget * vp):
	SyncAction(p, vp, "userCheck")
{
	FUNCTIONSETUP;

	(void) interactivesync_id;
}

CheckUser::~CheckUser()
{
	FUNCTIONSETUP;
}

/* virtual */ bool CheckUser::exec()
{
	FUNCTIONSETUP;

	QString guiUserName = KPilotSettings::userName();
	QString pilotUserName = PilotAppCategory::codec()->
		toUnicode(fHandle->getPilotUser()->getUserName());
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
				fHandle->getPilotUser()->
					setUserName(PilotAppCategory::codec()->fromUnicode(defaultUserName));
				guiUserName=defaultUserName;
				pilotUserName=defaultUserName;
			}

		}
		else
		{
			QString q = i18n("<qt>The handheld has a username set "
				"(<i>%1</i>) but KPilot does not. Should "
				"KPilot use this username in future?").
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
			QString q = i18n("<qt>KPilot has a username set "
				"(<i>%1</i>) but the handheld does not. "
				"Should KPilot's username be set in the "
				"handheld as well?").arg(guiUserName);

			if (questionYesNo(q, i18n("User Unknown") /* ,"askUserSome" */) ==
				KMessageBox::Yes)
			{
#ifdef DEBUG
				DEBUGDAEMON << fname
					<< ": Setting user name in pilot to "
					<< guiUserName << endl;
#endif

				QCString l1 = PilotAppCategory::codec()->fromUnicode(guiUserName);

				fHandle->getPilotUser()->setUserName(l1.data());
				pilotUserName=guiUserName;
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
					"but the usernames will not be changed.").
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
					fHandle->getPilotUser()->setUserName(
						PilotAppCategory::codec()->fromUnicode(guiUserName));
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
	DEBUGCONDUIT << fname
		<< ": User name set to gui<"
		<< guiUserName
		<< "> hh<"
		<< fHandle->getPilotUser()->getUserName() << ">" << endl;
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

	emit syncDone(this);
	return true;
}

class RestoreInfo
{
public:
	struct db DBInfo;
	QString path;
} ;

class RestoreAction::RestoreActionPrivate
{
public:
	QString fDatabaseDir;
	QValueList<RestoreInfo *> fDBList;
	QTimer fTimer;
	QValueList<RestoreInfo *>::ConstIterator fDBIterator;
	int fDBIndex;
};


RestoreAction::RestoreAction(KPilotDeviceLink * p, QWidget * visible ) :
	SyncAction(p, visible, "restoreAction")
{
	FUNCTIONSETUP;

	fP = new RestoreActionPrivate;
	fP->fDatabaseDir = KGlobal::dirs()->saveLocation("data",
		CSL1("kpilot/DBBackup/"));
}

/* virtual */ bool RestoreAction::exec()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Restoring from base directory "
		<< *(PilotLocalDatabase::getDBPath()) << endl;
#endif

	QString dirname = *(PilotLocalDatabase::getDBPath());

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Restoring user " << dirname << endl;
#endif

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
		emit syncDone(this);

		return true;
	}

	QDir dir(dirname, QString::null, QDir::Name,
		QDir::Files | QDir::Readable | QDir::NoSymLinks);

	if (!dir.exists())
	{
		kdWarning() << k_funcinfo
			<< ": Restore directory "
			<< dirname << " does not exist." << endl;
		fActionStatus = Error;
		addSyncLogEntry(i18n("Restore directory does not exist.") +
			CSL1(" ") + i18n("Restore not performed."));
		return false;
	}

	emit logProgress(i18n("Restoring %1...").arg(QString::null),1);

	for (unsigned int i = 0; i < dir.count(); i++)
	{
		QString s;
		RestoreInfo *dbi;
		struct DBInfo info;
		struct pi_file *f;

		s = dir[i];

#ifdef DEBUG
		DEBUGDAEMON << fname
			<< ": Adding " << s << " to restore list." << endl;
#endif

    char * fileName = qstrdup( QFile::encodeName(dirname + s) );
    f = pi_file_open( fileName );
    delete fileName;
		if (!f)
		{
			kdWarning() << k_funcinfo
				<< ": Can't open " << s << endl;
			logMessage(i18n("File '%1' cannot be read.").arg(s));
			continue;
		}

#if PILOT_LINK_NUMBER < PILOT_LINK_0_12_0
		if (!pi_file_get_info(f, &info))
#else
		pi_file_get_info(f,&info);
		if (true)
#endif
		{
			dbi = new RestoreInfo;
			memcpy(&dbi->DBInfo,&info,sizeof(struct DBInfo));
			dbi->path = dirname + s;
			fP->fDBList.append(dbi);
		}
		else
		{
			kdWarning() << k_funcinfo
				<< ": Can't open " << s << endl;
			logMessage(i18n("File '%1' cannot be read.").arg(s));
		}

		pi_file_close(f);
		f = 0L;
	}

	fP->fDBIndex = 0;
	fP->fDBIterator = fP->fDBList.begin();
	fActionStatus = GettingFileInfo;

	QObject::connect(&(fP->fTimer), SIGNAL(timeout()),
		this, SLOT(getNextFileInfo()));

	fP->fTimer.start(0, false);
	return true;
}

/* slot */ void RestoreAction::getNextFileInfo()
{
	FUNCTIONSETUP;

	Q_ASSERT(fActionStatus == GettingFileInfo);

	QObject::disconnect(&(fP->fTimer), SIGNAL(timeout()),
		this, SLOT(getNextFileInfo()));
	fP->fTimer.stop();

	qBubbleSort(fP->fDBList);

	fP->fDBIndex = 0;
	fP->fDBIterator = fP->fDBList.begin();
	fActionStatus = InstallingFiles;

	QObject::connect(&(fP->fTimer), SIGNAL(timeout()),
		this, SLOT(installNextFile()));
	fP->fTimer.start(0, false);
}

/* slot */ void RestoreAction::installNextFile()
{
	FUNCTIONSETUP;

	Q_ASSERT(fActionStatus == InstallingFiles);

	const RestoreInfo *dbi = 0L;

	if (fP->fDBIterator == fP->fDBList.end())
	{
		QObject::disconnect(&(fP->fTimer), SIGNAL(timeout()),
			this, SLOT(getNextFileInfo()));
		fP->fTimer.stop();

		fActionStatus = Done;
		addSyncLogEntry(i18n("OK."));
		delayDone();
		return;
	}

	dbi = *fP->fDBIterator;
	++(fP->fDBIterator);
	++(fP->fDBIndex);
#ifdef DEBUG
	DEBUGDAEMON << fname << ": Trying to install " << dbi->path << endl;
#endif

	if (openConduit() < 0)
	{
		kdWarning() << k_funcinfo
			<< ": Restore apparently canceled." << endl;
		logMessage(i18n("Restore incomplete."));
		fActionStatus = Done;
		emit syncDone(this);

		return;
	}

	QFileInfo databaseInfo(dbi->path);
	addSyncLogEntry(databaseInfo.fileName());
	emit logProgress(i18n("Restoring %1...").arg(databaseInfo.fileName()),
		(100*fP->fDBIndex) / (fP->fDBList.count()+1)) ;

	char * fileName = qstrdup( dbi->path.utf8() );
	pi_file *f  = pi_file_open( fileName );
	delete fileName;

	if (!f)
	{
		kdWarning() << k_funcinfo
			<< ": Can't open "
			<< dbi->path << " for restore." << endl;
		logError(i18n("Cannot open file `%1' for restore.")
			.arg(databaseInfo.fileName()));
		return;
	}

#if PILOT_LINK_NUMBER < PILOT_LINK_0_12_0
	if (pi_file_install(f, pilotSocket(), 0) < 0)
#else
	if (pi_file_install(f, pilotSocket(), 0, NULL) < 0)
#endif
	{
		kdWarning() << k_funcinfo
			<< ": Couldn't  restore " << dbi->path << endl;
		logError(i18n("Cannot restore file `%1'.")
			.arg(databaseInfo.fileName()));
	}

	pi_file_close(f);
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


