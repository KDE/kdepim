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
#include <kmsgbox.h>
#include "conduitSetup.moc"

#include "kpilot.h"


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

  KSimpleConfig* config = new KSimpleConfig(kapp->localconfigdir() + "/kpilotconduits");
  config->setGroup("Conduit Names");
  fInstalledConduitNames.clear();
  fAvailableConduitNames.clear();
  config->readListEntry("InstalledConduits", fInstalledConduitNames);
  delete config;
  QString conduitPath = kapp->kde_datadir() + "/kpilot/conduits";
  QDir availableDir(conduitPath);
  fAvailableConduitNames = *(availableDir.entryList());
  fAvailableConduitNames.remove(".");
  fAvailableConduitNames.remove("..");
  // Make sure that all the ones in fInstalledConduitNames are available
  cleanupLists(&fAvailableConduitNames, &fInstalledConduitNames);
  
  // Now actually fill the two list boxes, just make sure that nothing gets
  // listed in both.
  QStrListIterator availList(fAvailableConduitNames);
  while(availList.current())
    {
      if(fInstalledConduitNames.contains(availList.current()) == 0)
	fAvailableConduits->insertItem(availList.current());
      ++availList;
    }
  QStrListIterator installList(fInstalledConduitNames);
  while(installList.current())
    {
      fInstalledConduits->insertItem(installList.current());
      ++installList;
    }
  checkButtons();
}

// Removes any entries from installed that aren't in available
void
CConduitSetup::cleanupLists(const QStrList* available, QStrList* installed)
{
	FUNCTIONSETUP;

  QStrListIterator availList(*available);
  if(availList.current() == 0L)
    installed->clear();
  else
    {
      // Check all our installed ones to make sure they still exist.
      QStrListIterator installedOnes(*installed);
      while(installedOnes.current())
	{
	  if(available->contains(installedOnes.current()) == 0) 
	    // Not in fileList
	    installed->remove(installedOnes.current());
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
  QString conduitPath = kapp->kde_datadir() + "/kpilot/conduits";

  // Unfortunately we need to rewrite the whole file 
  // after a conduit setup, since we don't know what 
  // used to be in there and there's no deleteEntry() in KSimpleConfig.
  unlink(kapp->localconfigdir() + "/kpilotconduits");
  KSimpleConfig* config = new KSimpleConfig(kapp->localconfigdir() + "/kpilotconduits");
  config->setGroup("Conduit Names");
  config->writeEntry("InstalledConduits", fInstalledConduitNames);
  config->setGroup("Database Names");
  QStrListIterator iter(fInstalledConduitNames);
  while(iter.current())
    {
	QString currentConduit=conduitPath+'/'+iter.current();
	currentConduit+=" --info";
	if (debug_level)
	{
		currentConduit+=" --debug ";
		currentConduit+=QString().setNum(debug_level);
	}
	if (debug_level>TEDIOUS)
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
	  tmpMessage = klocale->translate("The conduit ");
	  tmpMessage = tmpMessage + iter.current();
	  tmpMessage = tmpMessage + klocale->translate(" did not identify what database it supports. "
						       "\nPlease check with the conduits author to correct it.");
	  
	  KMsgBox::message(0L, klocale->translate("Conduit error."), 
			   tmpMessage, KMsgBox::STOP);
	}
      else
	config->writeEntry(dbName, iter.current());
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

	if(fSetupConduitProcess.isRunning())
	{
		KMsgBox::message(this, 
			klocale->translate("Setup already in progress."), 
			klocale->translate(
			"A conduit is already being set up.\n"
			"Please complete that setup before starting another."),
			KMsgBox::STOP);
		return;
	}
	QString conduitName =   kapp->kde_datadir() + "/kpilot/conduits/";
	conduitName = conduitName + 
		fInstalledConduits->text(fInstalledConduits->currentItem());

	if (debug_level > UI_ACTIONS)
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
		fSetupConduitProcess << QString(debug_level);
	}
	fSetupConduitProcess.start(KProcess::DontCare);
}
