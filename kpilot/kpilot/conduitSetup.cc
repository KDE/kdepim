// conduitSetup.cc
//
// Copyright (C) 1998,1999,2000 Dan Pilone
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
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qlabel.h>

#include <ksimpleconfig.h>
#include <kapp.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kstddirs.h>
#include <kdebug.h>
#include <kuserprofile.h>
#include <kservice.h>
#include <kservicetype.h>

#include "conduitSetup.moc"

#include "kpilot.h"
#include "options.h"

static const char *id="$Id$";

CConduitSetup::CConduitSetup(QWidget *parent, char *name) 
	: KDialogBase(Plain,i18n("External Conduit Setup"),
		Ok | Cancel,Cancel,
		parent,name,TRUE)
{
	FUNCTIONSETUP;

	QWidget *top=plainPage();
	QGridLayout *grid=new QGridLayout(top,10,3,SPACING); 

  label1 = new QLabel(top, "label1");
  label1->setText(i18n("Available Conduits:"));
	label1->adjustSize();
	grid->addWidget(label1,1,1);

  fAvailableConduits = new QListBox(top, "fAvailableConduits");
  fAvailableConduits->setMultiSelection(FALSE);
	fAvailableConduits->resize(120,200);
	grid->addMultiCellWidget(fAvailableConduits,2,6,1,1);
	connect(fAvailableConduits, SIGNAL(highlighted(int)), 
		this, SLOT(slotSelectAvailable()));
	
	

  label2 = new QLabel(top, "label2");
  label2->setText(i18n("Installed Conduits:"));
	label2->adjustSize();
	grid->addWidget(label2,1,3);

  fInstalledConduits = new QListBox(top, "fInstalledConduits");
  fInstalledConduits->setMultiSelection(FALSE);
	fInstalledConduits->resize(120,200);
	grid->addMultiCellWidget(fInstalledConduits,2,6,3,3);

	connect(fInstalledConduits, SIGNAL(highlighted(int)), 
		this, SLOT(slotSelectInstalled()));

  fInstallConduit = new QPushButton(top, "fInstallConduit");
  fInstallConduit->setText(i18n("Install"));
  fInstallConduit->setDefault(FALSE);
  fInstallConduit->setToggleButton(FALSE);
  fInstallConduit->setAutoResize(FALSE);
	fInstallConduit->adjustSize();
	grid->addWidget(fInstallConduit,3,2);

  connect(fInstallConduit, SIGNAL(clicked()), this, SLOT(slotInstallConduit()));


  fRemoveConduit = new QPushButton(top, "fRemoveConduit");
  fRemoveConduit->setText(i18n("Uninstall"));
  fRemoveConduit->setDefault(FALSE);
  fRemoveConduit->setToggleButton(FALSE);
  fRemoveConduit->setAutoResize(FALSE);
	fRemoveConduit->adjustSize();
	grid->addWidget(fRemoveConduit,4,2);
  connect(fRemoveConduit, SIGNAL(clicked()), 
		this, SLOT(slotUninstallConduit()));

  fSetupConduit = new QPushButton(top, "fSetupConduit");
  fSetupConduit->setText(i18n("Setup"));
  fSetupConduit->setDefault(FALSE);
  fSetupConduit->setToggleButton(FALSE);
  fSetupConduit->setAutoResize(FALSE);
	fSetupConduit->adjustSize();
	grid->addWidget(fSetupConduit,5,2);
  connect(fSetupConduit, SIGNAL(clicked()), this, SLOT(slotSetupConduit()));
	  fillLists();
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
	KConfig* config = KPilotLink::getConfig("Conduit Names");
	QStringList potentiallyInstalled;
	fInstalledConduitNames.clear();
	fAvailableConduitNames.clear();
	potentiallyInstalled = config->readListEntry("InstalledConduits");
	delete config;

	KServiceTypeProfile::OfferList offers = 
		KServiceTypeProfile::offers("KPilotConduit");

	if (debug_level & UI_TEDIOUS)
	{
		QStringList::Iterator i = potentiallyInstalled.begin();

		kdDebug() << fname << ": Currently installed conduits are:"
			<< endl;

		while(i != potentiallyInstalled.end())
		{
			kdDebug() << fname << ": "
				<< (*i)
				<< endl;
			++i;
		}

		kdDebug() << fname << ": Currently available conduits are:"
			<< endl;
	}

	// Now actually fill the two list boxes, just make 
	// sure that nothing gets listed in both.
	//
	//
	QValueListIterator<KServiceOffer> availList(offers.begin());
	while(availList != offers.end())
	{
		kdDebug() << fname << ": "
			<< (*availList).service()->desktopEntryName()
			<< " = "
			<< (*availList).service()->name()
			<< endl;
		if(potentiallyInstalled.contains((*availList).service()->
			desktopEntryName()) == 0)
		{
			fAvailableConduits->insertItem(
				(*availList).service()->name());
		}
		else
		{
			fInstalledConduits->insertItem(
				(*availList).service()->name());
			fInstalledConduitNames.append((*availList).service()->
				desktopEntryName());
		}
		++availList;
	}
	checkButtons();
}

