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
#include <qsplitter.h>

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
#include "kpilotConfigDialog.h"

#include "conduitConfigDialog.moc"

#define CONDUIT_NAME    (0)
#define CONDUIT_COMMENT (1)
#define CONDUIT_DESKTOP (2)
#define CONDUIT_LIBRARY (3)
#define CONDUIT_ORDER	(4)

extern "C"
{
  KCModule *create_kpilotconfig( QWidget *parent, const char * )
  {
    return new ConduitConfigWidget( parent, "kcmkpilotconfig" );
  }
}


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
#define OLD_CONDUIT      (1)
#define BROKEN_CONDUIT   (2)
#define INTERNAL_CONDUIT (3)
#define INTERNAL_EXPLN   (4)
#define CONDUIT_EXPLN    (5)
#define GENERAL_EXPLN    (6)
#define GENERAL_ABOUT    (7)
#define NEW_CONDUIT      (8)


static QHBox *addDescriptionPage(QWidgetStack *parent,
	int pageno,
	const QString &text,
	bool buttons)
{
	QHBox *h = 0L;
	QVBox *v = new QVBox(parent);
	QLabel *l = new QLabel(v);

	v->setFrameShape(QLabel::Box);

	l->setText(text);
	l->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);

	if (buttons)
	{
		h = new QHBox(v);
		l = new QLabel(v);
	}

	parent->addWidget(v,pageno);

	return h;
}


ConduitConfigWidgetBase::ConduitConfigWidgetBase(QWidget *parent, const char *n) :
	KCModule(parent, n)
{
	QBoxLayout *p = new QVBoxLayout( this );

	QSplitter *spl = new QSplitter( this, "ConduitSplitter" );
	spl->setOrientation( QSplitter::Horizontal );
	p->addWidget(spl);

	QWidget *w = 0L; // For spacing purposes only.
	QLabel *l = 0L;
	QVBox *v = 0L;
	QHBox *btns = 0L;

	// Create the left hand column
	v = new QVBox( spl );
	fConduitList = new QListView(v,"ConduitList");
	fConduitList->addColumn(i18n("Conduit"));
	v->setStretchFactor(fConduitList, 50);
	v->setSpacing(50);
	l = new QLabel(v);    // Just a placekeeper, to fix redraw problems.
	l->resize(30,30);
	v->setStretchFactor(l,3);

	// Right hand column
	fStack = new QWidgetStack(spl,"RightPart");

	// First page in stack (right hand column)
	addDescriptionPage(fStack,BROKEN_CONDUIT,
		i18n("<qt>This conduit appears to be broken and cannot "
		"be configured.</qt>"),false);

	// Second page, now with layout in a single column
	//
	//
	// Probably deprecated.
	//
	btns = addDescriptionPage(fStack,OLD_CONDUIT,
		i18n("<qt>This is an old-style conduit.</qt>"),true);
	w = new QWidget(btns);
	btns->setStretchFactor(w,50);
	fConfigureButton = new QPushButton(btns);
	fConfigureButton->setText(i18n("Configure..."));
	w = new QWidget(btns);
	btns->setStretchFactor(w,50);

	// Page 3
	addDescriptionPage(fStack,INTERNAL_CONDUIT,
		i18n("<qt>This is an internal conduit which has no "
		"configuration options.</qt>"),false);

	// Page 4 - explanation of what "actions" are.
	addDescriptionPage(fStack,INTERNAL_EXPLN,
		i18n("<qt><i>Actions</i> lists actions that can occur "
		"during a HotSync but that require no further configuration. "
		"</qt>"),false);

	// Page 5 - explanation about conduits
	addDescriptionPage(fStack,CONDUIT_EXPLN,
		i18n("<qt><i>Conduits</i> are external (possibly third-party) "
		"programs that perform synchronization actions. They may "
		"have individual configurations. Select a conduit to configure it, "
		"and enable it by clicking on its checkbox. "
		"</qt>"),false);

	// Page 6 - explanation about general setup
	//
	// TODO: add wizard-startup buttons here.
	btns = addDescriptionPage(fStack,GENERAL_EXPLN,
		i18n("<qt><p>The <i>general</i> portion of KPilot's setup "
		"contains settings for your hardware and the way KPilot "
		"should display your data. The HotSync settings are "
		"various esoteric things.</p>"
		"<p>You can enable an action or conduit by clicking on its checkbox. "
		"Checked conduits will be run during a HotSync. "
		"Select a conduit to configure it.</p>"
		"</qt>"),true);
	w = new QWidget(btns);
	btns->setStretchFactor(w,50);
	fConfigureWizard = new QPushButton(i18n("Configuration Wizard"),btns);
	w = new QWidget(btns);
	btns->setStretchFactor(w,10);
	fConfigureKontact = new QPushButton(i18n("Kontact Wizard"),btns);
	w = new QWidget(btns);
	btns->setStretchFactor(w,50);


	fStack->addWidget(UIDialog::aboutPage(fStack,0L),GENERAL_ABOUT);
}

