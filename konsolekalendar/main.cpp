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
#include "konsolekalendarepoch.h"

using namespace KCal;
using namespace std;

static const char *description = I18N_NOOP("KonsoleKalendar");

static KCmdLineOptions options[] =
{
  { "help", I18N_NOOP("Prints this help"), 0 },
  { "verbose", I18N_NOOP("Output helpful (?) debug info"), 0 },
  { "file <calendar-file>", I18N_NOOP("Specify which calendar you want to use."), 0 },
  { "import <import-file>", I18N_NOOP("Import this calendar to main calendar"), 0 },
  { "next", I18N_NOOP("Next activity in calendar"), 0 },
  { "date <date>", I18N_NOOP("Show selected day's calendar"), 0 },
  { "time <time>", I18N_NOOP("Show selected time at calendar"), 0 },
  { "start-date <start-date>", I18N_NOOP("Start from this day"), 0 },
  { "start-time <start-time>", I18N_NOOP("Start from this time [mm:hh]"), 0 },
  { "end-date <end-date>", I18N_NOOP("End to this day"), 0 },
  { "end-time <end-time>", I18N_NOOP("End to this time [mm:hh]"), 0 },
  { "epoch-end <epoch-time>", I18N_NOOP("End time in epoch format"), 0 },
  { "epoch-start <epoch-time>", I18N_NOOP("Start time in epoch format"), 0 },
  { "description <description>", I18N_NOOP("Add description to event (works with add and change)"), 0 },
  { "summary <summary>", I18N_NOOP("Add description to event (works with add and change)"), 0 },
  { "all", I18N_NOOP("Show all entries"), 0 },
  { "create", I18N_NOOP("if calendar not available new caledar file"), 0 },
  { "add", I18N_NOOP("Add an event"), 0 },
  { "change", I18N_NOOP("Delete an event (currently not implemented)"), 0 },
  { "delete", I18N_NOOP("Delete an event (currently not implemented)"), 0 },

  KCmdLineLastOption
};

