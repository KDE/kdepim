/******************************************************************************
 * konsolekalendarvariables.h                                                 *
 *                                                                            *
 * KonsoleKalendar is a command line interface to KDE calendars               *
 * Copyright (C) 2002-2004  Tuukka Pasanen <illuusio@mailcity.com>            *
 * Copyright (C) 2003-2005  Allen Winter <winter@kde.org>                     *
 *                                                                            *
 * This program is free software; you can redistribute it and/or modify       *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 2 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License along    *
 * with this program; if not, write to the Free Software  Foundation, Inc.,   *
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.               *
 *                                                                            *
 * As a special exception, permission is given to link this program           *
 * with any edition of Qt, and distribute the resulting executable,           *
 * without including the source code for Qt in the source distribution.       *
 *                                                                            *
 *****************************************************************************/

#ifndef KONSOLEKALENDARVARIABLES_H
#define KONSOLEKALENDARVARIABLES_H

#include <KCalCore/Event>
#include <Akonadi/Calendar/FetchJobCalendar>
#include <AkonadiCore/Collection>

#include <QtCore/QString>
#include <QtCore/QDateTime>

/**
 * @file konsolekalendarvariables.h
 * Provides the KonsoleKalendarVariables class definition.
 */

/**
 * ExportType is the type of Export output
 */
enum ExportType {
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
 * @brief
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
    bool getUseEvents() const;

    /**
     * Set switch to use Todos.
     * @param useTodos if true, operation uses Todos.
     */
    void setUseTodos( bool useTodos );
    /**
     * Get use Todos switch.
     * @return true if operation using Todos is specified.
     */
    bool getUseTodos() const;

    /**
     * Set switch to use Journals.
     * @param useJournals if true, operation uses Journals.
     */
    void setUseJournals( bool useJournals );
    /**
     * Get use Journals switch.
     * @return true if operation using Journals is specified.
     */
    bool getUseJournals() const;

    /**
     * Sets start date.
     * @param start is the start date.
     */
    void setStartDateTime( const QDateTime &start );

    /**
     * Get start date.
     * @return start date.
     */
    QDateTime getStartDateTime() const;

    /**
     * Is there start date?
     * @return true if there is false is there isn't.
     */
    bool isStartDateTime() const;

    /**
     * Sets end date.
     * @param end is the enddate.
     */
    void setEndDateTime( const QDateTime &end );

    /**
     * Get end date.
     * @return end date.
     */
    QDateTime getEndDateTime() const;

    /**
     * Is there end date?
     * @return true if there is false is there isn't.
     */
    bool isEndDateTime() const;

    /**
     * Sets the UID, the unique tag for VCard entry.
     * @param uid unique tag for VCard entry.
     */
    void setUID( const QString &uid );

    /**
     * Get UID, the unique tag for VCard entry.
     * @return UID number.
     */
    QString getUID() const;

    /**
     * Is there UID set?
     * @return true there is UID false there isn't.
     */
    bool isUID() const;

    /**
     * Show only next activity and exit.
     * @param next true or false.
     */
    void setNext( bool next );

    /**
     * Should we show only next activity and exit?
     */
    bool isNext() const;

    /**
     * Should program be more verbose?
     * @param verbose a flag to set verbosity.
     */
    void setVerbose( bool verbose );

    /**
     * Should program be more verbose?
     */
    bool isVerbose() const;

    /**
     * Should we only try to run it and do nothing?
     * @param dryrun false no and true just test it.
     */
    void setDryRun( bool dryrun );

    /**
     * Is this program only in testing mode?
     * @return true yes false no.
     */
    bool isDryRun() const;

    /**
     * Set calendar file
     * @param calendar Calendar files full path.
     */
    void setCalendarFile( const QString &calendar );

    /**
     * Returns fullpath to calendar file.
     * @return calendar file.
     */
    QString getCalendarFile() const;

    /**
     * Set file to import active calendar.
     * @param calendar Calendar file to import.
     */
    void setImportFile( const QString &calendar );

