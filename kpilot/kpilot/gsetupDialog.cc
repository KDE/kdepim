/* gsetupDialog.cc			KPilot
**
** Copyright (C) 2000-2001 by Dan Pilone
** 
** This defines a base class for all setup dialogs in KPilot and
** KPilot's conduits.
** 
** You can compile this file with -DUSE_STANDALONE to produce
** applications that do not depend on KPilotLink. The application
** will then use the standard configuration file (i.e. appnamerc).
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



static const char *gsetupdialog_id="$Id$";

#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif

#include <iostream.h>

#ifndef QFILEINF_H
#include <qfileinf.h>
#endif
#ifndef QLABEL_H
#include <qlabel.h>
#endif
#ifndef QPUSHBUTTON_H
#include <qpushbutton.h>
#endif
#ifndef QLAYOUT_H
#include <qlayout.h>
#endif
#ifndef QMULTILINEEDIT_H
#include <qmultilineedit.h>
#endif

#ifndef _KMESSAGEBOX_H
#include <kmessagebox.h>
#endif
#ifndef _KCONFIG_H
#include <kconfig.h>
#endif
#ifndef _KAPP_H
#include <kapp.h>
#endif
#ifndef _KLOCALE_H
#include <klocale.h>
#endif
#ifndef _KFILEDIALOG_H
#include <kfiledialog.h>
#endif
#ifndef _KABOUTDATA_H
#include <kaboutdata.h>
#endif
#ifndef _KABOUTAPPLICATION_H
#include <kaboutapplication.h>
#endif
#ifndef _KDEBUG_H
#include <kdebug.h>
#endif

#ifndef _KPILOT_KPILOTCONFIG_H
#include "kpilotConfig.h"
#endif

#include "gsetupDialog.moc"
 


//---------------------------------------------------
//
// setupDialog page is not an exciting class.
// It's just a placeholder in the inheritance hierarchy,
// and its virtual functions all do nothing.
//
//
setupDialogPage::setupDialogPage(
	const QString &s,
	setupDialog *parent) :
	QWidget(parent),
	fTabName(s)
{
	FUNCTIONSETUP;

	parentSetup=parent;
}

/* virtual */ int setupDialogPage::commitChanges(KConfig& c)
{
	FUNCTIONSETUP;
	// Avoid unused-parameter warning
	(void) c;

	return 0;
	/* NOTREACHED */
	(void) gsetupdialog_id;
}

/* virtual */ int setupDialogPage::cancelChanges(KConfig& c)
{
	FUNCTIONSETUP;
	// Avoid unused-parameter warning
	(void) c;

	return 0;
}


/* virtual */ int setupDialogPage::validateChanges(KConfig &)
{
	// (void) c;
	return 0;
}

//---------------------------------------------------
//
// setupDialog is a base class for organising
// the behavior of conduit setup dialogs. 
//
//
setupDialog::setupDialog(QWidget *parent,
	const QString &name,
	const QString &caption,
	bool modal) :
	QTabDialog(parent,name.latin1(),modal),
	fGroupName(name),
	quitOnClose(!modal),
	fConfigVersion(0)
{
	FUNCTIONSETUP;

	const KAboutData *p=KGlobal::instance()->aboutData();
#ifdef DEBUG
	if (modal && debug_level & UI_TEDIOUS)
	{
		kdDebug() << fname << ": This is a modal dialog." << endl;
	}
#endif

	setCancelButton();
	setCaption(p->programName() + QString(" ") +
		p->version());
	// Avoid some unused parameter warnings.
	//
	//
	(void) name.length();
	(void) caption.length();

	connect(this,SIGNAL(applyButtonPressed()),
                this, SLOT(commitChanges()));
	connect(this, SIGNAL(cancelButtonPressed()),
		this, SLOT(cancelChanges()));
}


void setupDialog::commitChanges()
{
	FUNCTIONSETUP;
	QListIterator<setupDialogPage> i(pages);
	int r;

	KConfig& config=KPilotConfig::getConfig();
	config.setGroup(groupName());

	r=0;
	for (i.toFirst(); i.current(); ++i)
	{
		// in case pages change it (which they shouldn't)
		// set the group back to the dialog's group.
		//
		//
		config.setGroup(groupName()); 
		r |= i.current()->validateChanges(config);
		if (r)
		{
			kdDebug() << ": Tab page "
				<< i.current()->tabName()
				<< " failed validation."
				<< endl;
		}
	}
	if (r)
	{
		kdDebug() << ": Validation failed." << endl;
	}

	config.setGroup(groupName()); 
	if (fConfigVersion)
	{
		config.writeEntry("Configured",fConfigVersion);
	}

	for (i.toFirst(); i.current(); ++i)
	{
		// in case pages change it (which they shouldn't)
		// set the group back to the dialog's group.
		//
		//
		config.setGroup(groupName()); 
		i.current()->commitChanges(config);
	}

	config.sync();
	setResult(1);


	if (quitOnClose)
	{
		kapp->quit();
		close();
	}
}

void setupDialog::cancelChanges()
{
	FUNCTIONSETUP;
	QListIterator<setupDialogPage> i(pages);

	KConfig& config=KPilotConfig::getConfig();
	config.setGroup(groupName());

	for (i.toFirst(); i.current(); ++i)
	{
		// in case pages change it (which they shouldn't)
		// set the group back to the dialog's group.
		//
		//
		config.setGroup(groupName()); 
		i.current()->cancelChanges(config);
	}

	setResult(0);


	if (quitOnClose)
	{
		kapp->quit();
		close();
	}
}

