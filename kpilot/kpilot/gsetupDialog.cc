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
#include <qvbox.h>
#include <qmultilineedit.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kapp.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kaboutdata.h>
#include <kaboutapplication.h>

#include "gsetupDialog.moc"
#include "kpilotlink.h"
 
// Handle code differences between the standalone and
// the kpilot version of this file.
//
//
#ifdef USE_STANDALONE
#define CONFIG	KGlobal::config()
#else
#define CONFIG	KPilotLink::getConfig()
#endif


//---------------------------------------------------
//
// setupDialog page is not an exciting class.
// It's just a placeholder in the inheritance hierarchy,
// and its virtual functions all do nothing.
//
//
setupDialogPage::setupDialogPage(
	const QString &s,
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
	quitOnClose(!modal),
	fConfigVersion(0)
{
	FUNCTIONSETUP;

	if (modal && debug_level & UI_TEDIOUS)
	{
		cerr << fname << ": This is a modal dialog." << endl;
	}

	setCancelButton();
	setCaption(caption.isNull() ? name : caption);

	connect(this,SIGNAL(applyButtonPressed()),
                this, SLOT(commitChanges()));
	connect(this, SIGNAL(cancelButtonPressed()),
		this, SLOT(cancelChanges()));
}


void setupDialog::commitChanges()
{
	FUNCTIONSETUP;
	QListIterator<setupDialogPage> i(pages);

	KConfig *config=CONFIG;
	config->setGroup(groupName());

	if (fConfigVersion)
	{
		config->writeEntry("Configured",fConfigVersion);
	}

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

	KConfig *config=CONFIG;
	config->setGroup(groupName());

	for (i.toFirst(); i.current(); ++i)
	{
		// in case pages change it (which they shouldn't)
		// set the group back to the dialog's group.
		//
		//
		config->setGroup(groupName()); 
		i.current()->cancelChanges(config);
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

	if (debug_level & UI_TEDIOUS)
	{
		cerr << fname << ": Checking for existence of "
			<< filelabel << ' ' << filename << endl;
	}

	QFileInfo info(filename);
	if (!info.exists())
	{
		QString msg=filelabel.arg(filename);
		msg+='\n';
		msg+=i18n("Really use this file?");

		if (debug_level)
		{
			cerr << fname << ": " << msg << endl;
		}


		int rc=KMessageBox::questionYesNo(parent,
			msg,
			filelabel);
		
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

/* static */ int setupDialog::getConfigurationVersion(KConfig *c,
	const QString &g)
{
	int r;

	if (c==0L) return 0;
	c->setGroup(g);

	return c->readNumEntry("Configured",0);
}






setupInfoPage::setupInfoPage(setupDialog *parent,bool includeabout) :
	setupDialogPage(i18n("About"),parent)
{
	FUNCTIONSETUP;


	QLabel *text;
	KIconLoader *l=KGlobal::iconLoader();
	QString s;

	text=new QLabel(this);
	text->setPixmap(l->loadIcon(KGlobal::instance()->instanceName(),
		KIcon::Desktop));
	text->adjustSize();
	text->move(10,10);

	// Now we use a QVBox to arrange all the
	// text on the page so that it doesn't interfere with the icon.
	// We use the about data for this application to fill in
	// the text.
	//
	QVBox *frame=new QVBox(this,0L,0,false);
	frame->setSpacing(10);
	frame->move(10+text->width()+10,10);

	const KAboutData *p=KGlobal::instance()->aboutData();

	text=new QLabel(frame);
	s=p->programName();
	s+=' ';
	s+=p->version();
	s+='\n';
	s+=p->copyrightStatement();
	text->setText(s);

	text=new QLabel(frame);
	s=p->shortDescription();
	text->setText(s);

	text=new QLabel(frame);
	s=p->homepage();
	s+='\n';
	s+=i18n("Send bugs reports to ");
	s+=p->bugAddress();
	text->setText(s);

	if (includeabout)
	{
		QHBox *hb=new QHBox(frame);
		QPushButton *but=new QPushButton(i18n("More About ..."),
			hb);
		connect(but, SIGNAL(clicked()),
			this, SLOT(showAbout()));
		but->adjustSize();
		text=new QLabel(hb);
		text->setText(" ");
		text->adjustSize();
		hb->setStretchFactor(text,100);
	}

	frame->adjustSize();
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

