
#include <qfile.h>

#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <qvaluelist.h>
#include <klocale.h>
#include <qstring.h>
#include <kdebug.h>

#include <todosyncee.h>

#include <opie/categoryedit.h>
#include <opie/todo.h>
#include <opie/metatodo.h>

static const char *description =
	I18N_NOOP("Testapp");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE


static KCmdLineOptions options[] =
{
    { "cat <path>" , I18N_NOOP("Path to the Category file"), 0},
    { "tod <path>" , I18N_NOOP("Path to the Todolist.xml"),  0},
    { "out <path>" , I18N_NOOP("Path to where this app should write") ,  0 },
    { "met <path>" , I18N_NOOP("Desribes meta Data"),  0 },
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
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  KApplication::addCmdLineOptions();
  KApplication a;


  if ( !args->isSet("cat") || !args->isSet("tod") ) {
      kdDebug() << "enter paths" << endl;
      return 0;
  }
  QString cat = QString::fromLocal8Bit( args->getOption("cat") );
  QString tod = QString::fromLocal8Bit( args->getOption("tod") );
  OpieHelper::CategoryEdit edit( cat );
  OpieHelper::ToDo todoDB( &edit );

  KSync::TodoSyncee* syncee = todoDB.toKDE( tod );

  KSync::TodoSyncEntry* entry;
  for ( entry = (KSync::TodoSyncEntry*) syncee->firstEntry();
        entry != 0;
        entry = (KSync::TodoSyncEntry*) syncee->nextEntry() ) {
      kdDebug() << "uid: " << entry->id();
      kdDebug() << "Summary: " << entry->name() << endl;
      kdDebug() << "Desc: " << entry->todo()->description() << endl;
      kdDebug() << "Prio: " << entry->todo()->priority() << endl;
      kdDebug() << "Due: " << entry->todo()->hasDueDate() << endl;
      if ( entry->todo()->hasDueDate() )
          kdDebug() << "DueDt: " << entry->todo()->dtDue().toString() << endl;

      kdDebug() << "Progress: " <<
              entry->todo()->percentComplete() << endl;

      kdDebug() << "Completed:"
                << entry->todo()->isCompleted() << endl;

      kdDebug() << "Categories:"
                << entry->todo()->categories().join(";") << endl;

      kdDebug() << "-----" << endl;
  }
  if ( args->isSet("out") ) { // weheee safe it
      QString path = QString::fromLocal8Bit( args->getOption("out") );
      QFile file( path );
      if ( !file.open(IO_WriteOnly ) )
          return 0;
      QByteArray ar = todoDB.fromKDE( syncee );
      file.writeBlock( ar );
      file.close();
  }
  if (args->isSet("met") ) { // check meta data
      /**
       * load from path first
       */
      QString path = QString::fromLocal8Bit( args->getOption("met") );
      KSync::TodoSyncee* synceeOld = todoDB.toKDE( path );
      if ( !synceeOld )
          return 0;
      OpieHelper::MetaTodo diffTodo;
      syncee = diffTodo.doMeta( syncee,  synceeOld );
      QPtrList<KSync::SyncEntry> changed = syncee->added() ;
      KSync::SyncEntry* entry;
      kdDebug() << "Added ---------------------------" << endl;
      for (entry = syncee->firstEntry(); entry != 0; entry = syncee->nextEntry() ) {
          kdDebug() << "Id " << entry->id() << endl;
          kdDebug() << "Name" << entry->name() << endl;
          kdDebug() << "State " << entry->state() << endl;
          kdDebug() << "------ " << endl;
      }


  };
}
