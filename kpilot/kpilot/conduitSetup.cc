// conduitSetup.cc
//
// Copyright (C) 1998,1999 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 



// REVISION HISTORY 
//
// 3.1b9	By Dan Pilone
// 3.1b10	By Adriaan de Groot: comments added all over the place,
//		removed superfluous "Conduit Setup" label in dialog,
//		moved some UI elements around and added a "Cancel"
//		button.
//
//		Remaining questions are marked with QADE.



#include <stdio.h>
#include <unistd.h>
#include <iostream.h>
#include <qdir.h>
#include <ksimpleconfig.h>
#include <kapp.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kstddirs.h>

#include "conduitSetup.moc"

#include "kpilot.h"
#include "options.h"

static const char *id="$Id$";

CConduitSetup::CConduitSetup(QWidget *parent, char *name) 
  : QDialog(parent,name,TRUE) //,TRUE,WStyle_Customize | WStyle_NormalBorder )
{
	FUNCTIONSETUP;
	
  int V=20;	// Initial vertical position in the dialog

  setCaption(i18n("External Conduit Setup"));
  resize(451,324);
  
  label1 = new QLabel(this, "label1");
  label1->setText(i18n("Available Conduits:"));
  label1->setFont(QFont("times",14,QFont::Normal,(int)0));
  label1->setAlignment(AlignLeft | AlignTop);
  label1->setFrameStyle(QFrame::NoFrame | QFrame::NoFrame);
  label1->setGeometry(14,V,130,23);
	
  label2 = new QLabel(this, "label2");
  label2->setText(i18n("Installed Conduits:"));
  label2->setFont(QFont("times",14,QFont::Normal,(int)0));
  label2->setAlignment(AlignLeft | AlignTop);
  label2->setFrameStyle(QFrame::NoFrame | QFrame::NoFrame);
  label2->setGeometry(261,V,130,23);

	V+=label2->height()+SPACING;

  fInstalledConduits = new QListBox(this, "fInstalledConduits");
  fInstalledConduits->setAutoScrollBar(TRUE);
  fInstalledConduits->setMultiSelection(FALSE);
  fInstalledConduits->setAutoBottomScrollBar(TRUE);
  fInstalledConduits->setDragSelect(TRUE);
  fInstalledConduits->setBottomScrollBar(FALSE);
  fInstalledConduits->setScrollBar(FALSE);
  fInstalledConduits->setGeometry(261,V,182,217);
  fInstalledConduits->setFont(QFont("times",14,QFont::Normal,(int)0));
	connect(fInstalledConduits, SIGNAL(highlighted(int)), 
		this, SLOT(slotSelectInstalled()));

  fAvailableConduits = new QListBox(this, "fAvailableConduits");
  fAvailableConduits->setAutoScrollBar(TRUE);
  fAvailableConduits->setMultiSelection(FALSE);
  fAvailableConduits->setAutoBottomScrollBar(TRUE);
  fAvailableConduits->setDragSelect(TRUE);
  fAvailableConduits->setBottomScrollBar(FALSE);
  fAvailableConduits->setScrollBar(FALSE);
  fAvailableConduits->setGeometry(11,V,167,217);
  fAvailableConduits->setFont(QFont("times",14,QFont::Normal,(int)0));
	connect(fAvailableConduits, SIGNAL(highlighted(int)), 
		this, SLOT(slotSelectAvailable()));
	
  fDoneButton = new QPushButton(this, "fDoneButton");
  fDoneButton->setText(i18n("Done"));
  fDoneButton->setFont(QFont("times",14,QFont::Normal,(int)0));
  fDoneButton->setDefault(FALSE);
  fDoneButton->setToggleButton(FALSE);
  fDoneButton->setAutoResize(FALSE);
  fDoneButton->setGeometry(187,V+fAvailableConduits->height()+SPACING,64,30);
  connect(fDoneButton, SIGNAL(clicked()), this, SLOT(slotDone()));

	fCancelButton = new QPushButton(this,"fCancelButton");
	fCancelButton->setText(i18n("Cancel"));
	fCancelButton->setDefault(TRUE);
	fCancelButton->setGeometry(451-64-SPACING,
		V+fAvailableConduits->height()+SPACING,
		64,30);
	connect(fCancelButton,SIGNAL(clicked()),this,SLOT(slotCancel()));

	V+=label2->height()+2*SPACING;

  // label3 = new QLabel(this, "label3");
  // label3->setText("Conduit Setup");
  // label3->setFont(QFont("times",18,QFont::Bold,(int)1));
  // label3->setAlignment(AlignLeft | AlignTop);
  // label3->setFrameStyle(QFrame::NoFrame | QFrame::NoFrame);
  // label3->setGeometry(161,20,122,30);
	
  fInstallConduit = new QPushButton(this, "fInstallConduit");
  fInstallConduit->setText(i18n("Install"));
  fInstallConduit->setFont(QFont("times",14,QFont::Normal,(int)0));
  fInstallConduit->setDefault(FALSE);
  fInstallConduit->setToggleButton(FALSE);
  fInstallConduit->setAutoResize(FALSE);
  fInstallConduit->setGeometry(188,V,64,30);
  connect(fInstallConduit, SIGNAL(clicked()), this, SLOT(slotInstallConduit()));

	V+=fInstallConduit->height()+SPACING;

  fRemoveConduit = new QPushButton(this, "fRemoveConduit");
  fRemoveConduit->setText(i18n("Uninstall"));
  fRemoveConduit->setFont(QFont("times",14,QFont::Normal,(int)0));
  fRemoveConduit->setDefault(FALSE);
  fRemoveConduit->setToggleButton(FALSE);
  fRemoveConduit->setAutoResize(FALSE);
  fRemoveConduit->setGeometry(188,V,64,30);
  connect(fRemoveConduit, SIGNAL(clicked()), 
		this, SLOT(slotUninstallConduit()));

	V+=fRemoveConduit->height()+SPACING;
	
  fSetupConduit = new QPushButton(this, "fSetupConduit");
  fSetupConduit->setText(i18n("Setup"));
  fSetupConduit->setFont(QFont("times",14,QFont::Normal,(int)0));
  fSetupConduit->setDefault(FALSE);
  fSetupConduit->setToggleButton(FALSE);
  fSetupConduit->setAutoResize(FALSE);
  fSetupConduit->setGeometry(188,V,64,30);
  connect(fSetupConduit, SIGNAL(clicked()), this, SLOT(slotSetupConduit()));
	  fillLists();


	V+=fSetupConduit->height()+SPACING;

}