void setupDialog::setupWidget()
{
	FUNCTIONSETUP;
	int x=0;
	int y=0;
	QListIterator<setupDialogPage> i(pages);



	if (pages.count()==0)
	{
		kdWarning() << __FUNCTION__ << ": setupDialog doesn't "
			"have any pages." << endl;
		return;
	}

#ifdef DEBUG
	if (debug_level & UI_MINOR)
	{
		kdDebug() << fname << ": setupDialog has " << pages.count()
			<< " pages." << endl;
	}
#endif

	for (i.toFirst(); i.current(); ++i)
	{
		setupDialogPage *p=i.current();

		p->adjustSize();
		if (p->width() > x) x=p->width();
		if (p->height() > y) y=p->height();
	}

#ifdef DEBUG
	if (debug_level & UI_TEDIOUS)
	{
		kdDebug() << fname << ": setupDialog has size "
			<< x << 'x' << y << endl;
	}
#endif

	resize(x+2*SPACING,y+8*SPACING);
}


int setupDialog::addPage(setupDialogPage *p)
{
	FUNCTIONSETUP;

	if (p==0L) 
	{
		kdError() << __FUNCTION__ << ": NULL page passed to addPage"
			<< endl;
		return -1;
	}

	addTab(p,p->tabName());

	pages.append(p);
	return pages.count();
}



/* static */ int setupDialog::queryFile(
	QWidget *parent,
	const QString& filelabel,
	const QString &filename)
{
	FUNCTIONSETUP;

	if (filename.isNull()) return QueryNull;
	if (filename.length()<1) return QueryNull;

#ifdef DEBUG
	if (debug_level & UI_TEDIOUS)
	{
		kdDebug() << fname << ": Checking for existence of "
			<< filelabel.latin1() << ' ' << filename.latin1() << endl;
	}
#endif

	QFileInfo info(filename);
	if (!info.exists())
	{
		QString msg=filelabel.arg(filename);
		msg+='\n';
		msg+=i18n("Really use this file?");

#ifdef DEBUG
		if (debug_level & UI_MAJOR)
		{
			kdDebug() << fname << ": " << msg.latin1() << endl;
		}
#endif


		int rc=KMessageBox::questionYesNo(parent,
			msg,
			i18n("Missing file?"));
		
#ifdef DEBUG
		if (debug_level)
		{
			kdDebug() << fname << ": User said "
				<< rc << " about using "
				"nonexistent file." << endl;
		}
#endif

		return rc;
	}

	return QueryFileExists;
}

/* static */ int setupDialog::getConfigurationVersion(KConfig& c,
	const QString &g)
{
	c.setGroup(g);

	return c.readNumEntry("Configured",0);
}






setupInfoPage::setupInfoPage(setupDialog *parent,bool includeabout) :
	setupDialogPage(i18n("About"),parent)
{
	FUNCTIONSETUP;


	QString s;
	QLabel *text;
	KIconLoader *l=KGlobal::iconLoader();
	const KAboutData *p=KGlobal::instance()->aboutData();

	QGridLayout *grid = new QGridLayout(this,4,4,SPACING);
	grid->addColSpacing(0,SPACING);
	grid->addColSpacing(4,SPACING);


	text=new QLabel(this);
	text->setPixmap(l->loadIcon(KGlobal::instance()->instanceName(),
		KIcon::Desktop));
	text->adjustSize();
	grid->addWidget(text,0,1);


	text=new QLabel(this);
	s=p->programName();
	s+=' ';
	s+=p->version();
	s+='\n';
	s+=p->copyrightStatement();
	text->setText(s);
	grid->addMultiCellWidget(text,0,0,2,3);

	text=new QLabel(this);
	s=p->shortDescription();
	text->setText(s);
	grid->addMultiCellWidget(text,1,1,2,3);

	text=new QLabel(this);
	s=p->homepage();
	s+='\n';
	s+=i18n("Send bugs reports to ");
	s+=p->bugAddress();
	text->setText(s);
	grid->addMultiCellWidget(text,2,2,2,3);

	if (includeabout)
	{
		QPushButton *but=new QPushButton(i18n("More About ..."),
			this);
		connect(but, SIGNAL(clicked()),
			this, SLOT(showAbout()));
		but->adjustSize();
		grid->addWidget(but,3,2);
	}

	grid->setRowStretch(4,100);
	grid->setColStretch(3,100);
}

/* slot */ void setupInfoPage::showAbout()
{
	KAboutApplication *kap=new KAboutApplication(this);
	kap->exec();
	// Experience crashes when deleting kap
	//
	//
	// delete kap;
}


// $Log$
// Revision 1.17  2001/03/27 11:10:39  leitner
// ported to Tru64 unix: changed all stream.h to iostream.h, needed some
// #ifdef DEBUG because qstringExpand etc. were not defined.
//
// Revision 1.16  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.15  2001/02/08 08:13:44  habenich
// exchanged the common identifier "id" with source unique <sourcename>_id for --enable-final build
//
// Revision 1.14  2001/02/05 20:55:07  adridg
// Fixed copyright headers for source releases. No code changed
//
