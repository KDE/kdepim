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
	bool m) : UIDialog(_w, n, Ok|Cancel,m)
{
	FUNCTIONSETUP;

	enableButtonSeparator(true);
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

/* virtual */ bool ConduitConfigDialog::validate()
{
	return fConfigWidget->release();
}

/* virtual */ void ConduitConfigDialog::commitChanges()
{
	fConfigWidget->commitChanges();
}

void ConduitConfigDialog::selected(QListViewItem *)
{
}

// Page numbers in the widget stack
#define INTRO		(0)
#define OLD_CONDUIT	(1)
#define BROKEN_CONDUIT	(2)
#define NEW_CONDUIT	(3)

ConduitConfigWidget::ConduitConfigWidget(QWidget *p, const char *n,
	bool) :
	ConduitConfigWidgetBase(p,n),
	fConfigure(0L),
	fCurrentConduit(0L),
	fCurrentConfig(0L),
	fCurrentOldStyle(0L)
{
	FUNCTIONSETUP;

	// fConduitList->removeColumn(CONDUIT_COMMENT);
	fillLists();
	fConduitList->adjustSize();
	fConduitList->show();

	QObject::connect(fConduitList,
		SIGNAL(selectionChanged(QListViewItem *)),
		this,SLOT(selected(QListViewItem *)));
	QObject::connect(fConfigureButton,
		SIGNAL(clicked()),
		this,SLOT(configure()));

	selected(0L);
	adjustSize();
	fStack->raiseWidget(INTRO);

	(void) new ConduitTip(fConduitList);
}

ConduitConfigWidget::~ConduitConfigWidget()
{
	FUNCTIONSETUP;
	release();
}

void ConduitConfigWidget::fillLists()
{
	FUNCTIONSETUP;

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

void ConduitConfigWidget::loadAndConfigure(QListViewItem *p) // ,bool exec)
{
	FUNCTIONSETUP;

	if (!p)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Executed NULL conduit?"
			<< endl;
#endif
		fStack->raiseWidget(INTRO);
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
		fStack->raiseWidget(BROKEN_CONDUIT);
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
		fStack->raiseWidget(BROKEN_CONDUIT);
		warnNoLibrary(p);
		return;
	}

	QStringList a;
	a.append(CSL1("modal"));

	// QObject *o = f->create(this, 0L, "ConduitConfig",a);
	QObject *o = f->create(fStack, 0L, "ConduitConfigBase", a);
	bool oldstyle=false;

	if (!o)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Can't create ConduitConfigBase - must be old conduit."
			<< endl;
#endif

		o = f->create(this,0L,"ConduitConfig",a);
		oldstyle=true;

		if (!o)
		{
#ifdef DEBUG
			DEBUGKPILOT << fname
				<< ": No ConduitConfig either."
				<< endl;
#endif
			KLibLoader::self()->unloadLibrary(
				library);
			fStack->raiseWidget(BROKEN_CONDUIT);
			warnNoLibrary(p);
			return;
		}
	}

	if (oldstyle)
	{
		ConduitConfig *d = dynamic_cast<ConduitConfig *>(o);

		if (!d)
		{
#ifdef DEBUG
			DEBUGKPILOT << fname
				<< ": Can't cast to config dialog."
				<< endl;
#endif
			fStack->raiseWidget(BROKEN_CONDUIT);
			warnNoLibrary(p);
			return;
		}
		fStack->raiseWidget(OLD_CONDUIT);
		fOldStyleLabel->setText(i18n("<qt>The conduit <i>%1</i> "
			"is an old-style conduit. To configure it, "
			"click the configure button below.")
				.arg(p->text(CONDUIT_NAME)));

		fCurrentOldStyle=d;
		d->setConfig(&KPilotConfig::getConfig());
	}
	else
	{
		ConduitConfigBase *d = dynamic_cast<ConduitConfigBase *>(o);

		if (!d)
		{
#ifdef DEBUG
			DEBUGKPILOT << fname
				<< ": Can't cast to config base object."
				<< endl;
#endif
			fStack->raiseWidget(BROKEN_CONDUIT);
			warnNoLibrary(p);
			return;
		}

		if (fStack->addWidget(d->widget(),NEW_CONDUIT)<0)
		{
	#ifdef DEBUG
			DEBUGKPILOT << fname
				<< ": Can't add config widget to stack."
				<< endl;
	#endif
		}
		else
		{
			d->load(&KPilotConfig::getConfig());
			fStack->erase();
			fStack->raiseWidget(NEW_CONDUIT);
			d->widget()->show();
			d->widget()->adjustSize();
			fCurrentConfig=d;
			fStack->resize(d->widget()->size());
			fStack->setMinimumSize(d->widget()->size());
			fStack->adjustSize();
			adjustSize();
		}
	}
	fConduitList->repaint();
}

bool ConduitConfigWidget::release()
{
	FUNCTIONSETUP;
	if (fCurrentConfig)
	{
		if (!fCurrentConfig->maybeSave(&KPilotConfig::getConfig()))
			return false;
		fStack->raiseWidget(0);
		delete fCurrentConfig;
	}
	if (fCurrentOldStyle)
	{
		fStack->raiseWidget(0);
		delete fCurrentOldStyle;
	}
	if (fCurrentConduit)
	{
		KLibLoader::self()->unloadLibrary(
			QFile::encodeName(fCurrentConduit->text(CONDUIT_LIBRARY)));
	}
	fCurrentConduit=0L;
	fCurrentConfig=0L;
	fCurrentOldStyle=0L;
	return true;
}

void ConduitConfigWidget::selected(QListViewItem *p)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGKPILOT << fname << ": " 
		<< ( p ? p->text(CONDUIT_NAME) : CSL1("None") )
		<< endl;
#endif
	if (p!=fCurrentConduit)
	{
		if (!release())
		{
			p->setSelected(false);
			fCurrentConduit->setSelected(true);
		}
	}
	fCurrentConduit=p;
	loadAndConfigure(p);
	// Workaround for repaint problems
	p->repaint();
	fConduitList->repaint();
}

void ConduitConfigWidget::configure()
{
	if (!fCurrentOldStyle)
	{
		loadAndConfigure(fConduitList->selectedItem());
	}
	if (fCurrentOldStyle)
	{
		fCurrentOldStyle->exec();
	}
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




