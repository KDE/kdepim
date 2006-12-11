/* KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2006 Adriaan de Groot <groot@kde.org>
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

#include <kdialogbase.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

#include <kapplication.h>

#include "pilotUser.h"
#include "pilotRecord.h"
#include "pilotLocalDatabase.h"
#include "kpilotConfig.h"
#include "kpilotlink.h"

#include "interactiveSync.moc"


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
// TODO: add userName() to pilotUser class
	QString pilotUserName = Pilot::fromPilot(fHandle->getPilotUser().getUserName());
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
				fHandle->getPilotUser().setUserName(defaultUserName);
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
				DEBUGKPILOT << fname
					<< ": Setting user name in pilot to "
					<< guiUserName << endl;
#endif

				fHandle->getPilotUser().setUserName(guiUserName);
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
					fHandle->getPilotUser().setUserName(guiUserName);
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
		<< ": User name set to gui<"
		<< guiUserName
		<< "> hh<"
		<< fHandle->getPilotUser().getUserName() << ">" << endl;
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
		kdWarning() << k_funcinfo
			<< ": Restore directory "
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
			kdWarning() << k_funcinfo
				<< ": Can't open " << s << endl;
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
		kdWarning() << k_funcinfo
			<< ": Restore apparently canceled." << endl;
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
		kdWarning() << k_funcinfo
			<< ": Couldn't  restore " << dbi.path << endl;
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


