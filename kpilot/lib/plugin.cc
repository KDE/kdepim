/* KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This file defines the base class of all KPilot conduit plugins configuration
** dialogs. This is necessary so that we have a fixed API to talk to from
** inside KPilot.
**
** The factories used by KPilot plugins are also documented here.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <stdlib.h>

#include <qstringlist.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qregexp.h>
#include <qtextcodec.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <klibloader.h>

#include "pilotSerialDatabase.h"
#include "pilotLocalDatabase.h"
#include "pilotAppCategory.h"

#include "plugin.moc"

ConduitConfigBase::ConduitConfigBase(QWidget *parent,
	const char *name) :
	QObject(parent,name),
	fModified(false),
	fWidget(0L),
	fConduitName(i18n("Unnamed"))
{
	FUNCTIONSETUP;
}

ConduitConfigBase::~ConduitConfigBase()
{
	FUNCTIONSETUP;
}

/* slot */ void ConduitConfigBase::modified()
{
	fModified=true;
	emit changed(true);
}

/* virtual */ QString ConduitConfigBase::maybeSaveText() const
{
	FUNCTIONSETUP;

	return i18n("<qt>The <i>%1</i> conduit's settings have been changed. Do you "
		"want to save the changes before continuing?</qt>").arg(this->conduitName());
}

/* virtual */ bool ConduitConfigBase::maybeSave()
{
	FUNCTIONSETUP;

	if (!isModified()) return true;

	int r = KMessageBox::questionYesNoCancel(fWidget,
		maybeSaveText(),
		i18n("%1 Conduit").arg(this->conduitName()));
	if (r == KMessageBox::Cancel) return false;
	if (r == KMessageBox::Yes) commit();
	return true;
}

ConduitAction::ConduitAction(KPilotDeviceLink *p,
	const char *name,
	const QStringList &args) :
	SyncAction(p,name),
	fDatabase(0L),
	fLocalDatabase(0L),
	fSyncDirection(args),
	fConflictResolution(SyncAction::eAskUser),
	fFirstSync(false)
{
	FUNCTIONSETUP;

	QString cResolution(args.grep(QRegExp(CSL1("--conflictResolution \\d*"))).first());
	if (cResolution.isEmpty())
	{
		fConflictResolution=(SyncAction::ConflictResolution)
			cResolution.replace(QRegExp(CSL1("--conflictResolution (\\d*)")), CSL1("\\1")).toInt();
	}

#ifdef DEBUG
	for (QStringList::ConstIterator it = args.begin();
		it != args.end();
		++it)
	{
		DEBUGCONDUIT << fname << ": " << *it << endl;
	}

	DEBUGCONDUIT << fname << ": Direction=" << fSyncDirection.name() << endl;
#endif
}

/* virtual */ ConduitAction::~ConduitAction()
{
	FUNCTIONSETUP;
	KPILOT_DELETE(fDatabase);
	KPILOT_DELETE(fLocalDatabase);
}

bool ConduitAction::openDatabases_(const QString &name, bool *retrieved)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Trying to open database "
		<< name << endl;
#endif

	KPILOT_DELETE(fLocalDatabase);
	PilotLocalDatabase *localDB = new PilotLocalDatabase(name, true);

	if (!localDB)
	{
		kdWarning() << k_funcinfo
			<< ": Could not initialize object for local copy of database \""
			<< name
			<< "\"" << endl;
		if (retrieved) *retrieved = false;
		return false;
	}

	// if there is no backup db yet, fetch it from the palm, open it and set the full sync flag.
	if (!localDB->isDBOpen() )
	{
		QString dbpath(localDB->dbPathName());
		KPILOT_DELETE(localDB);
#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< ": Backup database "<< dbpath <<" could not be opened. Will fetch a copy from the palm and do a full sync"<<endl;
#endif
		struct DBInfo dbinfo;
		if (fHandle->findDatabase(PilotAppCategory::codec()->fromUnicode( name ), &dbinfo)<0 )
		{
#ifdef DEBUG
			DEBUGCONDUIT << fname
				<< ": Could not get DBInfo for "<<name<<"! "<<endl;
#endif
			if (retrieved) *retrieved = false;
			return false;
		}
#ifdef DEBUG
		DEBUGCONDUIT << fname
				<< ": Found Palm database: "<<dbinfo.name<<endl
				<<"type = "<< dbinfo.type<<endl
				<<"creator = "<< dbinfo.creator<<endl
				<<"version = "<< dbinfo.version<<endl
				<<"index = "<< dbinfo.index<<endl;
#endif
		dbinfo.flags &= ~dlpDBFlagOpen;

		// make sure the dir for the backup db really exists!
		QFileInfo fi(dbpath);
		QString path(QFileInfo(dbpath).dir(TRUE).absPath());
		if (!path.endsWith(CSL1("/"))) path.append(CSL1("/"));
		if (!KStandardDirs::exists(path))
		{
#ifdef DEBUG
			DEBUGCONDUIT << fname << ": Trying to create path for database: <"
				<< path << ">" << endl;
#endif
			KStandardDirs::makeDir(path);
		}
		if (!KStandardDirs::exists(path))
		{
#ifdef DEBUG
			DEBUGCONDUIT << fname << ": Database directory does not exist." << endl;
#endif
			if (retrieved) *retrieved = false;
			return false;
		}

		if (!fHandle->retrieveDatabase(dbpath, &dbinfo) )
		{
#ifdef DEBUG
			DEBUGCONDUIT << fname << ": Could not retrieve database "<<name<<" from the handheld."<<endl;
#endif
			if (retrieved) *retrieved = false;
			return false;
		}
		localDB = new PilotLocalDatabase(name, true);
		if (!localDB || !localDB->isDBOpen())
		{
#ifdef DEBUG
			DEBUGCONDUIT << fname << ": local backup of database "<<name<<" could not be initialized."<<endl;
#endif
			if (retrieved) *retrieved = false;
			return false;
		}
		if (retrieved) *retrieved=true;
	}
	fLocalDatabase = localDB;

	fDatabase = new PilotSerialDatabase(pilotSocket(), name /* On pilot */);

	if (!fDatabase)
	{
		kdWarning() << k_funcinfo
			<< ": Could not open database \""
			<< name
			<< "\" on the pilot."
			<< endl;
	}

	return (fDatabase && fDatabase->isDBOpen() &&
	        fLocalDatabase && fLocalDatabase->isDBOpen() );
}