    /**
     * Return import filename.
     * @return File that should be imported.
     */
    QString getImportFile() const;

    /**
     * Add description.
     * @param description to event.
     */
    void setDescription( const QString &description );

    /**
     * Return description.
     * @return description of event.
     */
    QString getDescription() const;

    /**
     * Is there an event description?
     * @return true is there is description false there isn't.
     */
    bool isDescription() const;

    /**
     * Add location information.
     * @param location location where the event occurs.
     */
    void setLocation( const QString &location );

    /**
     * Return location information.
     * @return location where event is occurring.
     */
    QString getLocation() const;

    /**
     * Is there event location information available?
     * @return true is there is description false there isn't.
     */
    bool isLocation() const;

    /**
     * Add summary.
     * @param summary event summary.
     */
    void setSummary( const QString &summary );

    /**
     * Get summary.
     * @return summary.
     */
    QString getSummary() const;

    /**
     * Is there an event summary?
     * @return true there is false there isn't.
     */
    bool isSummary() const;

    /**
     * View all option.
     * @param all flag to view all Events.
     */
    void setAll( bool all );
    /**
     * Return all option.
     */
    bool getAll() const;
    /**
     * Is the all option set?
     */
    bool isAll() const;

    /**
     * Set if Event is floating.
     * @param floating if true then the Event is floating.
     */
    void setFloating( bool floating );

    /**
     * Returns if Event is floating.
     */
    bool getFloating() const;

    /**
     * Sets the calendar resources for global use.
     *
     * @param resources is a pointer to the calendar to use.
     */
    void setCalendar( const Akonadi::FetchJobCalendar::Ptr & );

    /**
     * Get global calendar resources.
     */
    Akonadi::FetchJobCalendar::Ptr getCalendar() const;

    /**
     * Sets the output file name to @p export_file.
     *
     * @param export_file is the name of the export file.
     */
    void setExportFile( const QString &export_file );

    /**
     * To what file we'll output.
     */
    QString getExportFile() const;

    /**
     * Has an Export File been set?
     */
    bool isExportFile() const;

    /**
     * Sets the #ExportType to use.
     *
     * @param exportType is the #ExportType to use.
     */
    void setExportType( ExportType exportType );

    /**
     * What export type to use.
     */
    ExportType getExportType() const;

    /**
     * Sets how many day should be seen.
     *
     * @param count is the number of days to be shown.
     */
    void setDaysCount( int count );

    /**
     * Is there some cound of days should be seen.
     */
    bool isDaysCount() const;

    /**
     * Get how many day should be seen.
     */
    int getDaysCount() const;

    /**
     * Sets whether to allow using resources with potential GUI dependencies.
     */
    void setAllowGui( bool allow );

    void setCollectionId(Akonadi::Collection::Id);
    Akonadi::Collection::Id collectionId() const;

    /**
     * Returns whether to allow using resources with potential GUI dependencies.
     */
    bool allowGui() const;

  private:
    //@cond PRIVATE
    bool m_bIsUID;
    QString m_UID;
    bool m_bIsStartDateTime;
    QDateTime m_startDateTime;
    bool m_bIsEndDateTime;
    QDateTime m_endDateTime;
    bool m_bNext;
    bool m_bVerbose;
    bool m_bDryRun;
    bool m_bUseEvents;
    bool m_bUseTodos;
    bool m_bUseJournals;
    QString m_calendarFile;
    QString m_import;
    ExportType m_exportType;
    bool m_bIsExportFile;
    QString m_exportFile;
    bool m_bAll;
    bool m_bDescription;
    QString m_description;
    bool m_bLocation;
    QString m_location;
    bool m_bSummary;
    QString m_summary;
    bool m_bFloating;
    bool m_bDaysCount;
    int m_daysCount;
    bool m_bAllowGui;
    Akonadi::Collection::Id m_collectionId;
    Akonadi::FetchJobCalendar::Ptr m_calendar;
    //@endcond
};

#endif
