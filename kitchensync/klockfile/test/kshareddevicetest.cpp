
#include "../ksharedfiledevice.h"
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <qtextstream.h>

static KCmdLineOptions options[] =
{
  KCmdLineLastOption
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[] )
{
  KAboutData aboutData( "dcopclient", I18N_NOOP("Testapp"),
    "0.01","", KAboutData::License_GPL,
    "(c) 2001, Holger  Freyther", 0, 0, "freyther@kde.org");
  aboutData.addAuthor("Holger  Freyther",0, "freyther@kde.org");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
  KApplication a;
  qWarning("main");
  KSharedFileDevice file;
  file.setName("kuick,crash" );
  qWarning("open" );

  if( file.open(IO_WriteOnly ) )
  {
  qWarning("opened" );
  QTextStream stream(&file );
  }
  KSharedFileDevice file2("kuick,crash");
  if(file2.open(IO_ReadOnly) ){
    qWarning("Test" );
  }else{
    qWarning("Could not read a second time" );
  }
  file.close();
  file.open(IO_ReadWrite);
  file.close();
  return 0;
}
