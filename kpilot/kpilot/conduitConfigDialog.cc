/* conduitConfigDialog.cc                KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2002-2004 by Adriaan de Groot
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
#include <qheader.h>
#include <qlabel.h>
#include <qtimer.h>

#include <kservice.h>
#include <kservicetype.h>
#include <kuserprofile.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klibloader.h>
#include <kseparator.h>
#include <kconfigskeleton.h>

#include "plugin.h"
#include "kpilotConfig.h"
#include "kpilotConfigDialog.h"

#include "kpilotConfigWizard.h"

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

	ConfigWizard *create_wizard(QWidget *parent, int m)
	{
		return new ConfigWizard(parent,"Wizard", m);
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

// implement our own check list items so we can detect if a given item was checked/unchecked. We need
// this to prevent the modified signal if one only wants to display a conduit's config widget. Currently,
// KListView doesn't provide any signal that indicates that the checked state of a checklist item was changed.
class KPilotCheckListItem : public QCheckListItem
{
public:
	KPilotCheckListItem ( QListViewItem * parent, const QString & text, Type tt = RadioButtonController ) : QCheckListItem(parent, text, tt),mOriginalState(false) {}
	~KPilotCheckListItem() {}

	void setOriginalState(bool state) { mOriginalState=state; setOn(state);}
	bool isOriginalState() { return isOn() == mOriginalState; }

protected:
	bool mOriginalState;
};


// Page numbers in the widget stack
#define OLD_CONDUIT      (1)
#define BROKEN_CONDUIT   (2)
#define INTERNAL_CONDUIT (3)
#define INTERNAL_EXPLN   (4)
#define CONDUIT_EXPLN    (5)
#define GENERAL_EXPLN    (6)
#define GENERAL_ABOUT    (7)
#define NEW_CONDUIT      (8)


/*
** Create a page in the widget stack @p parent on page @p pageno,
** bearing the given @p text. The remainder of the parameters are
** for esoteric things like:
**  @p buttons set to non-null to include (and return) a QHBox suitable
**     for displaying a row of buttons in on the page.
**  @p label set to non-null to return the QLabel used to display @p text.
*/
static void addDescriptionPage(QWidgetStack *parent,
	int pageno,
	const QString &text,
	QHBox **buttons = 0L,
	QLabel **label = 0L)
{
	QVBox *v = new QVBox(parent);
	QLabel *l = 0L;

	v->setFrameShape(QLabel::NoFrame);
	v->setMargin(SPACING);

	l = new QLabel(v);
	l->setText(text);
	l->setAlignment(Qt::AlignLeft | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);

	if (label) { *label = l; }

	if (buttons)
	{
		*buttons = new QHBox(v);
		l = new QLabel(v);
	}

	parent->addWidget(v,pageno);
}


