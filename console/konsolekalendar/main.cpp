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
#include <time.h>

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include <kcal/calformat.h>
#include <kcal/calendarresources.h>
#include <kcal/resourcelocal.h>

#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include <stdlib.h>
#include <iostream>

#include "stdcalendar.h"
#include "konsolekalendar.h"
#include "konsolekalendarepoch.h"

#include "konsolekalendarvariables.h"

using namespace KCal;
using namespace std;

//@cond IGNORE
static const char progName[] = "konsolekalendar";
static const char progDisplay[] = "KonsoleKalendar";
static const char progVersion[] = "1.3.5";
static const char progDesc[] = "A command line interface to KDE calendars";
static const char progURL[] = "pim.kde.org/components/konsolekalendar.php";

int main( int argc, char *argv[] )
{
  KAboutData aboutData(
    progName, 0,                 // internal program name
    ki18n( progDisplay ),        // displayable program name.
    progVersion,                 // version string
    ki18n( progDesc ),           // short porgram description
    KAboutData::License_GPL,     // license type
    ki18n( "(c) 2002-2007, Tuukka Pasanen and Allen Winter" ),
    ki18n( 0 ),                  // any free form text
    progURL,                     // program home page address
    "bugs.kde.org"               // bug report email address
    );

  aboutData.addAuthor(
    ki18n( "Allen Winter" ),     // developer's name
    ki18n( "Maintainer" ),       // task or role
    "winter@kde.org",            // email address
    0                            // home page or relevant link
    );
  aboutData.addAuthor(
    ki18n( "Tuukka Pasanen" ),   // developer's name
    ki18n( "Author" ),           // task or role
    "illuusio@mailcity.com",     // email address
    0                            // home page or relevant link
    );

  KCmdLineArgs::init( argc, argv, &aboutData, KCmdLineArgs::CmdLineArgNone );

  KCmdLineOptions options;
  options.add( "verbose",
               ki18n( "Print helpful runtime messages" ) );
  options.add( "dry-run",
               ki18n( "Print what would have been done, but do not execute" ) );
  options.add( "file <calendar-file>",
               ki18n( "Specify which calendar you want to use" ) );
  options.add( ":",
               ki18n( "Incidence types (these options can be combined):" ) );
  options.add( "event",
               ki18n( "  Operate for Events only (Default)" ) );
  options.add( "todo",
               ki18n( "  Operate for To-dos only [NOT WORKING YET]" ) );
  options.add( "journal",
               ki18n( "  Operate for Journals only [NOT WORKING YET]" ) );
  options.add( ":",
               ki18n( "Major operation modes:" ) );
  options.add( "view",
               ki18n( "  Print incidences in specified export format" ) );
  options.add( "add",
               ki18n( "  Insert an incidence into the calendar" ) );
  options.add( "change",
               ki18n( "  Modify an existing incidence" ) );
  options.add( "delete",
               ki18n( "  Remove an existing incidence" ) );
  options.add( "create",
               ki18n( "  Create new calendar file if one does not exist" ) );
  options.add( "import <import-file>",
               ki18n( "  Import this calendar to main calendar" ) );
  options.add( ":",
               ki18n( "Operation modifiers:" ) );
  options.add( "all",
               ki18n( "  View all calendar entries, ignoring date/time options" ) );
  options.add( "next",
               ki18n( "  View next activity in calendar" ) );
  options.add( "show-next <days>",
               ki18n( "  From start date show next # days' activities" ) );
  options.add( "uid <uid>",
               ki18n( "  Incidence Unique-string identifier" ) );
  options.add( "date <start-date>",
               ki18n( "  Start from this day [YYYY-MM-DD]" ) );
  options.add( "time <start-time>",
               ki18n( "  Start from this time [HH:MM:SS]" ) );
  options.add( "end-date <end-date>",
               ki18n( "  End at this day [YYYY-MM-DD]" ) );
  options.add( "end-time <end-time>",
               ki18n( "  End at this time [HH:MM:SS]" ) );
  options.add( "epoch-start <epoch-time>",
               ki18n( " Start from this time [secs since epoch]" ) );
  options.add( "epoch-end <epoch-time>",
               ki18n( "  End at this time [secs since epoch]" ) );
  options.add( "summary <summary>",
               ki18n( "  Add summary to incidence (for add/change modes)" ) );
  options.add( "description <description>",
               ki18n( "Add description to incidence (for add/change modes)" ) );
  options.add( "location <location>",
               ki18n( "  Add location to incidence (for add/change modes)" ) );
  options.add( ":",
               ki18n( "Export options:" ) );
  options.add( "export-type <export-type>",
               ki18n( "Export file type (Default: text)" ) );
  options.add( "export-file <export-file>",
               ki18n( "Export to file (Default: stdout)" ) );
  options.add( "export-list",
               ki18n( "  Print list of export types supported and exit" ) );
  options.add( "",
               ki18n( "Examples:\n"
                      "  konsolekalendar --view\n"
                      "  konsolekalendar --add --date 2003-06-04 "
                      "--time 10:00 --end-time 12:00 \\\n"
                      "                  --summary \"Doctor Visit\" "
                      "--description \"Get My Head Examined\"\n"
                      "  konsolekalendar --delete --uid KOrganizer-1740326.803" ) );
  options.add( "",
               ki18n( "For more information visit the program home page at:\n"
                      "  http://pim.kde.org/components/konsolekalendar.php" ) );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KApplication app(
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
    cout << i18n( "%1 supports these export formats:",
                  QString(progDisplay) ).toLocal8Bit().data()
         << endl;
    cout << i18n( "  %1 [Default]", QString( "Text" ) ).toLocal8Bit().data()
         << endl;
    cout << i18n( "  %1 (like %2, but more compact)",
                  QString( "Short" ), QString( "Text" ) ).toLocal8Bit().data()
         << endl;
    cout << i18n( "  %1", QString( "HTML" ) ).toLocal8Bit().data()
         << endl;
    cout << i18n( "  %1 (like %2, but in a month view)",
                  QString( "HTMLmonth" ), QString( "HTML" ) ).toLocal8Bit().data()
         << endl;
    cout << i18n( "  %1 (Comma-Separated Values)",
                  QString( "CSV" ) ).toLocal8Bit().data()
         << endl;
    cout << endl;
    return 0;
  }

  /*
   * Set incidence type(s)
   */
  if ( args->isSet( "event" ) ) {
    variables.setUseEvents( true );
    kDebug(5860) << "main | parse options | use Events";
  }
  if ( args->isSet( "todo" ) ) {
    variables.setUseTodos( true );
    kDebug(5860) << "main | parse options | use To-dos";
    cout << i18n( "Sorry, To-dos are not working yet." ).toLocal8Bit().data()
         << endl;
    return 1;
  }
  if ( args->isSet( "journal" ) ) {
    variables.setUseJournals( true );
    kDebug(5860) << "main | parse options | use Journals";
    cout << i18n( "Sorry, Journals are not working yet." ).toLocal8Bit().data()
         << endl;
    return 1;
  }
  // Use Events if no incidence type is specified on the command line
  if ( !args->isSet( "event" ) &&
       !args->isSet( "todo" ) &&
       !args->isSet( "journal" ) ) {
    variables.setUseEvents( true );
    kDebug(5860) << "main | parse options | use Events (Default)";
  }

  /*
   * Switch on exporting
   */
  variables.setExportType( ExportTypeText );
  if ( args->isSet( "export-type" ) ) {
    option = args->getOption( "export-type" );

    if ( option.toUpper() == "HTML" ) {
      kDebug(5860) << "main | export-type | Export to HTML";
      variables.setExportType( ExportTypeHTML );
    } else if ( option.toUpper() == "HTMLMONTH" ) {
      kDebug(5860) << "main | export-type | Export to HTML by Month";
      variables.setExportType( ExportTypeMonthHTML );
    } else if ( option.toUpper() == "CSV" ) {
      kDebug(5860) << "main | export-type | Export to CSV";
      variables.setExportType( ExportTypeCSV );
    } else if ( option.toUpper() == "TEXT" ) {
      kDebug(5860) << "main | export-type | Export to TEXT (default)";
      variables.setExportType( ExportTypeText );
    } else if ( option.toUpper() == "SHORT" ) {
      kDebug(5860) << "main | export-type | Export to TEXT-SHORT";
      variables.setExportType( ExportTypeTextShort );
    } else {
      cout << i18n( "Invalid Export Type Specified: %1",
         option ).toLocal8Bit().data()
           << endl;
      return 1;
    }
  }

  /*
   * Switch on export file name
   */
  if ( args->isSet( "export-file" ) ) {
    option = args->getOption( "export-file" );

    kDebug(5860) << "main | parse options |"
                 << "Export File:"
                 << "(" << option << ")";

    variables.setExportFile( option );
  }

  /*
   * Switch on View (Print Entries).  This is the default mode of operation.
   */
  if ( args->isSet( "view" ) ) {
    view = true;

    kDebug(5860) << "main | parse options |"
                 << "Mode: (Print incidences)";
  }

  /*
   * Switch on Add (Insert Entry)
   */
  if ( args->isSet( "add" ) ) {
    view = false;
    add = true;

    kDebug(5860) << "main | parse options |"
                 << "Mode: (Add incidence)";
  }

  /*
   * Switch on Change (Modify Entry)
   */
  if ( args->isSet( "change" ) ) {
    view = false;
    change = true;

    kDebug(5860) << "main | parse options |"
                 << "Mode: (Change incidence)";
  }

  /*
   * Switch on Delete (Remove Entry)
   */
  if ( args->isSet( "delete" ) ) {
    view = false;
    del = true;

    kDebug(5860) << "main | parse options |"
                 << "Mode: (Delete incidence)";
  }

  /*
   * Switch on Create
   */
  if ( args->isSet( "create" ) ) {
    view = false;
    create = true;

    kDebug(5860) << "main | parse options |"
                 << "Calendar File: (Create)";
  }

  /*
   * If there is summary attached.
   */
  if ( args->isSet( "summary" ) ) {
    option = args->getOption( "summary" );

    kDebug(5860) << "main | parse options |"
                 << "Summary:"
                 << "(" << option << ")";

    variables.setSummary( option );
  }

  /*
   * If there is description attached.
   */
  if ( args->isSet( "description" ) ) {
    option = args->getOption( "description" );

    kDebug(5860) << "main | parse options |"
                 << "Description:"
                 << "(" << option << ")";

    variables.setDescription( option );
  }

  /*
   * If there is location information
   */
  if ( args->isSet( "location" ) ) {
    option = args->getOption( "location" );

    kDebug(5860) << "main | parse options |"
                 << "Location:"
                 << "(" << option << ")";

    variables.setLocation( option );
  }

  /*
   * Show next happening and exit
   */
  if ( args->isSet( "next" ) ) {
    kDebug(5860) << "main | parse options |"
                 << "Show next incidence only";

    variables.setNext( true );
  }

  /*
   * Set incidence unique string identifier
   */
  if ( args->isSet( "uid" ) ) {
    option = args->getOption( "uid" );

    kDebug(5860) << "main | parse options |"
                 << "incidence UID:"
                 << "(" << option << ")";

    variables.setUID( option );
  }

  /*
   * Set starting date for calendar
   */
  if ( args->isSet( "date" ) ) {
    option = args->getOption( "date" );

    kDebug(5860) << "main | parse options |"
                 << "Start date before conversion:"
                 << "(" << option << ")";

    startdate = QDate::fromString( option, Qt::ISODate );
    if ( !startdate.isValid() ) {
      cout << i18n( "Invalid Start Date Specified: %1",
         option ).toLocal8Bit().data()
           << endl;
      return 1;
    }
    kDebug(5860) << "main | parse options |"
                 << "Start date after conversion:"
                 << "(" << startdate.toString() << ")";
  }

  /*
   * Set starting time
   */
  if ( args->isSet( "time" ) ) {
    option = args->getOption( "time" );

    kDebug(5860) << "main | parse options |"
                 << "Start time before conversion :"
                 << "(" << option << ")";

    if ( option.toUpper() != "FLOAT" ) {
      starttime = QTime::fromString( option, Qt::ISODate );
      if ( !starttime.isValid() ) {
        cout << i18n( "Invalid Start Time Specified: %1", option ).toLocal8Bit().data()
             << endl;
        return 1;
      }
      kDebug(5860) << "main | parse options |"
                   << "Start time after conversion:"
                   << "(" << starttime.toString() << ")";
    } else {
      variables.setFloating( true );
      kDebug(5860) << "main | parse options |"
                   << "Floating event time specified";
    }
  }

  /*
   * Set end date for calendar
   */
  if ( args->isSet( "end-date" ) ) {
    option = args->getOption( "end-date" );

    kDebug(5860) << "main | parse options |"
                 << "End date before conversion:"
                 << "(" << option << ")";

    enddate = QDate::fromString( option, Qt::ISODate );
    if ( !enddate.isValid() ) {
      cout << i18n( "Invalid End Date Specified: %1",
         option ).toLocal8Bit().data()
           << endl;
      return 1;
    }
    kDebug(5860) << "main | parse options |"
                 << "End date after conversion:"
                 << "(" << enddate.toString() << ")";
  }

  /*
   * Show next # days and exit
   */
  if ( args->isSet( "show-next" ) ) {
    bool ok;

    option = args->getOption( "show-next" );
    kDebug(5860) << "main | parse options |"
                 << "Show" << option << "days ahead";
    variables.setDaysCount( option.toInt( &ok, 10 ) );

    if ( !ok ) {
      cout << i18n( "Invalid Date Count Specified: %1", option ).toLocal8Bit().data()
           << endl;
      return 1;
    }

    enddate = startdate;
    enddate = enddate.addDays( variables.getDaysCount() );
    kDebug(5860) << "main | parse options |"
                 << "End date after conversion:"
                 << "(" << enddate.toString() << ")";
  }

  /*
   * Set ending time
   */
  if ( args->isSet( "end-time" ) ) {
    option = args->getOption( "end-time" );

    kDebug(5860) << "main | parse options |"
                 << "End time before conversion:"
                 << "(" << option << ")";

    if ( option.toUpper() != "FLOAT" ) {
      endtime = QTime::fromString( option, Qt::ISODate );
      if ( !endtime.isValid() ) {
        cout << i18n( "Invalid End Time Specified: %1", option ).toLocal8Bit().data()
             << endl;
        return 1;
      }

      kDebug(5860) << "main | parse options |"
                   << "End time after conversion:"
                   << "(" << endtime.toString() << ")";
    } else {
      variables.setFloating( true );
      kDebug(5860) << "main | parse options |"
                   << "Floating event time specified";
    }
  }

  /*
   * Set start date/time from epoch
   */
  time_t epochstart = 0;
  if ( args->isSet( "epoch-start" ) ) {
    option = args->getOption( "epoch-start" );

    kDebug(5860) << "main | parse options |"
                 << "Epoch start:"
                 << "(" << option << ")";

    epochstart = ( time_t ) option.toULong( 0, 10 );
  }

  /*
   * Set end date/time from epoch
   */
  time_t epochend = 0;
  if ( args->isSet( "epoch-end" ) ) {
    option = args->getOption( "epoch-end" );

    kDebug(5860) << "main | parse options |"
                 << "Epoch end:"
                 << "(" << option << ")";

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

    kDebug(5860) << "main | parse options |"
                 << "importing file from:"
                 << "(" << option << ")";
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
    KUrl url( variables.getCalendarFile() );
    if ( url.isLocalFile() ) {
      variables.setCalendarFile( url.path() );
      exists = QFile::exists( variables.getCalendarFile() );
      remote = false;
    } else if ( !variables.getCalendarFile().contains( '/' ) ) {
      QFileInfo info( variables.getCalendarFile() );
      variables.setCalendarFile( info.absoluteFilePath() );
      exists = QFile::exists( variables.getCalendarFile() );
      remote = false;
    } else {
      exists = true; // really have no idea if the remote file exists
      remote = true;
    }

    if ( create ) {

      kDebug(5860) << "main | createcalendar |"
                   << "check if calendar file already exists";

      if ( remote ) {
        cout << i18n( "Attempting to create a remote file %1",
                      variables.getCalendarFile() ).toLocal8Bit().data()
             << endl;
        return 1;
      } else {
        if ( exists ) {
          cout << i18n( "Calendar %1 already exists",
                        variables.getCalendarFile() ).toLocal8Bit().data()
               << endl;
          return 1;
        }
      }

      if ( konsolekalendar->createCalendar() ) {
        cout << i18n( "Calendar %1 successfully created",
                      variables.getCalendarFile() ).toLocal8Bit().data()
             << endl;
        return 0;
      } else {
        cout << i18n( "Unable to create calendar: %1",
                      variables.getCalendarFile() ).toLocal8Bit().data()
             << endl;
        return 1;
      }
    }

    if ( !exists ) {
      cout << i18n( "Calendar file not found %1",
                    variables.getCalendarFile() ).toLocal8Bit().data()
           << endl;
      cout << i18n( "Try --create to create new calendar file" ).toLocal8Bit().data()
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
    calendarResource = new StdCalendar(
      KStandardDirs::locateLocal( "data", "korganizer/std.ics" ), i18n( "Default Calendar" ) );
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
    kDebug(5860) << "main | datetimestamp |"
                 << "setting enddate to startdate";
  } else if ( args->isSet( "end-date" ) && !args->isSet( "date" ) ) {
    startdate = enddate;
    kDebug(5860) << "main | datetimestamp |"
                 << "setting startdate to enddate";
  }

  // NOTE: If neither date nor end-date specified, then event will be today.

  // Case:
  //   End time (or epoch) unspecified, and start time (or epoch) IS specified.
  //   In this case, set the ending to 1 hour after starting.
  if ( !args->isSet( "end-time" ) && !args->isSet( "epoch-end" ) ) {
    if ( args->isSet( "time" ) ) {
      endtime = starttime.addSecs( 60 * 60 );  // end is 1 hour after start
      kDebug(5860) << "main | datetimestamp |"
                   << "setting endtime 1 hour after starttime";
    } else if ( args->isSet( "epoch-start" ) ) {
      startdatetime = epochs.epoch2QDateTime( epochstart );
      enddatetime = startdatetime.addSecs( 60 * 60 );
      kDebug(5860) << "main | datetimestamp |"
                   << "setting endtime 1 hour after epochstart";
    }
  }

  // Case:
  //   Time (or epoch) unspecified, and end-time (or epoch) IS specified.
  //   In this case, set the starting to 1 hour before ending.
  if ( !args->isSet( "time" ) && !args->isSet( "epoch-start" ) ) {
    if ( args->isSet( "end-time" ) ) {
      starttime = endtime.addSecs( -60 * 60 );  // start is 1 hour before end
      kDebug(5860) << "main | datetimestamp |"
                   << "setting starttime 1 hour before endtime";
    } else if ( args->isSet( "epoch-end" ) ) {
      enddatetime = epochs.epoch2QDateTime( epochend );
      startdatetime = enddatetime.addSecs( -60 * 60 );
      kDebug(5860) << "main | datetimestamp |"
                   << "setting starttime 1 before after epochend";
    }
  }

  // Case:
  //   Time (or epoch) unspecified, and end-time (or epoch) unspecified.
  if ( !args->isSet( "time" )     && !args->isSet( "epoch-start" ) &&
       !args->isSet( "end-time" ) && !args->isSet( "epoch-end" ) ) {
    // set default start date/time
    startdatetime = QDateTime::QDateTime( startdate, starttime );
    kDebug(5860) << "main | datetimestamp |"
                 << "setting startdatetime from"
                 << "default startdate (today) and starttime";
    // set default end date/time
    enddatetime = QDateTime::QDateTime( enddate, endtime );
    kDebug(5860) << "main | datetimestamp |"
                 << "setting enddatetime from"
                 << "default enddate (today) and endtime";
  }

  // Set startdatetime, enddatetime if still necessary
  if ( startdatetime.isNull() ) {
    startdatetime = QDateTime::QDateTime( startdate, starttime );
    kDebug(5860) << "main | datetimestamp |"
                 << "setting startdatetime from startdate and starttime";
  }
  if ( enddatetime.isNull() ) {
    enddatetime = QDateTime::QDateTime( enddate, endtime );
    kDebug(5860) << "main | datetimestamp |"
                 << "setting enddatetime from enddate and endtime";
  }

  // Float check for add mode:
  //   Events float if time AND end-time AND epoch times are UNspecified
  if ( add ) {
    if ( !args->isSet( "time" )        && !args->isSet( "end-time" ) &&
         !args->isSet( "epoch-start" ) && !args->isSet( "epoch-end" ) ) {
      variables.setFloating( true );
      kDebug(5860) << "main | floatingcheck |"
                   << "turn-on floating event";
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
  kDebug(5860) << "main | datetimestamp | StartDate="
               << startdatetime.toString( Qt::TextDate );
  kDebug(5860) << "main | datetimestamp | EndDate="
               << enddatetime.toString( Qt::TextDate );

  /***************************************************************************
   * Sanity checks                                                           *
   ***************************************************************************/

  // Cannot combine modes
  if ( create + view + add + change + del > 1 ) {
    cout << i18n( "Only 1 operation mode "
                  "(view, add, change, delete, create) "
                  "permitted at any one time" ).toLocal8Bit().data() << endl;
    return 1;
  }

  // Cannot have a ending before starting
  if ( startdatetime > enddatetime ) {
    cout << i18n( "Ending Date/Time occurs before the Starting Date/Time" ).toLocal8Bit().data()
         << endl;
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
      cout << i18n( "Calendar %1 successfully imported",
                    variables.getImportFile() ).toLocal8Bit().data()
           << endl;
      return 0;
    } else {
      cout << i18n( "Unable to import calendar: %1",
                    variables.getImportFile() ).toLocal8Bit().data()
           << endl;
      return 1;
    }
  }

  if ( add ) {
    if ( !konsolekalendar->isEvent( startdatetime, enddatetime,
                                    variables.getSummary() ) ) {
      kDebug(5860) << "main | modework |"
                   << "calling addEvent()";
      konsolekalendar->addEvent();
    } else {
      cout << i18n( "Attempting to insert an event that already exists" ).toLocal8Bit().data()
           << endl;
      return 1;
    }
  }

  if ( change ) {
    kDebug(5860) << "main | modework |"
                 << "calling changeEvent()";
    if ( !variables.isUID() ) {
      cout << i18n( "Missing event UID: "
                    "use --uid command line option" ).toLocal8Bit().data()
           << endl;
      return 1;
    }
    if ( konsolekalendar->changeEvent() != true ) {
      cout << i18n( "No such event UID: change event failed" ).toLocal8Bit().data()
           << endl;
      return 1;
    }
    kDebug(5860) << "main | modework |"
                 << "successful changeEvent()";
  }

  if ( del ) {
    kDebug(5860) << "main | modework |"
                 << "calling deleteEvent()";
    if ( !variables.isUID() ) {
      cout << i18n( "Missing event UID: "
                    "use --uid command line option" ).toLocal8Bit().data()
           << endl;
      return 1;
    }
    if ( konsolekalendar->deleteEvent() != true ) {
      cout << i18n( "No such event UID: delete event failed" ).toLocal8Bit().data()
           << endl;
      return 1;
    }
    kDebug(5860) << "main | modework |"
                 << "successful deleteEvent()";
  }

  if ( view ) {
    kDebug(5860) << "main | modework |"
                 << "calling showInstance() to view events";
    if ( !konsolekalendar->showInstance() ) {
      cout << i18n( "Cannot open specified export file: %1",
                    variables.getExportFile() ).toLocal8Bit().data()
           << endl;
      return 1;
    }
  }

  delete konsolekalendar;

  calendarResource->close();
  delete calendarResource;

  kDebug(5860) << "main | exiting";

  return 0;
}
//@endcond
