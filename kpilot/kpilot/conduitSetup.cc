/* conduitSetup.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This file defines the setup dialog used by KPilot to 
** install and activate conduits.
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/
#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif


#include <stdio.h>
#include <unistd.h>
#include <iostream.h>

#ifndef QDIR_H
#include <qdir.h>
#endif
#ifndef QHBOX_H
#include <qhbox.h>
#endif
#ifndef QTOOLTIP_H
#include <qtooltip.h>
#endif

#ifndef _KSIMPLECONFIG_H
#include <ksimpleconfig.h>
#endif
#ifndef _KAPP_H
#include <kapp.h>
#endif
#ifndef _KPROCESS_H
#include <kprocess.h>
#endif
#ifndef _KMESSAGEBOX_H
#include <kmessagebox.h>
#endif
#ifndef _KSTDDIRS_H
#include <kstddirs.h>
#endif
#ifndef _KDEBUG_H
#include <kdebug.h>
#endif
#ifndef _KUSERPROFILE_H
#include <kuserprofile.h>
#endif
#ifndef _KSERVICE_H
#include <kservice.h>
#endif
#ifndef _KSERVICETYPE_H
#include <kservicetype.h>
#endif

#ifndef _KPILOT_KPILOTCONFIG_H
#include "kpilotConfig.h"
#endif

#include "conduitSetup.moc"


static const char *conduitsetup_id="$Id$";

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
	QToolTip::add(categories,
		i18n("You can drag and drop conduits between the\n"
			"active and available groups. Only the conduits\n"
			"in the active group will run when you do a HotSync."));


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

	DEBUGKPILOT << fname
		<< ": Executing conduit "
		<< p->text(0)
		<< endl;

	QString execPath = findExecPath(p);

	DEBUGKPILOT << fname
		<< ": Exec path="
		<< execPath
		<< endl;

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
	*conduitSetup << execPath << "--setup" ;
	*conduitSetup << "-geometry"
		<< QString("+%1+%2").arg(x()+20).arg(y()+20);
#ifdef DEBUG
	if (debug_level)
	{
		*conduitSetup << "--debug" << QString::number(debug_level);
	}
#endif

	connect(conduitSetup,
		SIGNAL(processExited(KProcess *)),
		this,
		SLOT(setupDone(KProcess *))
		);
	if (!conduitSetup->start(KProcess::NotifyOnExit))
	{
		kdWarning() << __FUNCTION__
			<< ": Could not start process for conduit setup!"
			<< endl;
		warnNoExec(p);
		delete conduitSetup;
		conduitSetup=0L;
	}
}

/* slot */ void CConduitSetup::setupDone(KProcess *p)
{
	FUNCTIONSETUP;

	if (p!=conduitSetup)
	{
		DEBUGKPILOT << fname << ": Process other than setup exited?"
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

void
CConduitSetup::fillLists()
{
	FUNCTIONSETUP;

	KConfig& config = KPilotConfig::getConfig("Conduit Names");
	QStringList potentiallyInstalled =
		config.readListEntry("InstalledConduits");
	KServiceTypeProfile::OfferList offers = 
		KServiceTypeProfile::offers("KPilotConduit");

#ifdef DEBUG
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

	KConfig& config = KPilotConfig::getConfig("Conduit Names");
	config.writeEntry("InstalledConduits", 
		categories->listSiblings(active->firstChild(),CONDUIT_DESKTOP));
	config.setGroup("Database Names");

	const QListViewItem *p=active->firstChild();
	while (p)
	{
		FILE *conduitpipe;

#ifdef DEBUG
		{
			kdDebug() << fname << ": Current conduit = "
				<< p->text(CONDUIT_NAME)
				<< endl;
		}
#endif

#ifdef DEBUG
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
#ifdef DEBUG
		if (debug_level)
		{
			currentConduit+=" --debug ";
			currentConduit+=QString().setNum(debug_level);
		}
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

	/* NOTREACHED */
	(void) conduitsetup_id;
}


// $Log$
// Revision 1.24  2001/04/23 21:30:20  adridg
// Betteer support of missing conduit executables
//
// Revision 1.23  2001/04/23 06:30:38  adridg
// XML UI updates
//
// Revision 1.22  2001/04/16 13:54:17  adridg
// --enable-final file inclusion fixups
//
// Revision 1.21  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.20  2001/02/08 08:13:44  habenich
// exchanged the common identifier "id" with source unique <sourcename>_id for --enable-final build
//
// Revision 1.19  2001/02/05 20:55:07  adridg
// Fixed copyright headers for source releases. No code changed
//
