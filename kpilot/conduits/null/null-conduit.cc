// Conduit for KPilot <--> POP3 
// (c) 1998 Dan Pilone


#include <qdir.h>
#include <kapp.h>
#include <kconfig.h>
#include <kmsgbox.h>
#include <ksock.h>
#include "conduitApp.h"
#include "null-conduit.h"
#include "setupDialog.h"


int main(int argc, char* argv[])
{
	ConduitApp a(argc, argv, "null-conduit");
	NullConduit conduit(a.getMode());
	a.setConduit(&conduit);
	return a.exec();
}

NullConduit::NullConduit(eConduitMode mode)
  : BaseConduit(mode)
{
}

NullConduit::~NullConduit()
{
}

void
NullConduit::doSync()
{
	KConfig* config = kapp->getConfig();
	config->setGroup(NullOptions::groupName());
	pilotLink->addSyncLogEntry(config->readEntry("Text"));
}

QWidget*
NullConduit::aboutAndSetup()
{
	return new NullOptions;
}


