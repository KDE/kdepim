#ifndef _KONSOLEKALENDARVARIABLES_H_
#define _KONSOLEKALENDARVARIABLES_H_

/***************************************************************************
                       konsolekalendarvariables.h
            Konsolekalendar variables contains global variables that are
            used with this marvelous app;)
                           -------------------
    begin                : Sun Jan 6 2002
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

#include <qdatetime.h>
#include <qstring.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/calendarresources.h>
#include <libkcal/resourcelocal.h>
#include <libkcal/resourcecalendar.h>
#include <libkcal/event.h>


   /*
    * Our export types
    */
  #define   NONE                     0
  #define   TEXT_KONSOLEKALENDAR     1
  #define   HTML                     2
  #define   XHTML                    3
  #define   XML                      4
  #define   CSV                      5
  #define   VCARD                    6


namespace KCal {

class KonsoleKalendarVariables
{
  public:
    KonsoleKalendarVariables();
    ~KonsoleKalendarVariables();

    void setStartDateTime( QDateTime start );
    QDateTime getStartDateTime();
    bool isStartDateTime();

    void setEndDateTime( QDateTime end );
    QDateTime getEndDateTime();
    bool isEndDateTime();

    void setNext( bool next );
    bool isNext();

    void setVerbose( bool verbose );
    bool isVerbose();

    void setDryRun( bool dryrun );
    bool isDryRun();

    void setCalendarFile( QString calendar );
    QString getCalendarFile();

    void setImportFile( QString calendar );
    QString getImportFile();

    void setDescription( QString description );
    QString getDescription();
    bool isDescription();

    void setSummary( QString description );
    QString getSummary();
    bool isSummary();

    void setAll( bool all );
    bool getAll();
    bool isAll();

    void setFloating( bool floating );
    bool getFloating();

    void printSpecs( QString mode );

    QDate parseDate( QString string );
    QTime parseTime( QString str );

    /**
     * Set calendar file for global use
     */

   void setCalendar( Calendar *calendar );

   /**
    * Get global calendar
    */

   Calendar *getCalendar();

   /**
    * Set output file
    */
   
   void setExportFile( QString export_file );
   
   /**
    *  To what file we'll output
    */
   
   QString getExportFile();
   
   /*
    * Has an Export File been set?
    */
    
   bool isExportFile();

   /**
     * Set export type that'll we use
     */
   
   void setExportType( int export_type );

   /**
    * what export type konsolekalendar will use
    */

   int getExportType();

   /**
    * Add to Calendar Resources
    */
   CalendarResourceManager *getCalendarResourceManager();

   /**
    * Add to Calendar Resources 
    */
   bool addCalendarResources( ResourceCalendar *cal );
   
  /**
   * Calendar resource is the new way 
   */
   void setCalendarResources( CalendarResources *resource );
   
   /**
   * Calendar resource is the new way 
   */  
  CalendarResources *getCalendarResources();

   
  /*
   * Loads calendar resources 
   */
  bool loadCalendarResources( KConfig *config ); 
   
   
  private:
   int findNumber( const QString &str, int &pos, int &startpos );
   char findSeparator( const QString &str, int &pos, int &seppos );
   void skipWhiteSpace( const QString &str, int &pos );

   
   QDateTime m_startDateTime;
   QDateTime m_endDateTime;
   bool m_bIsStartDateTime;
   bool m_bIsEndDateTime;
   QString m_calendar;
   QString m_import;
   QString m_description;
   QString m_summary;
   QString m_export_file;
   bool m_bSummary;
   bool m_bNext;
   bool m_bVerbose;
   bool m_bDryRun;
   bool m_bAll;
   bool m_bDescription;
   bool m_bFloating;
   int str_length;
   int m_export_type;
   QString m_exportFile;
   bool m_bIsExportFile;
   // New resource stuff will over-ride old pne
   CalendarResources *m_resource;
   // We can use this from everywhere
   //CalendarLocal *m_calendarLocal;   
   Calendar *m_resourceCalendar;
   
 

};

}

#endif
