// gsetupDialog.cc
//
// Copyright (C) 2000 Adriaan de Groot
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING.
//
// This is the version of gsetupDialog.cc for KDE 2 / KPilot 4.
// We have tried to keep it source compatible with the KDE 1 / KPilot 3
// version.

static const char *id="$Id$";

#include "options.h"

#ifdef KDE2
#include <stream.h>
#include <qfileinf.h>
#include <qlabel.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kapp.h>
#include <klocale.h>
#include <kfiledialog.h>

#include "gsetupDialog.moc"
#include "kpilotlink.h"
#else
#include <stream.h>
// Note that we're using QFileDialog instead
// of KFileDialog *only* because my own kdeui
// library is broken.
//
//
#include <qfiledialog.h>
#include <qfileinf.h>
#include <qtstream.h>
#include <qdir.h>
#include <qlabel.h>
 
#include <kconfig.h>
#include <kmsgbox.h>
#include <kiconloader.h>
#include "gsetupDialog.moc"
#include "kpilot.h"
#endif
 


//---------------------------------------------------
//
// setupDialog page is not an exciting class.
// It's just a placeholder in the inheritance hierarchy,
// and its virtual functions all do nothing.
//
//
setupDialogPage::setupDialogPage(
#ifdef KDE2
	const QString &s,
#else
	const char *s,
#endif
	setupDialog *parent,
	KConfig *c) :
	QWidget(parent),
	fTabName(s)
{
	FUNCTIONSETUP;

	parentSetup=parent;
}

/* virtual */ int setupDialogPage::commitChanges(KConfig *c)
{
	FUNCTIONSETUP;

	return 0;
}

/* virtual */ int setupDialogPage::cancelChanges(KConfig *c)
{
	FUNCTIONSETUP;

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
	QTabDialog(parent,name,modal),
	fGroupName(name),
	pageCount(0),
	quitOnClose(!modal)
{
	FUNCTIONSETUP;

	if (modal && debug_level & UI_TEDIOUS)
	{
		cerr << fname << ": This is a modal dialog." << endl;
	}

	setCancelButton();
#ifdef KDE2
	setCaption(caption.isNull() ? name : caption);
#else
	setCaption(caption!=0L ? caption : name);
#endif
	connect(this,SIGNAL(applyButtonPressed()),
                this, SLOT(commitChanges()));
	connect(this, SIGNAL(cancelButtonPressed()),
		this, SLOT(cancelChanges()));
}


void setupDialog::commitChanges()
{
	FUNCTIONSETUP;
	QListIterator<setupDialogPage> i(pages);

	KConfig *config=KGlobal::config();
	config->setGroup(groupName());

	for (i.toFirst(); i.current(); ++i)
	{
		// in case pages change it (which they shouldn't)
		// set the group back to the dialog's group.
		//
		//
		config->setGroup(groupName()); 
		i.current()->commitChanges(config);
	}

	config->sync();
	setResult(1);

	delete config;

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

	KConfig *config=KPilotLink::getConfig();
	config->setGroup(groupName());

	for (i.toFirst(); i.current(); ++i)
	{
		// in case pages change it (which they shouldn't)
		// set the group back to the dialog's group.
		//
		//
		config->setGroup(groupName()); 
		i.current()->commitChanges(config);
	}

	setResult(0);

	delete config;

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



	if (pages.count()==0 && debug_level)
	{
		cerr << fname << ": setupDialog doesn't "
			"have any pages." << endl;
		return;
	}

	if (debug_level & UI_MINOR)
	{
		cerr << fname << ": setupDialog has " << pages.count()
			<< " pages." << endl;
	}

	for (i.toFirst(); i.current(); ++i)
	{
		setupDialogPage *p=i.current();

		p->adjustSize();
		if (p->width() > x) x=p->width();
		if (p->height() > y) y=p->height();
	}

	if (debug_level & UI_TEDIOUS)
	{
		cerr << fname << ": setupDialog has size "
			<< x << 'x' << y << endl;
	}

	resize(x+2*SPACING,y+8*SPACING);
}


int setupDialog::addPage(setupDialogPage *p)
{
	FUNCTIONSETUP;

	if (p==0L) 
	{
		if (debug_level)
		{
			cerr << fname << ": NULL page passed to addPage"
				<< endl;
		}
		return -1;
	}

	addTab(p,p->tabName());

	pages.append(p);
	return pages.count();
}



/* static */ int setupDialog::queryFile(
	QWidget *parent,
	const QString& filelabel,
	const char *filename)
{
	FUNCTIONSETUP;

	if (filename==0L) return 0;

	if (debug_level & UI_TEDIOUS)
	{
		cerr << fname << ": Checking for existence of "
			<< filelabel << ' ' << filename << endl;
	}

	QFileInfo info(filename);
	if (!info.exists())
	{
		if (debug_level)
		{
			cerr << fname << ": " << filelabel << ' ' 
			<< filename << " doesn't exist." << endl;
		}

		// This is a complicated query, with
		// several internationalisation bits.
		//
		//
		QString msg=i18n("Cannot find the");
		msg+=' ';
		msg+=filelabel;
		msg+=' ';
		msg+='"';
		msg+=filename;
		msg+='"';
		msg+='\n';
		msg+=i18n("Really use this file?");

#ifdef KDE2
		int rc=KMessageBox::questionYesNo(parent,
			msg,
			filelabel);
#else
		int rc=KMsgBox::yesNo(parent,
			filelabel,msg,
			KMsgBox::STOP);
#endif

		
		if (debug_level)
		{
			cerr << fname << ": User said "
				<< rc << " about using "
				"nonexistent file." << endl;
		}

		return rc;
	}

	return 0;
}







setupInfoPage::setupInfoPage(setupDialog *parent,
#ifdef KDE2
	const QString &title,
	const QString &authors,
	const QString &comment,
#else
	const char *title,
	const char *authors,
	const char *comment,
#endif
	const char *programpath) :
	setupDialogPage(i18n("About"),parent)
{
	FUNCTIONSETUP;
	QLabel *ttllabel,*authlabel,*comlabel;
	QLabel *infoPixmap;
	KIconLoader l;
	int left;

	infoPixmap=new QLabel(this);
#ifdef KDE2
	infoPixmap->setPixmap(l.loadIcon("information.xpm",
		KIcon::Desktop));
#else
	infoPixmap->setPixmap(l.loadIcon("information.xpm"));
#endif
	infoPixmap->adjustSize();
	infoPixmap->move(10,14);
	left=RIGHT(infoPixmap);

	ttllabel=new QLabel(title,this);
	ttllabel->adjustSize();
	ttllabel->move(left,14);

	authlabel=new QLabel(authors,this);
	authlabel->adjustSize();
	authlabel->move(left,BELOW(ttllabel));

	if (comment)
	{
		comlabel=new QLabel(comment,this);
		comlabel->adjustSize();
		comlabel->move(left,BELOW(authlabel));
	}
	else
	{
		comlabel=authlabel;
	}


	if (programpath)
	{
		QLabel *pathlabel=new QLabel(programpath,this);
		pathlabel->adjustSize();
		pathlabel->move(left,BELOW(authlabel));
	}

}

