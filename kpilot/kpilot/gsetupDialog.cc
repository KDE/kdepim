// $Id$

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

#include "kpilot.h"
#include "gsetupDialog.moc"


//---------------------------------------------------
//
// setupDialog page is not an exciting class.
// It's just a placeholder in the inheritance hierarchy,
// and its virtual functions all do nothing.
//
//
setupDialogPage::setupDialogPage(setupDialog *parent,
	KConfig *c) :
	QWidget(parent)
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


/* virtual */ const char *setupDialogPage::tabName()
{
	return NULL;
}


//---------------------------------------------------
//
// setupDialog is a base class for organising
// the behavior of conduit setup dialogs. 
//
//
setupDialog::setupDialog(QWidget *parent,
	const char *name, const char *caption, bool modal) :
	QTabDialog(parent,name,modal),
	pageCount(0),
	quitOnClose(!modal)
{
	FUNCTIONSETUP;

	if (modal && debug_level>UI_ACTIONS)
	{
		cerr << fname << ": This is a modal dialog." << endl;
	}

	setCancelButton();
	setCaption(caption!=0L ? caption : name);
	connect(this,SIGNAL(applyButtonPressed()),
                this, SLOT(commitChanges()));
	connect(this, SIGNAL(cancelButtonPressed()),
		this, SLOT(cancelChanges()));
}


/* virtual */ const char *setupDialog::groupName()
{
	return NULL;
}

void setupDialog::commitChanges()
{
	FUNCTIONSETUP;
	int i;

	KConfig *config=kapp->getConfig();
	config->setGroup(groupName());

	for (i=0; i<pageCount; i++)
	{
		config->setGroup(groupName()); // in case pages change it
		if (pages[i])
		{
			pages[i]->commitChanges(config);
		}
	}

	config->sync();
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
	int i;

	KConfig *config=kapp->getConfig();
	config->setGroup(groupName());

	for (i=0; i<pageCount; i++)
	{
		if (pages[i])
		{
			pages[i]->cancelChanges(config);
		}
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
	int i;

	if (pageCount==0 && debug_level)
	{
		cerr << fname << ": setupDialog doesn't "
			"have any pages." << endl;
		return;
	}

	if (debug_level>UI_ACTIONS)
	{
		cerr << fname << ": setupDialog has " << pageCount
			<< " pages." << endl;
	}

	for (i=0; i<pageCount; i++)
	{
		pages[i]->adjustSize();
		if (pages[i]->width() > x) x=pages[i]->width();
		if (pages[i]->height() > y) y=pages[i]->height();
	}

	if (debug_level>TEDIOUS)
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
	if (pageCount>=setupDialog_MAX_OPTIONS_PAGES)
	{
		if (debug_level)
		{
			cerr << fname << ": Maximum number of pages ("
				<< setupDialog_MAX_OPTIONS_PAGES
				<< ") reached." << endl;
		}
		return -1;
	}

	addTab(p,p->tabName());

	pages[pageCount++]=p;
	return pageCount;
}



int setupDialog::queryFile(const QString& filelabel,
	const QString& filename)
{
	FUNCTIONSETUP;

	if (filename.isEmpty()) return 0;

	if (debug_level>TEDIOUS)
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
		msg+=i18n("Really use this file name?");

		int rc=KMsgBox::yesNo(this,
			filelabel,msg,
			KMsgBox::STOP);

		
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
	const char *title,
	const char *authors,
	const char *comment,
	const char *programpath) :
	setupDialogPage(parent)
{
	FUNCTIONSETUP;
	QLabel *ttllabel,*authlabel,*comlabel;
	QLabel *infoPixmap;
	KIconLoader l;
	int left;

	infoPixmap=new QLabel(this);
	infoPixmap->setPixmap(l.loadIcon("information.xpm"));
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

/* virtual */ const char *setupInfoPage::tabName()
{
	return i18n("About");
}
