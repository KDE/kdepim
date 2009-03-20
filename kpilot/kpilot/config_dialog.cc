/* KPilot
**
** Copyright (C) 2001 by Dan Pilone <dan@kpilot.org>
** Copyright (C) 2002-2004 by Adriaan de Groot <groot@kde.org>
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This file defines the configuration dialog for KPilot.
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtCore/QVariantList>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtGui/QSplitter>
#include <QtGui/QToolTip>
#include <QtGui/QHeaderView>
#include <QtGui/QStackedWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>

#include <kconfigskeleton.h>
#include <kdialog.h>
#include <kglobal.h>
#include <khbox.h>
#include <kpluginloader.h>
#include <kpluginfactory.h>
#include <kmessagebox.h>
#include <kseparator.h>
#include <kservice.h>
#include <kservicetype.h>
#include <kservicetypeprofile.h>
#include <kstandarddirs.h>
#include <kvbox.h>

#include "plugin.h"
#include "kpilotConfig.h"
#include "config_pages.h"
#include "config_dialog.moc"

#define CONDUIT_NAME    (0)
#define CONDUIT_COMMENT (1)
#define CONDUIT_DESKTOP (2)
#define CONDUIT_LIBRARY (3)
#define CONDUIT_ORDER	(4)



K_PLUGIN_FACTORY(ConduitConfigFactory, registerPlugin<ConduitConfigWidget>();)
K_EXPORT_PLUGIN(ConduitConfigFactory("kcmkpilotconfig"))

// Page numbers in the widget stack
#define BROKEN_CONDUIT   (0)
#define INTERNAL_CONDUIT (1)
#define CONDUIT_EXPLN    (2)
#define GENERAL_EXPLN    (3)
#define GENERAL_ABOUT    (4)
#define NEW_CONDUIT      (5)

/*
** Create a page in the widget stack @p parent on page @p pageno,
** bearing the given @p text. The remainder of the parameters are
** for esoteric things like:
**  @p buttons set to non-null to include (and return) a QHBox suitable
**     for displaying a row of buttons in on the page.
**  @p label set to non-null to return the QLabel used to display @p text.
*/
static void addDescriptionPage(QStackedWidget  *parent,
	int pageno,
	const QString &text,
	KHBox **buttons = 0L,
	QLabel **label = 0L)
{
	FUNCTIONSETUPL(4);
	KVBox *v = new KVBox(parent);
	QLabel *l = 0L;

	v->setFrameShape(QLabel::NoFrame);
	v->setMargin(SPACING);

	l = new QLabel(v);
	l->setText(text);
	l->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	l->setWordWrap( true );

	if (label) { *label = l; }

	if (buttons)
	{
		*buttons = new KHBox(v);
		l = new QLabel(v);
	}

	int actual_pageno = parent->insertWidget (pageno,v);
	DEBUGKPILOT << "Requested index " << pageno
		<< " Received index " << actual_pageno;
}


