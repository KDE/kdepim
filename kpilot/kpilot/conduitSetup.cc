// conduitSetup.cc
//
// Copyright (C) 1998,1999,2000 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING.



// REVISION HISTORY
//
// 3.1b9	By Dan Pilone
// 3.1b10	By Adriaan de Groot: comments added all over the place,
//		removed superfluous "Conduit Setup" label in dialog,
//		moved some UI elements around and added a "Cancel"
//		button.
//
//		Remaining questions are marked with QADE.

#include "options.h"


#include <stdio.h>
#include <unistd.h>
#include <iostream.h>

#include <qdir.h>
#include <qhbox.h>

#include <ksimpleconfig.h>
#include <kapp.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kstddirs.h>
#include <kdebug.h>
#include <kuserprofile.h>
#include <kservice.h>
#include <kservicetype.h>

#include "conduitSetup.moc"

#include "kpilot.h"

static const char *id="$Id$";

#include "listCat.h"


// We're going to hide all kinds of information in the
// QListViewItems we're manipulating. These constants
// define which information goes in which text().
//
// This order must be respected in the constructors
// for QListViewItems in fillLists().
//
//
#define CONDUIT_NAME	(0)
#define CONDUIT_COMMENT	(1)
#define CONDUIT_DESKTOP	(2)
#define CONDUIT_EXEC	(3)

CConduitSetup::CConduitSetup(QWidget *parent, const char *name) :
	KDialogBase(parent,name,true,
		i18n("External Conduit Setup"),
		Ok | Cancel,Cancel),
	conduitSetup(0L)
{
	FUNCTIONSETUP;

	QHBox *top = makeHBoxMainWidget();

	categories = new ListCategorizer(top,"conduits");
	categories->setStartOpen(true);

	active=categories->addCategory(i18n("Active"),
		i18n("Conduits that will run when a hot-sync is done"));
	available=categories->addCategory(i18n("Available"),
		i18n("Conduits installed on your system but not active"));

	connect(categories,SIGNAL(executed(QListViewItem *)),
		this,
		SLOT(conduitExecuted(QListViewItem *)));


	fillLists();
	adjustSize();

	conduitPaths = KGlobal::dirs()->resourceDirs("conduits");
}

/* virtual */ CConduitSetup::~CConduitSetup()
{
	FUNCTIONSETUP;
}

void CConduitSetup::conduitExecuted(QListViewItem *p)
{
	FUNCTIONSETUP;
	if (!p)
	{
		kdDebug() << fname << ": Executed NULL conduit?" << endl;
		return;
	}

	if (!p->parent())
	{
		kdDebug() << fname << ": Executed a category?" << endl;
		return;
	}

#ifdef DEBUG
	if (debug_level & SYNC_MAJOR)
	{
		kdDebug() << fname
			<< ": Executing conduit "
			<< p->text(0)
			<< endl;
	}
#endif

	QString execPath = findExecPath(p);
	if (execPath.isNull()) 
	{
		warnNoExec(p);
		return;
	}

	if (conduitSetup)
	{
		warnSetupRunning();
		return;
	}

	conduitSetup = new KProcess;
	*conduitSetup << execPath.local8Bit() << "--setup" ;
	*conduitSetup << "-geometry"
		<< QString("+%1+%2").arg(x()+20).arg(y()+20);
	if (debug_level)
	{
		*conduitSetup << "--debug" << QString::number(debug_level);
	}

	connect(conduitSetup,
		SIGNAL(processExited(KProcess *)),
		this,
		SLOT(setupDone(KProcess *))
		);
	conduitSetup->start(KProcess::NotifyOnExit);
}

/* slot */ void CConduitSetup::setupDone(KProcess *p)
{
	FUNCTIONSETUP;

	if (p!=conduitSetup)
	{
		kdDebug() << fname << ": Process other than setup exited?"
			<< endl;
		return;
	}

	delete p;
	conduitSetup=0L;
}

void CConduitSetup::slotOk()
{
	FUNCTIONSETUP;
	writeInstalledConduits();
	KDialogBase::slotOk();
}

#if 0
void CConduitSetup::slotCancel()
{
	FUNCTIONSETUP;
	setResult(0);
	close();
}
#endif

void
CConduitSetup::fillLists()
{
	FUNCTIONSETUP;

	KConfig& config = KPilotLink::getConfig("Conduit Names");
	QStringList potentiallyInstalled =
		config.readListEntry("InstalledConduits");
	KServiceTypeProfile::OfferList offers = 
		KServiceTypeProfile::offers("KPilotConduit");

#ifdef DEBUG
	if (debug_level & UI_TEDIOUS)
	{
		QStringList::Iterator i = potentiallyInstalled.begin();


		kdDebug() << fname << ": Currently active conduits are:"
			<< endl;

		while(i != potentiallyInstalled.end())
		{
			kdDebug() << fname << ": "
				<< (*i)
				<< endl;
			++i;
		}

		kdDebug() << fname << ": Currently installed conduits are:"
			<< endl;
	}
#endif

	// Now actually fill the two list boxes, just make 
	// sure that nothing gets listed in both.
	//
	//
	QValueListIterator<KServiceOffer> availList(offers.begin());
	while(availList != offers.end())
	{
		KSharedPtr<KService> o = (*availList).service();

#ifdef DEBUG
		if (debug_level & UI_TEDIOUS)
		{
			kdDebug() << fname << ": "
				<< o->desktopEntryName()
				<< " = "
				<< o->name()
				<< endl;
		}
#endif
		if(potentiallyInstalled.contains(o->desktopEntryName()) == 0)
		{
			(void) new QListViewItem(available,
				o->name(),
				o->comment(),
				o->desktopEntryName(),
				o->exec());
		}
		else
		{
			(void) new QListViewItem(active,
				o->name(),
				o->comment(),
				o->desktopEntryName(),
				o->exec());
		}

		++availList;
	}
}


