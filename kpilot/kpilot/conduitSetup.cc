/* conduitSetup.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This file defines the setup dialog used by KPilot to 
** install and activate conduits.
*/

// THIS FILE IS TOTALLY DEPRECATED. IT IS NOT USED
// IN THE MAIN PART OF KPILOT'S CODE.

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
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"


#include <stdio.h>
#include <unistd.h>

#ifndef QDIR_H
#include <qdir.h>
#endif
#ifndef QHBOX_H
#include <qhbox.h>
#endif
#include <qwhatsthis.h>
#include <qheader.h>

#ifndef _KSIMPLECONFIG_H
#include <ksimpleconfig.h>
#endif
#ifndef _KAPP_H
#include <kapplication.h>
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


static const char *conduitsetup_id =
	"$Id$";

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

CConduitSetup::CConduitSetup(QWidget * parent,
	const char *name) :
	KDialogBase(parent, name, true,
		i18n("External Conduit Setup"), 
		User1 | User2 | User3 | Ok | Cancel, Cancel,
		true,
		i18n("Activate"),
		i18n("Deactivate"),
		i18n("Configure")), 
	conduitSetup(0L)
{
	FUNCTIONSETUP;

	QHBox *top = makeHBoxMainWidget();

	categories = new ListCategorizer(top, "conduits");
	categories->setStartOpen(true);

	active = categories->addCategory(i18n("Active"),
		i18n("Conduits that will run when a HotSync is done"));
	available = categories->addCategory(i18n("Available"),
		i18n("Conduits installed on your system but not active"));

	categories->adjustSize();
	categories->setColumnWidthMode(CONDUIT_COMMENT,QListView::Manual);
	categories->header()->setResizeEnabled(false,CONDUIT_COMMENT);

	connect(categories, SIGNAL(selectionChanged(QListViewItem *)),
		this, SLOT(conduitSelected(QListViewItem *)));

	QWhatsThis::add(categories,
		i18n("You can drag and drop conduits between the\n"
			"active and available groups. Only the conduits\n"
			"in the active group will run when you do a HotSync."));


	fillLists();
	adjustSize();
	conduitSelected(0L);

	conduitPaths = KGlobal::dirs()->resourceDirs("conduits");
}

/* virtual */ CConduitSetup::~CConduitSetup()
{
	FUNCTIONSETUP;
}

/* slot */ void CConduitSetup::slotUser1()
{
	FUNCTIONSETUP;

	QListViewItem *p = categories->selectedItem();

	if (!p)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": No item selected, but activate clicked?"
			<< endl;
#endif
		return;
	}

	if (p->parent() != available)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Active conduit selected, but activate clicked?"
			<< endl;
#endif
		return;
	}

	categories->moveItem(p,active,0L);
	categories->setSelected(p,true);
	categories->ensureItemVisible(p);

	conduitSelected(p);
}

/* slot */ void CConduitSetup::slotUser2()
{
	FUNCTIONSETUP;

	QListViewItem *p = categories->selectedItem();

	if (!p)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": No item selected, but activate clicked?"
			<< endl;
#endif
		return;
	}

	if (p->parent() != active)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": InActive conduit selected, but deactivate clicked?"
			<< endl;
#endif
		return;
	}

	categories->moveItem(p,available,0L);
	categories->setSelected(p,true);
	categories->ensureItemVisible(p);

	conduitSelected(p);
}

/* slot */ void CConduitSetup::slotUser3()
{
	FUNCTIONSETUP;

	QListViewItem *p = categories->selectedItem();

	if (!p)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": No item selected, but configure clicked?"
			<< endl;
#endif
		return;
	}

	if (p->parent() != active)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Inactive conduti selected, but configure clicked?"
			<< endl;
#endif
		return;
	}

	conduitExecuted(p);
}