ConduitConfigWidgetBase::ConduitConfigWidgetBase(QWidget *parent, const QVariantList &args) :
	KCModule(ConduitConfigFactory::componentData(),parent, args),
	fConduitList(0L),
	fStack(0L),
	fActionDescription(0L)
{
	FUNCTIONSETUP;

	KHBox *btns = 0L;

	QBoxLayout *mainLayout = new QHBoxLayout(this);
	mainLayout->setSpacing(10);

	// Create the left hand column
	fConduitList = new QTreeWidget(this);
	fConduitList->setObjectName("ConduitList");
	fConduitList->setColumnCount(1);
	fConduitList->header()->hide();
	fConduitList->setSortingEnabled(false);
	fConduitList->setSizePolicy(
		QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
	fConduitList->setWhatsThis(
		i18n("This list box contains both general configuration items such as the device used for HotSync and the databases to be backed up as well as a list of conduits that KPilot may run during a HotSync. Click on an item to configure it. Conduits which are checked in this list will run during a HotSync."));
	connect( fConduitList, SIGNAL(itemChanged( QTreeWidgetItem *, int )), this, SLOT(changed()) );
	mainLayout->addWidget(fConduitList);

	// Create the title
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->setMargin(0);
	vbox->setSpacing(KDialog::spacingHint());

	// String below is just to make space; no need to translate.
	fTitleText = new QLabel(CSL1("Conduit Setup - Fake Title"), this);
	QFont titleFont(fTitleText->font());
	titleFont.setBold(true);
	fTitleText->setFont(titleFont);
	vbox->addWidget(fTitleText, 0, Qt::AlignLeft);
	vbox->addWidget(new KSeparator(Qt::Horizontal, this));

	// Right hand column
	fStack = new QStackedWidget (this);
	vbox->addWidget(fStack, 10);

	mainLayout->addLayout(vbox);

	addDescriptionPage(fStack,BROKEN_CONDUIT,
		i18n("<qt>This conduit appears to be broken and cannot "
		"be configured.</qt>"));

	addDescriptionPage(fStack,INTERNAL_CONDUIT,
		QString(),0L,&fActionDescription);

	addDescriptionPage(fStack,CONDUIT_EXPLN,
		i18n("<qt><p><i>Conduits</i> are external (possibly third-party) "
		"programs that perform synchronization actions. They may "
		"have individual configurations. Select a conduit to configure it, "
		"and enable it by clicking on its checkbox. "
		"</p></qt>"));

	addDescriptionPage(fStack,GENERAL_EXPLN,
		i18n("<qt><p>The <i>general</i> portion of KPilot's setup "
		"contains settings for your hardware and the way KPilot "
		"should display your data. For the basic setup, which should fulfill "
		"the need of most users, just use the setup wizard below.</p>"
		"<p>If you need some special settings, this dialog provides all the options "
		"for fine-tuning KPilot. But be warned: The HotSync settings are "
		"various esoteric things.</p>"
		"<p>You can enable an action or conduit by clicking on its checkbox. "
		"Checked conduits will be run during a HotSync. "
		"Select a conduit to configure it.</p>"
		"</qt>"),&btns);

	fStack->insertWidget(GENERAL_ABOUT,ConduitConfigBase::aboutPage(fStack,0L));
}

ConduitConfigWidget::ConduitConfigWidget(QWidget *parent, const QVariantList &args) :
	ConduitConfigWidgetBase(parent,args),
	fCurrentConduit(0L),
	fGeneralPage(0L),
	fConduitsItem(0L),
	fCurrentConfig(0L)
{
	FUNCTIONSETUP;

	fillLists();

	fConduitList->resize(fConduitList->sizeHint());
	fConduitList->setMinimumSize(QSize(200,200));
	fConduitList->setColumnWidth(0, fConduitList->sizeHint().width());

	fStack->resize(fStack->sizeHint()+QSize(10,40));
	fStack->setMinimumSize(QSize(520,400));

	QObject::connect(fConduitList,
		SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
		this,SLOT(selected(QTreeWidgetItem *, QTreeWidgetItem *)));

	fGeneralPage->setSelected(true);
	fConduitList->setCurrentItem(fGeneralPage);

	selected(fGeneralPage,0L);

	setButtons(Apply);
}

ConduitConfigWidget::~ConduitConfigWidget()
{
	FUNCTIONSETUP;
	release();
}


static QTreeWidgetItem *createItem( QTreeWidgetItem *parent,
	const QString &name,
	const QString &comment,
	const QString &library)
{
	QTreeWidgetItem *q = new QTreeWidgetItem(parent);
	q->setText(CONDUIT_NAME,name);
	q->setToolTip(CONDUIT_NAME,comment);
	q->setText(CONDUIT_COMMENT,comment);
	q->setText(CONDUIT_LIBRARY,library);
	return q;
}

static QTreeWidgetItem *createCheckableItem( QTreeWidgetItem *parent,
	const QString &name,
	const QString &comment,
	const QString &library,
	const QString &desktop = QString() )
{
	QTreeWidgetItem *q = createItem(parent,name,comment,library);
	q->setText(CONDUIT_DESKTOP,desktop.isEmpty() ? library : desktop);
	q->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable   | Qt::ItemIsEnabled);
	q->setCheckState(CONDUIT_NAME, Qt::Unchecked);
	return q;
}

