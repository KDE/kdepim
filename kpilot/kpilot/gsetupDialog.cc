// gsetupDialog.cc
//
// Copyright (C) 2000 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING.
//
// This is the version of gsetupDialog.cc for KDE 2 / KPilot 4.
//
// You can compile this file with -DUSE_STANDALONE to produce
// applications that do not depend on KPilotLink. The application
// will then use the standard configuration file (i.e. appnamerc).
//
//

static const char *id="$Id$";

#include "options.h"

#include <stream.h>

#include <qfileinf.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmultilineedit.h>

#include <kmessagebox.h>
#include <kconfig.h>
#include <kapp.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kaboutdata.h>
#include <kaboutapplication.h>
#include <kdebug.h>

#include "gsetupDialog.moc"
#include "kpilotlink.h"
 
#define CONFIG	KPilotLink::getConfig()


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
#ifdef DEBUG
	/* NOTREACHED */
	(void) id;
#endif
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
	if (modal && debug_level & UI_TEDIOUS)
	{
		cerr << fname << ": This is a modal dialog." << endl;
	}

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

	KConfig& config=CONFIG;
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

	KConfig& config=CONFIG;
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
		cerr << fname << ": NULL page passed to addPage"
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

	if (debug_level & UI_TEDIOUS)
	{
		cerr << fname << ": Checking for existence of "
			<< filelabel.latin1() << ' ' << filename.latin1() << endl;
	}

	QFileInfo info(filename);
	if (!info.exists())
	{
		QString msg=filelabel.arg(filename);
		msg+='\n';
		msg+=i18n("Really use this file?");

		if (debug_level)
		{
			cerr << fname << ": " << msg.latin1() << endl;
		}


		int rc=KMessageBox::questionYesNo(parent,
			msg,
			i18n("Missing file?"));
		
		if (debug_level)
		{
			cerr << fname << ": User said "
				<< rc << " about using "
				"nonexistent file." << endl;
		}

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

