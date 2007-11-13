/*******************************************************************************
 * main.cpp                                                                    *
 *                                                                             *
 * KonsoleKalendar is a command line interface to KDE calendars                *
 * Copyright (C) 2002-2004  Tuukka Pasanen <illuusio@mailcity.com>             *
 * Copyright (C) 2003-2005  Allen Winter <winter@kde.org>                      *
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. *
 *                                                                             *
 * As a special exception, permission is given to link this program            *
 * with any edition of Qt, and distribute the resulting executable,            *
 * without including the source code for Qt in the source distribution.        *
 *                                                                             *
 ******************************************************************************/
/**
 * @file main.cpp
 * KonsoleKalendar main program.
 * @author Tuukka Pasanen
 * @author Allen Winter
 */
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
#include <qfileinfo.h>

#include <stdlib.h>
#include <iostream>

#include "stdcalendar.h"
#include "konsolekalendar.h"
#include "konsolekalendarepoch.h"

#include "konsolekalendarvariables.h"

using namespace KCal;
using namespace std;

static const char progName[] = "konsolekalendar";
static const char progDisplay[] = "KonsoleKalendar";
static const char progVersion[] = "1.3.5";
static const char progDesc[] = "A command line interface to KDE calendars";
static const char progURL[] = "pim.kde.org/components/konsolekalendar.php";


