#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kaboutdata.h>

#include <kdebug.h>
#include <kapabilities.h>
#include <konnectormanager.h>

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
  KAboutData aboutData( "testapp", I18N_NOOP("Testapp"),
			"0.01", description, KAboutData::License_GPL,
			"(c) 2001,2002,2003, Holger  Freyther", 0, 0, "freyther@kde.org");
  aboutData.addAuthor("Holger  Freyther",0, "freyther@kde.org");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
  KApplication a;

  KSync::KonnectorManager *konnector = KSync::KonnectorManager::self();
  KSync::Device::ValueList device;
  device = konnector->query();
  kdDebug(5202) << "Starting it " << endl;
  for(KSync::Device::ValueList::Iterator it = device.begin(); it != device.end(); ++it ){
    kdDebug(5201) << "KDevice: " <<  (*it).identify() << endl;
    QString outp = konnector->load( (*it) );
    kdDebug(5202) << "UID " <<  outp << endl;
    KSync::Kapabilities caps = konnector->capabilities( outp );
    caps.setUser("ich" );
    caps.setPassword("doesntmatter");
//    QHostAddress adr;
//    adr.setAddress("192.168.0.10" );
    caps.setDestIP( "192.168.0.10" );
    caps.setMetaSyncingEnabled( true );
    konnector->setCapabilities( outp, caps );
//    konnector->startSync( outp );
    if(outp.isEmpty() ){
      kdDebug(5202) << "couldn't load" << endl;
    }
  }
  return a.exec();
}



