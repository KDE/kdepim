/* conduitConfigDialog.cc                KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines a .ui-based configuration dialog for conduits.
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
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qlistview.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qfile.h>

#include <kservice.h>
#include <kservicetype.h>
#include <kuserprofile.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <klibloader.h>

#include "kpilotConfig.h"

#include "conduitConfigDialog_base.h"
#include "conduitConfigDialog.moc"

#define CONDUIT_NAME    (0)
#define CONDUIT_COMMENT (1)
#define CONDUIT_DESKTOP (2)
#define CONDUIT_EXEC    (3)
#define CONDUIT_LIBRARY (4)

class ConduitTip : public QToolTip
{
public:
	ConduitTip(QListView *parent);
	virtual ~ConduitTip();

protected:
	virtual void maybeTip(const QPoint &);

	QListView *fListView;
} ;


ConduitTip::ConduitTip(QListView *p) :
	QToolTip(p->viewport(),0L),
	fListView(p)
{
	FUNCTIONSETUP;
}

ConduitTip::~ConduitTip()
{
	FUNCTIONSETUP;
}

/* virtual */ void ConduitTip::maybeTip(const QPoint &p)
{
	FUNCTIONSETUP;

	QListViewItem *l = fListView->itemAt(p);

	if (!l) return;

	// ConduitListItem *q = static_cast<ConduitListItem *>(l);

#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Tip over "
		<< l->text(CONDUIT_NAME)
		<< " with text "
		<< l->text(CONDUIT_COMMENT)
		<< endl;
#endif

	QString s = l->text(CONDUIT_COMMENT);

	if (s.isEmpty()) return;
	if (s.find("<qt>",0,false) == -1)
	{
		s.prepend("<qt>");
		s.append("</qt>");
	}

	tip(fListView->itemRect(l),s);
}


ConduitConfigDialog::ConduitConfigDialog(QWidget * w, const char *n,
	bool m) : UIDialog(w, n, m),
	conduitSetup(0L)
{
	FUNCTIONSETUP;

	fConfigWidget = new ConduitConfigWidget(widget());

	fillLists();

	fConfigWidget->active->adjustSize();
	fConfigWidget->available->adjustSize();

	int w = QMAX(fConfigWidget->active->width(),
		fConfigWidget->available->width());


	fConfigWidget->available->resize(w,fConfigWidget->available->height());
	fConfigWidget->active->resize(w,fConfigWidget->active->height());
	fConfigWidget->available->setColumnWidth(0,w);
	fConfigWidget->active->setColumnWidth(0,w);
	fConfigWidget->available->setColumnWidthMode(0,QListView::Manual);
	fConfigWidget->active->setColumnWidthMode(0,QListView::Manual);

	QObject::connect(fConfigWidget->active,
		SIGNAL(selectionChanged(QListViewItem *)),
		this,SLOT(selected(QListViewItem *)));
	QObject::connect(fConfigWidget->available,
		SIGNAL(selectionChanged(QListViewItem *)),
		this,SLOT(selected(QListViewItem *)));

	QObject::connect(fConfigWidget->enableButton,
		SIGNAL(clicked()),
		this,SLOT(enableConduit()));
	QObject::connect(fConfigWidget->disableButton,
		SIGNAL(clicked()),
		this,SLOT(disableConduit()));
	QObject::connect(fConfigWidget->configButton,
		SIGNAL(clicked()),
		this,SLOT(configureConduit()));

	fConfigWidget->adjustSize();

	(void) new ConduitTip(fConfigWidget->active);
	(void) new ConduitTip(fConfigWidget->available);

	selected(0L);
}

ConduitConfigDialog::~ConduitConfigDialog()
{
	FUNCTIONSETUP;
}

void ConduitConfigDialog::fillLists()
{
	FUNCTIONSETUP;

	QStringList potentiallyInstalled =
		KPilotConfig::getConfig().setConduitGroup().
		getInstalledConduits();
	KServiceTypeProfile::OfferList offers =
		KServiceTypeProfile::offers("KPilotConduit");

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

		QListViewItem *p = 0L;

		if (potentiallyInstalled.contains(o->desktopEntryName()) == 0)
		{
			p = new QListViewItem(fConfigWidget->available,
				o->name(),
				o->comment(),
				o->desktopEntryName(),
				o->exec(),
				o->library());
		}
		else
		{
			p = new QListViewItem(fConfigWidget->active,
				o->name(),
				o->comment(),
				o->desktopEntryName(),
				o->exec(),
				o->library());
		}

		++availList;
	}
}

void ConduitConfigDialog::selected(QListViewItem *p)
{
	FUNCTIONSETUP;

	if (!p) 
	{
		fConfigWidget->configButton->setEnabled(false);
		fConfigWidget->enableButton->setEnabled(false);
		fConfigWidget->disableButton->setEnabled(false);
		return;
	}

	if (p->listView() == fConfigWidget->active)
	{
		fConfigWidget->configButton->setEnabled(true);
		fConfigWidget->enableButton->setEnabled(false);
		fConfigWidget->disableButton->setEnabled(true);
		fConfigWidget->available->clearSelection();
	}
	else
	{
		fConfigWidget->configButton->setEnabled(false);
		fConfigWidget->enableButton->setEnabled(true);
		fConfigWidget->disableButton->setEnabled(false);
		fConfigWidget->active->clearSelection();
	}
}