void CConduitSetup::conduitExecuted(QListViewItem * p)
{
	FUNCTIONSETUP;
	if (!p)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname << ": Executed NULL conduit?" << endl;
#endif
		return;
	}

	if (!p->parent())
	{
#ifdef DEBUG
		DEBUGKPILOT << fname << ": Executed a category?" << endl;
#endif
		return;
	}

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Executing conduit " << p->text(0) << endl;
#endif

	QString execPath = findExecPath(p);

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Exec path=" << execPath << endl;
#endif

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
	*conduitSetup << execPath << "--setup";
	*conduitSetup << "-geometry"
		<< CSL1("+%1+%2").arg(x() + 20).arg(y() + 20);
#ifdef DEBUG
	if (debug_level)
	{
		*conduitSetup << "--debug" << QString::number(debug_level);
	}
#endif

	connect(conduitSetup,
		SIGNAL(processExited(KProcess *)),
		this, SLOT(setupDone(KProcess *)));
	if (!conduitSetup->start(KProcess::NotifyOnExit))
	{
		kdWarning() << k_funcinfo
			<< ": Could not start process for conduit setup!"
			<< endl;
		warnNoExec(p);
		delete conduitSetup;

		conduitSetup = 0L;
	}
}

/* slot */ void CConduitSetup::setupDone(KProcess * p)
{
	FUNCTIONSETUP;

	if (p != conduitSetup)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname << ": Process other than setup exited?"
			<< endl;
#endif
		return;
	}

	delete p;

	conduitSetup = 0L;
}

void CConduitSetup::slotOk()
{
	FUNCTIONSETUP;
	writeInstalledConduits();
	KDialogBase::slotOk();
}

void CConduitSetup::fillLists()
{
	FUNCTIONSETUP;

	QStringList potentiallyInstalled =
		KPilotConfig::getConfig().setConduitGroup().
		getInstalledConduits();
	KServiceTypeProfile::OfferList offers =
		KServiceTypeProfile::offers(CSL1("KPilotConduit"));

#ifdef DEBUG
	{
		QStringList::Iterator i = potentiallyInstalled.begin();


		DEBUGKPILOT << fname
			<< ": Currently active conduits are:" << endl;

		while (i != potentiallyInstalled.end())
		{
			DEBUGKPILOT << fname << ": " << (*i) << endl;
			++i;
		}

		DEBUGKPILOT << fname
			<< ": Currently installed conduits are:" << endl;
	}
#endif

	// Now actually fill the two list boxes, just make 
	// sure that nothing gets listed in both.
	//
	//
	QValueListIterator < KServiceOffer > availList(offers.begin());
	while (availList != offers.end())
	{
		KSharedPtr < KService > o = (*availList).service();

#ifdef DEBUG
		DEBUGKPILOT << fname << ": "
			<< o->desktopEntryName()
			<< " = " << o->name() << endl;
#endif

		RichListViewItem *p = 0L;

		if (potentiallyInstalled.contains(o->desktopEntryName()) == 0)
		{
			p=new RichListViewItem(available,o->name(),6);
		}
		else
		{
			p=new RichListViewItem(active,o->name(),6);
		}
		if (p)
		{
			p->setText(CONDUIT_COMMENT,o->comment());
			p->setText(CONDUIT_DESKTOP,o->desktopEntryName());
			p->setText(CONDUIT_EXEC,o->exec());

			p->setRich(CONDUIT_COMMENT,true);
		}

		++availList;
	}
}


