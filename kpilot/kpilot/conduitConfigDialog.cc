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

static const char *conduitconfigdialog_id =
	"$Id$";

#include "options.h"

#include <qlistview.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qfile.h>
#include <qpushbutton.h>

#include <kservice.h>
#include <kservicetype.h>
#include <kuserprofile.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klibloader.h>

#include "plugin.h"
#include "kpilotConfig.h"

#include "conduitConfigDialog_base.h"
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
	if (s.find("<qt>",0,false) == -1)
	{
		s.prepend("<qt>");
		s.append("</qt>");
	}

	tip(fListView->itemRect(l),s);
}


#if 0
/**
*** As-yet unused class for a list of QCheckBoxes, to replace
*** the unwieldy two-listbox layout we have now. This code
*** pretty much copies things from KDebugDialog.
***
*** We'll be inserting conduitDescription objects into the list,
*** which subsume the hacked QListViewItems we use now. 
**/

/*
** KCheckBoxList
*/

#include <qscrollview.h>
#include <qptrlist.h>
#include <qvbox.h>
#include <qcheckbox.h>

class KCheckBoxList : public QScrollView
{
public:
	KCheckBoxList(QWidget *parent=0L);
	virtual ~KCheckBoxList();

	void addBoxes(const QStringList &);
	bool addBox(QCheckBox *p) 
		{ if (p->parent()==boxParent()) 
			{ m_boxes.append(p); return true; }
		  else return false; } ;
	QPtrList<QCheckBox> checkedBoxes() const;
	const QPtrList<QCheckBox> allBoxes() const { return m_boxes; } ;
	
	void selectAllBoxes(bool);

	QWidget *boxParent() const { return m_box; } ;
	
protected:
	QVBox *m_box;
	QPtrList<QCheckBox> m_boxes;
} ;

KCheckBoxList::KCheckBoxList(QWidget *parent) :
	QScrollView(parent)
{
	setResizePolicy(QScrollView::AutoOneFit);
	m_box = new QVBox( viewport() );
	addChild(m_box);
}

KCheckBoxList::~KCheckBoxList()
{
	// All of the QCheckBoxes are my children,
	// they die with the widget.
}

void KCheckBoxList::addBoxes(const QStringList &l)
{
	QStringList::ConstIterator it = l.begin();
	for ( ; it != l.end(); ++it)
	{
		QCheckBox *cb = new QCheckBox(*it,m_box);
		m_boxes.append(cb);
	}
}

void KCheckBoxList::selectAllBoxes(bool b)
{
	QCheckBox *p;
	
	for (p=m_boxes.first(); p; p=m_boxes.next())
	{
		p->setChecked(b);
	}
}

QPtrList<QCheckBox> KCheckBoxList::checkedBoxes() const
{
	QPtrList<QCheckBox> l;
	QPtrListIterator<QCheckBox> it(m_boxes);
	QCheckBox *p;
	
	for ( ; (p = it.current()) ; ++it)
	{
		if (p->isChecked()) l.append(p);
	}
	
	return l;
}

class ConduitDescription : public QCheckBox
{
public:
	ConduitDescription(KCheckBoxList *owner,
		const QString &name,
		const QString &comment,
		const QString &desktop,
		const QString &library);
	virtual ~ConduitDescription();
	
	QString conduit() const { return text(); } ;
	QString comment() const { return fComment; } ;
	QString desktop() const { return fDesktop; } ;
	QString library() const { return fLibrary; } ;
	
protected:
	QString fComment,fDesktop,fLibrary;
} ;

ConduitDescription::ConduitDescription(KCheckBoxList *owner,
	const QString &name,
	const QString &comment,
	const QString &desktop,
	const QString &library) :
	QCheckBox(name,owner->boxParent()),
	fComment(comment),
	fDesktop(desktop),
	fLibrary(library)
{
	owner->addBox(this);
}

ConduitDescription::~ConduitDescription()
{
}
#endif


ConduitConfigDialog::ConduitConfigDialog(QWidget * _w, const char *n,
	bool m) : UIDialog(_w, n, m)
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
	QObject::connect(fConfigWidget->active,
		SIGNAL(doubleClicked(QListViewItem *)),
		this,SLOT(configureConduit()));

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

	(void) conduitconfigdialog_id;
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

		if (!o->exec().isEmpty())
		{
			kdWarning() << k_funcinfo
				<< ": Old-style conduit found "
				<< o->name()
				<< endl;
		}

		if (potentiallyInstalled.contains(o->desktopEntryName()) == 0)
		{
			p = new QListViewItem(fConfigWidget->available,
				o->name(),
				o->comment(),
				o->desktopEntryName(),
				o->library());
		}
		else
		{
			p = new QListViewItem(fConfigWidget->active,
				o->name(),
				o->comment(),
				o->desktopEntryName(),
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

	const char *library = p->text(CONDUIT_LIBRARY);

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
	a.append("modal");

	QObject *o = f->create(this, 0L, "ConduitConfig",a);


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

	ConduitConfig *d = dynamic_cast<ConduitConfig *>(o);

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

	d->setConfig(&KPilotConfig::getConfig());
	d->readSettings();
	d->exec();

	delete d;
	KLibLoader::self()->unloadLibrary(
		library);
}


void ConduitConfigDialog::warnNoExec(const QListViewItem * p)
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

void ConduitConfigDialog::warnNoLibrary(const QListViewItem *p)
{
	FUNCTIONSETUP;

	QString msg = i18n("<qt>There was a problem loading the library "
		"for the conduit %1. This means that the "
		"conduit was not installed properly.</qt>")
		.arg(p->text(CONDUIT_NAME));

	KMessageBox::error(this, msg, i18n("Conduit Error"));
}

/* virtual */ void ConduitConfigDialog::commitChanges()
{
	FUNCTIONSETUP;

	QStringList activeConduits;
	const QListViewItem *p = fConfigWidget->active->firstChild();
	KPilotConfigSettings & config = KPilotConfig::getConfig();



	while (p)
	{
		activeConduits.append(p->text(CONDUIT_DESKTOP));
		p = p->nextSibling();
	}
	config.setConduitGroup().setInstalledConduits(activeConduits);
	config.sync();
}




// $Log$
// Revision 1.10  2002/12/31 13:22:07  mueller
// CVS_SILENT fixincludes
//
// Revision 1.9  2002/04/20 13:03:31  binner
// CVS_SILENT Capitalisation fixes.
//
// Revision 1.8  2002/04/16 18:18:13  adridg
// Minor debugging fixups by David B
//
// Revision 1.7  2002/01/26 15:00:11  adridg
// Dblclick to configure
//
// Revision 1.6  2002/01/02 11:42:19  bero
// Fix build.
//
// Revision 1.5  2001/12/31 09:26:15  adridg
// Removed support for old-style Exec= conduits
//
// Revision 1.4  2001/11/18 16:59:55  adridg
// New icons, DCOP changes
//
// Revision 1.3  2001/10/19 14:03:04  adridg
// Qt3 include fixes
//
// Revision 1.2  2001/10/08 22:20:18  adridg
// Changeover to libkpilot, prepare for lib-based conduits
//
// Revision 1.1  2001/10/04 16:53:57  adridg
// New files for newstyle config
//
