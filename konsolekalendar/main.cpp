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

#include "konsolekalendarvariables.h"

using namespace KCal;
using namespace std;

static const char *description = I18N_NOOP("A command line interface to KDE calendars");

static KCmdLineOptions options[] =
{
  { "help", I18N_NOOP("Print this help and exit"), 0 },
  { "verbose", I18N_NOOP("Print helpful runtime messages"), 0 },
  { "file <calendar-file>", I18N_NOOP("Specify which calendar you want to use"), 0 },
  { "import <import-file>", I18N_NOOP("Import this calendar to main calendar"), 0 },
  { "export-type <export-type>", I18N_NOOP("Export file type (Default: text)"), 0 },
  { "export-file <export-file>", I18N_NOOP("Export to file (Default: stdout)"), 0 },
  { "export-list", I18N_NOOP("Print list of export types supported and exit"), 0 },
  { "next", I18N_NOOP("Next activity in calendar"), 0 },
  { "date <date>", I18N_NOOP("Show selected day's calendar"), 0 },
  { "time <time>", I18N_NOOP("Show selected time at calendar"), 0 },
  { "start-date <start-date>", I18N_NOOP("Start from this day"), 0 },
  { "start-time <start-time>", I18N_NOOP("Start from this time [mm:hh]"), 0 },
  { "end-date <end-date>", I18N_NOOP("End at this day"), 0 },
  { "end-time <end-time>", I18N_NOOP("End at this time [mm:hh]"), 0 },
  { "epoch-start <epoch-time>", I18N_NOOP("Start from this time [secs since epoch]"), 0 },
  { "epoch-end <epoch-time>", I18N_NOOP("End at this time [secs since epoch]"), 0 },
  { "description <description>", I18N_NOOP("Add description to event (works with add and change)"), 0 },
  { "summary <summary>", I18N_NOOP("Add summary to event (works with add and change)"), 0 },
  { "all", I18N_NOOP("Show all calendar entries"), 0 },
  { "create", I18N_NOOP("Create new calendar file if one does not exist"), 0 },
  { "add", I18N_NOOP("Add an event"), 0 },
  { "change", I18N_NOOP("Change an event"), 0 },
  { "delete", I18N_NOOP("Delete an event"), 0 },

  KCmdLineLastOption
};

