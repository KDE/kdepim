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

#include <qdir.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qtabwidget.h>
#include <qtimer.h>
#include <QPixmap>
#include <QString>
#include <qtextedit.h>

#include <kglobal.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <klibloader.h>
#include <kmessagebox.h>
#include <kservice.h>
#include <kservicetype.h>
#include <kstandarddirs.h>

#include "pilotSerialDatabase.h"
#include "pilotLocalDatabase.h"

#include "plugin.moc"

ConduitConfigBase::ConduitConfigBase(QWidget *parent,
	const char *name) :
	QObject(parent),
	fModified(false),
	fWidget(0L),
	fConduitName(i18n("Unnamed"))
{
	FUNCTIONSETUP;
	if (name)
	{
		setObjectName(name);
	}
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
		"want to save the changes before continuing?</qt>",this->conduitName());
}

/* virtual */ bool ConduitConfigBase::maybeSave()
{
	FUNCTIONSETUP;

	if (!isModified()) return true;

	int r = KMessageBox::questionYesNoCancel(fWidget,
		maybeSaveText(),
		i18n("%1 Conduit",this->conduitName()), KStandardGuiItem::save(), KStandardGuiItem::discard());
	if (r == KMessageBox::Cancel) return false;
	if (r == KMessageBox::Yes) commit();
	return true;
}

QWidget *ConduitConfigBase::aboutPage(QWidget *parent, KAboutData *ad)
{
	FUNCTIONSETUP;

	QWidget *w = new QWidget(parent);
	w->setObjectName("aboutpage");

	QString s;
	QLabel *text;
	KIconLoader *l = KIconLoader::global();
	const KAboutData *p = ad ? ad : KGlobal::mainComponent().aboutData();

	QGridLayout *grid = new QGridLayout(w);
	grid->setSpacing(SPACING);

	grid->setColumnMinimumWidth(0, SPACING);
	grid->setColumnMinimumWidth(4, SPACING);


	QPixmap applicationIcon =
		l->loadIcon(p->appName(),
		K3Icon::Desktop,
		64, K3Icon::DefaultState,
		QStringList(),
		0L, true);

	if (applicationIcon.isNull())
	{
		applicationIcon = l->loadIcon(QString::fromLatin1("kpilot"),
			K3Icon::Desktop);
	}

	text = new QLabel(w);
	// Experiment with a long non-<qt> string. Use that to find
	// sensible widths for the columns.
	//
	text->setText(i18n("Send questions and comments to kdepim-users@kde.org"));
	text->adjustSize();

	int linewidth = text->size().width();
	int lineheight = text->size().height();

	// Use the label to display the application icon
	text->setText(QString::null);
	text->setPixmap(applicationIcon);
	text->adjustSize();
	grid->addWidget(text, 0, 1);


	QTextEdit *linktext = new QTextEdit(w);
	linktext->setReadOnly(true);
	linktext->setMinimumSize(linewidth,qMax(260,60+12*lineheight));
	linktext->setFixedHeight(qMax(260,60+12*lineheight));
	text = new QLabel(w);
	grid->addWidget(text,0,2);
	grid->addWidget(linktext,1,2,1,1);

	// Now set the program and copyright information.
	s = CSL1("<qt><h3>");
	s += p->programName();
	s += ' ';
	s += p->version();
	s += CSL1("</h3>");
	s += p->copyrightStatement() + CSL1("<br/></qt>");
	text->setText(s);

	linktext->append(p->shortDescription() + CSL1("<br>"));

	if (!p->homepage().isEmpty())
	{
		s = QString::null;
		s += CSL1("<a href=\"%1\">").arg(p->homepage());
		s += p->homepage();
		s += CSL1("</a><br>");
		linktext->append(s);
	}

	s = QString::null;
	s += i18n("Send questions and comments to <a href=\"mailto:%1\">%2</a>.", CSL1("kdepim-users@kde.org"), CSL1("kdepim-users@kde.org") );
	s += ' ';
	s += i18n("Send bug reports to <a href=\"mailto:%1\">%2</a>.",p->bugAddress(),p->bugAddress());
	s += ' ';
	s += i18n("For trademark information, see the "
		"<a href=\"help:/kpilot/trademarks.html\">KPilot User's Guide</a>.");
	s += CSL1("<br>");
	linktext->append(s);
	linktext->append(QString::null);



	QList<KAboutPerson> pl = p->authors();
	QList<KAboutPerson>::ConstIterator i;

	s = i18n("<b>Authors:</b> ");

	QString comma = CSL1(", ");

	int count=1;
	for (i=pl.begin(); i!=pl.end(); ++i)
	{
		s.append(CSL1("%1 (<i>%2</i>)%3")
			.arg((*i).name())
			.arg((*i).task())
			.arg( (count<pl.count()) ? comma : QString::null));
		count++;
	}
	linktext->append(s);

	s = QString::null;
	pl = p->credits();
	if (pl.count()>0)
	{
		count=1;
		s.append(i18n("<b>Credits:</b> "));
		for (i=pl.begin(); i!=pl.end(); ++i)
		{
			s.append(CSL1("%1 (<i>%2</i>)%3")
				.arg((*i).name())
				.arg((*i).task())
				.arg(count<pl.count() ? comma : QString::null)
				);
			count++;
		}
	}
	linktext->append(s);

	w->adjustSize();

	return w;
}

