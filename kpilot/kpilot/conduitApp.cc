#include <iostream.h>
#include <string.h>
#include "conduitApp.moc"

ConduitApp::ConduitApp(int& argc, char** argv)
  : KApplication(argc, argv), fConduit(0L)
{
  if(argc > 1)
    {
      if(strcmp(argv[1], "-setup") == 0)
	fMode = BaseConduit::Setup;
      else if(strcmp(argv[1], "-info") == 0)
	fMode = BaseConduit::DBInfo;
      else if(strcmp(argv[1], "-backup") == 0)
	fMode = BaseConduit::Backup;
      else
	{
	  fMode = BaseConduit::Error;
	  cerr << "ConduitApp:  Error, unknown command: " << argv[1] << endl;
	  cerr << "usage: " << argv[0] << " [-setup | -info | -backup]" << endl;
	}
    }
  else
    fMode = BaseConduit::HotSync;
}

ConduitApp::ConduitApp(int& argc, char** argv, const QString& rAppName)
  : KApplication(argc, argv, rAppName), fConduit(0L)
{
  if(argc > 1)
    {
      if(strcmp(argv[1], "-setup") == 0)
	fMode = BaseConduit::Setup;
      else if(strcmp(argv[1], "-info") == 0)
	fMode = BaseConduit::DBInfo;
      else if(strcmp(argv[1], "-backup") == 0)
	fMode = BaseConduit::Backup;
      else
	{
	  cerr << "ConduitApp:  Error, unknown command: " << argv[1] << endl;
	  cerr << "usage: " << argv[0] << " [-setup | -info | -backup]" << endl;
	  exit(-1);
	}
    }
  else
    fMode = BaseConduit::HotSync;
}

void
ConduitApp::setConduit(BaseConduit* conduit)
{
  fConduit = conduit;
  if(fMode == BaseConduit::DBInfo)
    cout << conduit->dbInfo();
  else if(fMode == BaseConduit::HotSync)
    conduit->doSync();
  else if(fMode == BaseConduit::Backup)
    conduit->doBackup();
}

int
ConduitApp::exec()
{
  if(fMode == BaseConduit::Setup)
    {
      QWidget* widget = fConduit->aboutAndSetup();
      KApplication::setMainWidget(widget);
      widget->show();
      return KApplication::exec();
    }
  return 0;
}
