#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include <addressbooksyncee.h>
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
  KAboutData aboutData( "ab_syncee", I18N_NOOP("Testapp"),
			"0.01", description, KAboutData::License_GPL,
			"(c) 2001, Holger  Freyther", 0, 0, "freyther@kde.org");
  aboutData.addAuthor("Holger  Freyther",0, "freyther@kde.org");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
  KApplication a;
  kdDebug() << "Welcome to the address syncee/syncentry test suite" << endl << endl;

  KSync::AddressBookSyncee* syncee;
  KSync::AddressBookSyncEntry *entry;
  syncee = new KSync::AddressBookSyncee();

  for (int i = 0; i <= 10; i++ ) {
      KABC::Addressee ab;
      ab.setName("name " + QString::number(i ) );
      entry = new KSync::AddressBookSyncEntry( ab );

      kdDebug() << "Ad " << i << " " << entry->addressee().uid() << " "
                << entry->id() << endl;
      kdDebug() << "Event " << i << " Summary:" << entry->addressee().name() << endl;
      entry->setState( KSync::SyncEntry::Added );
      kdDebug() << "Event " << i << " State: " << entry->state() << endl;

      syncee->addEntry( entry );

      entry = (KSync::AddressBookSyncEntry*) entry->clone();
      kdDebug() << "Cloned sum: "<< entry->id() << entry->name() << endl;
      kdDebug() << "Cloned " << entry->state() << endl;
      kdDebug() << "-----------" << endl;

      delete entry;
  };

  kdDebug() << "Testing now the firstEntry / nextEntry iteratoren" << endl;

  for ( entry = (KSync::AddressBookSyncEntry*)syncee->firstEntry();
        entry != 0;
        entry = (KSync::AddressBookSyncEntry*)syncee->nextEntry() ) {

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
      kdDebug() << "State" << clonedE->state() << endl;
      kdDebug() << "------" << endl;
  }

  kdDebug() << "Checking for added" << endl;
  QPtrList<KSync::SyncEntry> list = cloned->added();
  if (list.isEmpty() ) kdDebug() << "List is empty"  << endl;
  for (clonedE = list.first(); clonedE != 0; clonedE = list.next() ) {
      kdDebug() << "Id " << clonedE->id() << endl;
      kdDebug() << "State" << clonedE->state() << endl;
      kdDebug() << "----" << endl;

  };
  return 0;
};
