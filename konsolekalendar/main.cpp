/*******************************************************************************
 * main.cpp                                                                    *
 *                                                                             *
 * KonsoleKalendar is a command line interface to KDE calendars                *
 * Copyright (C) 2002-2004  Tuukka Pasanen <illuusio@mailcity.com>             *
 * Copyright (C) 2003-2004  Allen Winter <awinterz@users.sourceforge.net>      *
 *                                                                             *
 * This program is free software; you can redistribute it and/or modify        *
 * it under the terms of the GNU General Public License as published by        *
 * the Free Software Foundation; either version 2 of the License, or           *
 * (at your option) any later version.                                         *
 *                                                                             *
 * This program is distributed in the hope that it will be useful,             *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                *
 * GNU General Public License for more details.                                *
 *                                                                             *
 * You should have received a copy of the GNU General Public License           *
 * along with this program; if not, write to the Free Software                 *
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. *
 *                                                                             *
 * As a special exception, permission is given to link this program            *
 * with any edition of Qt, and distribute the resulting executable,            *
 * without including the source code for Qt in the source distribution.        *
 *                                                                             *
 ******************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include <libkcal/calformat.h>
#include <libkcal/calendarresources.h>
#include <libkcal/resourcelocal.h>

#include <qdatetime.h>
#include <qfile.h>

#include <stdlib.h>
#include <iostream>

#include "konsolekalendar.h"
#include "konsolekalendarepoch.h"

#include "konsolekalendarvariables.h"

using namespace KCal;
using namespace std;

static const char progName[] = "konsolekalendar";
static const char progDisplay[] = "KonsoleKalendar";
static const char progVersion[] = "1.3.2";
static const char progDesc[] = "A command line interface to KDE calendars";
static const char progURL[] = "pim.kde.org/components/konsolekalendar.php";


static KCmdLineOptions options[] =
{
  { "help",
    I18N_NOOP( "Print this help and exit" ), 0 },
  { "verbose",
    I18N_NOOP( "Print helpful runtime messages" ), 0 },
  { "dry-run",
    I18N_NOOP( "Print what would have been done, but do not execute" ), 0 },
  { "file <calendar-file>",
    I18N_NOOP( "Specify which calendar you want to use" ), 0 },
  { "type <event | todo | journal | all>",
    I18N_NOOP( "Specify which incidence type you want to use" ), 0 },

  { ":",
    I18N_NOOP( "Major operation modes:" ), 0 },
  { "view",
    I18N_NOOP( "  Print calendar events in specified export format" ), 0 },
  { "add",
    I18N_NOOP( "  Insert an event into the calendar" ), 0 },
  { "change",
    I18N_NOOP( "  Modify an existing calendar event" ), 0 },
  { "delete",
    I18N_NOOP( "  Remove an existing calendar event" ), 0 },
  { "create",
    I18N_NOOP( "  Create new calendar file if one does not exist" ), 0 },
  { "import <import-file>",
    I18N_NOOP( "  Import this calendar to main calendar" ), 0 },
  { "parse-string <entries>",
    I18N_NOOP( "  Parses many entries in same line [YYYY-MM-DD,HH:MM:SS,description,location]" ), 0 },
  { ":",
    I18N_NOOP( "Operation modifiers:" ), 0 },
  { "all",
    I18N_NOOP( "  View all calendar entries" ), 0 },
  { "next",
    I18N_NOOP( "  View next activity in calendar" ), 0 },
  { "show-next <days>",
    I18N_NOOP( "  From start date show next # days' activities" ), 0 },
  { "uid <uid>",
    I18N_NOOP( "  Event Unique-string identifier" ), 0 },
  { "date <start-date>",
    I18N_NOOP( "  Start from this day [YYYY-MM-DD]" ), 0 },
  { "time <start-time>",
    I18N_NOOP( "  Start from this time [HH:MM:SS]" ), 0 },
  { "end-date <end-date>",
    I18N_NOOP( "  End at this day [YYYY-MM-DD]" ), 0 },
  { "end-time <end-time>",
    I18N_NOOP( "  End at this time [HH:MM:SS]" ), 0 },
  { "epoch-start <epoch-time>",
    I18N_NOOP( " Start from this time [secs since epoch]" ), 0 },
  { "epoch-end <epoch-time>",
    I18N_NOOP( "  End at this time [secs since epoch]" ), 0 },
  { "summary <summary>",
    I18N_NOOP( "  Add summary to event (for add/change modes)" ), 0 },
  { "description <description>",
    I18N_NOOP( "Add description to event (for add/change modes)" ), 0 },
  { "location <location>",
    I18N_NOOP( "  Add location to event (for add/change modes)" ), 0 },

  { ":", I18N_NOOP( "Export options:" ), 0 },
  { "export-type <export-type>",
    I18N_NOOP( "Export file type (Default: text)" ), 0 },
  { "export-file <export-file>",
    I18N_NOOP( "Export to file (Default: stdout)" ), 0 },
  { "export-list",
    I18N_NOOP( "  Print list of export types supported and exit" ), 0 },

  { "",
    I18N_NOOP( "Examples:\n"
               "  konsolekalendar --view\n"
               "  konsolekalendar --add --date 2003-06-04 "
               "--time 10:00 --end-time 12:00 \\\n"
               "                  --summary \"Doctor Visit\" "
               "--description \"Get My Head Examined\"\n"
               "  konsolekalendar --delete --uid KOrganizer-1740326.803" ), 0 },

  { "",
    I18N_NOOP( "For more information visit the program home page at:\n"
               "  http://pim.kde.org/components/konsolekalendar.php" ), 0 },

  KCmdLineLastOption
};

int main( int argc, char *argv[] )
{
  KAboutData aboutData(
    progName,                        // internal program name
    I18N_NOOP( progDisplay ),        // displayable program name.
    progVersion,                     // version string
    I18N_NOOP( progDesc ),           // short porgram description
    KAboutData::License_GPL,         // license type
    "(c) 2002-2004, Tuukka Pasanen and Allen Winter", // copyright statement
    0,                               // any free form text
    progURL,                         // program home page address
    "bugs.kde.org"                   // bug report email address
    );

  aboutData.addAuthor(
    "Tuukka Pasanen",                // developer's name
    I18N_NOOP( "Primary Author" ),   // task or role
    "illuusio@mailcity.com",         // email address
    0                                // home page or relevant link
    );
  aboutData.addAuthor(
    "Allen Winter",                  // developer's name
    I18N_NOOP( "Author" ),           // task or role
    "awinterz@users.sourceforge.net",// email address
    0                                // home page or relevant link
    );



  // KCmdLineArgs::init() final 'true' argument indicates no commandline options
  // for QApplication/KApplication (no KDE or Qt options)
  KCmdLineArgs::init( argc, argv, &aboutData, true );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KInstance ins( progName );

// Replace the KApplication call below with the three lines above
// will make this a pure non-GUI application
//   -- thanks for the info Stephan Kulow.

//  KApplication app(
//      false, //do not allowstyles - disable the loading on plugin based styles
//      false  //GUI is not enabled - disable all GUI stuff
//      );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  // Default values for start date/time (today at 07:00)
  QDate startdate = QDate::currentDate();
  QTime starttime( 7 ,0 );

  // Default values for end date/time (today at 17:00)
  QDate enddate = QDate::currentDate();
  QTime endtime( 17, 0 );

  // Default values for switches
  bool view = true;
  bool add = false;
  bool change = false;
  bool del = false;
  bool create = false;
  bool calendarFile = false;
  bool importFile = false;

  QString option;

  KonsoleKalendarVariables variables;
  KonsoleKalendarEpoch epochs;

  variables.setFloating( false ); // by default, new events do NOT float

  if ( args->isSet( "verbose" ) ) {
    variables.setVerbose( true );
  }

  if ( args->isSet( "dry-run" ) ) {
    variables.setDryRun( true );
  }

  /*
   *  Switch on export list
   *
   */
  if ( args->isSet( "export-list" ) ) {
    cout << i18n(
      "\nKonsoleKalendar supports these export formats:\n"
      "  Text [Default]\n"
      "  Short (like Text, but more compact)\n"
      "  HTML\n"
      "  CSV (Comma-Separated Values)\n"
      ).local8Bit()
         << endl;
    return 0;
  }

  /*
   *  Switch on Incidence type
   *
   */
  variables.setIncidenceType( IncidenceTypeEvent );
  if ( args->isSet( "type" ) ) {
    option = args->getOption( "type" );
    if ( option.upper() == "EVENT" ) {
      kdDebug() << "main | incidence-type | Events (default)" << endl;
      variables.setIncidenceType( IncidenceTypeEvent );
    } else if ( option.upper() == "TODO" ) {
      kdDebug() << "main | incidence-type | Todos" << endl;;
      variables.setIncidenceType( IncidenceTypeTodo );
    } else if ( option.upper() == "JOURNAL" ) {
      kdDebug() << "main | incidence-type | Journals" << endl;
      variables.setIncidenceType( IncidenceTypeJournal );
    } else if ( option.upper() == "ALL" ) {
      kdDebug() << "main | incidence-type | All Types" << endl;
      variables.setIncidenceType( IncidenceTypeAll );
    } else {
      cout << i18n( "Invalid Incidence Type Specified: %1" ).
        arg( option ).local8Bit()
           << endl;
      return 1;
    }
  }

  /*
   *  Switch on exporting
   *
   */
  variables.setExportType( ExportTypeText );
  if ( args->isSet( "export-type" ) ) {
    option = args->getOption( "export-type" );

    if ( option.upper() == "HTML" ) {
      kdDebug() << "main | export-type | Export to HTML" << endl;
      variables.setExportType( ExportTypeHTML );
    } else if ( option.upper() == "CSV" ) {
      kdDebug() << "main | export-type | Export to CSV" << endl;
      variables.setExportType( ExportTypeCSV );
    } else if ( option.upper() == "TEXT" ) {
      kdDebug() << "main | export-type | Export to TEXT (default)" << endl;
      variables.setExportType( ExportTypeText );
    } else if ( option.upper() == "SHORT" ) {
      kdDebug() << "main | export-type | Export to TEXT-SHORT" << endl;
      variables.setExportType( ExportTypeTextShort );
    } else {
      cout << i18n( "Invalid Export Type Specified: %1" ).
        arg( option ).local8Bit()
           << endl;
      return 1;
    }
  }

  /*
   * If we like to use new Parsing system
   */

  if ( args->isSet( "parse-string" ) ) {
    option = args->getOption( "parse-string" );

    kdDebug() << "main | parse options | "
              << "Parse String: "
              << "(" << option << ")"
              << endl;

    variables.setParseString( option );
  }

  //testing purposes only!!
  //exit(0);

  /*
   *  Switch on export file name
   *
   */
  if ( args->isSet( "export-file" ) ) {
    option = args->getOption( "export-file" );

    kdDebug() << "main | parse options | "
              << "Export File: "
              << "(" << option << ")"
              << endl;

    variables.setExportFile( option );
  }

  /*
   *  Switch on View (Print Entries).  This is the default mode of operation.
   *
   */
  if ( args->isSet( "view" ) ) {
    view=true;

    kdDebug() << "main | parse options | "
              << "Mode: (Print events)"
              << endl;
  }

  /*
   *  Switch on Add (Insert Entry)
   *
   */
  if ( args->isSet( "add" ) ) {
    view=false;
    add=true;

    kdDebug() << "main | parse options | "
              << "Mode: (Add event)"
              << endl;
  }

  /*
   *  Switch on Change (Modify Entry)
   *
   */
  if ( args->isSet( "change" ) ) {
    view=false;
    change=true;

    kdDebug() << "main | parse options | "
              << "Mode: (Change event)"
              << endl;
  }

  /*
   *  Switch on Delete (Remove Entry)
   *
   */
  if ( args->isSet( "delete" ) ) {
    view=false;
    del=true;

    kdDebug() << "main | parse options | "
              << "Mode: (Delete event)"
              << endl;
  }

  /*
   *  Switch on Create
   *
   */
  if ( args->isSet( "create" ) ) {
    view=false;
    create=true;

    kdDebug() << "main | parse options | "
              << "Calendar File: (Create)"
              << endl;
  }


  /*
   *  If there is summary attached.
   *
   */
  if ( args->isSet( "summary" ) ) {
    option = args->getOption( "summary" );

    kdDebug() << "main | parse options | "
              << "Summary: "
              << "(" << option << ")"
              << endl;

    variables.setSummary( option );
  }

  /*
   *  If there is description attached.
   *
   */
  if ( args->isSet( "description" ) ) {
    option = args->getOption( "description" );

    kdDebug() << "main | parse options | "
              << "Description: "
              << "(" << option << ")"
              << endl;

    variables.setDescription( option );
  }

  /*
   *  If there is location information
   *
   */
  if ( args->isSet( "location" ) ) {
    option = args->getOption( "location" );

    kdDebug() << "main | parse options | "
              << "Location: "
              << "(" << option << ")"
              << endl;

    variables.setLocation( option );
  }

  /*
   *  Show next happening and exit
   *
   */
  if ( args->isSet( "next" ) )
  {
    kdDebug() << "main | parse options | "
              << "Show next event only"
              << endl;

    variables.setNext( true );
  }


  /*
   *  Set event unique string identifier
   *
   */
  if (args->isSet( "uid" ) ) {
    option = args->getOption( "uid" );

    kdDebug() << "main | parse options | "
              << "Event UID: "
              << "(" << option << ")"
              << endl;

    variables.setUID( option );
  }

  /*
   *  Set starting date for calendar
   *
   */
  if ( args->isSet( "date" ) ) {
    option = args->getOption( "date" );

    kdDebug() << "main | parse options | "
              << "Start date before conversion: "
              << "(" << option << ")"
              << endl;

    startdate = QDate::fromString( option,  Qt::ISODate );
    if ( ! startdate.isValid() ) {
      cout << i18n( "Invalid Start Date Specified: %1" ).
        arg( option ).local8Bit()
           << endl;
      return 1;
    }
    kdDebug() << "main | parse options | "
              << "Start date after conversion: "
              << "(" << startdate.toString() << ")"
              << endl;
  }

  /*
   *  Set starting time
   *
   */
  if ( args->isSet( "time" ) ) {
    option = args->getOption( "time" );

    kdDebug() << "main | parse options | "
              << "Start time before conversion : "
              << "(" << option << ")"
              << endl;

    if ( option.upper() != "FLOAT" ) {
      starttime = QTime::fromString( option,  Qt::ISODate );
      if ( ! starttime.isValid() ) {
        cout << i18n( "Invalid Start Time Specified: %1" ).
          arg( option ).local8Bit()
             << endl;
        return 1;
      }
      kdDebug() << "main | parse options | "
                << "Start time after conversion: "
                << "(" << starttime.toString() << ")"
                << endl;
    } else {
      variables.setFloating( true );
      kdDebug() << "main | parse options | "
                << "Floating event time specified"
                << endl;
    }
  }

  /*
   *  Set end date for calendar
   *
   */
  if ( args->isSet( "end-date" ) ) {
    QString option = args->getOption( "end-date" );

    kdDebug() << "main | parse options | "
              << "End date before conversion: "
              << "(" << option << ")"
              << endl;

    enddate = QDate::fromString( option,  Qt::ISODate );
    if ( ! enddate.isValid() ) {
      cout << i18n( "Invalid End Date Specified: %1" ).
        arg( option ).local8Bit()
           << endl;
      return 1;
    }
    kdDebug() << "main | parse options | "
              << "End date after conversion: "
              << "(" << enddate.toString() << ")"
              << endl;
  }

  /*
   *  Show next # days and exit
   *
   */
  if ( args->isSet( "show-next" ) )
  {
    bool ok;

    option = args->getOption( "show-next" );
    kdDebug() << "main | parse options | "
              << "Show " << option << " days ahead"
              << endl;
    variables.setDaysCount( option.toInt( &ok, 10 ) );

    if ( ! ok ) {
      cout << i18n( "Invalid Date Count Specified: %1" ).
        arg( option ).local8Bit()
           << endl;
      return 1;
    }

    enddate = startdate;
    enddate = enddate.addDays( variables.getDaysCount() );
    kdDebug() << "main | parse options | "
              << "End date after conversion: "
              << "(" << enddate.toString() << ")"
              << endl;
  }

  /*
   *  Set ending time
   *
   */
  if ( args->isSet( "end-time" ) ) {
    option = args->getOption( "end-time" );

    kdDebug() << "main | parse options | "
              << "End time before conversion: "
              << "(" << option << ")"
              << endl;

    if ( option.upper() != "FLOAT" ) {
      endtime = QTime::fromString( option,  Qt::ISODate );
      if ( ! endtime.isValid() ) {
        cout << i18n( "Invalid End Time Specified: %1" ).
          arg( option ).local8Bit()
             << endl;
        return 1;
      }

      kdDebug() << "main | parse options | "
                << "End time after conversion: "
                << "(" << endtime.toString() << ")"
                << endl;
    } else {
      variables.setFloating( true );
      kdDebug() << "main | parse options | "
                << "Floating event time specified"
                << endl;
    }
  }

  /*
   *  Set start date/time from epoch
   *
   */
  time_t epochstart=0;
  if ( args->isSet( "epoch-start" ) ) {
    option = args->getOption( "epoch-start" );

    kdDebug() << "main | parse options | "
              << "Epoch start: "
              << "(" << option << ")"
              << endl;

    epochstart = (time_t) option.toULong( 0, 10 );
  }

  /*
   *  Set end date/time from epoch
   *
   */
  time_t epochend=0;
  if ( args->isSet( "epoch-end" ) ) {
    option = args->getOption( "epoch-end" );

    kdDebug() << "main | parse options | "
              << "Epoch end: "
              << "(" << option << ")"
              << endl;

    epochend = (time_t) option.toULong( 0, 10 );
  }

  if ( args->isSet( "all" ) ) {
    variables.setAll( true );
  } else {
    variables.setAll( false );
  }

  if ( args->isSet( "import" ) ) {
    view = false;
    importFile = true;
    option = args->getOption( "import" );
    variables.setImportFile( option );

    kdDebug() << "main | parse options | "
              << "importing file from: "
              << "(" << option << ")"
              << endl;
  }

  KonsoleKalendar *konsolekalendar = new KonsoleKalendar( &variables );

  if ( args->isSet( "file" ) ) {
    calendarFile = true;
    option = args->getOption( "file" );
    variables.setCalendarFile( option );

    /*
     * All modes need to know if the calendar file exists
     * This must be done before we get to opening biz
     */
    bool exists = QFile::exists( variables.getCalendarFile() );

    if ( create ) {

      kdDebug() << "main | createcalendar | "
                << "check if calendar file already exists"
                << endl;

      if ( exists ) {
        cout << i18n( "Calendar %1 already exists" ).
          arg( variables.getCalendarFile() ).local8Bit()
             << endl;
        return 1;
      }
      if ( konsolekalendar->createCalendar() ) {
        cout << i18n( "Calendar %1 successfully created" ).
          arg( variables.getCalendarFile() ).local8Bit()
             << endl;
        return 0;
      } else {
        cout << i18n( "Unable to create calendar: %1" ).
          arg( variables.getCalendarFile() ).local8Bit()
             << endl;
        return 1;
      }
    }

    if ( ! exists ) {
      cout << i18n( "Calendar file not found %1" ).
        arg( option ).local8Bit()
           << endl;
      cout << i18n( "Try --create to create new calendar file" ).local8Bit()
           << endl;
      return 1;
    }
  }

  CalendarResources *calendarResource = NULL;
  CalendarLocal *localCalendar = NULL;

  /*
   * Should we use local calendar or resource?
   */
  variables.setTimeZoneId();
  if ( args->isSet( "file" ) ) {
    localCalendar = new CalendarLocal( variables.getTimeZoneId() );
    localCalendar->load( variables.getCalendarFile() );
    variables.setCalendar( localCalendar  );
  } else {
    calendarResource = new CalendarResources( variables.getTimeZoneId() );
    calendarResource->readConfig();
    calendarResource->load();
    variables.setCalendarResources( calendarResource );
  }

  /***************************************************************************
   * Glorious date/time checking and setting code                            *
   ***************************************************************************/
  QDateTime startdatetime, enddatetime;

  // Handle case with either date or end-date unspecified
  if ( ! args->isSet( "end-date" ) && args->isSet( "date" ) ) {
    enddate = startdate;
    kdDebug() << "main | datetimestamp | "
              << "setting enddate to startdate"
              << endl;
  } else if ( args->isSet( "end-date" ) && ! args->isSet( "date" ) ) {
    startdate = enddate;
    kdDebug() << "main | datetimestamp | "
              << "setting startdate to enddate"
              << endl;
  }

  // NOTE: If neither date nor end-date specified, then event will be today.

  // Case:
  //   End time (or epoch) unspecified, and start time (or epoch) IS specified.
  //   In this case, set the ending to 1 hour after starting.
  if ( ! args->isSet( "end-time" ) && ! args->isSet( "epoch-end" ) ) {
    if ( args->isSet( "time" ) ) {
      endtime = starttime.addSecs( 60*60 );  // end is 1 hour after start
      kdDebug() << "main | datetimestamp | "
                << "setting endtime 1 hour after starttime"
                << endl;
    } else if ( args->isSet( "epoch-start" ) ) {
      startdatetime = epochs.epoch2QDateTime( epochstart );
      enddatetime = startdatetime.addSecs( 60*60 );
      kdDebug() << "main | datetimestamp | "
                << "setting endtime 1 hour after epochstart"
                << endl;
    }
  }

  // Case:
  //   Time (or epoch) unspecified, and end-time (or epoch) IS specified.
  //   In this case, set the starting to 1 hour before ending.
  if ( ! args->isSet( "time" ) && ! args->isSet( "epoch-start" ) ) {
    if ( args->isSet( "end-time" ) ) {
      starttime = endtime.addSecs( -60*60 );  // start is 1 hour before end
      kdDebug() << "main | datetimestamp | "
                << "setting starttime 1 hour before endtime"
                << endl;
    } else if ( args->isSet( "epoch-end" ) ) {
      enddatetime = epochs.epoch2QDateTime( epochend );
      startdatetime = enddatetime.addSecs( -60*60 );
      kdDebug() << "main | datetimestamp | "
                << "setting starttime 1 before after epochend"
                << endl;
    }
  }

  // Case:
  //   Time (or epoch) unspecified, and end-time (or epoch) unspecified.
  if ( ! args->isSet( "time" )     && ! args->isSet( "epoch-start" ) &&
       ! args->isSet( "end-time" ) && ! args->isSet( "epoch-end" ) ) {
    // set default start date/time
    startdatetime = QDateTime::QDateTime( startdate, starttime );
    kdDebug() << "main | datetimestamp | "
              << "setting startdatetime from "
              << "default startdate (today) and starttime"
              << endl;
    // set default end date/time
    enddatetime = QDateTime::QDateTime( enddate, endtime );
    kdDebug() << "main | datetimestamp | "
              << "setting enddatetime from "
              << "default enddate (today) and endtime"
              << endl;
  }

  // Set startdatetime, enddatetime if still necessary
  if ( startdatetime.isNull() ) {
    startdatetime = QDateTime::QDateTime( startdate, starttime );
    kdDebug() << "main | datetimestamp | "
              << "setting startdatetime from startdate and starttime"
              << endl;
  }
  if ( enddatetime.isNull() ) {
    enddatetime = QDateTime::QDateTime( enddate, endtime );
    kdDebug() << "main | datetimestamp | "
              << "setting enddatetime from enddate and endtime"
              << endl;
  }

  // Float check for add mode:
  //   Events float if time AND end-time AND epoch times are UNspecified
  if ( add ) {
    if ( ! args->isSet( "time" )        && ! args->isSet( "end-time" ) &&
         ! args->isSet( "epoch-start" ) && ! args->isSet( "epoch-end" ) ) {
      variables.setFloating( true );
      kdDebug() << "main | floatingcheck | "
                << "turn-on floating event"
                << endl;
    }
  }

  // Finally! Set the start/end date times
  if ( ! change ) {
    variables.setStartDateTime( startdatetime );
    variables.setEndDateTime( enddatetime );
  } else {
    // Do NOT set start/end datetimes in change mode,
    //   unless they were specified on commandline
    if ( args->isSet( "time" )     || args->isSet( "epoch-start" ) ||
         args->isSet( "end-time" ) || args->isSet( "epoch-end" ) ) {
      variables.setStartDateTime( startdatetime );
      variables.setEndDateTime( enddatetime );
    }
  }

  // Some more debug prints
  kdDebug() << "main | datetimestamp | StartDate="
            << startdatetime.toString( Qt::TextDate )
            << endl;
  kdDebug() << "main | datetimestamp | EndDate="
            << enddatetime.toString( Qt::TextDate )
            << endl;

  /***************************************************************************
   * Sanity checks                                                           *
   ***************************************************************************/

  // Cannot combine modes
  if ( create + view + add + change + del > 1 ) {
    cout << i18n(
      "Only 1 operation mode "
      "(view, add, change, delete, create) "
      "permitted at any one time"
      ).local8Bit() << endl;
    return 1;
  }

  // Cannot have a ending before starting
  if ( startdatetime > enddatetime ) {
    cout << i18n(
      "Ending Date/Time occurs before the Starting Date/Time"
      ).local8Bit() << endl;
    return 1;
  }

  /***************************************************************************
   * And away we go with the real work...                                    *
   ***************************************************************************/

  args->clear(); // Free up some memory.

  /*
   * Set our application name for use in unique IDs and error messages,
   * and product ID for incidence PRODID property
   */
  QString prodId = "-//K Desktop Environment//NONSGML %1 %2//EN";
  CalFormat::setApplication( progDisplay,
                             prodId.arg( progDisplay).arg( progVersion ) );

  if ( importFile ) {
    if ( konsolekalendar->importCalendar() ) {
      cout << i18n( "Calendar %1 successfully imported" ).
        arg( variables.getImportFile() ).local8Bit()
           << endl;
      return 0;
    } else {
      cout << i18n( "Unable to import calendar: %1" ).
        arg( variables.getImportFile() ).local8Bit()
           << endl;
      return 1;
    }
  }

  if ( add ) {
    if ( ! konsolekalendar->isEvent( startdatetime, enddatetime,
                                     variables.getSummary() ) ) {
      kdDebug() << "main | modework | "
                << "calling addEvent()"
                << endl;
      konsolekalendar->addEvent();
    } else {
      cout << i18n(
        "Attempting to insert an event that already exists"
        ).local8Bit() << endl;
      return 1;
    }
  }

  if ( change ) {
    kdDebug() << "main | modework | "
              << "calling changeEvent()"
              << endl;
    if ( ! variables.isUID() ) {
      cout << i18n( "Missing event UID: "
                    "use --uid command line option" ).local8Bit()
           << endl;
      return 1;
    }
    if ( konsolekalendar->changeEvent() != true ) {
      cout << i18n( "No such event UID: change event failed" ).local8Bit()
           << endl;
      return 1;
    }
    kdDebug() << "main | modework | "
              << "successful changeEvent()"
              << endl;
  }

  if ( del ) {
    kdDebug() << "main | modework | "
              << "calling deleteEvent()"
              << endl;
    if ( ! variables.isUID() ) {
      cout << i18n( "Missing event UID: "
                    "use --uid command line option" ).local8Bit()
           << endl;
      return 1;
    }
    if ( konsolekalendar->deleteEvent() != true ) {
      cout << i18n( "No such event UID: delete event failed").local8Bit()
           << endl;
      return 1;
    }
    kdDebug() << "main | modework | "
              << "successful deleteEvent()"
              << endl;
  }

  if ( view ) {
    kdDebug() << "main | modework | "
              << "calling showInstance() to view events"
              << endl;
    if ( ! konsolekalendar->showInstance() ) {
      cout << i18n( "Cannot open specified export file: %1" ).
        arg( variables.getExportFile() ).local8Bit()
           << endl;
      return 1;
    }
  }

  delete konsolekalendar;

  if ( calendarFile ) {
    localCalendar->close();
    delete localCalendar;
  } else {
    calendarResource->close();
    delete calendarResource;
  }

  kdDebug() << "main | exiting"
            << endl;

  return 0;
}