#define PAGE_SIZE	QSize(440,300)

ConduitConfigWidget::ConduitConfigWidget(QWidget *parent, const char *n,
	bool) :
	ConduitConfigWidgetBase(parent,n),
	fConfigure(0L),
	fCurrentConduit(0L),
	fGeneralPage(0L),
	fCurrentConfig(0L),
	fCurrentOldStyle(0L)
{
	FUNCTIONSETUP;

	fConduitList->setSorting(-1);
	fConduitList->setRootIsDecorated(false);
	fConduitList->setTreeStepSize(10);
	// fConduitList->removeColumn(CONDUIT_COMMENT);
	fillLists();
	fConduitList->adjustSize();
	fConduitList->show();

	fStack->resize(PAGE_SIZE);
	fStack->setMinimumSize(PAGE_SIZE);

	QObject::connect(fConduitList,
		SIGNAL(selectionChanged(QListViewItem *)),
		this,SLOT(selected(QListViewItem *)));
	QObject::connect(fConduitList,
		SIGNAL(clicked(QListViewItem*)),
		this, SLOT(conduitsChanged(QListViewItem*)));

	// Deprecated?
	QObject::connect(fConfigureButton,
		SIGNAL(clicked()),
		this,SLOT(configure()));

	QObject::connect(fConfigureWizard,SIGNAL(clicked()),
		this,SLOT(configureWizard()));
	QObject::connect(fConfigureKontact,SIGNAL(clicked()),
		this,SLOT(configureKontact()));

	fGeneralPage->setSelected(true);
	fConduitList->setCurrentItem(fGeneralPage);
	selected(fGeneralPage);

	(void) new ConduitTip(fConduitList);
	setButtons(Apply);

	(void) conduitconfigdialog_id;
}

ConduitConfigWidget::~ConduitConfigWidget()
{
	FUNCTIONSETUP;
	release();
}