void ConduitConfigWidget::fillLists()
{
	FUNCTIONSETUP;

	QTreeWidgetItem *general,*conduits;
	QTreeWidgetItem *q = 0L;

	general = new QTreeWidgetItem( fConduitList );
	fConduitList->addTopLevelItem(general);
	general->setText( CONDUIT_NAME, i18n("General Setup" ) );
	general->setText( CONDUIT_LIBRARY, CSL1("expln_general") );
	general->setToolTip( CONDUIT_NAME,
		i18n("General setup of KPilot (User name, port, general sync settings)") );
	fGeneralPage = general;

	conduits = new QTreeWidgetItem( fConduitList );
	fConduitList->addTopLevelItem(conduits);
	conduits->setText( CONDUIT_NAME, i18n("Conduits"));
	conduits->setText( CONDUIT_LIBRARY,CSL1("expln_conduits"));
	conduits->setToolTip( CONDUIT_NAME,
		i18n("Actions for HotSync with individual configuration."));
	fConduitsItem = conduits;

	q = new QTreeWidgetItem();
	fConduitList->addTopLevelItem(q);
	q->setText( CONDUIT_NAME, i18n("About"));
	q->setToolTip( CONDUIT_NAME, i18n("About KPilot. Credits."));
	q->setText( CONDUIT_LIBRARY, CSL1("general_about"));

	conduits->setExpanded(true);
	general->setExpanded(true);


	// Create entries under general.
	createItem(general, i18n("Device"),
		i18n("Hardware settings and startup and exit options."),CSL1("general_setup"));
	createItem(general, i18n("HotSync"),
		i18n("Special behavior during HotSync."),CSL1("general_sync"));
	createItem(general, i18n("Backup"),
		i18n("Special settings for backup."),CSL1("general_backup"));
	createItem(general, i18n("Startup and Exit"),
		i18n("Behavior at startup and exit."), CSL1("general_startexit") );

	// List of installed (enabled) actions and conduits.
	QStringList potentiallyInstalled = KPilotSettings::installedConduits();

	// "Install Files" is treated as a conduit
	q = createCheckableItem( conduits, i18n("Install Files"),
		i18n("Install files that are dragged to KPilot onto the handheld."),
		CSL1("internal_fileinstall") );
	// "Install Files" is possibly enabled.
	if (potentiallyInstalled.indexOf(q->text(CONDUIT_DESKTOP))>=0)
	{
		q->setCheckState(CONDUIT_NAME, Qt::Checked);
	}

	// Get all conduits installed on the system
	KService::List offers = KServiceTypeTrader::self()->query( CSL1("KPilotConduit") );
	KService::List::ConstIterator e = offers.end();
	for (KService::List::ConstIterator i = offers.begin(); i!=e; ++i)
	{
		const KService::Ptr o = *i;
		bool is_active;

		if (!o->exec().isEmpty())
		{
			WARNINGKPILOT << "Old-style conduit found in ["
				<< o->name() << ']';
			continue;
		}

		q = createCheckableItem( conduits, o->name(),
			o->comment(),
			o->library(), o->desktopEntryName() );
		is_active = potentiallyInstalled.indexOf(q->text(CONDUIT_DESKTOP)) >= 0;
		
		if (is_active)
		{
			q->setCheckState(CONDUIT_NAME, Qt::Checked);
		}
		
		DEBUGKPILOT << '[' << o->desktopEntryName()
			<< "] = [" << o->name() << ']'
			<< ( is_active ?  " active" : "" );


	}
}


static ConduitConfigBase *handleGeneralPages(QWidget *w, QTreeWidgetItem *p)
{
	FUNCTIONSETUPL(2);
	ConduitConfigBase *o = 0L;

	QString s = p->text(CONDUIT_LIBRARY) ;

	if (s.startsWith(CSL1("general_setup")))
	{
		o = new DeviceConfigPage( w, QVariantList() << CSL1( "generalSetup" ) );
	}
	else if (s.startsWith(CSL1("general_sync")))
	{
		o = new SyncConfigPage( w, QVariantList() << CSL1( "syncSetup" ) );
	}
	else if (s.startsWith(CSL1("general_startexit")))
	{
		o = new StartExitConfigPage(w, QVariantList() << CSL1( "startSetup" ) );
	}
	else if (s.startsWith(CSL1("general_backup")))
	{
		o = new BackupConfigPage(w, QVariantList() << CSL1( "backupSetup" ) );
	}

	if (!o)
	{
		WARNINGKPILOT << "Got unknown page name" << s;
	}

	return o;
}

