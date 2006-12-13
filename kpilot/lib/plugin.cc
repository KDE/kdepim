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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
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

#include <dcopclient.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <klibloader.h>

#include "pilotSerialDatabase.h"
#include "pilotLocalDatabase.h"

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
		i18n("%1 Conduit").arg(this->conduitName()), KStdGuiItem::save(), KStdGuiItem::discard());
	if (r == KMessageBox::Cancel) return false;
	if (r == KMessageBox::Yes) commit();
	return true;
}

ConduitAction::ConduitAction(KPilotLink *p,
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

	for (QStringList::ConstIterator it = args.begin();
		it != args.end();
		++it)
	{
		DEBUGLIBRARY << fname << ": " << *it << endl;
	}

	DEBUGLIBRARY << fname << ": Direction=" << fSyncDirection.name() << endl;
}

/* virtual */ ConduitAction::~ConduitAction()
{
	FUNCTIONSETUP;
	KPILOT_DELETE(fDatabase);
	KPILOT_DELETE(fLocalDatabase);
}

bool ConduitAction::openDatabases(const QString &name, bool *retrieved)
{
	FUNCTIONSETUP;

	DEBUGLIBRARY << fname
		<< ": Trying to open database "
		<< name << endl;
	DEBUGLIBRARY << fname
		<< ": Mode="
		<< (syncMode().isTest() ? "test " : "")
		<< (syncMode().isLocal() ? "local " : "")
		<< endl ;

	KPILOT_DELETE(fLocalDatabase);

	QString localPathName = PilotLocalDatabase::getDBPath() + name;
	PilotLocalDatabase *localDB = new PilotLocalDatabase( localPathName );

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
	if (!localDB->isOpen() )
	{
		QString dbpath(localDB->dbPathName());
		KPILOT_DELETE(localDB);
		DEBUGLIBRARY << fname
			<< ": Backup database " << dbpath
			<< " not found." << endl;
		struct DBInfo dbinfo;

// TODO Extend findDatabase() with extra overload?
		if (deviceLink()->findDatabase(Pilot::toPilot( name ), &dbinfo)<0 )
		{
			kdWarning() << k_funcinfo
				<< ": Could not get DBInfo for " << name << endl;
			if (retrieved) *retrieved = false;
			return false;
		}

		DEBUGLIBRARY << fname
				<< ": Found Palm database: " << dbinfo.name <<endl
				<< fname << ": type = " << dbinfo.type
				<< " creator = " << dbinfo.creator
				<< " version = " << dbinfo.version
				<< " index = " << dbinfo.index << endl;
		dbinfo.flags &= ~dlpDBFlagOpen;

		// make sure the dir for the backup db really exists!
		QFileInfo fi(dbpath);
		QString path(QFileInfo(dbpath).dir(true).absPath());
		if (!path.endsWith(CSL1("/"))) path.append(CSL1("/"));
		if (!KStandardDirs::exists(path))
		{
			DEBUGLIBRARY << fname << ": Trying to create path for database: <"
				<< path << ">" << endl;
			KStandardDirs::makeDir(path);
		}
		if (!KStandardDirs::exists(path))
		{
			DEBUGLIBRARY << fname << ": Database directory does not exist." << endl;
			if (retrieved) *retrieved = false;
			return false;
		}

		if (!deviceLink()->retrieveDatabase(dbpath, &dbinfo) )
		{
			kdWarning() << k_funcinfo << ": Could not retrieve database "<<name<<" from the handheld."<<endl;
			if (retrieved) *retrieved = false;
			return false;
		}
		localDB = new PilotLocalDatabase( localPathName );
		if (!localDB || !localDB->isOpen())
		{
			kdWarning() << k_funcinfo << ": local backup of database "<<name<<" could not be initialized."<<endl;
			if (retrieved) *retrieved = false;
			return false;
		}
		if (retrieved) *retrieved=true;
	}
	fLocalDatabase = localDB;

	fDatabase = deviceLink()->database( name );

	if (!fDatabase)
	{
		kdWarning() << k_funcinfo
			<< ": Could not open database \""
			<< name
			<< "\" on the pilot."
			<< endl;
	}

	return (fDatabase && fDatabase->isOpen() &&
	        fLocalDatabase && fLocalDatabase->isOpen() );
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


namespace PluginUtility
{

QString findArgument(const QStringList &a, const QString &arg)
{
	FUNCTIONSETUP;

	QString search;

	if (arg.startsWith( CSL1("--") ))
	{
		search = arg;
	}
	else
	{
		search = CSL1("--") + arg;
	}
	search.append( CSL1("=") );


	QStringList::ConstIterator end = a.end();
	for (QStringList::ConstIterator i = a.begin(); i != end; ++i)
	{
		if ((*i).startsWith( search ))
		{
			QString s = (*i).mid(search.length());
			return s;
		}
	}

	return QString::null;
}

/* static */ bool isRunning(const QCString &n)
{
	DCOPClient *dcop = KApplication::kApplication()->dcopClient();
	QCStringList apps = dcop->registeredApplications();
	return apps.contains(n);
}


/* static */ unsigned long pluginVersion(const KLibrary *lib)
{
	QString symbol = CSL1("version_");
	symbol.append(lib->name());

	if (!lib->hasSymbol(symbol.latin1())) return 0;

	unsigned long *p = (unsigned long *)(lib->symbol(symbol.latin1()));
	return *p;
}


/* static */ QString pluginVersionString(const KLibrary *lib)
{
	QString symbol= CSL1("id_");
	symbol.append(lib->name());

	if (!lib->hasSymbol(symbol.latin1())) return QString::null;

	return QString::fromLatin1(*((const char **)(lib->symbol(symbol.latin1()))));
}


}

