/***************************************************************************
			  main.cpp  -  description
			     -------------------
    begin                : Sun Jan  6 11:50:14 EET 2002
    copyright            : (C) 2002 by Tuukka Pasanen
    email                : illuusio@mailcity.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include <qdatetime.h>

#include <stdlib.h>
#include <iostream>

#include "konsolekalendar.h"

using namespace KCal;
using namespace std;

static const char *description = I18N_NOOP("KonsoleKalendar");
	
static KCmdLineOptions options[] =
{
  { "help", I18N_NOOP("Prints this help"), 0 },
  { "verbose", I18N_NOOP("Output helpful (?) debug info"), 0 },
  { "file <calendarfile>", I18N_NOOP("Specify which calendar you want to use."), 0 },
  { "next", I18N_NOOP("Next activity in calendar"), 0 },
  { "date <date>", I18N_NOOP("Show day info"), 0 },
  { "startdate <startdate>", I18N_NOOP("From this day"), 0 },
  { "enddate <enddate>", I18N_NOOP("To this day"), 0 },
  { "all", I18N_NOOP("Show all entries"), 0 },
  { "date", I18N_NOOP("Date for which the calendar is shown."), 0},
  { "startdate", I18N_NOOP("Starting date."), 0},
  { "enddate", I18N_NOOP("Ending date."), 0},
  { "file", I18N_NOOP("Location of your calendar file."), 0}
};

int main(int argc, char *argv[])
{
  KAboutData aboutData( "konsolekalendar", I18N_NOOP( "KonsoleKalendar" ),
                        "0.1", description, KAboutData::License_GPL,
                        "(c) 2002, Tuukka Pasanen", 0, 0,
                        "illuusio@mailcity.com");
  aboutData.addAuthor("Tuukka Pasanen",0, "illuusio@mailcity.com");

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  QString KalendarFile;
  QDate date;
  QString option;

  KApplication app( false, false );
	
  KalendarVariables variables;

  if ( args->isSet("verbose") ){
     variables.setVerbose(true);
  }

  /*
   *  Show next happening and exit
   *
   */
  if ( args->isSet("next") ) {
    if(variables.isVerbose()) {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Show next happening in calendar and exit" << endl;
    }
    variables.setNext(true);
  }

  /*
   *  If we like to see some date
   *
   */
  if ( args->isSet("date") ) {
    option = args->getOption("date");
    if(variables.isVerbose()) {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Show date info and exit: (" << option << ")" << endl;             
    }

    date = variables.parseDate(option);

    variables.setDate(date);
  } else {
    variables.setDate(QDate::currentDate());
  }

  /*
   *  Set starting date for calendar
   *
   */
  if ( args->isSet("startdate") ) {
    option = args->getOption("startdate");
    if(variables.isVerbose()){
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Shows all entries from this date 30 days or to enddate: (" << option << ")" << endl;              
    }

    date = variables.parseDate(option);
	
    variables.setStartDate(date);
  }

  /*
   *  Set starting end date for calendar
   *
   */
  if ( args->isSet("enddate") ) {
    QString option = args->getOption("enddate");
    if(variables.isVerbose()) {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Shows all entries to this date: (" << option << ")" << endl;              
    }

    date = variables.parseDate(option);

    variables.setEndDate(date);
  }

  if( args->isSet("all") ) {
    variables.setAll( true );
  } else {
    variables.setAll( false );
  }

  if ( args->isSet("file") ) {
    option = args->getOption("file");
    variables.setCalendarFile(option);

    if(variables.isVerbose()){
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | using calendar at: (" << variables.getCalendarFile() << ")" << endl;              
    }    
  } else {
    KConfig cfg( locate( "config", "korganizerrc" ) );

    cfg.setGroup("General");
    KURL url( cfg.readEntry("Active Calendar") );
    if ( url.isLocalFile() ) {
      KalendarFile = url.path();
    
      variables.setCalendarFile(KalendarFile);
	
      if(variables.isVerbose()){
        cout << "main.cpp::int main(int argc, char *argv[]) | Calendar file currently is " << variables.getCalendarFile() << endl;
      }
    } else {
      cout << i18n("Remote files are not supported yet.") << endl;
    }
  }

  args->clear(); // Free up some memory.

  //variables->setCalendarFile(KalendarFile);
	
  KonsoleKalendar *konsolekalendar = new KonsoleKalendar(variables);
  konsolekalendar->showInstance();

  delete konsolekalendar;

  if(variables.isVerbose()){
    kdDebug() << "main.cpp::int main(int argc, char *argv[]) | exiting" << endl;
  }

  return 0;
}