QString CConduitSetup::findExecPath(const QListViewItem * p) const
{
	FUNCTIONSETUP;

	QString currentConduit(QString::null);

	if (conduitPaths.isEmpty())
	{
		currentConduit = KGlobal::dirs()->findResource("exe",
			p->text(CONDUIT_EXEC));
		if (currentConduit.isNull())
		{
			currentConduit = p->text(CONDUIT_EXEC);
		}
	}
	else
	{
		// Look in all the resource dirs for conduits
		// for this particular conduit.
		//
		//
		QStringList::ConstIterator i;

		currentConduit = QString::null;
		for (i = conduitPaths.begin(); i != conduitPaths.end(); ++i)
		{
			if (QFile::exists((*i) + '/' + p->text(CONDUIT_EXEC)))
			{
				currentConduit =
					(*i) + '/' + p->text(CONDUIT_EXEC);
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

	KPilotConfigSettings & config = KPilotConfig::getConfig();

	config.setConduitGroup().setInstalledConduits(
		categories->listSiblings(active->firstChild(), CONDUIT_DESKTOP));
	config.setDatabaseGroup();

	const QListViewItem *p = active->firstChild();

	while (p)
	{
		FILE *conduitpipe;

#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Current conduit = "
			<< p->text(CONDUIT_NAME) << endl;
		DEBUGKPILOT << fname
			<< ": Current conduit service from "
			<< p->text(CONDUIT_DESKTOP)
			<< " says exec=" << p->text(CONDUIT_EXEC) << endl;
#endif

		QString currentConduit = findExecPath(p);

		if (currentConduit.isNull())
		{
			warnNoExec(p);
			goto nextConduit;
		}



		currentConduit += CSL1(" --info");
#ifdef DEBUG
		if (debug_level)
		{
			currentConduit += CSL1(" --debug ");
			currentConduit += QString().setNum(debug_level);
		}

		DEBUGKPILOT << fname
			<< ": Conduit startup command line is: "
			<< currentConduit << endl;
#endif

		len = 0;
		conduitpipe = popen(currentConduit.local8Bit(), "r");
		if (conduitpipe)
		{
			len = fread(dbName, 1, 255, conduitpipe);
			pclose(conduitpipe);
		}
		conduitpipe = 0;
		dbName[len] = 0L;
		if (len == 0)
		{
			QString tmpMessage =
				i18n("The conduit %1 did not identify "
				"what database it supports. "
				"\nPlease check with the conduits "
				"author to correct it.").arg(p->
				text(CONDUIT_NAME));

			// Might not be correct in a caption,
			// but this file is deprecated anyway.
			KMessageBox::error(this, tmpMessage,
				i18n("Conduit Error."));
		}
		else if (strcmp(dbName, "<none>") == 0)
		{
#ifdef DEBUG
			DEBUGKPILOT << fname
				<< ": Conduit "
				<< p->text(0)
				<< " supports no databases." << endl;
#endif
		}
		else
		{
			QStringList l = QStringList::split(QChar(','),
				QString(QString::fromLatin1(dbName)));

			QStringList::Iterator i;
			const QString & m = p->text(CONDUIT_DESKTOP);

			for (i = l.begin(); i != l.end(); ++i)
			{
				config.setDatabaseConduit((*i).
					stripWhiteSpace(), m);
			}
		}
nextConduit:
		p = p->nextSibling();
	}
	config.sync();
}

void CConduitSetup::conduitSelected(QListViewItem *p)
{
	FUNCTIONSETUP;

	if (!p)
	{
		enableButton(User1,false);
		enableButton(User2,false);
		enableButton(User3,false);
		return;
	}

	if (!p->parent())
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Selected a category?"
			<< endl;
#endif
		return;
	}


	if (p->parent() == active)
	{
		enableButton(User1,false);
		enableButton(User2,true);
		enableButton(User3,true);
	}
	else
	{
		enableButton(User1,true);
		enableButton(User2,false);
		enableButton(User3,false);
	}
}



void CConduitSetup::warnNoExec(const QListViewItem * p)
{
	FUNCTIONSETUP;

	QString msg = i18n("No executable could be "
		"found for the conduit %1.").arg(p->text(CONDUIT_NAME));

#ifdef DEBUG
	DEBUGKPILOT << fname << ": " << msg << endl;
#endif

	KMessageBox::error(this, msg, i18n("Conduit Error"));
}

void CConduitSetup::warnSetupRunning()
{
	FUNCTIONSETUP;

	QString msg = i18n("A conduit is already being set up. "
		"Please complete that action before setting "
		"up another conduit.");

#ifdef DEBUG
	DEBUGKPILOT << fname << ": " << msg << endl;
#endif

	KMessageBox::error(this, msg, i18n("Conduit Setup Error"));

	/* NOTREACHED */
	(void) conduitsetup_id;
}


