#include "abbrowser.h"
#include <qstring.h>

#include <kapp.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kaboutdata.h>

static KCmdLineOptions kmoptions[] =
{
  { "a", 0 , 0 },
  { "addr <email>",	I18N_NOOP("Update entry with given email address"), 0 },
  { "n", 0 , 0 },
  { "name <full name>",	I18N_NOOP("Set full name of entry being updated"), 0 },
  { 0, 0, 0}
};


int main(int argc, char *argv[])
{
  KAboutData about("abbrowser", I18N_NOOP("Abbrowser"), 
                   "1.0", 
                   "Abbrowser --- KDE Address Book\n\n",
		   KAboutData::License_GPL,
                   "(c) 1997-2000, The KDE PIM Team" );

  KCmdLineArgs::init(argc, argv, &about);
  KCmdLineArgs::addCmdLineOptions( kmoptions ); // Add kmail options
  
  KApplication app(argc, argv, "Abbrowser");
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  // All session management is handled in the RESTORE macro
  if (app.isRestored())
  {
    RESTORE(Pab)
  }
  else
  {
    QString name, addr;

    QCString nameStr = args->getOption("name");
    if (!nameStr.isEmpty())
      name = QString::fromLocal8Bit( nameStr );
    QCString addrStr = args->getOption("addr");
    if (!addrStr.isEmpty())
      addr = QString::fromLocal8Bit( addrStr );
    
    args->clear();
    Pab *widget = new Pab;
    widget->show();
    
    if (!addr.isEmpty())
      widget->updateContact( addr, name );
  }

  return app.exec();
}