QString CConduitSetup::findExecPath(const QListViewItem *p) const
{
	FUNCTIONSETUP;

	QString currentConduit(QString::null);

	if (conduitPaths.isEmpty())
	{
		currentConduit=KGlobal::dirs()->findResource("exe",
			p->text(CONDUIT_EXEC));
		if (currentConduit.isNull())
		{
			currentConduit=p->text(CONDUIT_EXEC);
		}
	}
	else
	{
		// Look in all the resource dirs for conduits
		// for this particular conduit.
		//
		//
		QStringList::ConstIterator i;

		currentConduit=QString::null;
		for (i=conduitPaths.begin();
			i!=conduitPaths.end();
			++i)
		{
			if (QFile::exists(
				(*i)+'/'+p->text(CONDUIT_EXEC)))
			{
				currentConduit=
				(*i)+'/'+p->text(CONDUIT_EXEC);
				break;
			}
		}
	}

	return currentConduit;
}


void CConduitSetup::writeInstalledConduits()
{
	FUNCTIONSETUP;

	char dbName[255];
	int len = 0;

	KConfig& config = KPilotLink::getConfig("Conduit Names");
	config.writeEntry("InstalledConduits", 
		categories->listSiblings(active->firstChild(),CONDUIT_DESKTOP));
	config.setGroup("Database Names");

	const QListViewItem *p=active->firstChild();
	while (p)
	{
		FILE *conduitpipe;

#ifdef DEBUG
		if (debug_level & SYNC_TEDIOUS)
		{
			kdDebug() << fname << ": Current conduit = "
				<< p->text(CONDUIT_NAME)
				<< endl;
		}
#endif

#if 0
		KService::Ptr conduit = 
			KService::serviceByDesktopName(p->text(CONDUIT_DESKTOP));
		if (!conduit)
		{
			kdDebug() << fname << ": No service associated with "
				<< iter
				<< endl;
			continue;
		}
#endif

#ifdef DEBUG
		if (debug_level & SYNC_TEDIOUS)
		{
			kdDebug() << fname << ": Current conduit service from "
				<< p->text(CONDUIT_DESKTOP)
				<< " says exec="
				<< p->text(CONDUIT_EXEC)
				<< endl;
		}
#endif
		QString currentConduit = findExecPath(p);

		if (currentConduit.isNull())
		{
			warnNoExec(p);
			goto nextConduit;
		}



		currentConduit+=" --info";
		if (debug_level)
		{
			currentConduit+=" --debug ";
			currentConduit+=QString().setNum(debug_level);
		}
#ifdef DEBUG
		if (debug_level&SYNC_TEDIOUS)
		{
			kdDebug() << fname << ": Conduit startup command line is:\n"
				<< fname << ": " << currentConduit << endl;
		}
#endif

	      len=0;
	      conduitpipe = popen(currentConduit.local8Bit(), "r");
	      if(conduitpipe)
		{
			len = fread(dbName, 1, 255, conduitpipe);
		      pclose(conduitpipe);
		}
	      conduitpipe=0;
	      dbName[len] = 0L;
	      if (len == 0)
		{
		  QString tmpMessage = i18n("The conduit %1 did not identify "
					"what database it supports. "
					"\nPlease check with the conduits "
					"author to correct it.")
				.arg(p->text(CONDUIT_NAME));

		  KMessageBox::error(this, tmpMessage, i18n("Conduit error."));
		}
	      else if (strcmp(dbName,"<none>")==0)
	      {
#ifdef DEBUG
			if (debug_level & SYNC_TEDIOUS)
			{
				kdDebug() << fname
					<< ": Conduit "
					<< p->text(0)
					<< " supports no databases."
					<< endl;
			}
#endif
		}
	      else
		{
			QStringList l=QStringList::split(QChar(','),
				QString(dbName));
			QStringList::Iterator i;
			const QString &m=p->text(CONDUIT_DESKTOP);

			for (i=l.begin(); i!=l.end(); ++i)
			{
				config.writeEntry((*i).stripWhiteSpace(),m);
			}
		}
nextConduit:
		p=p->nextSibling();
	}
	config.sync();
}



void CConduitSetup::warnNoExec(const QListViewItem *p)
{
	FUNCTIONSETUP;

	QString msg = i18n("No executable could be "
		"found for the conduit %1.")
		.arg(p->text(CONDUIT_NAME));
#ifdef DEBUG
	kdDebug() << fname << ": " << msg << endl;
#endif
	KMessageBox::error(this,msg,i18n("Conduit error"));
}

void CConduitSetup::warnSetupRunning()
{
	FUNCTIONSETUP;

	QString msg = i18n("A conduit is already being set up. "
		"Please complete that action before setting "
		"up another conduit.");

#ifdef DEBUG
	kdDebug() << fname << ": " << msg << endl;
#endif
	KMessageBox::error(this,msg,i18n("Conduit Setup error"));
}