// This whole function is for debugging purposes only.
bool ConduitAction::openDatabases_(const QString &dbName,const QString &localPath)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Doing local test mode for " << dbName << endl;
#endif
	if (localPath.isNull())
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< ": local mode test for one database only."
			<< endl;
#endif
		fDatabase = new PilotLocalDatabase(dbName);
		fLocalDatabase = 0L;
		return false;
	}

	fDatabase = new PilotLocalDatabase(localPath,dbName);
	fLocalDatabase= new PilotLocalDatabase(dbName, true); // From default
	if (!fLocalDatabase || !fDatabase)
	{
#ifdef DEBUG
		const QString *where2 = PilotLocalDatabase::getDBPath();

		QString none = CSL1("<null>");
		DEBUGCONDUIT << fname
			<< ": Could not open both local copies of \""
			<< dbName
			<< "\"" << endl
			<< "Using \""
			<< (where2 ? *where2 : none)
			<< "\" and \""
			<< (localPath.isEmpty() ? localPath : none)
			<< "\""
			<< endl;
#endif
	}
#ifdef DEBUG
	if (fLocalDatabase)
	{
		DEBUGCONDUIT << fname
			<< ": Opened local database "
			<< fLocalDatabase->dbPathName()
			<< (fLocalDatabase->isDBOpen() ? " OK" : "")
			<< endl;
	}
	if (fDatabase)
	{
		DEBUGCONDUIT << fname
			<< ": Opened database "
			<< fDatabase->dbPathName()
			<< (fDatabase->isDBOpen() ? " OK" : "")
			<< endl;
	}
#endif

	return (fDatabase && fLocalDatabase);
}

bool ConduitAction::openDatabases(const QString &dbName, bool *retrieved)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Mode="
		<< (syncMode().isTest() ? "test " : "")
		<< (syncMode().isLocal() ? "local " : "")
		<< endl ;
#endif

	if (syncMode().isLocal())
	{
		return openDatabases_(dbName,CSL1("/tmp/"));
	}
	else
	{
		return openDatabases_(dbName, retrieved);
	}
}

bool ConduitAction::changeSync(SyncMode::Mode m)
{
	FUNCTIONSETUP;

	if ( fSyncDirection.isSync() && SyncMode::eFullSync == m)
	{
		fSyncDirection.setMode(m);
		return true;
	}
	return false;
}

int PluginUtility::findHandle(const QStringList &a)
{
	FUNCTIONSETUP;

	int handle = -1;
	for (QStringList::ConstIterator i = a.begin();
		i != a.end(); ++i)
	{
		if ((*i).left(7) == CSL1("handle="))
		{
			QString s = (*i).mid(7);
			if (s.isEmpty()) continue;

			handle = s.toInt();
#ifdef DEBUG
			DEBUGCONDUIT << fname
				<< ": Got handle "
				<< handle
				<< endl;
#endif
			if (handle<1)
			{
				kdWarning() << k_funcinfo
					<< ": Improbable handle value found."
					<< endl;
			}
			return handle;
		}
	}

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": No handle= parameter found."
		<< endl;
#endif

	return -1;
}

bool PluginUtility::isModal(const QStringList &a)
{
	return a.contains(CSL1("modal"));
}

/* static */ bool PluginUtility::isRunning(const QCString &n)
{
	DCOPClient *dcop = KApplication::kApplication()->dcopClient();
	QCStringList apps = dcop->registeredApplications();
	return apps.contains(n);
}


/* static */ long PluginUtility::pluginVersion(const KLibrary *lib)
{
	QString symbol = CSL1("version_");
	symbol.append(lib->name());

	if (!lib->hasSymbol(symbol.latin1())) return 0;

	long *p = (long *)(lib->symbol(symbol.latin1()));
	return *p;
}


/* static */ QString PluginUtility::pluginVersionString(const KLibrary *lib)
{
	QString symbol= CSL1("id_");
	symbol.append(lib->name());

	if (!lib->hasSymbol(symbol.latin1())) return QString::null;

	return QString::fromLatin1(*((const char **)(lib->symbol(symbol.latin1()))));
}


