/* KPilot
**
** Copyright (C) 2001 by Dan Pilone <dan@kpilot.org>
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

#include "plugin.h"
#include "options.h"

#include <stdlib.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QRegExp>
#include <QtCore/QVariantList>
#include <QtCore/QTimer>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPixmap>
#include <QtGui/QPushButton>
#include <QtGui/QTabWidget>

#include <kdebug.h>
#include <kglobal.h>
#include <ktextedit.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kpluginloader.h>
#include <kpluginfactory.h>
#include <kmessagebox.h>
#include <kservice.h>
#include <kservicetype.h>
#include <kstandarddirs.h>

#include "pilotSerialDatabase.h"
#include "pilotLocalDatabase.h"

#include "plugin.moc"

ConduitConfigBase::ConduitConfigBase(QWidget *parent, const QVariantList &args) :
	QObject(parent),
	fModified(false),
	fWidget(0L),
	fConduitName(i18n("Unnamed"))
{
	FUNCTIONSETUP;
	
	if( !args.isEmpty() )
	{
		setObjectName( args[0].toString() );
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
		KIconLoader::Desktop,
		64, KIconLoader::DefaultState,
		QStringList(),
		0L, true);

	if (applicationIcon.isNull())
	{
		applicationIcon = l->loadIcon(QString::fromLatin1("kpilot"),
			KIconLoader::Desktop);
	}

	text = new QLabel(w);
	// Experiment with a long non-<qt> string. Use that to find
	// sensible widths for the columns.
	text->setText(i18n("Send questions and comments to kdepim-users@kde.org"));
	text->adjustSize();

	// Use the label to display the application icon
	text->setText(QString());
	text->setPixmap(applicationIcon);
	text->adjustSize();
	grid->addWidget(text, 0, 1);


	KTextEdit *linktext = new KTextEdit(w);
	linktext->setReadOnly(true);

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
		s.clear();
		s += CSL1("<a href=\"%1\">").arg(p->homepage());
		s += p->homepage();
		s += CSL1("</a><br>");
		linktext->append(s);
	}

	s.clear();
	s += i18n("Send questions and comments to <a href=\"mailto:%1\">%2</a>.", CSL1("kdepim-users@kde.org"), CSL1("kdepim-users@kde.org") );
	s += ' ';
	s += i18n("Send bug reports to <a href=\"mailto:%1\">%2</a>.",p->bugAddress(),p->bugAddress());
	s += ' ';
	s += i18n("For trademark information, see the "
		"<a href=\"help:/kpilot/trademarks.html\">KPilot User's Guide</a>.");
	s += CSL1("<br>");
	linktext->append(s);
	linktext->append(QString());



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
			.arg( (count<pl.count()) ? comma : QString()));
		count++;
	}
	linktext->append(s);

	s.clear();
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
				.arg(count<pl.count() ? comma : QString())
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
	const QVariantList &args) :
	SyncAction(p, name),
	fDatabase(0L),
	fLocalDatabase(0L),
	fConflictResolution(SyncAction::eAskUser),
	fFirstSync(false)
{
	FUNCTIONSETUP;

	QStringList sArgs;
	for( int i = 0; i < args.size(); ++i )
	{
		sArgs << args.at( i ).toString();
	}
	
	fSyncDirection = SyncAction::SyncMode( sArgs );

	QStringList cResolutions = sArgs.filter(QRegExp(CSL1("--conflictResolution \\d*")));
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
		DEBUGKPILOT << "No conflict resolution given, defaulting to:"
			<< " SyncAction::eAskUser";
		fConflictResolution = SyncAction::eAskUser;
	}
 
	DEBUGKPILOT << "args passed in: [" << sArgs.join(",") << "]";
	DEBUGKPILOT << "Direction=" << fSyncDirection.name();
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

	DEBUGKPILOT << "Trying to open database [" << name << ']';
	DEBUGKPILOT << "Mode="
		<< (syncMode().isTest() ? "test " : "")
		<< (syncMode().isLocal() ? "local " : "")
		<< (syncMode().isSync() ? "sync " : "");

	KPILOT_DELETE(fLocalDatabase);

	QString localPathName = PilotLocalDatabase::getDBPath() + name;

	// we always want to use the conduits/ directory for our local
	// databases. this keeps our backups and data that our conduits use
	// for record keeping separate
	localPathName.replace(CSL1("DBBackup/"), CSL1("conduits/"));

	DEBUGKPILOT << "localPathName: [" << localPathName << ']';

	PilotLocalDatabase *localDB = new PilotLocalDatabase( localPathName );

	if (!localDB)
	{
		WARNINGKPILOT << "Could not initialize object for local copy of database ["
			<< name
			<< ']';
		if (retrieved) 
		{
			*retrieved = false;
		}
		return false;
	}

	// if there is no backup db yet, fetch it from the palm, open it and set the full sync flag.
	if (!localDB->isOpen() )
	{
		QString dbpath(localDB->dbPathName());
		KPILOT_DELETE(localDB);
		DEBUGKPILOT 
			<< "Backup database [" << dbpath
			<< "] not found.";
		struct DBInfo dbinfo;

// TODO Extend findDatabase() with extra overload?
		if (deviceLink()->findDatabase(Pilot::toPilot( name ), &dbinfo)<0 )
		{
			WARNINGKPILOT << "Could not get DBInfo for" << name;
			if (retrieved) *retrieved = false;
			return false;
		}

		DEBUGKPILOT << "Found Palm database: [" << dbinfo.name << ']';
		DEBUGKPILOT << "type = " << dbinfo.type
			<< " creator = " << dbinfo.creator
			<< " version = " << dbinfo.version
			<< " index = " << dbinfo.index;
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
			DEBUGKPILOT << "Trying to create path for database: ["
				<< path << ']';
			KStandardDirs::makeDir(path);
		}
		if (!KStandardDirs::exists(path))
		{
			DEBUGKPILOT << "Database directory does not exist.";
			if (retrieved) *retrieved = false;
			return false;
		}

		if (!deviceLink()->retrieveDatabase(dbpath, &dbinfo) )
		{
			WARNINGKPILOT << "Could not retrieve database ["
				<< name << "] from the handheld.";
			if (retrieved) *retrieved = false;
			return false;
		}
		localDB = new PilotLocalDatabase( localPathName );
		if (!localDB || !localDB->isOpen())
		{
			WARNINGKPILOT << "local backup of database" << name << " could not be initialized.";
			if (retrieved) *retrieved = false;
			return false;
		}
		if (retrieved) *retrieved=true;
	}
	fLocalDatabase = localDB;

	fDatabase = deviceLink()->database( name );

	if (!fDatabase)
	{
		WARNINGKPILOT << "Could not open database ["
			<< name << "] on the pilot.";
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
	ConduitAction(p,name.toLatin1(), QVariantList() << QVariant::fromValue(m) ),
	fDesktopName(name)
{
	FUNCTIONSETUP;
}

/* virtual */ bool ConduitProxy::exec()
{
	FUNCTIONSETUP;

	// query that service
    KService::Ptr service = KService::serviceByDesktopName(fDesktopName);
	if (!service)
	{
		WARNINGKPILOT << "Cannot find desktop file for conduit ["
			<< fDesktopName << ']';
		addSyncLogEntry(i18n("Could not find conduit %1.",fDesktopName));
		return false;
	}

    KPluginLoader loader(*service.constData());

    if (!loader.isLoaded()) {
		WARNINGKPILOT << "Cannot load library ["
            << service->library()
			<< "] - "
            << loader.errorString();
		addSyncLogEntry(i18n("Could not load conduit %1.",fDesktopName));
		return false;
    }
    
    if (loader.pluginVersion() != Pilot::PLUGIN_API) {
		WARNINGKPILOT << "Library [" << service->library()
			<< "] has version " << loader.pluginVersion();
		addSyncLogEntry(i18n("Conduit %1 has wrong version (%2).",fDesktopName,loader.pluginVersion()));
		return false;
    }

	KPluginFactory *factory = loader.factory();
	if (!factory)
	{
		WARNINGKPILOT << "Cannot find factory in library ["
			<< service->library() << ']';
		addSyncLogEntry(i18n("Could not initialize conduit %1.",fDesktopName));
		return false;
	}

	QStringList l = syncMode().list();

	DEBUGKPILOT << "Flags:" << syncMode().name();
	
	QVariant v;
	v.setValue( syncMode() );
	
	QObject *object = factory->create<ConduitAction>( fHandle, QVariantList() << v );

	if (!object)
	{
		WARNINGKPILOT << "Cannot create SyncAction.";
		addSyncLogEntry(i18n("Could not create conduit %1.",fDesktopName));
		return false;
	}

	fConduit = dynamic_cast<ConduitAction *>(object);

	if (!fConduit)
	{
		WARNINGKPILOT << "Cannot cast to ConduitAction.";
		addSyncLogEntry(i18n("Could not create conduit %1.", fDesktopName));
		return false;
	}
	
	addSyncLogEntry(i18n("[Conduit %1]\n",fDesktopName));
	DEBUGKPILOT << "Conduit: [" << fDesktopName << "]";

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
		WARNINGKPILOT << "Unknown conduit @"
			<< (void *) p
			<< " finished." ;
		delayDone();
		return;
	}

	addSyncLogEntry(CSL1("\n"),false); // Put bits of the conduit logs on separate lines

	p->deleteLater();

	delayDone();
}
