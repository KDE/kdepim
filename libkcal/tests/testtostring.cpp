
#include <iostream>
#include <kdebug.h>

#include "event.h"
#include "icalformat.h"

using namespace KCal;

int main( int, char ** )
{

 // std::cout << "Hello World!" << std::endl;
  Event *ev = new Event;
  ev->setSummary("Griazi");
  ICalFormat iformat;
  QString icalstr = iformat.toICalString(ev);
  kdDebug() << icalstr << endl;
  Incidence *ev2 = iformat.fromString(icalstr);
  kdDebug() << "Event reread!" << endl ;
  
  if (ev2)
    kdDebug() << iformat.toICalString(ev2) << endl;
  else
    kdDebug() << "Could not read incidence" << endl;
}
