#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <qvaluelist.h>
#include <klocale.h>
#include <qstring.h>

#include <kdevice.h>
#include <konnector.h>
 
static const char *description =
	I18N_NOOP("Testapp");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE
	
	
static KCmdLineOptions options[] =
{
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};



int main(int argc, char *argv[] )
{
  KAboutData aboutData( "dcopclient", I18N_NOOP("Testapp"),
			"0.01", description, KAboutData::License_GPL,
			"(c) 2001, Holger  Freyther", 0, 0, "freyther@kde.org");
  aboutData.addAuthor("Holger  Freyther",0, "freyther@kde.org");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
  KApplication a;

  Konnector konnector;
  QValueList<KDevice> device;
  device = konnector.query();
  for(QValueList<KDevice>::Iterator it = device.begin(); it != device.end(); ++it ){
    qWarning("KDevice: %s", (*it).identify().latin1() );
    QString outp = konnector.registerKonnector( (*it) );
    if(outp.isEmpty() ){
      qWarning("couldn't load" );
    }
  }
  return 0;
}


