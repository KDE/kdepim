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

  Konnector *konnector = new Konnector(0,0);
  QValueList<KDevice> device;
  device = konnector->query();
  for(QValueList<KDevice>::Iterator it = device.begin(); it != device.end(); ++it ){
    kdDebug() << "KDevice: " <<  (*it).identify() << endl;
    QString outp = konnector->registerKonnector( (*it) );
    kdDebug() << "UID " <<  outp;
    Kapabilities caps = konnector->capabilities( outp );
    caps.setUser("ich" );
    caps.setPassword("doesntmatter");
    QHostAddress adr;
    adr.setAddress("127.0.0.1" );
    caps.setDestIP(adr );
    konnector->setCapabilities( outp, caps ); 
    if(outp.isEmpty() ){
      qWarning("couldn't load" );
    }
  }
  return 0;
}


