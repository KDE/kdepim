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
#include <qvbox.h>

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

// Page numbers in the widget stack
#define INTRO            (0)
#define OLD_CONDUIT      (1)
#define BROKEN_CONDUIT   (2)
#define INTERNAL_CONDUIT (3)
#define NEW_CONDUIT      (4)


ConduitConfigWidgetBase::ConduitConfigWidgetBase(QHBox *p,const char *n) :
	QObject(p,n)
{
	p->setSpacing(10);

	QWidget *w = 0L; // For spacing purposes only.
	QLabel *l = 0L;
	QVBox *v = 0L;

	// Create the left hand column
	fConduitList = new QListView(p,"ConduitList");
	fConduitList->addColumn(i18n("Conduit"));

	// Right hand column
	fStack = new QWidgetStack(p,"RightPart");

	// Zero'th page in stack
	l = new QLabel(fStack);
	l->setFrameShape(QLabel::Box);
	l->setText(i18n("<qt>Select a conduit in the list to configure it. "
		"Checked conduits will be run during a HotSync.</qt>"));
	l->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	fStack->addWidget(l,INTRO);

	// First page in stack (right hand column)
	l = new QLabel(fStack);
	l->setFrameShape(QLabel::Box);
	l->setText(i18n("<qt>This conduit appears to be broken and cannot "
		"be configured.</qt>"));
	l->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	fStack->addWidget(l,BROKEN_CONDUIT);

	// Second page, now with layout in a single column
	v = new QVBox(fStack,"OldStyle");
	w = new QWidget(v);
	v->setStretchFactor(w,50);
	fOldStyleLabel = new QLabel(v);
	// Within this column, center the button box
	QHBox *h = new QHBox(v,"ButtonBox");
	w = new QWidget(h);
	h->setStretchFactor(w,50);
	fConfigureButton = new QPushButton(h);
	fConfigureButton->setText(i18n("Configure..."));
	w = new QWidget(h);
	h->setStretchFactor(w,50);
	// Add stretch beneath the button box
	w = new QWidget(v);
	v->setStretchFactor(w,50);
	fStack->addWidget(v,OLD_CONDUIT);

	// Page 3
	l = new QLabel(fStack);
	l->setFrameShape(QLabel::Box);
	l->setText(i18n("<qt>This conduit has no configuration options.</qt>"));
	l->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	fStack->addWidget(l,INTERNAL_CONDUIT);
}


ConduitConfigDialog::ConduitConfigDialog(QWidget * _w, const char *n,
	bool m) : UIDialog(_w, n, Ok|Cancel,m)
{
	FUNCTIONSETUP;

	enableButtonSeparator(true);
	// selected(0L);

	QHBox *h = dynamic_cast<QHBox *>(widget());
	if (!h)
	{
		h = new QHBox(widget());
	}

	fConfigWidget = new ConduitConfigWidget(h,0L);

	QObject::connect(fConfigWidget,SIGNAL(sizeChanged()),
		h,SLOT(adjustSize()));
	QObject::connect(fConfigWidget,SIGNAL(sizeChanged()),
		this,SLOT(adjustSize()));

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

#define PAGE_SIZE	QSize(440,300)

ConduitConfigWidget::ConduitConfigWidget(QHBox *p, const char *n,
	bool) :
	ConduitConfigWidgetBase(p,n),
	fConfigure(0L),
	fCurrentConduit(0L),
	fCurrentConfig(0L),
	fCurrentOldStyle(0L),
	fParentWidget(p)
{
	FUNCTIONSETUP;

	// fConduitList->removeColumn(CONDUIT_COMMENT);
	fillLists();
	fConduitList->adjustSize();
	fConduitList->show();

	fStack->resize(PAGE_SIZE);
	fStack->setMinimumSize(PAGE_SIZE);

	QObject::connect(fConduitList,
		SIGNAL(selectionChanged(QListViewItem *)),
		this,SLOT(selected(QListViewItem *)));
	QObject::connect(fConfigureButton,
		SIGNAL(clicked()),
		this,SLOT(configure()));

	selected(0L);
	// adjustSize();
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

		if (potentiallyInstalled.findIndex(o->desktopEntryName()) < 0)
		{
			p->setOn(false);
		}
		else
		{
			p->setOn(true);
		}

		++availList;
	}

	// Now the (statically compiled) internal conduits
	QCheckListItem *p = 0L;
#define IC(a,b,c) p = new QCheckListItem(fConduitList,i18n(a),QCheckListItem::CheckBox); \
	p->setText(CONDUIT_COMMENT,i18n(c)); \
	p->setText(CONDUIT_LIBRARY,"internal_" b); \
	p->setText(CONDUIT_DESKTOP,"internal_" b); \
	if (potentiallyInstalled.findIndex(p->text(CONDUIT_DESKTOP))>=0) \
		p->setOn(true);

	IC("Kroupware","kroupware",
		"Sync the handheld with a Kroupware client (for example, KMail).");
	IC("Install Files","fileinstall",
		"Install files that are dragged to KPilot onto the handheld.");

#undef IC
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

	if (p->text(CONDUIT_LIBRARY).startsWith(CSL1("internal_")))
	{
		fStack->raiseWidget(INTERNAL_CONDUIT);
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

		o = f->create(fParentWidget,0L,"ConduitConfig",a);
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
			"click the configure button below.</qt>")
				.arg(p->text(CONDUIT_NAME)));

		fCurrentOldStyle=d;
		d->setConfig(&KPilotConfig::getConfig());
		d->readSettings();
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
			fStack->raiseWidget(NEW_CONDUIT);
			d->widget()->show();
			fCurrentConfig=d;
		}
	}
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
	fStack->adjustSize();
#ifdef DEBUG
	DEBUGKPILOT << fname << ": New widget size "
		<< fStack->size().width() << "x" << fStack->size().height() << endl;
	DEBUGKPILOT << fname << ": Parent current size "
		<< fParentWidget->size().width() << "x"
		<< fParentWidget->size().height() << endl;
#endif
	emit sizeChanged();
#ifdef DEBUG
	DEBUGKPILOT << fname << ": Parent new size "
		<< fParentWidget->size().width() << "x"
		<< fParentWidget->size().height() << endl;
#endif

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
	DEBUGKPILOT << fname << ": No library for " 
		<< p->text(CONDUIT_NAME) << endl;
#endif

	KMessageBox::error(fParentWidget, msg, i18n("Conduit Error"));
}

void ConduitConfigWidget::warnNoLibrary(const QListViewItem *p)
{
	FUNCTIONSETUP;

	QString msg = i18n("<qt>There was a problem loading the library "
		"for the conduit %1. This means that the "
		"conduit was not installed properly.</qt>")
		.arg(p->text(CONDUIT_NAME));

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Can't load library for "
		<< p->text(CONDUIT_NAME) << endl;
#endif

	KMessageBox::error(fParentWidget, msg, i18n("Conduit Error"));
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




