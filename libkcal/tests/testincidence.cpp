#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "event.h"
#include "icalformat.h"

using namespace KCal;

int main(int argc,char **argv)
{
  KAboutData aboutData("testincidence","Test Incidence","0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);

//  KApplication app( false, false );
  KApplication app;

  Event *event = new Event;
  
  event->setSummary("Test Event");
  
  Incidence *event2 = event->clone();

  ICalFormat f;
  kdDebug() << "EVENT2 START:" << f.toString( event2 ) << "EVENT2 END" << endl;
}
