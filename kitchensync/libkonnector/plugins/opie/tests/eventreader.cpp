#include <qbitarray.h>
#include <qfile.h>

#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <qvaluelist.h>
#include <klocale.h>
#include <qstring.h>
#include <kdebug.h>

#include <eventsyncee.h>

#include <opie/categoryedit.h>
#include <opie/datebook.h>
#include <opie/metaevent.h>

static const char *description =
	I18N_NOOP("Testapp");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE


static KCmdLineOptions options[] =
{
    { "cat <path>" , I18N_NOOP("Path to the Category file"), 0},
    { "ev <path>" , I18N_NOOP("Path to the Todolist.xml"),  0},
    { "out <path>" , I18N_NOOP("Path to where this app should write") ,  0 },
    { "met <path>" , I18N_NOOP("Desribes meta Data"),  0 },
    { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};



int main(int argc, char *argv[] )
{
  KAboutData aboutData( "event", I18N_NOOP("Testapp"),
			"0.01", description, KAboutData::License_GPL,
			"(c) 2001, Holger  Freyther", 0, 0, "freyther@kde.org");
  aboutData.addAuthor("Holger  Freyther",0, "freyther@kde.org");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  KApplication::addCmdLineOptions();
  KApplication a;


  if ( !args->isSet("cat") || !args->isSet("ev") ) {
      kdDebug() << "enter paths" << endl;
      return 0;
  }
  QString cat = QString::fromLocal8Bit( args->getOption("cat") );
  QString tod = QString::fromLocal8Bit( args->getOption("ev") );
  OpieHelper::CategoryEdit edit( cat );
  OpieHelper::DateBook dateDB( &edit );

  KSync::EventSyncee* syncee = dateDB.toKDE( tod );

  KSync::EventSyncEntry* entry;
  for ( entry = (KSync::EventSyncEntry*) syncee->firstEntry();
        entry != 0;
        entry = (KSync::EventSyncEntry*) syncee->nextEntry() ) {
      kdDebug() << "uid: " << entry->id() << endl;
      kdDebug() << "Summary: " << entry->name() << endl;
      kdDebug() << "Desc: " << entry->incidence()->description() << endl;
      kdDebug() << "Location: " << entry->incidence()->location() << endl;
      kdDebug() << "Categories:"
                << entry->incidence()->categories().join(";") << endl;
      kdDebug() << "Start Date: " << entry->incidence()->dtStart().toString() <<endl;
      kdDebug() << "End Date: " << entry->incidence()->dtEnd().toString() << endl;
      kdDebug() << "is AllDay: " << entry->incidence()->doesFloat() << endl;
      QPtrList<KCal::Alarm> al = entry->incidence()->alarms();
      KCal::Alarm* alE;
      for ( alE = al.first(); alE != 0; alE = al.next() ) {
          kdDebug() << "Alarm Text: " << alE->text() << endl;
          kdDebug() << "Alarm Offset: " << (alE->startOffset().asSeconds() / -60 ) << endl;
          kdDebug() << "Alarm File: " << alE->audioFile() << endl;
      }
      KCal::Recurrence* rec = entry->incidence()->recurrence();
      kdDebug() << "Does recurr: " << rec->doesRecur() << endl;
      if ( rec->doesRecur() ) {
          QString type;
          switch ( rec->doesRecur() ) {
          case KCal::Recurrence::rDaily: {
              kdDebug() << "Recurrence daily" << endl;
              break;
          }
          case KCal::Recurrence::rWeekly: {
              kdDebug() << "Recurrence weekly" << endl;
              QBitArray array = rec->days();
              if ( array.testBit(0 ) ) kdDebug() << "recurrs Monday " << endl;
              if ( array.testBit(1 ) ) kdDebug() << "recurrs Tuesday" << endl;
              if ( array.testBit(2 ) ) kdDebug() << "recurrs Wednesday" << endl;
              if ( array.testBit(3 ) ) kdDebug() << "recurrs Thursday" << endl;
              if ( array.testBit(4 ) ) kdDebug() << "recurrs Friday" << endl;
              if ( array.testBit(5 ) ) kdDebug() << "recurrs Saturday" << endl;
              if ( array.testBit(6 ) ) kdDebug() << "recurrs Sunday" << endl;
              break;
          }
          case KCal::Recurrence::rMonthlyPos: {
              kdDebug() << "Recurss monthly by day " << endl;
              kdDebug() << " Rec Month: " << entry->incidence()->dtStart().date().toString() << endl;
              break;
          }
          case KCal::Recurrence::rMonthlyDay: {
              kdDebug() << "Monthly by date" << endl;
              break;
          }
          case KCal::Recurrence::rYearlyDay: {
              kdDebug() << "Returns yearly" << endl;
              break;
          }
          };
          kdDebug() << "Frequency " << rec->frequency() << endl;
          kdDebug() << "Recurrence duration: " << rec->duration() << endl;
          if ( rec->duration() == 0 )
              kdDebug() << "end date == " << rec->endDate().toString() << endl;
          kdDebug() << "Created " << rec->recurStart().toString() << endl;
      }
      kdDebug() << "-----" << endl;
  }
  if ( args->isSet("out") ) { // weheee safe it
      QString path = QString::fromLocal8Bit( args->getOption("out") );
      QFile file( path );
      if ( !file.open(IO_WriteOnly ) )
          return 0;
      QByteArray ar = dateDB.fromKDE( syncee );
      file.writeBlock( ar );
      file.close();
  }
  if (args->isSet("met") ) { // check meta data
      /**
       * load from path first
       */
      QString path = QString::fromLocal8Bit( args->getOption("met") );
      KSync::EventSyncee* synceeOld = dateDB.toKDE( path );
      if ( !synceeOld )
          return 0;
      OpieHelper::MetaEvent diffEvent;
      syncee = diffEvent.doMeta( syncee,  synceeOld );
      QPtrList<KSync::SyncEntry> changed = syncee->added() ;
      KSync::SyncEntry* entry;
      kdDebug() << "Added ---------------------------" << endl;
      for (entry = changed.first(); entry != 0; entry =changed.next() ) {
          kdDebug() << "Id " << entry->id() << endl;
          kdDebug() << "Name" << entry->name() << endl;
          kdDebug() << "State " << entry->state() << endl;
          kdDebug() << "------ " << endl;
      }
      kdDebug() << "Modified--------------------" << endl;
      changed = syncee->modified();
      for (entry = changed.first(); entry != 0; entry =changed.next() ) {
          kdDebug() << "Id " << entry->id() << endl;
          kdDebug() << "Name" << entry->name() << endl;
          kdDebug() << "State " << entry->state() << endl;
          kdDebug() << "------ " << endl;
      }
      kdDebug() << "Removed---------------------" << endl;
      changed = syncee->removed();
      for (entry = changed.first(); entry != 0; entry =changed.next() ) {
          kdDebug() << "Id " << entry->id() << endl;
          kdDebug() << "Name" << entry->name() << endl;
          kdDebug() << "State " << entry->state() << endl;
          kdDebug() << "------ " << endl;
      }
  };
}
