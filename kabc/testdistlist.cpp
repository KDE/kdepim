#include <qwidget.h>

#include <kaboutdata.h>
#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "stdaddressbook.h"

#include "distributionlisteditor.h"
#include "distributionlist.h"

using namespace KABC;

static const KCmdLineOptions options[] =
{
  {"list <listname>", I18N_NOOP("Show distribution list with name <listname>"), 0},
  {0,0,0}
};


int main(int argc,char **argv)
{
  KAboutData aboutData("testdistlist",I18N_NOOP("Test Distribution Lists"),"0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  if (args->isSet("list")) {
    QString name = args->getOption("list");
    
    DistributionListManager *manager =
        new DistributionListManager( StdAddressBook::self() );
    manager->load();
    DistributionList *list = manager->list( name );
    if ( !list ) {
      kdDebug() << "No list with name '" << name << "'" << endl;
      return 1;
    } else {
      kdDebug() << list->emails().join(", ") << endl;
      return 0; 
    }
  }

  DistributionListEditor *editor =
      new DistributionListEditor( StdAddressBook::self(), 0 );

  editor->show();

  QObject::connect( &app, SIGNAL( lastWindowClosed() ), &app, SLOT( quit() ) );

  app.exec();
  
  delete editor;
}