/* static */ void ConduitConfigBase::addAboutPage(QTabWidget *tw,
	KAboutData *ad)
{
	FUNCTIONSETUP;

	Q_ASSERT(tw);

	QWidget *w = aboutPage(tw,ad);
	QSize sz = w->size();

	if (sz.width() < tw->size().width())
	{
		sz.setWidth(tw->size().width());
	}
	if (sz.height() < tw->size().height())
	{
		sz.setHeight(tw->size().height());
	}

	tw->resize(sz);
	tw->addTab(w, i18n("About"));
	tw->adjustSize();
}



ConduitAction::ConduitAction(KPilotLink *p,
	const char *name,
	const QStringList &args) :
	SyncAction(p, name),
	fDatabase(0L),
	fLocalDatabase(0L),
	fSyncDirection(args),
	fConflictResolution(SyncAction::eAskUser),
	fFirstSync(false)
{
	FUNCTIONSETUP;
	
	QStringList cResolutions = args.filter(QRegExp(CSL1("--conflictResolution \\d*")));
	if(!cResolutions.isEmpty())
	{
		QString cResolution = cResolutions.first();
		{
			fConflictResolution=(SyncAction::ConflictResolution)
				cResolution.replace(QRegExp(CSL1("--conflictResolution (\\d*)"))
					, CSL1("\\1")).toInt();
		}
	}
	else
	{
		DEBUGKPILOT << fname <<": No conflict resolution given, defaulting to:"
			<< " SyncAction::eAskUser" << endl;
		fConflictResolution = SyncAction::eAskUser;
	}

	for (QStringList::ConstIterator it = args.begin();
		it != args.end();
		++it)
	{
		DEBUGKPILOT << fname <<":" << *it;
	}

	DEBUGKPILOT << fname <<": Direction=" << fSyncDirection.name();
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

	DEBUGKPILOT << fname
		<< ": Trying to open database "
		<< name << endl;
	DEBUGKPILOT << fname
		<< ": Mode="
		<< (syncMode().isTest() ? "test " : "")
		<< (syncMode().isLocal() ? "local " : "")
		<< endl ;

	KPILOT_DELETE(fLocalDatabase);

	QString localPathName = PilotLocalDatabase::getDBPath() + name;

	// we always want to use the conduits/ directory for our local
	// databases. this keeps our backups and data that our conduits use
	// for record keeping separate
	localPathName.replace(CSL1("DBBackup/"), CSL1("conduits/"));

	DEBUGKPILOT << fname <<": localPathName: [" << localPathName
		<< "]" << endl;

	PilotLocalDatabase *localDB = new PilotLocalDatabase( localPathName );

	if (!localDB)
	{
		WARNINGKPILOT <<"Could not initialize object for local copy of database \""
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
		DEBUGKPILOT << fname
			<< ": Backup database " << dbpath
			<< " not found." << endl;
		struct DBInfo dbinfo;

// TODO Extend findDatabase() with extra overload?
		if (deviceLink()->findDatabase(Pilot::toPilot( name ), &dbinfo)<0 )
		{
			WARNINGKPILOT <<"Could not get DBInfo for" << name;
			if (retrieved) *retrieved = false;
			return false;
		}

		DEBUGKPILOT << fname
				<< ": Found Palm database: " << dbinfo.name <<endl
				<< fname << ": type = " << dbinfo.type
				<< " creator = " << dbinfo.creator
				<< " version = " << dbinfo.version
				<< " index = " << dbinfo.index << endl;
		dbinfo.flags &= ~dlpDBFlagOpen;

		// make sure the dir for the backup db really exists!
		QFileInfo fi(dbpath);
		QString path(QFileInfo(dbpath).absolutePath());
		if (!path.endsWith(CSL1("/")))
		{
			path.append(CSL1("/"));
		}
		if (!KStandardDirs::exists(path))
		{
			DEBUGKPILOT << fname 
				<< ": Trying to create path for database: <"
				<< path << '>' << endl;
			KStandardDirs::makeDir(path);
		}
		if (!KStandardDirs::exists(path))
		{
			DEBUGKPILOT << fname <<": Database directory does not exist.";
			if (retrieved) *retrieved = false;
			return false;
		}

		if (!deviceLink()->retrieveDatabase(dbpath, &dbinfo) )
		{
			WARNINGKPILOT <<"Could not retrieve database"
				<< name << " from the handheld." << endl;
			if (retrieved) *retrieved = false;
			return false;
		}
		localDB = new PilotLocalDatabase( localPathName );
		if (!localDB || !localDB->isOpen())
		{
			WARNINGKPILOT <<"local backup of database" << name <<" could not be initialized.";
			if (retrieved) *retrieved = false;
			return false;
		}
		if (retrieved) *retrieved=true;
	}
	fLocalDatabase = localDB;

	fDatabase = deviceLink()->database( name );

	if (!fDatabase)
	{
		WARNINGKPILOT <<"Could not open database \""
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

ConduitProxy::ConduitProxy(KPilotLink *p,
	const QString &name,
	const SyncAction::SyncMode &m) :
	ConduitAction(p,name.toLatin1(),m.list()),
	fDesktopName(name)
{
	FUNCTIONSETUP;
}

/* virtual */ bool ConduitProxy::exec()
{
	FUNCTIONSETUP;

	// query that service
	KSharedPtr < KService > o = KService::serviceByDesktopName(fDesktopName);
	if (!o)
	{
		WARNINGKPILOT <<"Can't find desktop file for conduit"
			<< fDesktopName
			<< endl;
		addSyncLogEntry(i18n("Could not find conduit %1.",fDesktopName));
		return false;
	}


	// load the lib
	fLibraryName = o->library();
	DEBUGKPILOT << fname
		<< ": Loading desktop "
		<< fDesktopName
		<< " with lib "
		<< fLibraryName
		<< endl;

	KLibrary *library = KLibLoader::self()->library(
		QFile::encodeName(fLibraryName));
	if (!library)
	{
		WARNINGKPILOT <<"Can't load library"
			<< fLibraryName
			<< " - "
			<< KLibLoader::self()->lastErrorMessage()
			<< endl;
		addSyncLogEntry(i18n("Could not load conduit %1.",fDesktopName));
		return false;
	}

	unsigned long version = PluginUtility::pluginVersion(library);
	if ( Pilot::PLUGIN_API != version )
	{
		WARNINGKPILOT <<"Library"
			<< fLibraryName
			<< " has version "
			<< version
			<< endl;
		addSyncLogEntry(i18n("Conduit %1 has wrong version (%2).",fDesktopName,version));
		return false;
	}

	KLibFactory *factory = library->factory();
	if (!factory)
	{
		WARNINGKPILOT <<"Can't find factory in library"
			<< fLibraryName
			<< endl;
		addSyncLogEntry(i18n("Could not initialize conduit %1.",fDesktopName));
		return false;
	}

	QStringList l = syncMode().list();

	DEBUGKPILOT << fname <<": Flags:" << syncMode().name();

	QObject *object = factory->create(fHandle,"SyncAction",l);

	if (!object)
	{
		WARNINGKPILOT <<"Can't create SyncAction.";
		addSyncLogEntry(i18n("Could not create conduit %1.",fDesktopName));
		return false;
	}

	fConduit = dynamic_cast<ConduitAction *>(object);

	if (!fConduit)
	{
		WARNINGKPILOT <<"Can't cast to ConduitAction.";
		addSyncLogEntry(i18n("Could not create conduit %1.",fDesktopName));
		return false;
	}

	addSyncLogEntry(i18n("[Conduit %1]",fDesktopName));

	// Handle the syncDone signal properly & unload the conduit.
	QObject::connect(fConduit,SIGNAL(syncDone(SyncAction *)),
		this,SLOT(execDone(SyncAction *)));
	// Proxy all the log and error messages.
	QObject::connect(fConduit,SIGNAL(logMessage(const QString &)),
		this,SIGNAL(logMessage(const QString &)));
	QObject::connect(fConduit,SIGNAL(logError(const QString &)),
		this,SIGNAL(logError(const QString &)));
	QObject::connect(fConduit,SIGNAL(logProgress(const QString &,int)),
		this,SIGNAL(logProgress(const QString &,int)));

	QTimer::singleShot(0,fConduit,SLOT(execConduit()));
	return true;
}

void ConduitProxy::execDone(SyncAction *p)
{
	FUNCTIONSETUP;

	if (p!=fConduit)
	{
		WARNINGKPILOT <<"Unknown conduit @"
			<< (void *) p
			<< " finished."
			<< endl;
		emit syncDone(this);
		return;
	}

	addSyncLogEntry(CSL1("\n"),false); // Put bits of the conduit logs on separate lines

	KPILOT_DELETE(p);

	emit syncDone(this);
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

/* static */ unsigned long pluginVersion(const KLibrary *lib)
{
	FUNCTIONSETUP;
	QString symbol = CSL1("version_");
	symbol.append(lib->name());

	DEBUGKPILOT << fname <<": Symbol <" << symbol << '>';

	unsigned long *p = (unsigned long *)(lib->resolveSymbol(symbol.toLatin1()));
        if ( !p )
		return 0;

	return *p;
}

}