ConduitConfigWidgetBase::ConduitConfigWidgetBase(QWidget *parent, const char *n) :
	KCModule(parent, n),
	fConduitList(0L),
	fStack(0L),
	fConfigureButton(0L),
	fConfigureWizard(0L),
	fConfigureKontact(0L),
	fActionDescription(0L)
{
	QWidget *w = 0L; // For spacing purposes only.
	QHBox *btns = 0L;

	QHBoxLayout *mainLayout = new QHBoxLayout(this);
	mainLayout->setSpacing(10);

	// Create the left hand column
	fConduitList = new QListView(this ,"ConduitList");
	fConduitList->addColumn(QString::null);
	fConduitList->header()->hide();
	fConduitList->setSizePolicy(
		QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
	mainLayout->addWidget(fConduitList);

	// Create the title
	QVBoxLayout *vbox = new QVBoxLayout(this, 0, KDialog::spacingHint());
	fTitleText = new QLabel("Conduit Setup - Addressbook", this);
	QFont titleFont(fTitleText->font());
	titleFont.setBold(true);
	fTitleText->setFont(titleFont);
	vbox->addWidget(fTitleText, 0, AlignLeft);
	vbox->addWidget(new KSeparator(QFrame::HLine|QFrame::Plain, this));

	// Right hand column
	fStack = new QWidgetStack(this, "RightPart");
	vbox->addWidget(fStack, 10);

	mainLayout->addLayout(vbox);

	// First page in stack (right hand column)
	addDescriptionPage(fStack,BROKEN_CONDUIT,
		i18n("<qt>This conduit appears to be broken and cannot "
		"be configured.</qt>"));

	// Second page, now with layout in a single column
	//
	// Probably deprecated.
	//
	addDescriptionPage(fStack,OLD_CONDUIT,
		i18n("<qt>This is an old-style conduit.</qt>"),&btns);
	w = new QWidget(btns);
	btns->setStretchFactor(w,50);
	fConfigureButton = new QPushButton(btns);
	fConfigureButton->setText(i18n("Configure..."));
	w = new QWidget(btns);
	btns->setStretchFactor(w,50);

	// Page 3
	addDescriptionPage(fStack,INTERNAL_CONDUIT,
		QString::null,0L,&fActionDescription);

	// Page 5 - explanation about conduits
	addDescriptionPage(fStack,CONDUIT_EXPLN,
		i18n("<qt><i>Conduits</i> are external (possibly third-party) "
		"programs that perform synchronization actions. They may "
		"have individual configurations. Select a conduit to configure it, "
		"and enable it by clicking on its checkbox. "
		"</qt>"));

	// Page 6 - explanation about general setup
	addDescriptionPage(fStack,GENERAL_EXPLN,
		i18n("<qt><p>The <i>general</i> portion of KPilot's setup "
		"contains settings for your hardware and the way KPilot "
		"should display your data. For the basic setup, which should fulfill "
		"the need of most users, just use the setup wizard below.</p>"
		"If you need some special settings, this dialog provides all the options "
		"for fine-tuning KPilot. But be warned: The HotSync settings are "
		"various esoteric things.</p>"
		"<p>You can enable an action or conduit by clicking on its checkbox. "
		"Checked conduits will be run during a HotSync. "
		"Select a conduit to configure it.</p>"
		"</qt>"),&btns);
	w = new QWidget(btns);
	btns->setStretchFactor(w,50);
	fConfigureWizard = new QPushButton(i18n("Configuration Wizard"),btns);
	w = new QWidget(btns);
	btns->setStretchFactor(w,50);


	fStack->addWidget(UIDialog::aboutPage(fStack,0L),GENERAL_ABOUT);
}

ConduitConfigWidget::ConduitConfigWidget(QWidget *parent, const char *n,
	bool) :
	ConduitConfigWidgetBase(parent,n),
	fConfigure(0L),
	fCurrentConduit(0L),
	fGeneralPage(0L),
	fCurrentConfig(0L)
{
	FUNCTIONSETUP;

	fConduitList->setSorting(-1);
	fConduitList->setRootIsDecorated(true);
	fConduitList->setTreeStepSize(10);
	// fConduitList->removeColumn(CONDUIT_COMMENT);
	fillLists();

	fConduitList->resize(fConduitList->sizeHint());
	fConduitList->setMinimumSize(fConduitList->sizeHint());
	fConduitList->setColumnWidth(0, fConduitList->sizeHint().width());
	fConduitList->setResizeMode(QListView::AllColumns);

	fStack->resize(fStack->sizeHint()+QSize(10,40));
	fStack->setMinimumSize(fStack->sizeHint()+QSize(10,40));

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
	QListViewItem *general,*conduits;

	// And two generic pointers for the rest.
	QListViewItem *q = 0L;
	KPilotCheckListItem *p = 0L;

	q = new QListViewItem(fConduitList, i18n("About"));
	q->setText(CONDUIT_COMMENT, i18n("About KPilot. Credits."));
	q->setText(CONDUIT_LIBRARY, CSL1("general_about"));

	conduits = new QListViewItem(fConduitList, i18n("Conduits"));

	general = new QListViewItem( fConduitList, i18n("General Setup" ) );
	fGeneralPage = general;

	// Give them identifiers so they can be handled specially when selected.
	conduits->setText(CONDUIT_LIBRARY,CSL1("expln_conduits"));
	general->setText( CONDUIT_LIBRARY, CSL1("expln_general") );

	general->setText( CONDUIT_COMMENT,
		i18n("General setup of KPilot (User name, port, general sync settings)") );
	conduits->setText( CONDUIT_COMMENT,
		i18n("Actions for HotSync with individual configuration."));

	conduits->setOpen(true);
	general->setOpen(true);


	// Create entries under general.
#define CE(a,b,c) q = new QListViewItem(general,a) ; \
	q->setText(CONDUIT_COMMENT,b) ; \
	q->setText(CONDUIT_LIBRARY,c) ;

	CE(i18n("Startup and Exit"), i18n("Behavior at startup and exit."), CSL1("general_startexit") );
	CE(i18n("Viewers"), i18n("Viewer settings."), CSL1("general_view") );
	CE(i18n("Backup"),i18n("Special settings for backup."),CSL1("general_backup"));
	CE(i18n("HotSync"),i18n("Special behavior during HotSync."),CSL1("general_sync"));
	CE(i18n("Device"),i18n("Hardware settings and startup and exit options."),CSL1("general_setup"));

#undef CE


	// List of installed (enabled) actions and conduits.
	QStringList potentiallyInstalled = KPilotSettings::installedConduits();

	//  Create internal conduits.
	//
	//

#define IC(a,b,c) p = new KPilotCheckListItem(conduits,i18n(a),QCheckListItem::CheckBox); \
	p->setText(CONDUIT_COMMENT,i18n(c)); \
	p->setText(CONDUIT_LIBRARY,"internal_" b); \
	p->setText(CONDUIT_DESKTOP,"internal_" b); \
	if (potentiallyInstalled.findIndex(p->text(CONDUIT_DESKTOP))>=0) \
		p->setOriginalState(true);

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

		p = new KPilotCheckListItem(conduits,
			o->name(),
			QCheckListItem::CheckBox);
		p->setMultiLinesEnabled(true);
		p->setText(CONDUIT_COMMENT,o->comment());
		p->setText(CONDUIT_DESKTOP,o->desktopEntryName());
		p->setText(CONDUIT_LIBRARY,o->library());

		if (potentiallyInstalled.findIndex(o->desktopEntryName()) < 0)
		{
			p->setOriginalState(false);
		}
		else
		{
			p->setOriginalState(true);
		}

		++availList;
	}
}