void ConduitConfigDialog::enableConduit()
{
	FUNCTIONSETUP;

	QListViewItem *l = fConfigWidget->available->currentItem();
	if (!l) return;

	fConfigWidget->available->takeItem(l);
	fConfigWidget->active->clearSelection();
	fConfigWidget->active->insertItem(l);
	fConfigWidget->active->setSelected(l,true);
	selected(l);
}

void ConduitConfigDialog::disableConduit()
{
	FUNCTIONSETUP;

	QListViewItem *l = fConfigWidget->active->currentItem();
	if (!l) return;

	fConfigWidget->active->takeItem(l);
	fConfigWidget->available->clearSelection();
	fConfigWidget->available->insertItem(l);
	fConfigWidget->available->setSelected(l,true);
	selected(l);
	fConfigWidget->available->setFocus();
}


void ConduitConfigDialog::configureConduit()
{
	FUNCTIONSETUP;

	QListViewItem *p = fConfigWidget->active->currentItem();

	if (!p)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname << ": Executed NULL conduit?" << endl;
#endif
		return;
	}


#ifdef DEBUG
	DEBUGKPILOT << fname << ": Executing conduit " 
		<< p->text(CONDUIT_NAME) << endl;
#endif

	QString execPath = findExecPath(p);

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Exec path=" << execPath << endl;
#endif

	if (execPath.isNull())
	{
		if (p->text(CONDUIT_LIBRARY).isEmpty())
		{
			warnNoExec(p);
			return;
		}
		else
		{
			const char *library = p->text(CONDUIT_LIBRARY);

			KLibFactory *f = KLibLoader::self()->
				factory(library);
			if (!f)
			{
				DEBUGKPILOT << fname
					<< ": No conduit library "
					<< library
					<< " found."
					<< endl;
				warnNoExec(p);
				return;
			}

			QObject *o = f->create(this, 0L, "QWidget");


			if (!o)
			{
				DEBUGKPILOT << fname
					<< ": Can't create object."
					<< endl;

				KLibLoader::self()->unloadLibrary(
					library);
				warnNoExec(p);
				return;
			}

			KDialogBase *d = dynamic_cast<KDialogBase *>(o);

			if (!d)
			{
				DEBUGKPILOT << fname
					<< ": Can't cast to dialog."
					<< endl;
				delete o;
				KLibLoader::self()->unloadLibrary(
					library);
				warnNoExec(p);
				return;
			}

			d->exec();

			delete d;
			KLibLoader::self()->unloadLibrary(
				library);

			return;
		}
	}

	if (conduitSetup)
	{
		warnSetupRunning();
		return;
	}

	conduitSetup = new KProcess;
	*conduitSetup << execPath << "--setup";
	*conduitSetup << "-geometry"
		<< QString("+%1+%2").arg(x() + 20).arg(y() + 20);
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

/* slot */ void ConduitConfigDialog::setupDone(KProcess * p)
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


QString ConduitConfigDialog::findExecPath(const QListViewItem * p) const
{
	FUNCTIONSETUP;

	QString currentConduit(QString::null);

	if (p->text(CONDUIT_EXEC).isEmpty()) return QString::null;

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

void ConduitConfigDialog::warnNoExec(const QListViewItem * p)
{
	FUNCTIONSETUP;

	QString msg = i18n("No executable could be "
		"found for the conduit %1.").arg(p->text(CONDUIT_NAME));

#ifdef DEBUG
	DEBUGKPILOT << fname << ": " << msg << endl;
#endif

	KMessageBox::error(this, msg, i18n("Conduit error"));
}

void ConduitConfigDialog::warnSetupRunning()
{
	FUNCTIONSETUP;

	QString msg = i18n("A conduit is already being set up. "
		"Please complete that action before setting "
		"up another conduit.");

#ifdef DEBUG
	DEBUGKPILOT << fname << ": " << msg << endl;
#endif

	KMessageBox::error(this, msg, i18n("Conduit Setup error"));
}


/* virtual */ void ConduitConfigDialog::commitChanges()
{
	FUNCTIONSETUP;

	char dbName[255];
	int len = 0;
	QStringList activeConduits;
	const QListViewItem *p = fConfigWidget->active->firstChild();
	KPilotConfigSettings & config = KPilotConfig::getConfig();


	config.setDatabaseGroup();


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

		activeConduits.append(p->text(CONDUIT_DESKTOP));


		currentConduit += " --info";
#ifdef DEBUG
		if (debug_level)
		{
			currentConduit += " --debug ";
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

			KMessageBox::error(this, tmpMessage,
				i18n("Conduit error."));
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
				QString(dbName));

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
	config.setConduitGroup().setInstalledConduits(activeConduits);
	config.sync();
}




// $Log$
