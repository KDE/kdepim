/***************************************************************************
			  main.cpp  -  description
			     -------------------
    begin                : Sun Jan  6 11:50:14 EET 2002
    copyright            : (C) 2002-2003 by Tuukka Pasanen
    copyright            : (C) 2003 by Allen Winter
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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
  { "dry-run", I18N_NOOP("Print what would have been done, but do not execute"), 0 },
  { "file <calendar-file>", I18N_NOOP("Specify which calendar you want to use"), 0 },

  { ":", I18N_NOOP(" Major operation modes:"), 0 },
  { "view", I18N_NOOP("  Print calendar events in specified export format"), 0 },
  { "add", I18N_NOOP("  Insert an event into the calendar"), 0 },
  { "change", I18N_NOOP("  Modify an existing calendar event"), 0 },
  { "delete", I18N_NOOP("  Remove an existing calendar event"), 0 },
  { "import <import-file>", I18N_NOOP("  Import this calendar to main calendar"), 0 },
  { "create", I18N_NOOP("  Create new calendar file if one does not exist"), 0 },

  { ":", I18N_NOOP(" Operation modifiers:"), 0 },
  { "next", I18N_NOOP("  Next activity in calendar"), 0 },
  { "all", I18N_NOOP("  Show all calendar entries"), 0 },
  { "date <start-date>", I18N_NOOP("  Start from this day [YYYY-MM-DD]"), 0 },
  { "time <start-time>", I18N_NOOP("  Start from this time [HH:MM:SS]"), 0 },
  { "end-date <end-date>", I18N_NOOP("  End at this day [YYYY-MM-DD]"), 0 },
  { "end-time <end-time>", I18N_NOOP("  End at this time [HH:MM:SS]"), 0 },
  { "epoch-start <epoch-time>", I18N_NOOP(" Start from this time [secs since epoch]"), 0 },
  { "epoch-end <epoch-time>", I18N_NOOP("  End at this time [secs since epoch]"), 0 },
  { "description <description>", I18N_NOOP("Add description to event (for add/change modes)"), 0 },
  { "summary <summary>", I18N_NOOP("  Add summary to event (for add/change modes)"), 0 },

  { ":", I18N_NOOP(" Export options:"), 0 },
  { "export-type <export-type>", I18N_NOOP("Export file type (Default: text)"), 0 },
  { "export-file <export-file>", I18N_NOOP("Export to file (Default: stdout)"), 0 },
  { "export-list", I18N_NOOP("  Print list of export types supported and exit"), 0 },

  { "", I18N_NOOP("Examples:\n  konsolekalendar --view --all\n  konsolekalendar --add --date 2003-06-04 --time 10:00 --end-time 12:00 \\\n                  --summary \"Doctor Visit\" --description \"Get My Head Examined\""), 0 },

  KCmdLineLastOption
};

int main(int argc, char *argv[])
{

  KAboutData aboutData(
      "konsolekalendar",               // internal program name
      I18N_NOOP( "KonsoleKalendar" ),  // displayable program name.
      "0.9.7",                           // version string
      description,                     // short porgram description
      KAboutData::License_GPL,         // license type
      "(c) 2002-2003, Tuukka Pasanen and Allen Winter", // copyright statement
      0,                               // any free form text
      "http://pim.kde.org",            // program home page address
      "bugs.kde.org"                   // bug report email address
      );

  aboutData.addAuthor(
      "Tuukka Pasanen",                // developer's name
      I18N_NOOP("Primary Author"),     // task or role
      "illuusio@mailcity.com",         // email address
      0                                // home page or relevant link
      );
  aboutData.addAuthor(
      "Allen Winter",                  // developer's name
      I18N_NOOP("Author"),             // task or role
      "winterz@earthlink.net",         // email address
      0                                // home page or relevant link
      );



  // KCmdLineArgs::init() final 'true' argument indicates no commandline options
  // for QApplication/KApplication (no KDE or Qt options)
  KCmdLineArgs::init( argc, argv, &aboutData, true );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KLocale::setMainCatalogue("kio_help");
  KInstance ins("konsolekalendar");
  KGlobal::locale();

// Replace the KApplication call below with the three lines above
// will make this a pure non-GUI application -- thanks for the info Stephan Kulow.

//  KApplication app(
//      false, // do not allowstyles -- disable the loading on plugin based styles
//      false  // GUI is not enabled -- disable all GUI stuff
//      );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  QString KalendarFile;

  // Default values for start date/time (today at 07:00)
  QDate startdate = QDate::currentDate();
  QTime starttime(7,0);

  // Default values for end date/time (today at 17:00)
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

  KonsoleKalendarVariables variables;
  KonsoleKalendarEpoch epochs;

  variables.setExportType( NONE );

  if ( args->isSet("verbose") ) {
     variables.setVerbose(true);
  }

  if ( args->isSet("dry-run") ) {
     variables.setDryRun(true);
  }

  /*
   *
   *  Switch on export list
   */
  if ( args->isSet("export-list") ) {
     cout << i18n("\nKonsoleKalendar supports these export formats:\n  Text\n  Text-Organizer (not working yet)\n  HTML (not working yet)\n  CSV").local8Bit() << endl;
     return(0);
  }

  /*
   *  Switch on exporting
   *
   */                                                                     
  if ( args->isSet("export-type") ) {
     option = args->getOption("export-type");

     if ( option.upper() == "HTML" ) {
       kdDebug() << "main | exporttype | Export to HTML" << endl;
       variables.setExportType( HTML );
     } else if( option.upper() == "CSV" ) {
       kdDebug() << "main | exporttype | Export to CSV" << endl;
       variables.setExportType( CSV );
     } else if( option.upper() == "TEXT-ORGANIZER" ) {
       kdDebug() << "main | exporttype | Export to Korganizer TXT" << endl;
       variables.setExportType( TEXT_KORGANIZER );
     } else {
       kdDebug() << "main | exporttype | Export to TXT" << endl;
       variables.setExportType( TEXT_KONSOLEKALENDAR );
     }
  }


  /*
   *  Switch on View (Print Entries).  This is the default mode of operation.
   *
   */
  if ( args->isSet("view") ) {
    view=true;

    kdDebug() << "main | parse options | Mode: (Print events)" << endl;
  }

  /*
   *  Switch on Add (Insert Entry)
   *
   */
  if ( args->isSet("add") ) {
    view=false;
    add=true;

    kdDebug() << "main | parse options | Mode: (Add event)" << endl;
  }

  /*
   *  Switch on Change (Modify Entry)
   *
   */
  if ( args->isSet("change") ) {
    view=false;
    change=true;

    kdDebug() << "main | parse options | Mode: (Change event)" << endl;
  }

  /*
   *  Switch on Delete (Remove Entry)
   *
   */
  if ( args->isSet("delete") ) {
    view=false;
    del=true;

    kdDebug() << "main | parse options | Mode: (Delete event)" << endl;
  }

  /*
   *  Switch on Create
   *
   */
  if ( args->isSet("create") ) {
    view=false;
    create=true;

    kdDebug() << "main | parse options | Calendar File: (Create)" << endl;
  }


  /*
   *  If there is summary attached.
   *
   */
  if ( args->isSet("summary") ) {
    option = args->getOption("summary");

    kdDebug() << "main | parse options | Summary: (" << option << ")" << endl;

    variables.setSummary(option);
  }

  /*
   *  If there is description attached.
   *
   */
  if ( args->isSet("description") ) {
    option = args->getOption("description");

    kdDebug() << "main | parse options | Description: (" << option << ")" << endl;

    variables.setDescription(option);
  }

  /*
   *  Show next happening and exit
   *
   */
  if ( args->isSet("next") )
  {
      kdDebug() << "main | parse options | Show next event only" << endl;

    variables.setNext(true);
  }


  /*
   *  Set starting date for calendar
   *
   */
  if ( args->isSet("date") ) {
    option = args->getOption("date");

    kdDebug() << "main | parse options | Start date before conversion: (" << option << ")" << endl;

    startdate = QDate::fromString( option,  Qt::ISODate );
    if( ! startdate.isValid() ) {
      kdError() << i18n("Invalid Start Date Specified ").local8Bit() << option << endl;
      return(1);
    }
    kdDebug() << "main | parse options | Start date after conversion: (" << startdate.toString() << ")" << endl;
  }


  /*
   *  Set starting time
   *
   */
  if ( args->isSet("time") ) {
    option = args->getOption("time");

    kdDebug() << "main | parse options | Start time before conversion : (" << option << ")" << endl;

    starttime = QTime::fromString( option,  Qt::ISODate );
    if( ! starttime.isValid() ) {
      kdError() << i18n("Invalid Start Time Specified ").local8Bit() << option << endl;
      return(1);
    }
    kdDebug() << "main | parse options | Start time after conversion: (" << starttime.toString() << ")" << endl;
  }

  /*
   *  Set end date for calendar
   *
   */

  if ( args->isSet("end-date") ) {
    QString option = args->getOption("end-date");
    
    kdDebug() << "main | parse options | End date before conversion: (" << option << ")" << endl;

    enddate = QDate::fromString( option,  Qt::ISODate );
    if( ! enddate.isValid() ) {
      kdError() << i18n("Invalid End Date Specified ").local8Bit() << option << endl;
      return(1);
    }
    kdDebug() << "main | parse options | End date after converstion: (" << enddate.toString() << ")" << endl;
  }

  /*
   *  Set ending time
   *
   */
  if ( args->isSet("end-time") ) {
    option = args->getOption("end-time");

    kdDebug() << "main | parse options | End time before conversion: (" << option << ")" << endl;

    endtime = QTime::fromString( option,  Qt::ISODate );
    if( ! endtime.isValid() ) {
      kdError() << i18n("Invalid End Time Specified ").local8Bit() << option << endl;
      return(1);
    }

    kdDebug() << "main | parse options | End time after conversion: (" << endtime.toString() << ")" << endl;
  }

  /*
   *  Set start date/time from epoch
   *
   */
  time_t epochstart=0;
  if ( args->isSet("epoch-start") ) {
    option = args->getOption("epoch-start");

    kdDebug() << "main | parse options | Epoch start: (" << option << ")" << endl;

    epochstart = (time_t) option.toULong(0,10);
  }

  /*
   *  Set end date/time from epoch
   *
   */
  time_t epochend=0;
  if ( args->isSet("epoch-end") ) {
    option = args->getOption("epoch-end");

    kdDebug() << "main | parse options | Epoch end: (" << option << ")" << endl;

    epochend = (time_t) option.toULong(0,10);
  }

  if( args->isSet("all") ) {
    variables.setAll( true );
  } else {
    variables.setAll( false );
  }

  if ( args->isSet("import") ) {
    importFile = true;
    option = args->getOption("import");
    variables.setImportFile( option );

    kdDebug() << "main | parse options | importing file from: (" << option << ")" << endl;
  }


  if ( args->isSet("file") ) {
    calendarFile = true;
    option = args->getOption("file");
    variables.setCalendarFile( option );

      kdDebug() << "main | parse options | using calendar at: (" << variables.getCalendarFile() << ")" << endl;

  } else {
    KConfig cfg( locate( "config", "korganizerrc" ) );

    cfg.setGroup("General");
    KURL url( cfg.readPathEntry("Active Calendar") );
    if ( url.isLocalFile() )
    {
      KalendarFile = url.path();

      variables.setCalendarFile(KalendarFile);

      if( variables.isVerbose() ) {
        cout << i18n("Using calendar file ").local8Bit() << variables.getCalendarFile().local8Bit() << endl;
      }
    } else {
      cout << i18n("Remote files are not supported yet.").local8Bit() << endl;
      //TODO: do we exit here (kdError())?
    }
  }

  /***************************************************************************
   * Glorious date/time checking and setting code                            *
   ***************************************************************************/
  QDateTime startdatetime, enddatetime;

  // Handle case with either date or end-date unspecified
  if( !args->isSet("end-date") && args->isSet("date") ) {
    enddate = startdate;
    kdDebug() << "main | datetimestamp | setting enddate to startdate" << endl;
  } else if( args->isSet("end-date") && !args->isSet("date") ) {
    startdate = enddate;
    kdDebug() << "main | datetimestamp | setting startdate to enddate" << endl;
  }
    
  // NOTE: If neither date nor end-date specified, then event will be today.

  // Handle case with end time (or epoch) unspecified, and start time (or epoch) IS specified.
  // In this case, set the ending to 1 hour after starting.
  if( !args->isSet("end-time") && !args->isSet("epoch-end") ) {
    if( args->isSet("time") ) {
      endtime = starttime.addSecs(60*60);  // end is 1 hour after start
      kdDebug() << "main | datetimestamp | setting endtime 1 hour after starttime" << endl;
    } else if( args->isSet("epoch-start") ) {
      startdatetime = epochs.epoch2QDateTime(epochstart);
      enddatetime = startdatetime.addSecs(60*60);
      kdDebug() << "main | datetimestamp | setting endtime 1 hour after epochstart" << endl;
    }
  }

  // Handle case with time (or epoch) unspecified, and end-time (or epoch) IS specified.
  // In this case, set the starting to 1 hour before ending.
  if( !args->isSet("time") && !args->isSet("epoch-start") ) {
    if( args->isSet("end-time") ) {
      starttime = endtime.addSecs(-60*60);  // start is 1 hour before end
      kdDebug() << "main | datetimestamp | setting starttime 1 hour before endtime" << endl;
    } else if( args->isSet("epoch-end") ) {
      enddatetime = epochs.epoch2QDateTime(epochend);
      startdatetime = enddatetime.addSecs(-60*60);
      kdDebug() << "main | datetimestamp | setting starttime 1 before after epochend" << endl;
    }
  }

  // Handle case with time (or epoch) unspecified, and end-time (or epoch) unspecified.
  if( !args->isSet("time") && !args->isSet("epoch-start") &&
      !args->isSet("end-time") && !args->isSet("epoch-end") ) {
    // set default start date/time
    startdatetime = QDateTime::QDateTime(startdate, starttime);
    kdDebug() << "main | datetimestamp | setting startdatetime from default startdate (today) and starttime" << endl;
    // set default end date/time
    enddatetime = QDateTime::QDateTime(enddate, endtime);
    kdDebug() << "main | datetimestamp | setting enddatetime from default enddate (today) and endtime" << endl;
  }
    
  // Set startdatetime, enddatetime if still necessary
  if( startdatetime.isNull() ) {
    startdatetime = QDateTime::QDateTime(startdate, starttime);
    kdDebug() << "main | datetimestamp | setting startdatetime from startdate and starttime" << endl;
  }
  if( enddatetime.isNull() ) {
    enddatetime = QDateTime::QDateTime(enddate, endtime);
    kdDebug() << "main | datetimestamp | setting enddatetime from enddate and endtime" << endl;
  }

  // Finally!
  variables.setStartDateTime( startdatetime );
  variables.setEndDateTime( enddatetime );

  // Some more debug prints
  kdDebug() << "main | datetimestamp | StartDate=" << startdatetime.toString(Qt::TextDate) << endl;
  kdDebug() << "main | datetimestamp | EndDate=" << enddatetime.toString(Qt::TextDate) << endl;

  /***************************************************************************
   * Sanity checks                                                           *
   ***************************************************************************/

  // Cannot combine modes
  if( view + add + change + del > 1 ) {
    kdError() << i18n("Only 1 operation mode (view, add, change, delete) permitted at a time.").local8Bit() << endl;
    return(1);
  }

  // Cannot have a ending before starting
  if( startdatetime > enddatetime ) {
    kdError() << i18n("Ending Date/Time occurs before the Starting Date/Time").local8Bit() << endl;
    return(1);
  }

  /***************************************************************************
   * Mode Dependent Settings                                                 *
   ***************************************************************************/

  // In add mode, make a check for floating events
  if( add ) {

    // If time, end-time, or epoch times are specified, then the event is NOT floating
    if( args->isSet("time")  || args->isSet("end-time") ||
        args->isSet("epoch-start") || args->isSet("epoch-end") ) {
      variables.setFloating(false);
      kdDebug() << "main | floatingcheck | turn-off floating event" << endl;
    }
  }

  args->clear(); // Free up some memory.

  /***************************************************************************
   * And away we go with the real work...                                    *
   ***************************************************************************/

  KonsoleKalendar *konsolekalendar = new KonsoleKalendar(variables);

  if ( calendarFile && create ) {
    if( konsolekalendar->createCalendar() ) {
      kdDebug() << "main | createcalendar | successfully created calendarfile" << endl;
    } else {
      kdDebug() << "main | createcalendar | failed to create calendarfile" << endl;
    }
   }

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
      if( !konsolekalendar->isEvent( startdatetime, enddatetime, variables.getSummary() ) ) {
	kdDebug() << "main | modework | calling addEvent()" << endl;
	konsolekalendar->addEvent();
      } else {
	kdError() << i18n("Attempting to insert an event that already exists.").local8Bit() << endl;
	return(1);
      }
    }

    if( change ) {
      kdDebug() << "main | modework | calling changeEvent()" << endl;
      konsolekalendar->changeEvent();
    }

    if( del ) {
      kdDebug() << "main | modework | calling deleteEvent()" << endl;
      konsolekalendar->deleteEvent();
    }

    if( view ) {
      kdDebug() << "main | modework | calling showInstance() to view events" << endl;
      konsolekalendar->showInstance();
    }

    konsolekalendar->closeCalendar();

  } else {

    kdError() << i18n("Cannot open specified calendar file ").local8Bit() << variables.getCalendarFile() << endl;

  }

  delete konsolekalendar;

  kdDebug() << "main | exiting" << endl;

  return 0;
}