static void dumpConduitInfo(const KLibrary *lib)
{
	DEBUGKPILOT << "Plugin version = " << PluginUtility::pluginVersion(lib) << endl;
	DEBUGKPILOT << "Plugin id      = " << PluginUtility::pluginVersionString(lib) << endl;
}

static ConduitConfigBase *handleGeneralPages(QWidget *w, QListViewItem *p)
{
	ConduitConfigBase *o = 0L;

	QString s = p->text(CONDUIT_LIBRARY) ;

	if (s.startsWith(CSL1("general_setup")))
	{
		o = new DeviceConfigPage( w, "generalSetup" );
	}
	else if (s.startsWith(CSL1("general_sync")))
	{
		o = new SyncConfigPage( w, "syncSetup" );
	}
	else if (s.startsWith(CSL1("general_view")))
	{
		o = new ViewersConfigPage( w, "viewSetup" );
	}
	else if (s.startsWith(CSL1("general_startexit")))
	{
		o = new StartExitConfigPage(w,"startSetup");
	}
	else if (s.startsWith(CSL1("general_backup")))
	{
		o = new BackupConfigPage(w,"backupSetup");
	}

	return o;
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
		fActionDescription->setText(
			i18n("<qt>This is an internal action which has no "
			"configuration options. "
			"The action's description is: <i>%1</i> "
			"</qt>").arg(p->text(CONDUIT_COMMENT)));
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

	// Page 4: General setup
	if (p->text(CONDUIT_LIBRARY).startsWith(CSL1("general_")))
	{
		o = handleGeneralPages(fStack,p);
	}
	else
	{
		QCString library = QFile::encodeName(p->text(CONDUIT_LIBRARY));

		KLibFactory *f = KLibLoader::self()->factory(library);
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

		dumpConduitInfo(KLibLoader::self()->library(library));

		QStringList a;
		a.append(CSL1("modal"));

		o = f->create(fStack, 0L, "ConduitConfigBase", a);

		if (!o)
		{
#ifdef DEBUG
			DEBUGKPILOT << fname
				<< ": Can't create ConduitConfigBase - must be old conduit."
				<< endl;
#endif

			KLibLoader::self()->unloadLibrary(
				library);
			fStack->raiseWidget(BROKEN_CONDUIT);
			warnNoLibrary(p);
			return;
		}
	}

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
	if (fCurrentConduit)
	{
		KLibLoader::self()->unloadLibrary(
			QFile::encodeName(fCurrentConduit->text(CONDUIT_LIBRARY)));
	}
	fCurrentConduit=0L;
	fCurrentConfig=0L;
	return true;
}

void ConduitConfigWidget::unselect()
{
	fConduitList->setSelected( fCurrentConduit, true );
	fConduitList->setCurrentItem( fCurrentConduit );
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
			fConduitList->blockSignals(true);
			QTimer::singleShot(1,this,SLOT(unselect()));
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

	// set the dialog title to the selected item
	QListViewItem *pParent = p->parent();
	QString title;
	title = pParent ? pParent->text(CONDUIT_NAME) + " - " : "";
	title += p ? p->text(CONDUIT_NAME) : i18n("KPilot Setup");
	fTitleText->setText(title);
}

void ConduitConfigWidget::configure()
{
	loadAndConfigure(fConduitList->selectedItem());
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
		KPilotCheckListItem*p = dynamic_cast<KPilotCheckListItem*>(it.current());
		if ( p )
		{
			p->setOriginalState( p->isOn() );
			if ( p->isOn() )
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
	KPilotCheckListItem*i=dynamic_cast<KPilotCheckListItem*>(item);
	if (i)
	{
		if (!i->isOriginalState()) emit changed(true);
	}
}

void ConduitConfigWidget::reopenItem(QListViewItem *i)
{
	i->setOpen(true);
}

void ConduitConfigWidget::configureWizard()
{
	FUNCTIONSETUP;
	ConfigWizard wiz(this, "Wizard");
	if (wiz.exec()) {
		KPilotSettings::self()->readConfig();
		load();
	}
}