CConduitSetup::~CConduitSetup()
{
	FUNCTIONSETUP;

	delete fSetupConduit;
	fSetupConduit=NULL;
}

void
CConduitSetup::fillLists()
{
  FUNCTIONSETUP;
  KSimpleConfig* config = new KSimpleConfig("kpilotconduits");
//   KSimpleConfig* config = new KSimpleConfig(kapp->localconfigdir() + "/kpilotconduits");
  config->setGroup("Conduit Names");
  fInstalledConduitNames.clear();
  fAvailableConduitNames.clear();
  fInstalledConduitNames = config->readListEntry("InstalledConduits");
  delete config;
  QString conduitPath = KGlobal::dirs()->resourceDirs("conduits").first();
//   QString conduitPath = kapp->kde_datadir() + "/kpilot/conduits";
  QDir availableDir(conduitPath);
  fAvailableConduitNames = availableDir.entryList();
  fAvailableConduitNames.remove(".");
  fAvailableConduitNames.remove("..");
  // Make sure that all the ones in fInstalledConduitNames are available
  cleanupLists(&fAvailableConduitNames, &fInstalledConduitNames);
  
  // Now actually fill the two list boxes, just make sure that nothing gets
  // listed in both.
  QStringList::Iterator availList = fAvailableConduitNames.begin();
  while(availList != fAvailableConduitNames.end())
    {
      if(fInstalledConduitNames.contains(*availList) == 0)
	fAvailableConduits->insertItem(*availList);
      ++availList;
    }
  QStringList::Iterator installList = fInstalledConduitNames.begin();
  while(installList != fInstalledConduitNames.end())
    {
      fInstalledConduits->insertItem(*installList);
      ++installList;
    }
  checkButtons();
}

// Removes any entries from installed that aren't in available
void
CConduitSetup::cleanupLists(const QStringList* available, QStringList* installed)
{
	FUNCTIONSETUP;

  QStringList::ConstIterator availList = available->begin();
  if(availList == available->end())
    installed->clear();
  else
    {
      // Check all our installed ones to make sure they still exist.
      QStringList::Iterator installedOnes = installed->begin();
      while(installedOnes != installed->end())
	{
	  if(available->contains(*installedOnes) == 0) 
	    // Not in fileList
	    installed->remove(*installedOnes);
	  else
	    ++installedOnes;
	}
    }
}

