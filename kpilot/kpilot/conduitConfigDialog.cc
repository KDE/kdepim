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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

static const char *conduitconfigdialog_id =
	"$Id$";

#include "options.h"

#include <qlistview.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qfile.h>
#include <qpushbutton.h>
#include <qhbox.h>
#include <qlayout.h>
#include <qwidgetstack.h>

#include <kservice.h>
#include <kservicetype.h>
#include <kuserprofile.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klibloader.h>
#include <kseparator.h>

#include "plugin.h"
#include "kpilotConfig.h"

#include "conduitConfigDialog.moc"

#define CONDUIT_NAME    (0)
#define CONDUIT_COMMENT (1)
#define CONDUIT_DESKTOP (2)
#define CONDUIT_LIBRARY (3)

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
	if (s.find(CSL1("<qt>"),0,false) == -1)
	{
		s.prepend(CSL1("<qt>"));
		s.append(CSL1("</qt>"));
	}

	tip(fListView->itemRect(l),s);
}



ConduitConfigDialog::ConduitConfigDialog(QWidget * _w, const char *n,
	bool m) : UIDialog(_w, n, Ok|Cancel|User1,m)
{
	FUNCTIONSETUP;

	enableButtonSeparator(true);
	setButtonText(User1,i18n("Configure..."));
	selected(0L);

	fConfigWidget = new ConduitConfigWidget(widget(),0L);
	fConfigWidget->show();

	QObject::connect(fConfigWidget,
		SIGNAL(selectionChanged(QListViewItem *)),
		this,SLOT(selected(QListViewItem *)));

	(void) conduitconfigdialog_id;
}

ConduitConfigDialog::~ConduitConfigDialog()
{
	FUNCTIONSETUP;
}

/* virtual */ void ConduitConfigDialog::commitChanges()
{
	fConfigWidget->commitChanges();
}

void ConduitConfigDialog::selected(QListViewItem *p)
{
	enableButton(User1,p);
}

ConduitConfigWidget::ConduitConfigWidget(QWidget *p, const char *n,
	bool ownbuttons) :
	ConduitConfigWidgetBase(p,n),
	fConfigure(0L)
{
	FUNCTIONSETUP;

	fConduitList->removeColumn(CONDUIT_COMMENT);
	fillLists();
	fConduitList->adjustSize();
	fConduitList->show();

	if (ownbuttons)
	{
		QHBox *h = new QHBox(this);
		fConfigure = new QPushButton(i18n("Configure..."),h);
		fConduitConfigLayout->addWidget(h,5,0);
		h->setStretchFactor(fConfigure,0);
		
		QWidget *w = new QWidget(h);
		h->setStretchFactor(w,100);
		
		KSeparator *s = new KSeparator(this);
		fConduitConfigLayout->addWidget(s,4,0);
	}
	
	QObject::connect(fConduitList,
		SIGNAL(doubleClicked(QListViewItem *)),
		this,SLOT(configureConduit()));
	QObject::connect(fConduitList,
		SIGNAL(selectionChanged(QListViewItem *)),
		this,SLOT(selected(QListViewItem *)));

	selected(0L);
	adjustSize();
	fStack->raiseWidget(0);

	(void) new ConduitTip(fConduitList);
}

ConduitConfigWidget::~ConduitConfigWidget()
{
	FUNCTIONSETUP;
}

void ConduitConfigWidget::fillLists()
{
	FUNCTIONSETUP;

	fGeneralItem = new QListViewItem(fConduitList,
		i18n("General"),
		i18n("Settings that apply to all conduits."));

	QStringList potentiallyInstalled =
		KPilotConfig::getConfig().setConduitGroup().
		getInstalledConduits();
	KServiceTypeProfile::OfferList offers =
		KServiceTypeProfile::offers(CSL1("KPilotConduit"));

	QValueListIterator < KServiceOffer > availList(offers.begin());
	while (availList != offers.end())
	{
		KSharedPtr < KService > o = (*availList).service();

#ifdef DEBUG
		DEBUGKPILOT << fname << ": "
			<< o->desktopEntryName()
			<< " = " << o->name() << endl;
#endif

		QCheckListItem *p = 0L;

		if (!o->exec().isEmpty())
		{
			kdWarning() << k_funcinfo
				<< ": Old-style conduit found "
				<< o->name()
				<< endl;
		}

		p = new QCheckListItem(fConduitList,
			o->name(),
			QCheckListItem::CheckBox);
		p->setMultiLinesEnabled(true);
		p->setText(CONDUIT_COMMENT,o->comment());
		p->setText(CONDUIT_DESKTOP,o->desktopEntryName());
		p->setText(CONDUIT_LIBRARY,o->library());
		
		if (potentiallyInstalled.contains(o->desktopEntryName()) == 0)
		{
			p->setOn(false);
		}
		else
		{
			p->setOn(true);
		}

		++availList;
	}
}

