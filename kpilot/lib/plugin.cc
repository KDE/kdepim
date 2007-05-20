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
#include <q3hbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qtabwidget.h>
#include <q3textview.h>
#include <qtimer.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <QPixmap>

#include <k3activelabel.h>
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
		l->loadIcon(QString::fromLatin1(p->appName()),
		K3Icon::Desktop,
		64, K3Icon::DefaultState, 0L,
		true);

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



	Q3ValueList<KAboutPerson> pl = p->authors();
	Q3ValueList<KAboutPerson>::ConstIterator i;

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
	const QStringList &args) :
	SyncAction(p),
	fDatabase(0L),
	fLocalDatabase(0L),
	fCtrHH(0L),
	fCtrPC(0L),
	fSyncDirection(args),
	fConflictResolution(SyncAction::eAskUser),
	fFirstSync(false)
{
	FUNCTIONSETUP;

	QString cResolution(args.filter(QRegExp(CSL1("--conflictResolution \\d*"))).first());
	if (cResolution.isEmpty())
	{
		fConflictResolution=(SyncAction::ConflictResolution)
			cResolution.replace(QRegExp(CSL1("--conflictResolution (\\d*)")), CSL1("\\1")).toInt();
	}

	for (QStringList::ConstIterator it = args.begin();
		it != args.end();
		++it)
	{
		DEBUGKPILOT << fname << ": " << *it << endl;
	}

	DEBUGKPILOT << fname << ": Direction=" << fSyncDirection.name() << endl;
	fCtrHH = new CUDCounter(i18n("Handheld"));
	fCtrPC = new CUDCounter(i18n("PC"));
}

