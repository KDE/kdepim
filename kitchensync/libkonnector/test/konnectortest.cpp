#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <qvaluelist.h>
#include <klocale.h>
#include <qstring.h>
#include <qhostaddress.h>

#include <kdebug.h>
#include <kapabilities.h>
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

  KSync::Konnector *konnector = new KSync::Konnector(0,0);
  KSync::Device::ValueList device;
  device = konnector->query();
  for(Device::ValueList::Iterator it = device.begin(); it != device.end(); ++it ){
    kdDebug(5201) << "KDevice: " <<  (*it).identify() << endl;
    QString outp = konnector->registerKonnector( (*it) );
    kdDebug(5202) << "UID " <<  outp;
    Kapabilities caps = konnector->capabilities( outp );
    caps.setUser("ich" );
    caps.setPassword("doesntmatter");
//    QHostAddress adr;
//    adr.setAddress("192.168.0.10" );
    caps.setDestIP( "192.168.0.10" );
    caps.setMetaSyncingEnabled( true );
    konnector->setCapabilities( outp, caps );
    konnector->startSync( outp );
    if(outp.isEmpty() ){
      kdDebug(5202) << "couldn't load" << endl;
    }
  }
  return a.exec();
}