int main(int argc, char *argv[])
{
  KAboutData aboutData( "konsolekalendar", I18N_NOOP( "KonsoleKalendar" ),
                        "0.6", description, KAboutData::License_GPL,
                        "(c) 2002-2003, Tuukka Pasanen", 0, 0,
                        "illuusio@mailcity.com");

  aboutData.addAuthor("Tuukka Pasanen",0, "illuusio@mailcity.com");
  aboutData.addAuthor("Allen D. Winter", 0, "winterz@earthlink.net");

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  QString KalendarFile;
  QDate startdate = QDate::currentDate();
  QTime starttime(7,0);

  QDate enddate = QDate::currentDate();
  QTime endtime(17,0);

  QDate date = QDate::currentDate();
  QTime time = QTime::currentTime();

  bool view = true;
  bool add = false;
  bool change = false;
  bool del = false;
  bool create = false;
  bool calendarFile = false;
  bool importFile = false;

  QString option;

  KApplication app( false, false );

  KonsoleKalendarVariables variables;

  if ( args->isSet("verbose") ) {
     variables.setVerbose(true);
  }

  /*
   *  Switch on adding
   *
   */
  if ( args->isSet("add") ) {
    view=false;
    add=true;

    if(variables.isVerbose()) {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Add event" << endl;
    }


  }

  /*
   *  Switch on Create
   *
   */
  if ( args->isSet("create") ) {
    create=true;

    if(variables.isVerbose()) {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Create event" << endl;
    }


  }


  /*
   *  Switch on Change
   *
   */
  if ( args->isSet("change") ) {
    view=false;
    add=false;
    change=true;

    if(variables.isVerbose()) {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Change event" << endl;
    }


  }

  /*
   *  Switch on deleting
   *
   */
  if ( args->isSet("delete") ) {
    view=false;
    add=false;
    change=false;
    del=true;

    if( variables.isVerbose() ) {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Delete event" << endl;
    }


  }

  /*
   *  If there is summary attached.
   *
   */
  if ( args->isSet("summary") ) {
    option = args->getOption("summary");

    if( variables.isVerbose() ) {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Summary: (" << option << ")" << endl;
    }

    variables.setSummary(option);

    //variables.setDate(date);
  }

  /*
   *  If there is description attached.
   *
   */
  if ( args->isSet("description") ) {
    option = args->getOption("description");

    if( variables.isVerbose() ) {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Description: (" << option << ")" << endl;
    }

    variables.setDescription(option);

    //variables.setDate(date);
  }

/*
   *  Show next happening and exit
   *
   */
  if ( args->isSet("epoch-start") )
  {
     option = args->getOption("epoch-start");

    if( variables.isVerbose() ) {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Show next happening in calendar and exit" << endl;
    }

    // KonsoleKalendarEpoch::epoch2QDateTime( option );

    //variables.setNext(true);
  }




  /*
   *  Show next happening and exit
   *
   */
  if ( args->isSet("next") )
  {

    if( variables.isVerbose() ) {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Show next happening in calendar and exit" << endl;
    }
    variables.setNext(true);
  }

  /*
   *  If we like to see some date
   *
   */
  if ( args->isSet("date") )
  {
    option = args->getOption("date");

    if( variables.isVerbose() ) {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Show date info and exit: (" << option << ")" << endl;
    }

    date = variables.parseDate(option);

    //variables.setDate(date);
  }

  /*
   *  If we like to see some time
   *
   */
  if ( args->isSet("time") ) {
    option = args->getOption("time");

    if( variables.isVerbose() ) {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Show date info and exit: (" << option << ")" << endl;
    }

    time = variables.parseTime(option);

    //variables.setDate(date);
  }


  /*
   *  Set starting date for calendar
   *
   */
  if ( args->isSet("start-date") ) {
    option = args->getOption("start-date");

    if( variables.isVerbose() ) {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Start date: (" << option << ")" << endl;
    }

    startdate = variables.parseDate(option);

    //variables.setStartDate(date);
  }


  /*
   *  Set starting time
   *
   */
  if ( args->isSet("start-time") ) {
    option = args->getOption("start-time");

    if( variables.isVerbose() ) {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Start time: (" << option << ")" << endl;
    }

    starttime = variables.parseTime(option);

    //variables.setStartDate(date);
  }

  /*
   *  Set end date for calendar
   *
   */

  if ( args->isSet("end-date") ) {
    QString option = args->getOption("end-date");

    if( variables.isVerbose() ) {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | End date: (" << option << ")" << endl;
    }

    enddate = variables.parseDate(option);

    //variables.setEndDate(date);
  }

  /*
   *  Set ending time
   *
   */
  if ( args->isSet("end-time") ) {
    option = args->getOption("start-time");

    if( variables.isVerbose() ) {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | End time: (" << option << ")" << endl;
    }

    endtime = variables.parseTime(option);

    // variables.setStartDate(date);
  }




  if( args->isSet("all") ) {
    variables.setAll( true );
  } else {
    variables.setAll( false );
  } // else

  if ( args->isSet("import") ) {
    importFile = true;
    option = args->getOption("import");
    variables.setImportFile( option );

    if( variables.isVerbose() ) {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | importing file from: (" << option << ")" << endl;
    } // if verbose

  } // if


  if ( args->isSet("file") ) {
    calendarFile = true;
    option = args->getOption("file");
    variables.setCalendarFile( option );

    if( variables.isVerbose() ) {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | using calendar at: (" << variables.getCalendarFile() << ")" << endl;
    } // if verbose

  } else {
    KConfig cfg( locate( "config", "korganizerrc" ) );

    cfg.setGroup("General");
    KURL url( cfg.readPathEntry("Active Calendar") );
    if ( url.isLocalFile() )
    {
      KalendarFile = url.path();

      variables.setCalendarFile(KalendarFile);

      if( variables.isVerbose() ) {
        cout << "main.cpp::int main(int argc, char *argv[]) | Calendar file currently is " << variables.getCalendarFile().local8Bit() << endl;
      } // if verbose

    } else {
      cout << i18n("Remote files are not supported yet.").local8Bit() << endl;
    } // else
  } // else

  args->clear(); // Free up some memory.

  QDateTime startdatetime(startdate, starttime);
  QDateTime enddatetime(enddate, endtime);
  QDateTime datetime( date, time );
  variables.setStartDate( startdatetime );
  variables.setEndDate( enddatetime );
  variables.setDate( datetime );

  KonsoleKalendar *konsolekalendar = new KonsoleKalendar(variables);

   if( calendarFile && create ) {
     if( variables.isVerbose() ) {
       kdDebug() << "main.cpp::int main(int argc, char *argv[]) | create file" << endl;
     }
     konsolekalendar->createCalendar();
   } // if

  /*
   * Opens calendar file so we can use it;)
   * Because at this point we don't know what well
   * Do with it..
   *
   * Adds it to konsolekalendarvariables also..
   */
   if( konsolekalendar->openCalendar() ) {

     if( importFile ) {
       konsolekalendar->importCalendar();
     }

     if( add ) {
       konsolekalendar->addEvent();
     }

     if( view ) {
       konsolekalendar->showInstance();
     }

     konsolekalendar->closeCalendar();
   }

   delete konsolekalendar;

  if( variables.isVerbose() ) {
    kdDebug() << "main.cpp::int main(int argc, char *argv[]) | exiting" << endl;
  }

  return 0;
}
