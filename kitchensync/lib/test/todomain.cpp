#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include <todo.h>
#include <todosyncee.h>
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
  kdDebug() << "Welcome to the todo syncee/syncentry test suite" << endl << endl;

  KSync::TodoSyncee* syncee;
  KSync::TodoSyncEntry *entry;
  KCal::Todo *todo;
  syncee = new KSync::TodoSyncee();

  for (int i = 0; i <= 10; i++ ) {
      todo = new KCal::Todo();
      todo->setSummary("Test summary " + QString::number(i ) );
      entry = new KSync::TodoSyncEntry( todo );
      kdDebug() << "Todo " << i << " " << entry->todo()->uid() << " "
                << entry->id() << endl;
      kdDebug() << "Todo " << i << " Summary :" << entry->todo()->summary() << endl;
      kdDebug() << "Todo " << i << " State: " << entry->state() << endl;
      syncee->addEntry( entry );

      entry = (KSync::TodoSyncEntry*) entry->clone();
      kdDebug() << "Cloned sum: "<< entry->id() << entry->name() << endl;
      kdDebug() << "Cloned" << entry->todo()->summary() << endl;

      delete entry;
  };

  kdDebug() << "Testing now the firstEntry / nextEntry iteratoren" << endl;

  for ( entry = (KSync::TodoSyncEntry*)syncee->firstEntry();
        entry != 0;
        entry = (KSync::TodoSyncEntry*)syncee->nextEntry() ) {

      kdDebug() << entry->type() << " "<< syncee->type() << endl;
      kdDebug() << entry->id() << endl;
      kdDebug() << entry->name() << endl;
      kdDebug() << entry->state() << endl;
      kdDebug() << "-------------" << endl;
  };

  kdDebug() << "Now cloning the SYncee " << endl;
  KSync::Syncee* cloned;
  KSync::SyncEntry* clonedE;
  cloned = syncee->clone();
  for ( clonedE = cloned->firstEntry(); clonedE != 0; clonedE = cloned->nextEntry() ) {
      kdDebug() << "Summary " << clonedE->name() << endl;
      kdDebug() << "Id " << clonedE->id() << endl;
      kdDebug() << "Type" << clonedE->type() << endl;
      kdDebug() << "Type" << cloned->type() << endl;
      kdDebug() << "------" << endl;
  }

  return 0;
};