void ConduitConfigWidget::fillLists()
{
	FUNCTIONSETUP;

	// 3 QListViewItems for the three headings in the list
	QListViewItem *general,*conduits,*actions;

	// And two generic pointers for the rest.
	QListViewItem *q = 0L;
	QCheckListItem *p = 0L;


	conduits = new QListViewItem(fConduitList, i18n("Conduits"));
	actions = new QListViewItem(fConduitList, i18n("Actions"));
	general = new QListViewItem( fConduitList, i18n("General Setup" ) );
	fGeneralPage = general;

	// Give them identifiers so they can be handled specially when selected.
	conduits->setText(CONDUIT_LIBRARY,CSL1("expln_conduits"));
	actions->setText(CONDUIT_LIBRARY,CSL1("expln_actions"));
	general->setText( CONDUIT_LIBRARY, CSL1("expln_general") );

	general->setText( CONDUIT_COMMENT,
		i18n("General setup of KPilot (User name, port, general sync settings)") );
	actions->setText( CONDUIT_COMMENT,
		i18n("Simple actions for HotSync with no configuration."));
	conduits->setText( CONDUIT_COMMENT,
		i18n("Actions for HotSync with individual configuration."));

	conduits->setOpen(true);
	actions->setOpen(true);
	general->setOpen(true);

	// Prevent items from being collapsed by the user.
	connect(fConduitList,SIGNAL(collapsed(QListViewItem *)),
		this,SLOT(reopenItem(QListViewItem *)));


	// Create entries under general.
	q = new QListViewItem(general, i18n("About"));
	q->setText(CONDUIT_COMMENT, i18n("About KPilot. Credits."));
	q->setText(CONDUIT_LIBRARY, CSL1("general_about"));

	q = new QListViewItem(general, i18n("HotSync") );
	q->setText(CONDUIT_COMMENT,
		i18n("Special behavior during HotSync.") );
	q->setText(CONDUIT_LIBRARY, CSL1("general_sync") );

	q = new QListViewItem(general, i18n("Viewers") );
	q->setText(CONDUIT_COMMENT,
		i18n("Viewer settings.") );
	q->setText(CONDUIT_LIBRARY, CSL1("general_view") );

	q = new QListViewItem(general, i18n("Device") );
	q->setText(CONDUIT_COMMENT,
		i18n("Hardware settings and startup and exit options.") );
	q->setText(CONDUIT_LIBRARY, CSL1("general_setup") );




	// List of installed (enabled) actions and conduits.
	QStringList potentiallyInstalled = KPilotSettings::installedConduits();

	//  Create internal conduits.
	//
	//

#define IC(a,b,c) p = new QCheckListItem(actions,i18n(a),QCheckListItem::CheckBox); \
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

		if (!o->exec().isEmpty())
		{
			kdWarning() << k_funcinfo
				<< ": Old-style conduit found "
				<< o->name()
				<< endl;
		}

		p = new QCheckListItem(conduits,
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
		fStack->raiseWidget(GENERAL_EXPLN);
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

	if (p->text(CONDUIT_LIBRARY) == CSL1("expln_actions"))
	{
		fStack->raiseWidget(INTERNAL_EXPLN);
		return;
	}
	if (p->text(CONDUIT_LIBRARY) == CSL1("expln_conduits"))
	{
		fStack->raiseWidget(CONDUIT_EXPLN);
		return;
	}
	if (p->text(CONDUIT_LIBRARY) == CSL1("expln_general"))
	{
		fStack->raiseWidget(GENERAL_EXPLN);
		return;
	}

	if (p->text(CONDUIT_LIBRARY) == CSL1("general_about"))
	{
		fStack->raiseWidget(GENERAL_ABOUT);
		return;
	}

	QObject *o = 0L;
	bool oldstyle = false;

	// Page 4: General setup
	if (p->text(CONDUIT_LIBRARY).startsWith(CSL1("general_setup")))
	{
		o = new DeviceConfigPage( fStack, "generalSetup" );
	}
	else if (p->text(CONDUIT_LIBRARY).startsWith(CSL1("general_sync")))
	{
		o = new SyncConfigPage( fStack, "syncSetup" );
	}
	else if (p->text(CONDUIT_LIBRARY).startsWith(CSL1("general_view")))
	{
		o = new KPilotConfigPage( fStack, "syncSetup" );
	}
	else
	{
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
		o = f->create(fStack, 0L, "ConduitConfigBase", a);

		if (!o)
		{
#ifdef DEBUG
			DEBUGKPILOT << fname
				<< ": Can't create ConduitConfigBase - must be old conduit."
				<< endl;
#endif

			o = f->create(this, 0L, "ConduitConfig", a);
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

		// Remove the config widget from the stack before we can add the new one
		QWidget *oldConfigWidget = fStack->widget( NEW_CONDUIT );
		if ( oldConfigWidget )
		{
			fStack->removeWidget( oldConfigWidget );
			KPILOT_DELETE( oldConfigWidget );
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
			d->load();
			fStack->raiseWidget(NEW_CONDUIT);
			d->widget()->show();
			fCurrentConfig=d;
			// make sure the changed signal is propagated to the KCM*Dialog
			// and the apply button is enabled correspondingly.
			connect(d, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
		}
	}
}

bool ConduitConfigWidget::release()
{
	FUNCTIONSETUP;
	if (fCurrentConfig)
	{
		if (!fCurrentConfig->maybeSave())
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
			return;
		}
	}
	fCurrentConduit=p;
	loadAndConfigure(p);
//	fStack->adjustSize();
#ifdef DEBUG
	DEBUGKPILOT << fname << ": New widget size "
		<< fStack->size().width() << "x" << fStack->size().height() << endl;
	DEBUGKPILOT << fname << ": Current size "
		<< size().width() << "x"
		<< size().height() << endl;
#endif
	emit sizeChanged();
#ifdef DEBUG
	DEBUGKPILOT << fname << ": New size "
		<< size().width() << "x"
		<< size().height() << endl;
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

	KMessageBox::error(this, msg, i18n("Conduit Error"));
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

	KMessageBox::error(this, msg, i18n("Conduit Error"));
}

void ConduitConfigWidget::save()
{
	FUNCTIONSETUP;

	// Only new-style conduits and the general setup have changes that need to be commited
	// old-style conduits have their own config dlg which commits them itself
	if ( fStack->id( fStack->visibleWidget())==NEW_CONDUIT )
	{
		if (fCurrentConfig) fCurrentConfig->commit();
	}

	QStringList activeConduits;
	QListViewItemIterator it( fConduitList );
	while ( it.current() ) {
		const QCheckListItem*p = dynamic_cast<QCheckListItem*>(it.current());
		if ( p && p->isOn() )
		{
			activeConduits.append(p->text(CONDUIT_DESKTOP));
		}
		++it;
	}
	KPilotSettings::setInstalledConduits(activeConduits);
	KPilotSettings::self()->writeConfig();
}


void ConduitConfigWidget::load()
{
	FUNCTIONSETUP;
	KPilotSettings::self()->readConfig();

	QStringList potentiallyInstalled = KPilotSettings::installedConduits();
	QListViewItem*p = fConduitList->firstChild();
	while (p)
	{
		QListViewItem*q = p->firstChild();
		while (q)
		{
			QCheckListItem*qq=dynamic_cast<QCheckListItem*>(q);
			if (qq)
			{
				qq->setOn(! (potentiallyInstalled.findIndex(qq->text(CONDUIT_DESKTOP))<0) );
			}
			q = q->nextSibling();
		}
		p=p->nextSibling();
	}


	// Only new-style conduits and the general setup have changes that need to be commited
	// old-style conduits have their own config dlg which commits them itself
	if ( fStack->id( fStack->visibleWidget())==NEW_CONDUIT )
	{
		if (fCurrentConfig) fCurrentConfig->load();
	}
}


void ConduitConfigWidget::conduitsChanged(QListViewItem*item)
{
	QCheckListItem*i=dynamic_cast<QCheckListItem*>(item);
	if (i)
	{
		// TODO_Osnabrueck: find out if the item was actually checked/unchecked
		emit changed(true);
	}
}

void ConduitConfigWidget::reopenItem(QListViewItem *i)
{
	i->setOpen(true);
}

void ConduitConfigWidget::configureWizard()
{
	FUNCTIONSETUP;
	KMessageBox::sorry(this,
		i18n("Sorry, this configuration wizard is still unimplemented."),
		i18n("Unimplemented feature."));
}

void ConduitConfigWidget::configureKontact()
{
	FUNCTIONSETUP;
	KMessageBox::sorry(this,
		i18n("Sorry, this configuration wizard is still unimplemented."),
		i18n("Unimplemented feature."));
}

