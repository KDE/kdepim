/*******************************************************************************
 * konsolekalendarvariables.h                                                  *
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. *
 *                                                                             *
 * As a special exception, permission is given to link this program            *
 * with any edition of Qt, and distribute the resulting executable,            *
 * without including the source code for Qt in the source distribution.        *
 *                                                                             *
 ******************************************************************************/

#ifndef _KONSOLEKALENDARVARIABLES_H_
#define _KONSOLEKALENDARVARIABLES_H_

#include <libkcal/calendarlocal.h>
#include <libkcal/calendarresources.h>
#include <libkcal/resourcelocal.h>
#include <libkcal/resourcecalendar.h>
#include <libkcal/event.h>

#include <qstring.h>
#include <qdatetime.h>

/**
 * @file konsolekalendarvariables.h
 * Provides the KonsoleKalendarVariables class definition.
 */

namespace KCal
{
  /**
   * ExportType is the type of Export output
   */
  enum ExportType
  {
    /** Export none */
    ExportTypeNone,
    /** Export as text (default) */
    ExportTypeText,
    /** Export as compact text */
    ExportTypeTextShort,
    /** Export HTML for the specified time span */
    ExportTypeHTML,
    /** Export HTML for the time span on month boundaries */
    ExportTypeMonthHTML,
    /** Export XHTML (NOT AVAILABLE YET) */
    ExportTypeXHTML,
    /** Export XML (NOT AVAILABLE YET) */
    ExportTypeXML,
    /** Export Comma-Separated Values */
    ExportTypeCSV,
    /** Export VCard (NOT AVAILABLE YET) */
    ExportTypeVCard
  };

  /**
   * This class provides all the variables for the program.
   * @author Tuukka Pasanen
   * @author Allen Winter
   */
  class KonsoleKalendarVariables
  {
  public:
    /**
     * Construct an empty KonsoleKalendarVariables object.
     */
    KonsoleKalendarVariables();
    /**
     * Destructor
     */
    ~KonsoleKalendarVariables();

    /**
     * Set switch to use Events.
     * @param useEvents if true, operation uses Events.
     */
    void setUseEvents( bool useEvents );
    /**
     * Get use Events switch.
     * @return true if operation using Events is specified.
     */
    bool getUseEvents();

    /**
     * Set switch to use Todos.
     * @param useTodos if true, operation uses Todos.
     */
    void setUseTodos( bool useTodos );
    /**
     * Get use Todos switch.
     * @return true if operation using Todos is specified.
     */
    bool getUseTodos();

    /**
     * Set switch to use Journals.
     * @param useJournals if true, operation uses Journals.
     */
    void setUseJournals( bool useJournals );
    /**
     * Get use Journals switch.
     * @return true if operation using Journals is specified.
     */
    bool getUseJournals();

    /**
     * Sets start date.
     * @param start is the start date.
     */
    void setStartDateTime( QDateTime start );

    /**
     * Get start date.
     * @return start date.
     */
    QDateTime getStartDateTime();

    /**
     * Is there start date?
     * @return true if there is false is there isn't.
     */
    bool isStartDateTime();

    /**
     * Sets end date.
     * @param end is the enddate.
     */
    void setEndDateTime( QDateTime end );

    /**
     * Get end date.
     * @return end date.
     */
    QDateTime getEndDateTime();

    /**
     * Is there end date?
     * @return true if there is false is there isn't.
     */
    bool isEndDateTime();

    /**
     * Sets the timezone from the user or system environment.
     */
    void setTimeZoneId();

    /**
     * Get timezone id string.
     * @return timezone id string.
     */
    QString getTimeZoneId();

    /**
     * Is there a timezone set?
     * @return true if there is false if there isn't.
     */
    bool isTimeZoneId();

    /**
     * Sets the UID, the unique tag for VCard entry.
     * @param uid unique tag for VCard entry.
     */
    void setUID( QString uid );

    /**
     * Get UID, the unique tag for VCard entry.
     * @return UID number.
     */
    QString getUID();

    /**
     * Is there UID set?
     * @return true there is UID false there isn't.
     */
    bool isUID();

    /**
     * Show only next activity and exit.
     * @param next true or false.
     */
    void setNext( bool next );

    /**
     * Should we show only next activity and exit?
     */
    bool isNext();

    /**
     * Should program be more verbose?
     * @param verbose a flag to set verbosity.
     */
    void setVerbose( bool verbose );

    /**
     * Should program be more verbose?
     */
    bool isVerbose();

    /**
     * Should we only try to run it and do nothing?
     * @param dryrun false no and true just test it.
     */
    void setDryRun( bool dryrun );

