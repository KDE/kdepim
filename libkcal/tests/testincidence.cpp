#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "event.h"

using namespace KCal;

int main(int argc,char **argv)
{
  KAboutData aboutData("testincidence","Test Incidence","0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);

//  KApplication app( false, false );
  KApplication app;

  Calendar *cal = new CalendarLocal;

  Event *event = new Event;
  
  event->setSummary("Test Event");
  
  Incidence *event2 = event->clone();
  
  kdDebug() << "Event2: " << event2->summary() << endl;
}