// Removes any entries from installed that aren't in available
void
CConduitSetup::cleanupLists(const QStringList* available, QStringList* installed)
{
	FUNCTIONSETUP;
	QStringList accepted;

	if (!installed) 
	{ 
		kdDebug() << fname << ": Installed = NULL" << endl;
	}
	if (!available) 
	{ 
		kdDebug() << fname << ": Available = NULL" << endl;
		if (installed) installed->clear(); 
		return; 
	}

	QStringList::ConstIterator availList = available->begin();
	if(availList == available->end())
	{
		installed->clear();
		return;
	}
  else
    {
	if (debug_level & UI_TEDIOUS)
	{
		kdDebug() << fname << ": Checking installed ..." << endl;
		while (availList != available->end())
		{
			kdDebug() << fname << ": Available "
				<< *availList
				<< endl;
			++availList;
		}
	}

      // Check all our installed ones to make sure they still exist.
      QStringList::Iterator installedOnes = installed->begin();
      while(installedOnes != installed->end())
	{
	  if (debug_level & UI_TEDIOUS)
	  {
	  	kdDebug() << fname << ": Checking " << *installedOnes << endl;
	}
	  if(available->contains(*installedOnes) == 0) 
	  {
	  if (debug_level & UI_TEDIOUS)
	  {
	    kdDebug() << fname << ": Ignoring it." << endl;
	    }
	    // Not in fileList
	    // installed->remove(*installedOnes);
	    }
	  else
	  {
	  if (debug_level & UI_TEDIOUS)
	  {
	    kdDebug() << fname << ": Adding it." << endl;
	    }
	    accepted.append(*installedOnes);
	    }
	    ++installedOnes;
	}
    }


	(*installed)=accepted;
}