void ConduitConfigWidget::loadAndConfigure(QTreeWidgetItem *p)
{
	FUNCTIONSETUP;

	if (!p)
	{
		fStack->setCurrentIndex(GENERAL_EXPLN);
		return;
	}

	QString libraryName = p->text(CONDUIT_LIBRARY);

	DEBUGKPILOT << "Executing conduit ["
		<< p->text(CONDUIT_NAME)
		<< "] (library [" << libraryName << "])";

	if (libraryName.isEmpty())
	{
		fStack->setCurrentIndex(BROKEN_CONDUIT);
		warnNoExec(p);
		return;
	}

	if (libraryName.startsWith(CSL1("internal_")))
	{
		fStack->setCurrentIndex(INTERNAL_CONDUIT);
		fActionDescription->setText(
			i18n("<qt>This is an internal action which has no "
			"configuration options. "
			"The action's description is: <i>%1</i> "
			"</qt>",p->text(CONDUIT_COMMENT)));
		return;
	}

	if (libraryName == CSL1("expln_conduits"))
	{
		fStack->setCurrentIndex(CONDUIT_EXPLN);
		return;
	}
	if (libraryName == CSL1("expln_general"))
	{
		fStack->setCurrentIndex(GENERAL_EXPLN);
		return;
	}

	if (libraryName == CSL1("general_about"))
	{
		fStack->setCurrentIndex(GENERAL_ABOUT);
		return;
	}

	ConduitConfigBase *d = 0L;

	if (libraryName.startsWith(CSL1("general_")))
	{
        d = qobject_cast<ConduitConfigBase *>(handleGeneralPages(fStack,p));
	}
	else
	{
		KPluginLoader loader(libraryName);
        KPluginFactory *f = loader.factory();
		if (!f)
		{
			WARNINGKPILOT << "No conduit library ["
				<< libraryName
				<< "] found.";
			fStack->setCurrentIndex(BROKEN_CONDUIT);
			warnNoLibrary(p);
			return;
		}

        DEBUGKPILOT << "library: " << libraryName << " version: " << loader.pluginVersion();
		if (Pilot::PLUGIN_API > loader.pluginVersion())
		{
			WARNINGKPILOT << "Old conduit library found.";
			fStack->setCurrentIndex(BROKEN_CONDUIT);
			warnNoLibrary(p);
			return;
		}

		QVariantList a;
		a.append(QVariant(CSL1("modal")));

		d = f->create<ConduitConfigBase>(fStack, a);
	}

	if (!d)
	{
		DEBUGKPILOT << "Cannot cast to config base object.";
		fStack->setCurrentIndex(BROKEN_CONDUIT);
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
	if (fStack->insertWidget(NEW_CONDUIT,d->widget())<0)
	{
		DEBUGKPILOT << "Cannot add config widget to stack.";
	}
	else
	{
		d->load();
		fStack->setCurrentIndex (NEW_CONDUIT);
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
		fStack->setCurrentIndex (0);
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
	fConduitList->setCurrentItem( fCurrentConduit );
}

void ConduitConfigWidget::selected(QTreeWidgetItem *curr, QTreeWidgetItem *)
{
	FUNCTIONSETUP;
	DEBUGKPILOT << ( curr ? curr->text(CONDUIT_NAME) : CSL1("None") );

	if (curr!=fCurrentConduit)
	{
		if (!release())
		{
			fConduitList->blockSignals(true);
			QTimer::singleShot(1,this,SLOT(unselect()));
			return;
		}
	}
	fCurrentConduit=curr;
	loadAndConfigure(curr);
	emit sizeChanged();

	// set the dialog title to the selected item
	QTreeWidgetItem *pParent = curr->parent();
	QString title;
	title = pParent ? pParent->text(CONDUIT_NAME) + CSL1(" - ") : QString() ;
	title += curr->text(CONDUIT_NAME);
	fTitleText->setText(title);
}

void ConduitConfigWidget::configure()
{
	loadAndConfigure(fConduitList->currentItem());
}

void ConduitConfigWidget::warnNoExec(const QTreeWidgetItem * p)
{
	FUNCTIONSETUP;

	QString msg = i18n("<qt>No library could be "
		"found for the conduit %1. This means that the "
		"conduit was not installed properly.</qt>", p->text(CONDUIT_NAME));

	DEBUGKPILOT << "No library for [" << p->text(CONDUIT_NAME) << ']';

	KMessageBox::error(this, msg, i18n("Conduit Error"));
}

void ConduitConfigWidget::warnNoLibrary(const QTreeWidgetItem *p)
{
	FUNCTIONSETUP;

	QString msg = i18n("<qt>There was a problem loading the library "
		"for the conduit %1. This means that the "
		"conduit was not installed properly.</qt>", p->text(CONDUIT_NAME));

	DEBUGKPILOT << "Cannot load library for ["
		<< p->text(CONDUIT_NAME) << ']';

	KMessageBox::error(this, msg, i18n("Conduit Error"));
}

void ConduitConfigWidget::save()
{
	FUNCTIONSETUP;

	// Only new-style conduits and the general setup have changes that need to be committed
	// old-style conduits have their own config dlg which commits them itself
	if ( fStack->currentIndex ()==NEW_CONDUIT )
	{
		if (fCurrentConfig) fCurrentConfig->commit();
	}

	QStringList activeConduits;
	QTreeWidgetItemIterator it( fConduitList );
	while ( *it ) {
		QTreeWidgetItem *p = *it;
		if ( p && p->checkState(CONDUIT_NAME))
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
	if (fConduitsItem)
	{
		QTreeWidgetItem *q = 0L;
		for (unsigned int child_index=0; (q=fConduitsItem->child(child_index)); ++child_index)
		{
			q->setCheckState(CONDUIT_NAME,
				(potentiallyInstalled.indexOf(q->text(CONDUIT_DESKTOP))<0) ? Qt::Unchecked : Qt::Checked);
		}
	}


	// Only new-style conduits and the general setup have changes that need to be committed
	// old-style conduits have their own config dlg which commits them itself
	if ( fStack->currentIndex ()==NEW_CONDUIT )
	{
		if (fCurrentConfig) fCurrentConfig->load();
	}
}


void ConduitConfigWidget::reopenItem(QTreeWidgetItem *i)
{
	i->setExpanded(true);
}

void ConduitConfigWidget::autoDetectDevice()
{
	FUNCTIONSETUP;
}