/* virtual */ ConduitAction::~ConduitAction()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fDatabase);
	KPILOT_DELETE(fLocalDatabase);

	KPILOT_DELETE(fCtrHH);
	KPILOT_DELETE(fCtrPC);
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

	DEBUGKPILOT << fname << ": localPathName: [" << localPathName
		<< "]" << endl;

	PilotLocalDatabase *localDB = new PilotLocalDatabase( localPathName );

	if (!localDB)
	{
		WARNINGKPILOT << "Could not initialize object for local copy of database \""
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
			WARNINGKPILOT << "Could not get DBInfo for " << name << endl;
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
			DEBUGKPILOT << fname << ": Database directory does not exist." << endl;
			if (retrieved) *retrieved = false;
			return false;
		}

		if (!deviceLink()->retrieveDatabase(dbpath, &dbinfo) )
		{
			WARNINGKPILOT << "Could not retrieve database "
				<< name << " from the handheld." << endl;
			if (retrieved) *retrieved = false;
			return false;
		}
		localDB = new PilotLocalDatabase( localPathName );
		if (!localDB || !localDB->isOpen())
		{
			WARNINGKPILOT << "local backup of database " << name << " could not be initialized." << endl;
			if (retrieved) *retrieved = false;
			return false;
		}
		if (retrieved) *retrieved=true;
	}
	fLocalDatabase = localDB;

	fDatabase = deviceLink()->database( name );

	if (!fDatabase)
	{
		WARNINGKPILOT << "Could not open database \""
			<< name
			<< "\" on the pilot."
			<< endl;
	}
	else
	{
		fCtrHH->setStartCount(fDatabase->recordCount());
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

void ConduitAction::finished()
{
	FUNCTIONSETUP;

	if (fDatabase && fCtrHH)
		fCtrHH->setEndCount(fDatabase->recordCount());

	if (fCtrHH && fCtrPC)
	{
		addSyncLogEntry(fCtrHH->moo() +"\n",false);
		DEBUGKPILOT << fname << ": " << fCtrHH->moo() << endl;
		addSyncLogEntry(fCtrPC->moo() +"\n",false);
		DEBUGKPILOT << fname << ": " << fCtrPC->moo() << endl;

		// STEP2 of making sure we don't delete our little user's
		// precious data...
		// sanity checks for handheld...
		int hhVolatility = fCtrHH->percentDeleted() +
				 fCtrHH->percentUpdated() +
		    		 fCtrHH->percentCreated();

		int pcVolatility = fCtrPC->percentDeleted() +
				 fCtrPC->percentUpdated() +
		    		 fCtrPC->percentCreated();

		// TODO: allow user to configure this...
		// this is a percentage...
		int allowedVolatility = 70;

		QString caption = i18n("Large Changes Detected");
		// args are already i18n'd
		KLocalizedString template_query = ki18n("The %1 conduit has made a "
			         "large number of changes to your %2.  Do you want "
			         "to allow this change?\nDetails:\n\t%3");

		if (hhVolatility > allowedVolatility)
		{
			QString query = template_query.subs(fConduitName)
				.subs(fCtrHH->type()).subs(fCtrHH->moo()).toString();

			DEBUGKPILOT << fname << ": high volatility."
				<< "  Check with user: [" << query
				<< "]." << endl;

			/*
			int rc = questionYesNo(query, caption,
				QString::null, 0 );
			if (rc == KMessageBox::Yes)
			{
				// TODO: add commit and rollback code.
				// note: this will require some thinking,
				// since we have to undo changes to the
				// pilot databases, changes to the PC
				// resources, changes to the mappings files
				// (record id mapping, etc.)
			}
			*/
		}
		if (pcVolatility > allowedVolatility)
		{
			QString query = template_query.subs(fConduitName)
				.subs(fCtrPC->type()).subs(fCtrPC->moo()).toString();

			DEBUGKPILOT << fname << ": high volatility."
				<< "  Check with user: [" << query
				<< "]." << endl;
		}
	}

}


ConduitProxy::ConduitProxy(KPilotLink *p,
	const QString &name,
	const SyncAction::SyncMode &m) :
	ConduitAction(p,m.list()),
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
		WARNINGKPILOT << "Can't find desktop file for conduit "
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
		WARNINGKPILOT << "Can't load library "
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
		WARNINGKPILOT << "Library "
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
		WARNINGKPILOT << "Can't find factory in library "
			<< fLibraryName
			<< endl;
		addSyncLogEntry(i18n("Could not initialize conduit %1.",fDesktopName));
		return false;
	}

	QStringList l = syncMode().list();

	DEBUGKPILOT << fname << ": Flags: " << syncMode().name() << endl;

	QObject *object = factory->create(fHandle,"SyncAction",l);

	if (!object)
	{
		WARNINGKPILOT << "Can't create SyncAction." << endl;
		addSyncLogEntry(i18n("Could not create conduit %1.",fDesktopName));
		return false;
	}

	fConduit = dynamic_cast<ConduitAction *>(object);

	if (!fConduit)
	{
		WARNINGKPILOT << "Can't cast to ConduitAction." << endl;
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
		WARNINGKPILOT << "Unknown conduit @"
			<< (void *) p
			<< " finished."
			<< endl;
		emit syncDone(this);
		return;
	}

	// give our worker a chance to sanity check the results...
	fConduit->finished();

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

	DEBUGKPILOT << fname << ": Symbol <" << symbol << '>' << endl;

	unsigned long *p = (unsigned long *)(lib->resolveSymbol(symbol.toLatin1()));
        if ( !p )
		return 0;

	return *p;
}



}


CUDCounter::CUDCounter(QString s) :
	fC(0),fU(0),fD(0),fStart(0),fEnd(0),fType(s)
{
}

void CUDCounter::created(unsigned int c)
{
	fC += c;
}

void CUDCounter::updated(unsigned int c)
{
	fU += c;
}

void CUDCounter::deleted(unsigned int c)
{
	fD += c;
}

void CUDCounter::setStartCount(unsigned int t)
{
	fStart = t;
}

void CUDCounter::setEndCount(unsigned int t)
{
	fEnd = t;
}

QString CUDCounter::moo() const
{
	QString result = fType + ": " +
		i18n("Start: %1. End: %2. ",fStart,fEnd);

	if (fC > 0) result += i18n("%1 new. ",fC);
	if (fU > 0) result += i18n("%1 changed. ",fU);
	if (fD > 0) result += i18n("%1 deleted. ",fD);

	if ( (fC+fU+fD) <= 0) result += i18n("No changes made. ");

	return result;
}