void
CConduitSetup::slotOk()
{
  FUNCTIONSETUP;
  
  char dbName[255];
  int len = 0;
  QString conduitPath = KGlobal::dirs()->resourceDirs("conduits").first();
 
  // This is the KDE1 comment:
  //
  // Unfortunately we need to rewrite the whole file 
  // after a conduit setup, since we don't know what 
  // used to be in there and there's no deleteEntry() in KSimpleConfig.
  //
  // In KDE2 I can't even find an entryiterator, to
  // delete entries from section Database Names, so
  // some extra logic is added to registeredConduit
  // in pilotlink to find out whether the conduit
  // for the database should be run or not.
  //
  //
  KConfig* config = KPilotLink::getConfig("Conduit Names");
  config->writeEntry("InstalledConduits", fInstalledConduitNames);
  config->setGroup("Database Names");

	uint i=0;
	for (i=0; i<fInstalledConduits->count(); i++)
	{
		QString iter=fInstalledConduits->text(i);
		FILE *conduitpipe;

		if (debug_level & SYNC_TEDIOUS)
		{
			kdDebug() << fname << ": Current conduit iter = "
				<< iter
				<< endl;
		}

	KService::Ptr conduit = KService::serviceByName(iter);
	if (!conduit)
	{
		kdDebug() << fname << ": No service associated with "
			<< iter
			<< endl;
		continue;
	}

	if (debug_level & SYNC_TEDIOUS)
	{
		kdDebug() << fname << ": Got service." << endl ;
		kdDebug() << fname << ": Current conduit service from "
			<< (*conduit).desktopEntryPath()
			<< " says exec="
			<< (*conduit).exec()
			<< endl;
	}
	QString currentConduit;
	if (conduitPath.isNull())
	{
		currentConduit=KGlobal::dirs()->findResource("exe",
			(*conduit).exec());
		if (currentConduit.isNull())
		{
			currentConduit=(*conduit).exec();
		}
	}
	else
	{
		currentConduit=conduitPath+'/'+ (*conduit).exec();
	}
	currentConduit+=" --info";
	if (debug_level)
	{
		currentConduit+=" --debug ";
		currentConduit+=QString().setNum(debug_level);
	}
	if (debug_level&SYNC_TEDIOUS)
	{
		kdDebug() << fname << ": Conduit startup command line is:\n"
			<< fname << ": " << currentConduit << endl;
	}

      len=0;
      conduitpipe = popen(currentConduit.latin1(), "r");
      if(conduitpipe)
	{
		len = fread(dbName, 1, 255, conduitpipe);
	      pclose(conduitpipe);
	}
      conduitpipe=0;
      dbName[len] = 0L;
      if (len == 0)
	{
	  QString tmpMessage;
	  tmpMessage = i18n("The conduit %1 did not identify "
				"what database it supports. "
				"\nPlease check with the conduits "
				"author to correct it.").arg(iter);

	  KMessageBox::error(this, tmpMessage, i18n("Conduit error."));
	}
      else
      {
	config->writeEntry(dbName, (*conduit).desktopEntryName());
	}
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
	  kdDebug() << fname << ": No item selected " <<
	    "but installer called.\n" ;
	}
      return;
    }
  int item = fAvailableConduits->currentItem();
  QString itemText = fAvailableConduits->text(item);
  fAvailableConduits->removeItem(item);
	KService::Ptr conduit = KService::serviceByName(itemText);
	if (!conduit)
	{
		kdDebug() << fname << ": No conduit called "
			<< itemText
			<< endl;
	}
	else
	{
		fInstalledConduitNames.append(conduit->desktopEntryName());
	}
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
			kdDebug() << fname << ": No item selected but " <<
				"uninstaller called.\n";
		}
		return;
	}
  int item = fInstalledConduits->currentItem();
  QString itemText = fInstalledConduits->text(item);
  fInstalledConduits->removeItem(item);
	KService::Ptr conduit = KService::serviceByName(itemText);
	if (!conduit)
	{
		kdDebug() << fname << ": No conduit called "
			<< itemText
			<< endl;
	}
	else
	{
		fInstalledConduitNames.remove(conduit->desktopEntryName());
	}
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
	QString conduitName =   
		fInstalledConduits->text(fInstalledConduits->currentItem());
	KService::Ptr conduit = KService::serviceByName(conduitName);
	if (!conduit)
	{
		kdDebug() << fname << ": No conduit called "
			<< conduitName
			<< " could be found."
			<< endl;
		return;
	}
	// Misuse of variables
	conduitName=conduit->exec();

  
  if (debug_level & SYNC_TEDIOUS)
    {
      kdDebug() << fname << ": Starting setup for conduit "
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
      kdDebug() << fname << ": My position is "
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