int main(int argc, char *argv[])
{

 

  KAboutData aboutData(
      "konsolekalendar",               // internal program name
      I18N_NOOP( "KonsoleKalendar" ),  // displayable program name.
      "0.8",                           // version string
      description,                     // short porgram description
      KAboutData::License_GPL,         // license type
      "(c) 2002-2003, Tuukka Pasanen", // copyright statement
      0,                               // any free form text
      "http://pim.kde.org",            // program home page address
      "illuusio@mailcity.com"          // bug report email address
      );

  aboutData.addAuthor(
      "Tuukka Pasanen",                // developer's name
      I18N_NOOP("Primary Author"),     // task or role
      "illuusio@mailcity.com",         // email address
      0                                // home page or relevant link
      );
  aboutData.addAuthor(
      "Allen Winter",                  // developer's name
      I18N_NOOP("Janitor"),            // task or role
      "winterz@earthlink.net",         // email address
      0                                // home page or relevant link
      );


      
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  QString KalendarFile;

  // Default values (Now!) for which date and time to show
  QDate date = QDate::currentDate();
  QTime time = QTime::currentTime();

  // Default values for start (today at 0700) and end (today at 1700) timestamps
  QDate startdate = QDate::currentDate();
  QTime starttime(7,0);

  QDate enddate = QDate::currentDate();
  QTime endtime(17,0);

  // Default values for switches
  bool view = true;
  bool add = false;
  bool change = false;
  bool del = false;
  bool create = false;
  bool calendarFile = false;
  bool importFile = false;

  QString option;

  KApplication app(
      false, // do not allowstyles -- disable the loading on plugin based styles
      false  // GUI is not enabled -- disable all GUI stuff
      );

  KonsoleKalendarVariables variables;
  KonsoleKalendarEpoch epochs;

  if ( args->isSet("verbose") ) {
     variables.setVerbose(true);
  }
                                                 
  /*
   *
   *  Switch on export list
   */
  if ( args->isSet("export-list") ) {
     cout << i18n("\nKonsoleKalendar supports these export formats:\n  Text\n  Text-Organizer\n  HTML (not working yet)\n  CSV (not working yet)").local8Bit() << endl;
     return(0);
  }

  /*
   *  Switch on exporting
   *
   */                                                                     
  // TODO: Just playing around.  This isn't real code.
  if ( args->isSet("export-type") ) {
     option = args->getOption("export-type");

     if ( option.upper() == "HTML" ) {
       kdDebug() << "main.cpp::int main(int argc, char *argv[] | Export to HTML" << endl;
     } else if (option.upper() == "CSV" ) {
       kdDebug() << "main.cpp::int main(int argc, char *argv[] | Export to CSV" << endl;
     } else {
       kdDebug() << "main.cpp::int main(int argc, char *argv[] | Export to TXT" << endl;
     }

  }


  /*
   *  Switch on adding
   *
   */
  if ( args->isSet("add") ) {
    view=false;
    add=true;

    kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Add event" << endl;
  }

  /*
   *  Switch on Create
   *
   */
  if ( args->isSet("create") ) {
    create=true;

    kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Create event" << endl;
  }


  /*
   *  Switch on Change
   *
   */
  if ( args->isSet("change") ) {
    view=false;
    add=false;
    change=true;

    kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Change event" << endl;


  }

  /*
   *  Switch on Delete
   *
   */
  if ( args->isSet("delete") ) {
    view=false;
    add=false;
    change=false;
    del=true;

    kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Delete event" << endl;
  }

  /*
   *  If there is summary attached.
   *
   */
  if ( args->isSet("summary") ) {
    option = args->getOption("summary");

    kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Summary: (" << option << ")" << endl;

    variables.setSummary(option);

    //variables.setDate(date);
  }

  /*
   *  If there is description attached.
   *
   */
  if ( args->isSet("description") ) {
    option = args->getOption("description");

    kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Description: (" << option << ")" << endl;

    variables.setDescription(option);

    //variables.setDate(date);
  }

  /*
   *  Show next happening and exit
   *
   */
  if ( args->isSet("next") )
  {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Show next happening in calendar and exit" << endl;

    variables.setNext(true);
  }

  /*
   *  If we like to see some date
   *
   */
  if ( args->isSet("date") )
  {
    option = args->getOption("date");

    kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Show date info and exit: (" << option << ")" << endl;

    //date = variables.parseDate(option);

    date = QDate::fromString( option,  Qt::ISODate );      
    kdDebug() << "main.cpp::int main(int argc, char *argv[]) | date: [" << date.toString() << "]" << endl;

    //variables.setDate(date);
  }

  /*
   *  If we like to see some time
   *
   */
  if ( args->isSet("time") ) {
    option = args->getOption("time");

    kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Show date info and exit: (" << option << ")" << endl;

    //time = variables.parseTime(option);
    time = QTime::fromString( option,  Qt::ISODate );

    kdDebug() << "main.cpp::int main(int argc, char *argv[]) | time: [" << time.toString() << "]" << endl;

    //variables.setDate(date);
  }


  /*
   *  Set starting date for calendar
   *
   */
  if ( args->isSet("start-date") ) {
    option = args->getOption("start-date");

    kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Start date: (" << option << ")" << endl;

    //startdate = variables.parseDate(option);
    startdate = QDate::fromString( option,  Qt::ISODate );
    kdDebug() << "main.cpp::int main(int argc, char *argv[]) | date: [" << startdate.toString() << "]" << endl;


    //variables.setStartDate(date);
  }


  /*
   *  Set starting time
   *
   */
  if ( args->isSet("start-time") ) {
    option = args->getOption("start-time");

    kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Start time: (" << option << ")" << endl;

    //starttime = variables.parseTime(option);

    starttime = QTime::fromString( option,  Qt::ISODate );
    kdDebug() << "main.cpp::int main(int argc, char *argv[]) | time: [" << starttime.toString() << "]" << endl;


    //variables.setStartDate(date);
  }

  /*
   *  Set end date for calendar
   *
   */

  if ( args->isSet("end-date") ) {
    QString option = args->getOption("end-date");

      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | End date: (" << option << ")" << endl;

    //enddate = variables.parseDate(option);
    enddate = QDate::fromString( option,  Qt::ISODate );
    kdDebug() << "main.cpp::int main(int argc, char *argv[]) | date: [" << enddate.toString() << "]" << endl;

    //variables.setEndDate(date);
  }

  /*
   *  Set ending time
   *
   */
  if ( args->isSet("end-time") ) {
    option = args->getOption("end-time");

    kdDebug() << "main.cpp::int main(int argc, char *argv[]) | End time: (" << option << ")" << endl;

    //endtime = variables.parseTime(option);
    endtime = QTime::fromString( option,  Qt::ISODate );

    kdDebug() << "main.cpp::int main(int argc, char *argv[]) | time: [" << endtime.toString() << "]" << endl;


    // variables.setStartDate(date);
  }

  /*
   *  Set start date/time from epoch
   *
   */
  time_t epochstart=0;
  if ( args->isSet("epoch-start") ) {
    option = args->getOption("epoch-start");

    kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Epoch start: (" << option << ")" << endl;

    epochstart = (time_t) option.toULong(0,10);
  }

  /*
   *  Set end date/time from epoch
   *
   */
  time_t epochend=0;
  if ( args->isSet("epoch-end") ) {
    option = args->getOption("epoch-end");

    kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Epoch end: (" << option << ")" << endl;

    epochend = (time_t) option.toULong(0,10);
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

    kdDebug() << "main.cpp::int main(int argc, char *argv[]) | importing file from: (" << option << ")" << endl;

  } // if


  if ( args->isSet("file") ) {
    calendarFile = true;
    option = args->getOption("file");
    variables.setCalendarFile( option );

      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | using calendar at: (" << variables.getCalendarFile() << ")" << endl;

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

  QDateTime startdatetime, enddatetime;
  if ( args->isSet("epoch-start") ) {
    startdatetime = epochs.epoch2QDateTime(epochstart);
  } else {
    startdatetime = QDateTime::QDateTime(startdate, starttime);
  }
  if ( args->isSet("epoch-end") ) {
    enddatetime = epochs.epoch2QDateTime(epochend);
  } else {
    enddatetime = QDateTime::QDateTime(enddate, endtime);
  }
  QDateTime datetime( date, time );
  variables.setStartDate( startdatetime );
  variables.setEndDate( enddatetime );
  variables.setDate( datetime );

  // Some more debug prints
  kdDebug() << "DateTime=" << datetime.toString(Qt::TextDate) << endl;
  kdDebug() << "StartDate=" << startdatetime.toString(Qt::TextDate) << endl;
  kdDebug() << "EndDate=" << enddatetime.toString(Qt::TextDate) << endl;
  //cout << i18n("DateTime=").local8Bit() << datetime.toString(Qt::TextDate).local8Bit() << endl;
  //cout << i18n("StartDate=").local8Bit() << startdatetime.toString(Qt::TextDate).local8Bit() << endl;
  //cout << i18n("EndDate=").local8Bit() << enddatetime.toString(Qt::TextDate).local8Bit() << endl;

  // Sanity checks
  if ( startdatetime > enddatetime ) {
    kdError() << i18n("Ending Date occurs before the Starting Date").local8Bit() << endl;
    return(1);
  }

  args->clear(); // Free up some memory.

  KonsoleKalendar *konsolekalendar = new KonsoleKalendar(variables);

  if ( calendarFile && create ) {
    kdDebug() << "main.cpp::int main(int argc, char *argv[]) | create file" << endl;
    konsolekalendar->createCalendar();
   } // if

  /*
   * Opens calendar file so we can use it;)
   * Because at this point we don't know what we'll
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

    if( change ) {
      konsolekalendar->changeEvent();
    }

    if( del ) {
      konsolekalendar->deleteEvent();
    }

    if( view ) {
      konsolekalendar->showInstance();
    }

    konsolekalendar->closeCalendar();
  }

  delete konsolekalendar;

  kdDebug() << "main.cpp::int main(int argc, char *argv[]) | exiting" << endl;

  return 0;
}
