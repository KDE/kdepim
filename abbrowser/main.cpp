#include <stdlib.h>

#include <qstring.h>

#include <kcmdlineargs.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include "abbrowser.h"
#include "abbrowserapp.h"

// the dummy argument is required, because KMail apparently sends an empty
// argument.

static KCmdLineOptions kmoptions[] =
{
  { "a", 0 , 0 },
  { "addr <email>", I18N_NOOP("Update entry with given email address"), 0 },
  { "+[argument]", I18N_NOOP("dummy argument"), 0},
  { 0, 0, 0}
};


int main(int argc, char *argv[])
{
  KAboutData about("abbrowser", I18N_NOOP("Abbrowser"), 
                   "1.0", 
                   "Abbrowser --- KDE Address Book\n\n",
		   KAboutData::License_BSD,
                   "(c) 1997-2000, The KDE PIM Team" );

  KCmdLineArgs::init(argc, argv, &about);
  KCmdLineArgs::addCmdLineOptions( kmoptions ); // Add abbrowser options
  KUniqueApplication::addCmdLineOptions();
  
  if (!AbBrowserApp::start()) exit(0);

  AbBrowserApp app;

  return app.exec();
}