    /**
     * Is this program only in testing mode?
     * @return true yes false no.
     */
    bool isDryRun();

    /**
     * Set calendar file (Kinda obsolete!)
     * @param calendar Calendar files full path.
     */
    void setCalendarFile( QString calendar );

    /**
     * Returns fullpath to calendar file.
     * @return calendar file.
     */
    QString getCalendarFile();

    /**
     * Set file to import active calendar.
     * @param calendar Calendar file to import.
     */
    void setImportFile( QString calendar );

    /**
     * Return import filename.
     * @return File that should be imported.
     */
    QString getImportFile();

    /**
     * Add description.
     * @param description to event.
     */
    void setDescription( QString description );

    /**
     * Return description.
     * @return description of event.
     */
    QString getDescription();

    /**
     * Is there an event description?
     * @return true is there is description false there isn't.
     */
    bool isDescription();

    /**
     * Add location information.
     * @param location location where the event occurs.
     */
    void setLocation( QString location );

    /**
     * Return location information..
     * @return location where event is occurring.
     */
    QString getLocation();

    /**
     * Is there event location information available?
     * @return true is there is description false there isn't.
     */
    bool isLocation();

    /**
     * Add summary.
     * @param summary event summary.
     */
    void setSummary( QString summary );

    /**
     * Get summary.
     * @return summary.
     */
    QString getSummary();

    /**
     * Is there an event summary?
     * @return true there is false there isn't.
     */
    bool isSummary();

    /**
     * View all option.
     * @param all flag to view all Events.
     */
    void setAll( bool all );
    /**
     * Return all option.
     */
    bool getAll();
    /**
     * Is the all option set?
     */
    bool isAll();

    /**
     * Set if Event is floating.
     * @param floating if true then the Event is floating.
     */
    void setFloating( bool floating );
    /**
     * Return if Event is floating.
     */
    bool getFloating();

    /**
     * Set is calendar default resource.
     */
    void setDefault( bool def );

    /**
     * Return if calendar is default resource.
     */
    bool isDefault();

    /**
     * Set calendar file for global use.
     */

    void setCalendar( CalendarLocal *calendar );

    /**
     * Get global calendar.
     */

    CalendarLocal *getCalendar();

    /**
     * Set output file.
     */
    void setExportFile( QString export_file );

    /**
     * To what file we'll output.
     */
    QString getExportFile();

    /**
     * Has an Export File been set?
     */
    bool isExportFile();

    /**
     * Set export type that'll we use.
     */
    void setExportType( ExportType exportType );

    /**
     * What export type to use.
     */
    ExportType getExportType();

    /**
     * Do we use CalendarResources or LocalCalendar?
     */
    bool isCalendarResources();

    /**
     * Add to Calendar Resources.
     */
    CalendarResourceManager *getCalendarResourceManager();

    /**
     * Add to Calendar Resources.
     */
    bool addCalendarResources( ResourceCalendar *cal );

    /**
     * Calendar resource is the new way.
     */
    void setCalendarResources( CalendarResources *resource );

    /**
     * Calendar resource is the new way.
     */
    CalendarResources *getCalendarResources();

    /**
     * Loads calendar resources.
     */
    bool loadCalendarResources( KConfig *config );

    /**
     * Set how many day should be seen.
     */

    void setDaysCount( int count );

    /**
     * Is there some cound of days should be seen.
     */
    bool isDaysCount();

    /**
     * Get how many day should be seen.
     */

    int getDaysCount();

  private:
    bool m_bIsTimeZoneId;
    QString m_TimeZoneId;
    QDateTime m_startDateTime;
    QDateTime m_endDateTime;
    bool m_bIsStartDateTime;
    bool m_bIsEndDateTime;
    QString m_calendar;
    QString m_import;
    QString m_description;
    QString m_location;
    QString m_summary;
    QString m_export_file;
    QString m_UID;
    bool m_bUseEvents;
    bool m_bUseTodos;
    bool m_bUseJournals;
    bool m_bSummary;
    bool m_bNext;
    bool m_bVerbose;
    bool m_bDryRun;
    bool m_bAll;
    bool m_bDescription;
    bool m_bLocation;
    bool m_bFloating;
    bool m_bDaysCount;
    bool m_bIsUID;
    int str_length;
    ExportType m_exportType;
    int m_daysCount;
    QString m_exportFile;
    bool m_bIsExportFile;
    bool m_bIsDefault;
    bool m_bIsCalendarResources;
    // New resource stuff will over-ride old pne
    CalendarResources *m_resource;
    // We can use this from everywhere
    CalendarLocal *m_calendarLocal;
    Event::List m_eventList;
    Todo::List m_todoList;
  };

}

#endif