void ConduitConfigWidget::selected(QListViewItem *p)
{
	FUNCTIONSETUP;
	// Don't enable configure for the general item.
	if (p==fGeneralItem) 
	{
		fStack->raiseWidget(1);
		p=0L;
	}
	else
	{
		fStack->raiseWidget(0);
	}
	if (fConfigure) fConfigure->setEnabled(p);
	emit selectionChanged(p);
}

void ConduitConfigWidget::configureConduit()
{
	FUNCTIONSETUP;

	QListViewItem *p = fConduitList->selectedItem();
	if (p==fGeneralItem) return;
	if (!p)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Executed NULL conduit?"
			<< endl;
#endif
		return;
	}

#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Executing conduit "
		<< p->text(CONDUIT_NAME)
		<< endl;
#endif

	if (p->text(CONDUIT_LIBRARY).isEmpty())
	{
		warnNoExec(p);
		return;
	}

	QCString library = QFile::encodeName(p->text(CONDUIT_LIBRARY));

	KLibFactory *f = KLibLoader::self()->
		factory(library);
	if (!f)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": No conduit library "
			<< library
			<< " found."
			<< endl;
#endif
		warnNoLibrary(p);
		return;
	}

	QStringList a;
	a.append(CSL1("modal"));

	// QObject *o = f->create(this, 0L, "ConduitConfig",a);
	QObject *o = f->create(fStack, 0L, "ConduitConfigWidget",a);

	if (!o)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Can't create object."
			<< endl;
#endif

		KLibLoader::self()->unloadLibrary(
			library);
		warnNoLibrary(p);
		return;
	}

	QWidget *d = dynamic_cast<QWidget *>(o);

	if (!d)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Can't cast to dialog."
			<< endl;
#endif

		delete o;
		KLibLoader::self()->unloadLibrary(
			library);
		warnNoLibrary(p);
		return;
	}

	// d->setConfig(&KPilotConfig::getConfig());
	// d->readSettings();
	// d->exec();
	if (fStack->addWidget(d,2)<0)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Can't add config widget to stack."
			<< endl;
#endif
	}
	else
	{
		fStack->raiseWidget(2);
		d->show();
	}

	// delete d;
	// KLibLoader::self()->unloadLibrary(
	//	library);
}


void ConduitConfigWidget::warnNoExec(const QListViewItem * p)
{
	FUNCTIONSETUP;

	QString msg = i18n("<qt>No library could be "
		"found for the conduit %1. This means that the "
		"conduit was not installed properly.</qt>")
		.arg(p->text(CONDUIT_NAME));

#ifdef DEBUG
	DEBUGKPILOT << fname << ": " << msg << endl;
#endif

	KMessageBox::error(this, msg, i18n("Conduit Error"));
}

void ConduitConfigWidget::warnNoLibrary(const QListViewItem *p)
{
	FUNCTIONSETUP;

	QString msg = i18n("<qt>There was a problem loading the library "
		"for the conduit %1. This means that the "
		"conduit was not installed properly.</qt>")
		.arg(p->text(CONDUIT_NAME));

	KMessageBox::error(this, msg, i18n("Conduit Error"));
}

/* virtual */ void ConduitConfigWidget::commitChanges()
{
	FUNCTIONSETUP;

	QStringList activeConduits;
	const QCheckListItem *p = 
		dynamic_cast<QCheckListItem *>(fConduitList->firstChild());
	KPilotConfigSettings & config = KPilotConfig::getConfig();



	while (p)
	{
		if (p->isOn())
		{
			activeConduits.append(p->text(CONDUIT_DESKTOP));
		}
		p = dynamic_cast<QCheckListItem *>(p->nextSibling());
	}
	config.setConduitGroup().setInstalledConduits(activeConduits);
	config.sync();
}