static KCmdLineOptions options[] =
{
  { "verbose",
    I18N_NOOP( "Print helpful runtime messages" ), 0 },
  { "dry-run",
    I18N_NOOP( "Print what would have been done, but do not execute" ), 0 },
  { "file <calendar-file>",
    I18N_NOOP( "Specify which calendar you want to use" ), 0 },

  { ":",
    I18N_NOOP( "Incidence types (these options can be combined):" ), 0 },
  { "event",
    I18N_NOOP( "  Operate for Events only (Default)" ), 0 },
  { "todo",
    I18N_NOOP( "  Operate for To-dos only [NOT WORKING YET]" ), 0 },
  { "journal",
    I18N_NOOP( "  Operate for Journals only [NOT WORKING YET]" ), 0 },

  { ":",
    I18N_NOOP( "Major operation modes:" ), 0 },
  { "view",
    I18N_NOOP( "  Print incidences in specified export format" ), 0 },
  { "add",
    I18N_NOOP( "  Insert an incidence into the calendar" ), 0 },
  { "change",
    I18N_NOOP( "  Modify an existing incidence" ), 0 },
  { "delete",
    I18N_NOOP( "  Remove an existing incidence" ), 0 },
  { "create",
    I18N_NOOP( "  Create new calendar file if one does not exist" ), 0 },
  { "import <import-file>",
    I18N_NOOP( "  Import this calendar to main calendar" ), 0 },
  { ":",
    I18N_NOOP( "Operation modifiers:" ), 0 },
  { "all",
    I18N_NOOP( "  View all calendar entries" ), 0 },
  { "next",
    I18N_NOOP( "  View next activity in calendar" ), 0 },
  { "show-next <days>",
    I18N_NOOP( "  From start date show next # days' activities" ), 0 },
  { "uid <uid>",
    I18N_NOOP( "  Incidence Unique-string identifier" ), 0 },
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
    I18N_NOOP( "  Add summary to incidence (for add/change modes)" ), 0 },
  { "description <description>",
    I18N_NOOP( "Add description to incidence (for add/change modes)" ), 0 },
  { "location <location>",
    I18N_NOOP( "  Add location to incidence (for add/change modes)" ), 0 },

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
    "(c) 2002-2005, Tuukka Pasanen and Allen Winter", // copyright statement
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
    "winter@kde.org",                // email address
    0                                // home page or relevant link
    );

  //TODO: KDE 4.0, add the StdCmdLineArgs option to init()
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KApplication app(
    false, //do not allowstyles - disable the loading on plugin based styles
    false  //GUI is not enabled - disable all GUI stuff
    );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  // Default values for start date/time (today at 07:00)
  QDate startdate = QDate::currentDate();
  QTime starttime( 7, 0 );

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
   * Switch on export list
   */
  if ( args->isSet( "export-list" ) ) {
    cout << endl;
    cout << i18n( "%1 supports these export formats:" ).
      arg( progDisplay ).local8Bit()
         << endl;
    cout << i18n( "  %1 [Default]" ).
      arg( "Text" ).local8Bit()
         << endl;
    cout << i18n( "  %1 (like %2, but more compact)" ).
      arg( "Short", "Text" ).local8Bit()
         << endl;
    cout << i18n( "  %1" ).
      arg( "HTML" ).local8Bit()
         << endl;
    cout << i18n( "  %1 (like %2, but in a month view)" ).
      arg( "HTMLmonth", "HTML" ).local8Bit()
         << endl;
    cout << i18n( "  %1 (Comma-Separated Values)" ).
      arg( "CSV" ).local8Bit()
         << endl;
    cout << endl;
    return 0;
  }

  /*
   * Set incidence type(s)
   */
  if ( args->isSet( "event" ) ) {
    variables.setUseEvents( true );
    kdDebug() << "main | parse options | use Events" << endl;
  }
  if ( args->isSet( "todo" ) ) {
    variables.setUseTodos( true );
    kdDebug() << "main | parse options | use To-dos" << endl;
    cout << i18n( "Sorry, To-dos are not working yet." ).local8Bit()
         << endl;
    return 1;
  }
  if ( args->isSet( "journal" ) ) {
    variables.setUseJournals( true );
    kdDebug() << "main | parse options | use Journals" << endl;
    cout << i18n( "Sorry, Journals are not working yet." ).local8Bit()
         << endl;
    return 1;
  }
  // Use Events if no incidence type is specified on the command line
  if ( !args->isSet( "event" ) &&
       !args->isSet( "todo" ) &&
       !args->isSet( "journal" ) ) {
    variables.setUseEvents( true );
    kdDebug() << "main | parse options | use Events (Default)" << endl;
  }

  /*
   * Switch on exporting
   */
  variables.setExportType( ExportTypeText );
  if ( args->isSet( "export-type" ) ) {
    option = args->getOption( "export-type" );

    if ( option.upper() == "HTML" ) {
      kdDebug() << "main | export-type | Export to HTML" << endl;
      variables.setExportType( ExportTypeHTML );
    } else if ( option.upper() == "HTMLMONTH" ) {
      kdDebug() << "main | export-type | Export to HTML by Month" << endl;
      variables.setExportType( ExportTypeMonthHTML );
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
   * Switch on export file name
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
   * Switch on View (Print Entries).  This is the default mode of operation.
   */
  if ( args->isSet( "view" ) ) {
    view = true;

    kdDebug() << "main | parse options | "
              << "Mode: (Print incidences)"
              << endl;
  }

  /*
   * Switch on Add (Insert Entry)
   */
  if ( args->isSet( "add" ) ) {
    view = false;
    add = true;

    kdDebug() << "main | parse options | "
              << "Mode: (Add incidence)"
              << endl;
  }

  /*
   * Switch on Change (Modify Entry)
   */
  if ( args->isSet( "change" ) ) {
    view = false;
    change = true;

    kdDebug() << "main | parse options | "
              << "Mode: (Change incidence)"
              << endl;
  }

  /*
   * Switch on Delete (Remove Entry)
   */
  if ( args->isSet( "delete" ) ) {
    view = false;
    del = true;

    kdDebug() << "main | parse options | "
              << "Mode: (Delete incidence)"
              << endl;
  }

  /*
   * Switch on Create
   */
  if ( args->isSet( "create" ) ) {
    view = false;
    create = true;

    kdDebug() << "main | parse options | "
              << "Calendar File: (Create)"
              << endl;
  }

  /*
   * If there is summary attached.
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
   * If there is description attached.
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
   * If there is location information
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
   * Show next happening and exit
   */
  if ( args->isSet( "next" ) ) {
    kdDebug() << "main | parse options | "
              << "Show next incidence only"
              << endl;

    variables.setNext( true );
  }

  /*
   * Set incidence unique string identifier
   */
  if ( args->isSet( "uid" ) ) {
    option = args->getOption( "uid" );

    kdDebug() << "main | parse options | "
              << "incidence UID: "
              << "(" << option << ")"
              << endl;

    variables.setUID( option );
  }

  /*
   * Set starting date for calendar
   */
  if ( args->isSet( "date" ) ) {
    option = args->getOption( "date" );

    kdDebug() << "main | parse options | "
              << "Start date before conversion: "
              << "(" << option << ")"
              << endl;

    startdate = QDate::fromString( option, Qt::ISODate );
    if ( !startdate.isValid() ) {
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
   * Set starting time
   */
  if ( args->isSet( "time" ) ) {
    option = args->getOption( "time" );

    kdDebug() << "main | parse options | "
              << "Start time before conversion : "
              << "(" << option << ")"
              << endl;

    if ( option.upper() != "FLOAT" ) {
      starttime = QTime::fromString( option, Qt::ISODate );
      if ( !starttime.isValid() ) {
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
   * Set end date for calendar
   */
  if ( args->isSet( "end-date" ) ) {
    option = args->getOption( "end-date" );

    kdDebug() << "main | parse options | "
              << "End date before conversion: "
              << "(" << option << ")"
              << endl;

    enddate = QDate::fromString( option, Qt::ISODate );
    if ( !enddate.isValid() ) {
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
   * Show next # days and exit
   */
  if ( args->isSet( "show-next" ) ) {
    bool ok;

    option = args->getOption( "show-next" );
    kdDebug() << "main | parse options | "
              << "Show " << option << " days ahead"
              << endl;
    variables.setDaysCount( option.toInt( &ok, 10 ) );

    if ( !ok ) {
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
   * Set ending time
   */
  if ( args->isSet( "end-time" ) ) {
    option = args->getOption( "end-time" );

    kdDebug() << "main | parse options | "
              << "End time before conversion: "
              << "(" << option << ")"
              << endl;

    if ( option.upper() != "FLOAT" ) {
      endtime = QTime::fromString( option, Qt::ISODate );
      if ( !endtime.isValid() ) {
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
   * Set start date/time from epoch
   */
  time_t epochstart = 0;
  if ( args->isSet( "epoch-start" ) ) {
    option = args->getOption( "epoch-start" );

    kdDebug() << "main | parse options | "
              << "Epoch start: "
              << "(" << option << ")"
              << endl;

    epochstart = ( time_t ) option.toULong( 0, 10 );
  }

  /*
   * Set end date/time from epoch
   */
  time_t epochend = 0;
  if ( args->isSet( "epoch-end" ) ) {
    option = args->getOption( "epoch-end" );

    kdDebug() << "main | parse options | "
              << "Epoch end: "
              << "(" << option << ")"
              << endl;

    epochend = ( time_t ) option.toULong( 0, 10 );
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
    bool exists, remote;
    KURL url = KURL::fromPathOrURL( variables.getCalendarFile() );
    if ( url.isLocalFile() ) {
      variables.setCalendarFile( url.path() );
      exists = QFile::exists( variables.getCalendarFile() );
      remote = false;
    } else if ( url.protocol().isEmpty() ) {
      QFileInfo info( variables.getCalendarFile() );
      variables.setCalendarFile( info.absFilePath() );
      exists = QFile::exists( variables.getCalendarFile() );
      remote = false;
    } else {
      exists = true; // really have no idea if the remote file exists
      remote = true;
    }

    if ( create ) {

      kdDebug() << "main | createcalendar | "
                << "check if calendar file already exists"
                << endl;

      if ( remote ) {
        cout << i18n( "Attempting to create a remote file %1" ).
          arg( variables.getCalendarFile() ).local8Bit() << endl;
        return 1;
      } else {
        if ( exists ) {
          cout << i18n( "Calendar %1 already exists" ).
            arg( variables.getCalendarFile() ).local8Bit()
               << endl;
          return 1;
        }
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

    if ( !exists ) {
      cout << i18n( "Calendar file not found %1" ).
        arg( variables.getCalendarFile() ).local8Bit()
           << endl;
      cout << i18n( "Try --create to create new calendar file" ).local8Bit()
           << endl;
      return 1;
    }
  }

  CalendarResources *calendarResource = NULL;
  /*
   * Should we use local calendar or resource?
   */
  if ( args->isSet( "file" ) ) {
    calendarResource = new StdCalendar( variables.getCalendarFile(),
                                        i18n( "Active Calendar" ) );
  } else {
    // TODO: when certain resources (kolab) don't try to gain access to
    // an X server, or dcopserver, then put back the following line which
    // supports all resources, not just the standard resource.
    // calendarResource = new StdCalendar();
    calendarResource = new StdCalendar( locateLocal( "data",
                                                     "korganizer/std.ics" ),
                                        i18n( "Default Calendar" ) );
  }
  if ( !args->isSet( "import" ) ) {
    variables.setCalendar( calendarResource );
    calendarResource->load();
  }

  /***************************************************************************
   * Glorious date/time checking and setting code                            *
   ***************************************************************************/
  QDateTime startdatetime, enddatetime;

  // Handle case with either date or end-date unspecified
  if ( !args->isSet( "end-date" ) && !args->isSet( "show-next" ) &&
       args->isSet( "date" ) ) {
    enddate = startdate;
    kdDebug() << "main | datetimestamp | "
              << "setting enddate to startdate"
              << endl;
  } else if ( args->isSet( "end-date" ) && !args->isSet( "date" ) ) {
    startdate = enddate;
    kdDebug() << "main | datetimestamp | "
              << "setting startdate to enddate"
              << endl;
  }

  // NOTE: If neither date nor end-date specified, then event will be today.

  // Case:
  //   End time (or epoch) unspecified, and start time (or epoch) IS specified.
  //   In this case, set the ending to 1 hour after starting.
  if ( !args->isSet( "end-time" ) && !args->isSet( "epoch-end" ) ) {
    if ( args->isSet( "time" ) ) {
      endtime = starttime.addSecs( 60 * 60 );  // end is 1 hour after start
      kdDebug() << "main | datetimestamp | "
                << "setting endtime 1 hour after starttime"
                << endl;
    } else if ( args->isSet( "epoch-start" ) ) {
      startdatetime = epochs.epoch2QDateTime( epochstart );
      enddatetime = startdatetime.addSecs( 60 * 60 );
      kdDebug() << "main | datetimestamp | "
                << "setting endtime 1 hour after epochstart"
                << endl;
    }
  }

  // Case:
  //   Time (or epoch) unspecified, and end-time (or epoch) IS specified.
  //   In this case, set the starting to 1 hour before ending.
  if ( !args->isSet( "time" ) && !args->isSet( "epoch-start" ) ) {
    if ( args->isSet( "end-time" ) ) {
      starttime = endtime.addSecs( -60 * 60 );  // start is 1 hour before end
      kdDebug() << "main | datetimestamp | "
                << "setting starttime 1 hour before endtime"
                << endl;
    } else if ( args->isSet( "epoch-end" ) ) {
      enddatetime = epochs.epoch2QDateTime( epochend );
      startdatetime = enddatetime.addSecs( -60 * 60 );
      kdDebug() << "main | datetimestamp | "
                << "setting starttime 1 before after epochend"
                << endl;
    }
  }

  // Case:
  //   Time (or epoch) unspecified, and end-time (or epoch) unspecified.
  if ( !args->isSet( "time" )     && !args->isSet( "epoch-start" ) &&
       !args->isSet( "end-time" ) && !args->isSet( "epoch-end" ) ) {
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
    if ( !args->isSet( "time" )        && !args->isSet( "end-time" ) &&
         !args->isSet( "epoch-start" ) && !args->isSet( "epoch-end" ) ) {
      variables.setFloating( true );
      kdDebug() << "main | floatingcheck | "
                << "turn-on floating event"
                << endl;
    }
  }

  // Finally! Set the start/end date times
  if ( !change ) {
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
                             prodId.arg( progDisplay ).arg( progVersion ) );

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
    if ( !konsolekalendar->isEvent( startdatetime, enddatetime,
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
    if ( !variables.isUID() ) {
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
    if ( !variables.isUID() ) {
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
    if ( !konsolekalendar->showInstance() ) {
      cout << i18n( "Cannot open specified export file: %1" ).
        arg( variables.getExportFile() ).local8Bit()
           << endl;
      return 1;
    }
  }

  delete konsolekalendar;

  calendarResource->close();
  delete calendarResource;

  kdDebug() << "main | exiting"
            << endl;

  return 0;
}