void
CConduitSetup::slotDone()
{
	FUNCTIONSETUP;

  FILE* conduit;
  char dbName[255];
  int len = 0;
  QString conduitPath = KGlobal::dirs()->resourceDirs("conduits").first();
//   QString conduitPath = kapp->kde_datadir() + "/kpilot/conduits";

  // Unfortunately we need to rewrite the whole file 
  // after a conduit setup, since we don't know what 
  // used to be in there and there's no deleteEntry() in KSimpleConfig.

  // FIXME: Do we still need to do this?
//   unlink(kapp->localconfigdir() + "/kpilotconduits");
  KSimpleConfig* config = new KSimpleConfig("kpilotconduits");
  config->setGroup("Conduit Names");
  config->writeEntry("InstalledConduits", fInstalledConduitNames);
  config->setGroup("Database Names");
  QStringList::Iterator iter = fInstalledConduitNames.begin();
  while(iter != fInstalledConduitNames.end())
    {
	QString currentConduit=conduitPath+'/'+ *iter;
	currentConduit+=" --info";
	if (debug_level)
	{
		currentConduit+=" --debug ";
		currentConduit+=QString().setNum(debug_level);
	}
	if (debug_level&SYNC_TEDIOUS)
	{
		cerr << fname << ": Conduit startup command line is:\n"
			<< fname << ": " << currentConduit << endl;
	}

      conduit = popen(currentConduit, "r");
      if(conduit)
	len = fread(dbName, 1, 255, conduit);
      pclose(conduit);
      dbName[len] = 0L;
      if (len == 0)
	{
	  QString tmpMessage;
	  tmpMessage = i18n("The conduit ");
	  tmpMessage = tmpMessage + *iter;
	  tmpMessage = tmpMessage + i18n(" did not identify what database it supports. "
						       "\nPlease check with the conduits author to correct it.");
	  
	  KMessageBox::error(0L, tmpMessage, i18n("Conduit error."));
	}
      else
	config->writeEntry(dbName, *iter);
      ++iter;
    }
  config->sync();
  delete config;
// delete this;
	
	slotCancel();
}

void CConduitSetup::slotCancel()
{
	FUNCTIONSETUP;

	hide();
	close();
}

void
CConduitSetup::slotInstallConduit()
{
	FUNCTIONSETUP;

	if(fAvailableConduits->currentItem() == -1)
	{
		if (debug_level)
		{
			cerr << fname << ": No item selected " <<
				"but installer called.\n" ;
		}
		return;
	}
  int item = fAvailableConduits->currentItem();
  QString itemText = fAvailableConduits->text(item);
  fAvailableConduits->removeItem(item);
  fInstalledConduitNames.append(itemText);
  fInstalledConduits->insertItem(itemText);
  checkButtons();
}

void
CConduitSetup::slotUninstallConduit()
{
	FUNCTIONSETUP;

	if(fInstalledConduits->currentItem() == -1)
	{
		if (debug_level)
		{
			cerr << fname << ": No item selected but " <<
				"uninstaller called.\n";
		}
		return;
	}
  int item = fInstalledConduits->currentItem();
  QString itemText = fInstalledConduits->text(item);
  fInstalledConduits->removeItem(item);
  fInstalledConduitNames.remove(itemText);
  fAvailableConduits->insertItem(itemText);
  checkButtons();
}

void
CConduitSetup::checkButtons()
{
	FUNCTIONSETUP;

	if(fAvailableConduits->currentItem() == -1)
	{
		fInstallConduit->setEnabled(false);
	}
	else
	{
		fInstallConduit->setEnabled(true);
	}




	if(fInstalledConduits->currentItem() == -1)
	{
		fRemoveConduit->setEnabled(false);
		fSetupConduit->setEnabled(false);
	}
	else
	{
		fRemoveConduit->setEnabled(true);
		fSetupConduit->setEnabled(true);
	}
}

void CConduitSetup::slotSelectAvailable()
{
	FUNCTIONSETUP;

	fInstalledConduits->clearSelection();
	checkButtons();
}

void CConduitSetup::slotSelectInstalled()
{
	FUNCTIONSETUP;

	fAvailableConduits->clearSelection();
	checkButtons();
}


void
CConduitSetup::slotSetupConduit()
{
	FUNCTIONSETUP;

	QString numericArg;


	if(fSetupConduitProcess.isRunning())
	{
		KMessageBox::error(this, 
				     i18n("A conduit is already being set up.\n"
					  "Please complete that setup before starting another."),
				     i18n("Setup already in progress"));
		return;
	}
	QString conduitName =   KGlobal::dirs()->resourceDirs("conduits").first();
	conduitName = conduitName + 
		fInstalledConduits->text(fInstalledConduits->currentItem());

	if (debug_level & SYNC_TEDIOUS)
	{
		cerr << fname << ": Starting setup for conduit "
			<< conduitName << endl;
	}
	fSetupConduitProcess.clearArguments();
	fSetupConduitProcess << conduitName;
	// Changed now that conduits use GNU-style long options
	//
	//
	fSetupConduitProcess << "--setup";
	if (debug_level)
	{
		fSetupConduitProcess << "--debug";
		numericArg.setNum(debug_level);
		fSetupConduitProcess << numericArg ;

	}



	// Next try to place the conduit setup screen "close"
	// to the current mouse position.
	//
	//
	QPoint p=pos();

	if (debug_level & SYNC_TEDIOUS)
	{
		cerr << fname << ": My position is "
			<< p.x() << ',' << p.y()
			<< endl;
	}

	// We use long-style options, but KDE
	// uses short (Xt) style options, so
	// that's why we use "-geometry" and not
	// --geometry.
	//
	//
	fSetupConduitProcess << "-geometry";
	fSetupConduitProcess <<
		QString("+")+numericArg.setNum(p.x()+20)+
		QString("+")+numericArg.setNum(p.y()+20) ;


	fSetupConduitProcess.start(KProcess::DontCare);
}
